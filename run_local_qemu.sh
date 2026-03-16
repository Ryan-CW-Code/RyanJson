#!/usr/bin/env bash
set -euo pipefail

# 本地一键跑 QEMU 单测链路。
# 目标：
# - 用交叉编译产物在 QEMU 上执行完整 unitMain
# - 校验对齐访问与预期 fault 语义
# - 保持与 local base 相同的三组语义宏矩阵

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib/common.sh
source "${scriptDir}/scripts/lib/common.sh"
repoRoot="$(ryanjson_repo_root_from_source "${BASH_SOURCE[0]}" 0)"
cd "${repoRoot}"

userMachineSet=0
userCpuSet=0
userTargetSet=0
if [[ -n "${QEMU_MACHINE+x}" ]]; then userMachineSet=1; fi
if [[ -n "${QEMU_CPU+x}" ]]; then userCpuSet=1; fi
if [[ -n "${QEMU_TARGET+x}" ]]; then userTargetSet=1; fi

qemuMode="${QEMU_MODE:-full}"
qemuMachine="${QEMU_MACHINE:-mps2-an386}"
qemuCpu="${QEMU_CPU:-cortex-m4}"
qemuTimeoutSec="${QEMU_TIMEOUT_SEC:-120}"
qemuTarget="${QEMU_TARGET:-RyanJsonQemu}"
qemuForceClean="${QEMU_FORCE_CONFIG_CLEAN:-0}"
qemuStopOnFail="${QEMU_STOP_ON_FAIL:-1}"
qemuLogRoot="${QEMU_LOG_ROOT:-coverage/qemu}"
qemuMemory="${QEMU_MEMORY:-64M}"
qemuConsoleLog="${QEMU_CONSOLE_LOG:-1}"
qemuSaveLog="${QEMU_SAVE_LOG:-0}"
qemuMaxCases="${QEMU_MAX_CASES:-0}"
qemuSemihostingMode="legacy"

# 关键日志标记：既要有 unit 全量通过，也要能看到对齐异常的预期现场。
readonly qemuPassPattern='^\[QEMU\]\[RESULT\] UNIT_PASS code=0 tick=[0-9]+\r?$'
readonly qemuAlignedPattern='^\[QEMU\]\[ALIGN\] aligned_access PASS read=0x[0-9A-Fa-f]+\r?$'
readonly qemuUnalignedPattern='^\[QEMU\]\[ALIGN\] unaligned_access TRIGGER addr=0x[0-9A-Fa-f]+\r?$'
readonly qemuExpectedFaultPattern='^\[QEMU\]\[RESULT\] EXPECTED_UNALIGNED_FAULT (cfsr|fallbackAddr)=0x[0-9A-Fa-f]+\r?$'
readonly qemuHardFaultPattern='^\[QEMU\]\[HARDFAULT\] CFSR=0x[0-9A-Fa-f]+ HFSR=0x[0-9A-Fa-f]+ BFAR=0x[0-9A-Fa-f]+ MMFAR=0x[0-9A-Fa-f]+\r?$'
readonly qemuFallbackFaultPattern='^\[QEMU\]\[HARDFAULT\] fallback_soft_trap_no_hw_fault addr=0x[0-9A-Fa-f]+\r?$'
readonly qemuFailurePattern='^\[QEMU\]\[RESULT\] UNIT_FAIL code=-?[0-9]+\r?$'

declare -a caseList=()

# 环境依赖检查
requireQemuToolchain() {
	ryanjson_require_cmds "可执行: bash ./scripts/setup/install_qemu_deps.sh" \
		xmake arm-none-eabi-gcc arm-none-eabi-objcopy qemu-system-arm
}

detectQemuSemihostingMode() {
	if qemu-system-arm -help 2>&1 | grep -q -- "-semihosting-config"; then
		qemuSemihostingMode="config"
	else
		qemuSemihostingMode="legacy"
	fi
}

machine_supported() {
	local machineName="$1"
	qemu-system-arm -machine help | awk '{print $1}' | grep -Fxq "${machineName}"
}

# 根据 QEMU 版本能力与用户输入修正 machine/cpu/target
resolveQemuMachineDefaults() {
	# 优先 mps2-an386（CM4F），若当前 QEMU 不支持则自动回退到 mps2-an385（CM3）。
	if ! machine_supported "${qemuMachine}"; then
		if [[ "${qemuMachine}" == "mps2-an386" ]] && machine_supported "mps2-an385"; then
			ryanjson_log_info "当前 QEMU 不支持 mps2-an386，自动回退到 mps2-an385。"
			qemuMachine="mps2-an385"
			if [[ "${userCpuSet}" -eq 0 ]]; then
				qemuCpu="cortex-m3"
			fi
			if [[ "${userTargetSet}" -eq 0 ]]; then
				qemuTarget="RyanJsonQemuCm3"
			fi
		else
			ryanjson_log_error "QEMU 不支持 machine=${qemuMachine}"
			ryanjson_log_info "可用 machine 列表："
			qemu-system-arm -machine help | sed -n '1,120p'
			return 1
		fi
	fi

	# 用户直接选择 an385 且未显式指定 target/cpu 时，自动切到 CM3 构建目标。
	if [[ "${qemuMachine}" == "mps2-an385" ]]; then
		if [[ "${userCpuSet}" -eq 0 ]]; then
			qemuCpu="cortex-m3"
		fi
		if [[ "${userTargetSet}" -eq 0 ]]; then
			qemuTarget="RyanJsonQemuCm3"
		fi
	fi
}

qemu_run_xmake_config() {
	# QEMU 构建专用 xmake 配置（cross/arm）
	local caseName="$1"

	if [[ "${qemuForceClean}" == "1" ]]; then
		ryanjson_log_phase "正在执行 xmake 配置（clean 模式）..."
		if ! xmake f -c -p cross -a arm; then
			ryanjson_log_error "xmake 配置失败：${caseName}"
			return 1
		fi
	else
		ryanjson_log_phase "正在执行 xmake 配置（增量模式）..."
		if ! xmake f -p cross -a arm; then
			ryanjson_log_error "xmake 配置失败：${caseName}"
			return 1
		fi
	fi
}

qemu_build_target() {
	# QEMU 目标构建（带编译错误兜底检查）
	local caseName="$1"
	local buildLogPath=""

	buildLogPath="$(mktemp "/tmp/${caseName}.build.XXXX.log")"
	if ! xmake -b "${qemuTarget}" 2>&1 | tee "${buildLogPath}"; then
		ryanjson_log_error "xmake 构建失败，已终止本用例并跳过 QEMU 运行。"
		tail -n 120 "${buildLogPath}" || true
		rm -f "${buildLogPath}" >/dev/null 2>&1 || true
		return 1
	fi
	if grep -Eq '(^|[^[:alpha:]])error:' "${buildLogPath}"; then
		ryanjson_log_error "构建日志检测到编译错误，已终止本用例并跳过 QEMU 运行。"
		tail -n 120 "${buildLogPath}" || true
		rm -f "${buildLogPath}" >/dev/null 2>&1 || true
		return 1
	fi
	rm -f "${buildLogPath}" >/dev/null 2>&1 || true
}

qemu_find_elf() {
	# 查找 QEMU 目标输出的 ELF
	local targetName="$1"
	find ./build -type f -name "${targetName}.elf" | head -n 1 || true
}

# 生成语义矩阵并校验 QEMU 参数
prepareQemuCaseList() {
	local caseText=""

	if ! caseText="$(ryanjson_emit_semantic_cases "${qemuMode}" "QEMU_MODE")"; then
		return 1
	fi
	mapfile -t caseList <<< "${caseText}"

	if ! ryanjson_require_nonneg_int "QEMU_MAX_CASES" "${qemuMaxCases}"; then
		return 1
	fi

	if ! ryanjson_require_01 "QEMU_SAVE_LOG" "${qemuSaveLog}"; then
		return 1
	fi

	if ! ryanjson_require_01 "QEMU_CONSOLE_LOG" "${qemuConsoleLog}"; then
		return 1
	fi

	if [[ "${qemuConsoleLog}" == "0" && "${qemuSaveLog}" == "0" ]]; then
		ryanjson_log_info "QEMU_CONSOLE_LOG=0 且 QEMU_SAVE_LOG=0 无可见输出，自动切换 QEMU_CONSOLE_LOG=1。"
		qemuConsoleLog="1"
	fi

	if [[ "${qemuSaveLog}" == "1" ]]; then
		mkdir -p "${qemuLogRoot}"
	fi

	if ((qemuMaxCases > 0)) && ((qemuMaxCases < ${#caseList[@]})); then
		caseList=("${caseList[@]:0:qemuMaxCases}")
	fi
}

# 打印统一 banner
printQemuBanner() {
	ryanjson_print_banner_begin "QEMU 本地链路启动（完整 localbase 单测 + 对齐语义）"
	ryanjson_print_banner_kv "MODE" "${qemuMode}"
	ryanjson_print_banner_kv "TARGET" "${qemuTarget}"
	ryanjson_print_banner_kv "MACHINE" "${qemuMachine}"
	ryanjson_print_banner_kv "CPU" "${qemuCpu}"
	ryanjson_print_banner_kv "TIMEOUT" "${qemuTimeoutSec}s"
	ryanjson_print_banner_kv "LOG_ROOT" "${qemuLogRoot}"
	ryanjson_print_banner_kv "MEMORY" "${qemuMemory}"
	ryanjson_print_banner_kv "CONSOLE_LOG" "${qemuConsoleLog}"
	ryanjson_print_banner_kv "SAVE_LOG" "${qemuSaveLog}"
	ryanjson_print_banner_kv "SEMIHOSTING" "${qemuSemihostingMode}"
	ryanjson_print_banner_kv "MAX_CASES" "${qemuMaxCases}"
	ryanjson_print_banner_end
}

# 清洗串口输出，保证 marker 匹配稳定
cleanQemuStream() {
	if command -v perl >/dev/null 2>&1; then
		# 规范化 UART 输出：保留 ANSI 颜色，去掉 CR 和无意义控制字符，
		# 这样后面的 marker 检查可以稳定使用 ^...$。
		perl -ne 's/\r//g; s/[\x00-\x08\x0B\x0C\x0E-\x1A\x1C-\x1F\x7F]//g; next if length($_) > 4096; print;'
	else
		tr -d '\000\r'
	fi
}

# 子进程定位与强制结束
getQemuChildPid() {
	local parentPid="$1"
	pgrep -P "${parentPid}" qemu-system-arm | head -n 1 || true
}

stopQemuRun() {
	local parentPid="$1"
	local childPid=""
	local waitDeadline=0

	childPid="$(getQemuChildPid "${parentPid}")"
	if [[ -n "${childPid}" ]]; then
		kill -TERM "${childPid}" >/dev/null 2>&1 || true
	fi

	waitDeadline=$((SECONDS + 3))
	while kill -0 "${parentPid}" >/dev/null 2>&1; do
		if ((SECONDS >= waitDeadline)); then
			break
		fi
		sleep 0.2
	done

	if kill -0 "${parentPid}" >/dev/null 2>&1; then
		kill -TERM "${parentPid}" >/dev/null 2>&1 || true
		sleep 1
		kill -KILL "${parentPid}" >/dev/null 2>&1 || true
	fi
}

# 日志 marker 校验
qemuLogHasRequiredMarkers() {
	local logPath="$1"

	if ! grep -Eq "${qemuPassPattern}" "${logPath}"; then
		return 1
	fi
	if ! grep -Eq "${qemuAlignedPattern}" "${logPath}"; then
		return 1
	fi
	if ! grep -Eq "${qemuUnalignedPattern}" "${logPath}"; then
		return 1
	fi
	if ! grep -Eq "${qemuExpectedFaultPattern}" "${logPath}"; then
		return 1
	fi
	if ! grep -Eq "${qemuHardFaultPattern}" "${logPath}" \
		&& ! grep -Eq "${qemuFallbackFaultPattern}" "${logPath}"; then
		return 1
	fi

	return 0
}

qemuLogHasFailureMarkers() {
	local logPath="$1"
	if grep -Eq "${qemuFailurePattern}" "${logPath}"; then
		return 0
	fi
	return 1
}

reportMissingLogMarkers() {
	local logPath="$1"
	local missing=0

	if ! grep -Eq "${qemuPassPattern}" "${logPath}"; then
		ryanjson_log_error "日志缺少关键标记: [QEMU][RESULT] UNIT_PASS code=0 tick=<num>"
		missing=1
	fi
	if ! grep -Eq "${qemuAlignedPattern}" "${logPath}"; then
		ryanjson_log_error "日志缺少关键标记: [QEMU][ALIGN] aligned_access PASS read=0x<hex>"
		missing=1
	fi
	if ! grep -Eq "${qemuUnalignedPattern}" "${logPath}"; then
		ryanjson_log_error "日志缺少关键标记: [QEMU][ALIGN] unaligned_access TRIGGER addr=0x<hex>"
		missing=1
	fi
	if ! grep -Eq "${qemuExpectedFaultPattern}" "${logPath}"; then
		ryanjson_log_error "日志缺少关键标记: [QEMU][RESULT] EXPECTED_UNALIGNED_FAULT <dynamic>"
		missing=1
	fi
	if ! grep -Eq "${qemuHardFaultPattern}" "${logPath}" \
		&& ! grep -Eq "${qemuFallbackFaultPattern}" "${logPath}"; then
		ryanjson_log_error "日志缺少 fault 现场标记（HARDFAULT cfsr/hfsr 或 fallback addr）"
		missing=1
	fi

	return "${missing}"
}

# 处理临时日志
cleanupCaseLogIfNeeded() {
	local keepCaseLog="$1"
	local caseLogPath="$2"
	if [[ "${keepCaseLog}" == "0" && -n "${caseLogPath}" ]]; then
		rm -f "${caseLogPath}" >/dev/null 2>&1 || true
	fi
}

# 执行单个语义用例（构建 + 运行 + 日志校验）
run_case() {
	local strictKey="$1"
	local addAtHead="$2"
	local scientific="$3"
	local caseName="$(ryanjson_semantic_case_name "${strictKey}" "${addAtHead}" "${scientific}")"
	local caseLogPath=""
	local keepCaseLog="0"
	local deadlineSec=0
	local qemuPid=0
	local qemuRc=0
	local elfPath=""
	local -a qemuArgs=()

	ryanjson_print_banner_begin "【用例】${caseName}"
	ryanjson_print_semantic_kv "${strictKey}" "${addAtHead}" "${scientific}"
	ryanjson_print_banner_end

	ryanjson_export_semantic_macros "${strictKey}" "${addAtHead}" "${scientific}"

	if ! qemu_run_xmake_config "${caseName}"; then
		return 1
	fi

	# 清理旧产物，避免构建失败时误用历史 ELF。
	find ./build -type f -name "${qemuTarget}.elf" -delete >/dev/null 2>&1 || true

	if ! qemu_build_target "${caseName}"; then
		return 1
	fi

	elfPath="$(qemu_find_elf "${qemuTarget}")"
	if [[ -z "${elfPath}" ]]; then
		ryanjson_log_error "未找到 ELF 输出（${qemuTarget}.elf）"
		return 1
	fi

	if [[ "${qemuSaveLog}" == "1" ]]; then
		caseLogPath="${qemuLogRoot}/${caseName}.log"
		keepCaseLog="1"
	else
		caseLogPath="$(mktemp "/tmp/${caseName}.XXXX.log")"
		keepCaseLog="0"
	fi

	ryanjson_log_info "ELF: ${elfPath}"
	if [[ "${qemuSaveLog}" == "1" ]]; then
		ryanjson_log_phase "启动 QEMU 并抓取日志 -> ${caseLogPath}"
	else
		ryanjson_log_phase "启动 QEMU（终端实时输出，日志不落盘）"
	fi

	qemuArgs=(
		-M "${qemuMachine}"
		-cpu "${qemuCpu}"
		-nographic
		-kernel "${elfPath}"
	)
	if [[ "${qemuSemihostingMode}" == "config" ]]; then
		qemuArgs+=(-semihosting-config enable=on,target=native)
	else
		qemuArgs+=(-semihosting)
	fi
	if [[ -n "${qemuMemory}" ]]; then
		qemuArgs+=(-m "${qemuMemory}")
	fi

	: > "${caseLogPath}"
	deadlineSec=$((SECONDS + qemuTimeoutSec))

	set +e
	if [[ "${qemuConsoleLog}" == "1" ]]; then
		(
			qemu-system-arm "${qemuArgs[@]}" 2>&1 | cleanQemuStream | tee -a "${caseLogPath}"
		) &
	else
		(
			qemu-system-arm "${qemuArgs[@]}" 2>&1 | cleanQemuStream >> "${caseLogPath}"
		) &
	fi
	qemuPid=$!

	while true; do
		if ! kill -0 "${qemuPid}" >/dev/null 2>&1; then
			wait "${qemuPid}"
			qemuRc=$?
			break
		fi

		if ((SECONDS >= deadlineSec)); then
			stopQemuRun "${qemuPid}"
			wait "${qemuPid}"
			qemuRc=124
			break
		fi

		sleep 1
	done
	set -e

	# 给日志管道极短的 flush 时间，避免尾部 marker 还没完全写入。
	sleep 0.1

	if [[ "${qemuRc}" -eq 124 ]]; then
		ryanjson_log_error "QEMU 超时（${qemuTimeoutSec}s）"
		tail -n 120 "${caseLogPath}"
		cleanupCaseLogIfNeeded "${keepCaseLog}" "${caseLogPath}"
		return 1
	fi

	if qemuLogHasFailureMarkers "${caseLogPath}"; then
		ryanjson_log_error "用例失败（检测到 [QEMU][RESULT] UNIT_FAIL）"
		tail -n 120 "${caseLogPath}"
		cleanupCaseLogIfNeeded "${keepCaseLog}" "${caseLogPath}"
		return 1
	fi

	if ! qemuLogHasRequiredMarkers "${caseLogPath}"; then
		reportMissingLogMarkers "${caseLogPath}" || true
		ryanjson_log_error "用例失败，日志尾部："
		tail -n 120 "${caseLogPath}"
		cleanupCaseLogIfNeeded "${keepCaseLog}" "${caseLogPath}"
		return 1
	fi

	cleanupCaseLogIfNeeded "${keepCaseLog}" "${caseLogPath}"
	ryanjson_log_done "通过: ${caseName}"
	return 0
}

# 执行矩阵
runQemuMatrix() {
	local totalCases="${#caseList[@]}"
	local caseIndex=0
	local failedCases=0
	local entry=""
	local strictKey=""
	local addAtHead=""
	local scientific=""

	for entry in "${caseList[@]}"; do
		caseIndex=$((caseIndex + 1))
		read -r strictKey addAtHead scientific <<< "${entry}"

		echo
		ryanjson_print_banner_begin "【QEMU 用例 ${caseIndex}/${totalCases}】"
		ryanjson_print_banner_end

		if run_case "${strictKey}" "${addAtHead}" "${scientific}"; then
			:
		else
			failedCases=$((failedCases + 1))
			if [[ "${qemuStopOnFail}" == "1" ]]; then
				ryanjson_log_error "按 QEMU_STOP_ON_FAIL=1 提前终止。"
				return 1
			fi
		fi
	done

	echo
	ryanjson_log_done "QEMU 单测矩阵执行完成。"
	ryanjson_log_info "模式: ${qemuMode}"
	ryanjson_log_info "总用例: ${totalCases}"
	ryanjson_log_info "失败用例: ${failedCases}"
	if [[ "${qemuSaveLog}" == "1" ]]; then
		ryanjson_log_info "日志目录: ${qemuLogRoot}"
	else
		ryanjson_log_info "日志输出: 终端实时输出（不落盘）"
	fi

	if [[ "${failedCases}" -gt 0 ]]; then
		return 1
	fi
}

# 入口函数
ryanjson_qemu_main() {
	requireQemuToolchain
	detectQemuSemihostingMode
	resolveQemuMachineDefaults
	prepareQemuCaseList
	printQemuBanner
	runQemuMatrix
}

main() {
	ryanjson_qemu_main "$@"
}

main "$@"
