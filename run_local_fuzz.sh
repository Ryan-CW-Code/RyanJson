#!/bin/bash
set -euo pipefail

# 本地一键 Fuzz（默认低内存并发，可通过环境变量覆盖）。
# 用途：封装常用 fuzz 参数，避免每次手工传 FUZZ_WORKERS/FUZZ_JOBS/FUZZ_RSS_LIMIT_MB。
# 默认行为：
#   默认模式 nightly，默认生成覆盖率
#   workers/jobs 默认为 1/9（可覆盖）
#   默认走固定轮次 FUZZ_RUNS=100000；若显式提供 FUZZ_MAX_TOTAL_TIME 且未设置 FUZZ_RUNS，则走时间预算模式
#   FUZZ_ENABLE_MERGE=1 时先将原始 corpus merge 到临时目录，再覆盖原始 corpus 后继续 fuzz
# 可覆盖参数：
#   FUZZ_MODE/FUZZ_SKIP_COV/FUZZ_RUNS/FUZZ_MAX_TOTAL_TIME/FUZZ_TIMEOUT/FUZZ_MAX_LEN
#   FUZZ_WORKERS/FUZZ_JOBS/FUZZ_RSS_LIMIT_MB/FUZZ_MALLOC_LIMIT_MB
#   FUZZ_ENABLE_MERGE
#   FUZZ_LOG_PATH: 日志输出文件路径（默认 ./fuzz.log，覆盖写）
#     - 想保留“每次单独日志”可自行设置：FUZZ_LOG_PATH=./fuzz-$(date +%Y%m%d-%H%M%S).log
#   FUZZ_LOG_STRIP_ANSI=1: 写入日志时去掉 ANSI 颜色控制码（默认开启，确保日志为纯 UTF-8 文本）
#   说明：
#     - 日志仅记录“运行阶段”，不包含编译输出
# 三个语义宏默认值（可覆盖）：
#   RYANJSON_STRICT_OBJECT_KEY_CHECK=false
#   RYANJSON_DEFAULT_ADD_AT_HEAD=false
#   RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC=false

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib/common.sh
source "${scriptDir}/scripts/lib/common.sh"
repoRoot="$(ryanjson_repo_root_from_source "${BASH_SOURCE[0]}" 0)"
cd "${repoRoot}"

# ================= 可手改默认值（不想传参就改这里） =================
DEFAULT_FUZZ_MODE="nightly"
DEFAULT_FUZZ_SKIP_COV="0"
DEFAULT_FUZZ_RUNS="100000000"
DEFAULT_FUZZ_MAX_TOTAL_TIME=""
DEFAULT_FUZZ_TIMEOUT="4"
DEFAULT_FUZZ_MAX_LEN="8192"
DEFAULT_FUZZ_WORKERS="1"
DEFAULT_FUZZ_JOBS="9"
DEFAULT_FUZZ_RSS_LIMIT_MB="4096"
DEFAULT_FUZZ_MALLOC_LIMIT_MB=""
DEFAULT_FUZZ_ENABLE_MERGE="0"
DEFAULT_FUZZ_LOG_PATH="./fuzz.log"
DEFAULT_FUZZ_LOG_STRIP_ANSI="1"

DEFAULT_RYANJSON_STRICT_OBJECT_KEY_CHECK="false"
DEFAULT_RYANJSON_DEFAULT_ADD_AT_HEAD="false"
DEFAULT_RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC="false"
# =================================================================

fuzz_log_line() {
	# 输出一行日志（终端 + 可选落盘）
	local line="$1"

	if [[ -n "${FUZZ_LOG_PATH}" ]]; then
		if [[ "${FUZZ_LOG_STRIP_ANSI}" == "1" ]]; then
			echo "${line}" | tee >(sed -r 's/\x1B\[[0-?]*[ -/]*[@-~]//g' >> "${FUZZ_LOG_PATH}")
		else
			echo "${line}" | tee -a "${FUZZ_LOG_PATH}"
		fi
	else
		echo "${line}"
	fi
}

fuzz_log_begin() {
	# 初始化 fuzz 运行日志（覆盖写）
	local runId="$1"

	mkdir -p "$(dirname "${FUZZ_LOG_PATH}")"
	if [[ "${FUZZ_LOG_STRIP_ANSI}" == "1" ]]; then
		echo "[信息] Fuzz 日志将保存到：${FUZZ_LOG_PATH}（覆盖写，去除 ANSI 控制码，仅运行日志）"
	else
		echo "[信息] Fuzz 日志将保存到：${FUZZ_LOG_PATH}（覆盖写，仅运行日志）"
	fi

	: > "${FUZZ_LOG_PATH}"
	fuzz_log_line "==================== FUZZ BEGIN ${runId} ===================="
	local runsLabel="${FUZZ_RUNS:-默认}"
	local timeLabel="${FUZZ_MAX_TOTAL_TIME:-}"
	if [[ -z "${timeLabel}" ]]; then
		timeLabel="-"
	fi
	fuzz_log_line "[信息] 时间=$(date '+%Y-%m-%d %H:%M:%S') 模式=${FUZZ_MODE} runs=${runsLabel} max_total_time=${timeLabel} workers/jobs=${FUZZ_WORKERS}/${FUZZ_JOBS}"
}

fuzz_log_end() {
	# 结束标记，写入退出码
	local runId="$1"
	local status="$2"
	fuzz_log_line "==================== FUZZ END ${runId} rc=${status} ===================="
}

run_fuzz_flow() (
	set -euo pipefail

	# 读取参数与默认值（保持与旧脚本一致）
	local fuzzMode="${FUZZ_MODE:-quick}"
	local fuzzSkipCov="${FUZZ_SKIP_COV:-0}"
	local fuzzTimeout="${FUZZ_TIMEOUT:-4}"
	local fuzzMaxLen="${FUZZ_MAX_LEN:-8192}"
	local fuzzVerbosity="${FUZZ_VERBOSITY:-0}"
	local fuzzCorpusDir="${FUZZ_CORPUS_DIR:-./test/fuzzer/corpus}"
	local fuzzDictPath="${FUZZ_DICT_PATH:-./test/fuzzer/RyanJsonFuzzer.dict}"
	local fuzzRuns="${FUZZ_RUNS:-}"
	local fuzzMaxTotalTime="${FUZZ_MAX_TOTAL_TIME:-}"
	local fuzzWorkers="${FUZZ_WORKERS:-}"
	local fuzzJobs="${FUZZ_JOBS:-}"
	local fuzzRssLimitMb="${FUZZ_RSS_LIMIT_MB:-}"
	local fuzzMallocLimitMb="${FUZZ_MALLOC_LIMIT_MB:-}"
	local fuzzEnableMerge="${FUZZ_ENABLE_MERGE:-0}"
	local xmakeForceClean="${XMAKE_FORCE_CLEAN:-0}"
	local defaultRuns=100000
	local defaultWorkers=""
	local defaultJobs=""

	# 环境依赖检查
	ryanjson_require_cmd xmake

	# 模式校验与并发默认值
	case "${fuzzMode}" in
	quick)
		defaultWorkers=2
		defaultJobs=2
		;;
	nightly)
		defaultWorkers=4
		defaultJobs=4
		;;
	full)
		defaultWorkers=6
		defaultJobs=6
		;;
	*)
		ryanjson_log_error "FUZZ_MODE 仅支持 quick/nightly/full，当前值：${fuzzMode}"
		return 1
		;;
	esac

	# merge 参数校验
	if ! ryanjson_require_01 "FUZZ_ENABLE_MERGE" "${fuzzEnableMerge}"; then
		return 1
	fi

	# 0/1 与数值类参数校验
	if ! ryanjson_require_01 "FUZZ_SKIP_COV" "${fuzzSkipCov}"; then
		return 1
	fi
	if ! ryanjson_require_pos_int "FUZZ_TIMEOUT" "${fuzzTimeout}" "秒"; then
		return 1
	fi
	if ! ryanjson_require_pos_int "FUZZ_MAX_LEN" "${fuzzMaxLen}"; then
		return 1
	fi
	if ! ryanjson_require_nonneg_int "FUZZ_VERBOSITY" "${fuzzVerbosity}"; then
		return 1
	fi

	# 补齐未显式设置的默认值
	if [[ -z "${fuzzWorkers}" ]]; then
		fuzzWorkers="${defaultWorkers}"
	fi
	if [[ -z "${fuzzJobs}" ]]; then
		fuzzJobs="${defaultJobs}"
	fi
	if ! ryanjson_require_pos_int "FUZZ_WORKERS" "${fuzzWorkers}"; then
		return 1
	fi
	if ! ryanjson_require_pos_int "FUZZ_JOBS" "${fuzzJobs}"; then
		return 1
	fi
	if ! ryanjson_require_nonneg_int_optional "FUZZ_RSS_LIMIT_MB" "${fuzzRssLimitMb}"; then
		return 1
	fi
	if ! ryanjson_require_nonneg_int_optional "FUZZ_MALLOC_LIMIT_MB" "${fuzzMallocLimitMb}"; then
		return 1
	fi

	# runs / max_total_time 处理与校验
	if [[ -n "${fuzzRuns}" ]]; then
		if ! ryanjson_require_pos_int "FUZZ_RUNS" "${fuzzRuns}"; then
			return 1
		fi
		if [[ -n "${fuzzMaxTotalTime}" ]]; then
			ryanjson_log_warn "同时设置 FUZZ_RUNS 与 FUZZ_MAX_TOTAL_TIME，已优先使用 FUZZ_RUNS，忽略时间预算。"
			fuzzMaxTotalTime=""
		fi
	fi

	if [[ -z "${fuzzRuns}" && -n "${fuzzMaxTotalTime}" ]]; then
		if ! ryanjson_require_pos_int "FUZZ_MAX_TOTAL_TIME" "${fuzzMaxTotalTime}" "秒"; then
			return 1
		fi
	fi

	if [[ -z "${fuzzRuns}" && -z "${fuzzMaxTotalTime}" ]]; then
		fuzzRuns="${defaultRuns}"
	fi

	# 语义宏组合（用于 caseName 与构建配置）
	local strictKey="${RYANJSON_STRICT_OBJECT_KEY_CHECK:-false}"
	local addAtHead="${RYANJSON_DEFAULT_ADD_AT_HEAD:-true}"
	local scientific="${RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC:-true}"
	local caseName=""
	caseName="$(ryanjson_semantic_case_name "${strictKey}" "${addAtHead}" "${scientific}")"

	# 覆盖率输出目录
	local coverageRoot="test/fuzzer/coverage"
	local profileRoot="${coverageRoot}/profiles"
	ryanjson_prepare_clean_dir "${coverageRoot}"
	mkdir -p "${profileRoot}"

	local profraw="${profileRoot}/coverage.profraw"
	local profdata="${coverageRoot}/coverage.profdata"
	local reportTxt="${coverageRoot}/report.txt"
	local reportHtml="${coverageRoot}/html"

	# 打印执行配置
	ryanjson_print_banner_begin "Fuzzer 执行配置"
	ryanjson_print_banner_kv "模式" "${fuzzMode}"
	ryanjson_print_banner_kv "配置" "${caseName}"
	ryanjson_print_banner_kv "Corpus" "${fuzzCorpusDir}"
	ryanjson_print_banner_kv "字典" "${fuzzDictPath}"
	ryanjson_print_banner_kv "merge" "${fuzzEnableMerge}"
	if [[ "${fuzzEnableMerge}" == "1" ]]; then
		ryanjson_print_banner_kv "merged_corpus" "${fuzzCorpusDir} (in-place)"
	fi
	ryanjson_print_banner_kv "timeout" "${fuzzTimeout}"
	ryanjson_print_banner_kv "max_len" "${fuzzMaxLen}"
	ryanjson_print_banner_kv "workers/jobs" "${fuzzWorkers}/${fuzzJobs}"
	if [[ -n "${fuzzRssLimitMb}" ]]; then
		ryanjson_print_banner_kv "rss_limit_mb" "${fuzzRssLimitMb}"
	fi
	if [[ -n "${fuzzMallocLimitMb}" ]]; then
		ryanjson_print_banner_kv "malloc_limit_mb" "${fuzzMallocLimitMb}"
	fi
	if [[ -n "${fuzzRuns}" ]]; then
		ryanjson_print_banner_kv "runs" "${fuzzRuns}"
	else
		ryanjson_print_banner_kv "max_total_time" "${fuzzMaxTotalTime}s"
	fi
	ryanjson_print_banner_end

	# 配置与构建
	ryanjson_run_xmake_config "${xmakeForceClean}" "${caseName}"
	ryanjson_run_xmake_build "RyanJsonFuzz" "${caseName}"
	ryanjson_log_info "xmake 构建完成（target=RyanJsonFuzz）"

	# 确保 corpus 存在
	if [[ ! -d "${fuzzCorpusDir}" ]]; then
		mkdir -p "${fuzzCorpusDir}"
		ryanjson_log_info "Corpus 目录不存在，已自动创建：${fuzzCorpusDir}"
	fi

	local actualFuzzCorpusDir="${fuzzCorpusDir}"
	# 可选：merge 输入 corpus，避免无序膨胀
	if [[ "${fuzzEnableMerge}" == "1" ]]; then
		local mergeTempDir="${coverageRoot}/merged-corpus.tmp"

		ryanjson_prepare_clean_dir "${mergeTempDir}"
		ryanjson_log_phase "正在 merge corpus..."
		./build/linux/x86/release/RyanJsonFuzz -merge=1 "${mergeTempDir}" "${fuzzCorpusDir}"

		rm -rf "${fuzzCorpusDir}"
		mv "${mergeTempDir}" "${fuzzCorpusDir}"
		actualFuzzCorpusDir="${fuzzCorpusDir}"
		ryanjson_log_info "merge 完成，后续 fuzz 使用：${actualFuzzCorpusDir}"
	fi

	# 组装 libFuzzer 参数
	local -a dictArgs=()
	if [[ -f "${fuzzDictPath}" ]]; then
		dictArgs+=("-dict=${fuzzDictPath}")
	else
		ryanjson_log_warn "未找到字典文件 ${fuzzDictPath}，已跳过 -dict 参数。"
	fi

	local -a fuzzArgs=()
	fuzzArgs+=("${actualFuzzCorpusDir}")
	fuzzArgs+=("${dictArgs[@]}")
	fuzzArgs+=("-timeout=${fuzzTimeout}")
	fuzzArgs+=("-verbosity=${fuzzVerbosity}")
	fuzzArgs+=("-max_len=${fuzzMaxLen}")
	fuzzArgs+=("-workers=${fuzzWorkers}")
	fuzzArgs+=("-jobs=${fuzzJobs}")
	if [[ -n "${fuzzRssLimitMb}" ]]; then
		fuzzArgs+=("-rss_limit_mb=${fuzzRssLimitMb}")
	fi
	if [[ -n "${fuzzMallocLimitMb}" ]]; then
		fuzzArgs+=("-malloc_limit_mb=${fuzzMallocLimitMb}")
	fi
	if [[ -n "${fuzzRuns}" ]]; then
		fuzzArgs+=("-runs=${fuzzRuns}")
	else
		fuzzArgs+=("-max_total_time=${fuzzMaxTotalTime}")
	fi
	if [[ -n "${FUZZ_EXTRA_ARGS:-}" ]]; then
		local -a extraArgs=()
		read -r -a extraArgs <<< "${FUZZ_EXTRA_ARGS}"
		fuzzArgs+=("${extraArgs[@]}")
	fi

	# 运行 fuzz（只记录运行阶段到日志）
	local runSummary=""
	if [[ -n "${fuzzRuns}" ]]; then
		runSummary="runs=${fuzzRuns}"
	else
		runSummary="max_total_time=${fuzzMaxTotalTime}"
	fi
	local limitSummary=""
	if [[ -n "${fuzzRssLimitMb}" ]]; then
		limitSummary="${limitSummary} rss_limit_mb=${fuzzRssLimitMb}"
	fi
	if [[ -n "${fuzzMallocLimitMb}" ]]; then
		limitSummary="${limitSummary} malloc_limit_mb=${fuzzMallocLimitMb}"
	fi

	fuzz_log_line "[阶段] 正在运行 fuzz 二进制..."
	fuzz_log_line "[信息] Fuzz 参数摘要: ${runSummary} workers/jobs=${fuzzWorkers}/${fuzzJobs} timeout=${fuzzTimeout} max_len=${fuzzMaxLen}${limitSummary}"

	if [[ -n "${FUZZ_LOG_PATH}" ]]; then
		if [[ "${FUZZ_LOG_STRIP_ANSI}" == "1" ]]; then
			LLVM_PROFILE_FILE="${profraw}" ./build/linux/x86/release/RyanJsonFuzz "${fuzzArgs[@]}" 2>&1 \
				| tee >(sed -r 's/\x1B\[[0-?]*[ -/]*[@-~]//g' >> "${FUZZ_LOG_PATH}")
		else
			LLVM_PROFILE_FILE="${profraw}" ./build/linux/x86/release/RyanJsonFuzz "${fuzzArgs[@]}" 2>&1 \
				| tee -a "${FUZZ_LOG_PATH}"
		fi
	else
		LLVM_PROFILE_FILE="${profraw}" ./build/linux/x86/release/RyanJsonFuzz "${fuzzArgs[@]}"
	fi

	# 覆盖率生成
	if [[ "${fuzzSkipCov}" == "1" ]]; then
		ryanjson_log_info "FUZZ_SKIP_COV=1，已跳过覆盖率生成。"
		ryanjson_log_info "输出目录：${coverageRoot}"
		return 0
	fi

	if ! command -v llvm-profdata >/dev/null 2>&1; then
		ryanjson_log_error "未找到 llvm-profdata，无法生成覆盖率。"
		return 1
	fi
	if ! command -v llvm-cov >/dev/null 2>&1; then
		ryanjson_log_error "未找到 llvm-cov，无法生成覆盖率。"
		return 1
	fi

	llvm-profdata merge -sparse "${profraw}" -o "${profdata}"

	echo "---------------- 覆盖率摘要（llvm-cov report） ----------------"
	llvm-cov report ./build/linux/x86/release/RyanJsonFuzz \
		-instr-profile="${profdata}" \
		-show-mcdc-summary \
		--use-color \
		-sources ./RyanJson

	llvm-cov report ./build/linux/x86/release/RyanJsonFuzz \
		-instr-profile="${profdata}" \
		-show-mcdc-summary \
		-sources ./RyanJson > "${reportTxt}"

	llvm-cov show ./build/linux/x86/release/RyanJsonFuzz \
		-instr-profile="${profdata}" \
		-format=html \
		-output-dir="${reportHtml}" \
		-show-mcdc-summary \
		-show-branches=count \
		-show-expansions \
		-show-regions \
		-show-line-counts-or-regions \
		-sources ./RyanJson

	ryanjson_log_done "Fuzzer 执行结束，覆盖率已生成。"
	ryanjson_log_info "输出目录：${coverageRoot}"
	ryanjson_log_info "覆盖率文本报告：${reportTxt}"
	ryanjson_log_info "覆盖率HTML目录：${reportHtml}"
)

main() {
	: "${FUZZ_MODE:=${DEFAULT_FUZZ_MODE}}"
	: "${FUZZ_SKIP_COV:=${DEFAULT_FUZZ_SKIP_COV}}"
	: "${FUZZ_RUNS:=${DEFAULT_FUZZ_RUNS}}"
	: "${FUZZ_MAX_TOTAL_TIME:=${DEFAULT_FUZZ_MAX_TOTAL_TIME}}"
	: "${FUZZ_TIMEOUT:=${DEFAULT_FUZZ_TIMEOUT}}"
	: "${FUZZ_MAX_LEN:=${DEFAULT_FUZZ_MAX_LEN}}"
	: "${FUZZ_WORKERS:=${DEFAULT_FUZZ_WORKERS}}"
	: "${FUZZ_JOBS:=${DEFAULT_FUZZ_JOBS}}"
	: "${FUZZ_RSS_LIMIT_MB:=${DEFAULT_FUZZ_RSS_LIMIT_MB}}"
	: "${FUZZ_MALLOC_LIMIT_MB:=${DEFAULT_FUZZ_MALLOC_LIMIT_MB}}"
	: "${FUZZ_ENABLE_MERGE:=${DEFAULT_FUZZ_ENABLE_MERGE}}"
	# 默认写入“单一日志文件”，便于判断上次 fuzz 是否完整结束。
	# 若需要每次单独文件，可通过 FUZZ_LOG_PATH 显式覆盖。
	: "${FUZZ_LOG_PATH:=${DEFAULT_FUZZ_LOG_PATH}}"
	: "${FUZZ_LOG_STRIP_ANSI:=${DEFAULT_FUZZ_LOG_STRIP_ANSI}}"

	: "${RYANJSON_STRICT_OBJECT_KEY_CHECK:=${DEFAULT_RYANJSON_STRICT_OBJECT_KEY_CHECK}}"
	: "${RYANJSON_DEFAULT_ADD_AT_HEAD:=${DEFAULT_RYANJSON_DEFAULT_ADD_AT_HEAD}}"
	: "${RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC:=${DEFAULT_RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC}}"

	export RYANJSON_STRICT_OBJECT_KEY_CHECK
	export RYANJSON_DEFAULT_ADD_AT_HEAD
	export RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC
	export FUZZ_MODE FUZZ_SKIP_COV FUZZ_RUNS FUZZ_MAX_TOTAL_TIME FUZZ_TIMEOUT FUZZ_MAX_LEN
	export FUZZ_WORKERS FUZZ_JOBS FUZZ_RSS_LIMIT_MB FUZZ_MALLOC_LIMIT_MB FUZZ_ENABLE_MERGE

	# 记录一次运行的唯一标识，方便在总日志中定位开始/结束段。
	fuzzRunId="$(date +%Y%m%d-%H%M%S)"

	ryanjson_print_banner_begin "本地 Fuzz 启动"
	ryanjson_print_banner_kv "FUZZ_MODE" "${FUZZ_MODE}"
	ryanjson_print_banner_kv "FUZZ_SKIP_COV" "${FUZZ_SKIP_COV}"
	ryanjson_print_banner_kv_optional "FUZZ_RUNS" "${FUZZ_RUNS}"
	ryanjson_print_banner_kv_optional "FUZZ_MAX_TOTAL_TIME" "${FUZZ_MAX_TOTAL_TIME}"
	ryanjson_print_banner_kv "FUZZ_TIMEOUT" "${FUZZ_TIMEOUT}"
	ryanjson_print_banner_kv "FUZZ_MAX_LEN" "${FUZZ_MAX_LEN}"
	ryanjson_print_banner_kv "FUZZ_WORKERS" "${FUZZ_WORKERS}"
	ryanjson_print_banner_kv "FUZZ_JOBS" "${FUZZ_JOBS}"
	ryanjson_print_banner_kv_optional "FUZZ_RSS_LIMIT_MB" "${FUZZ_RSS_LIMIT_MB}"
	ryanjson_print_banner_kv_optional "FUZZ_MALLOC_LIMIT_MB" "${FUZZ_MALLOC_LIMIT_MB}"
	ryanjson_print_banner_kv "FUZZ_ENABLE_MERGE" "${FUZZ_ENABLE_MERGE}"
	ryanjson_print_banner_kv "FUZZ_LOG_PATH" "${FUZZ_LOG_PATH}"
	ryanjson_print_banner_kv "RYANJSON_STRICT_OBJECT_KEY_CHECK" "${RYANJSON_STRICT_OBJECT_KEY_CHECK}"
	ryanjson_print_banner_kv "RYANJSON_DEFAULT_ADD_AT_HEAD" "${RYANJSON_DEFAULT_ADD_AT_HEAD}"
	ryanjson_print_banner_kv "RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC" "${RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC}"
	ryanjson_print_banner_end

	if [[ -n "${FUZZ_LOG_PATH}" ]]; then
		local status=0
		fuzz_log_begin "${fuzzRunId}"
		set +e
		run_fuzz_flow
		status=$?
		set -e
		fuzz_log_end "${fuzzRunId}" "${status}"
		return "${status}"
	fi

	run_fuzz_flow
}

main "$@"
