#!/usr/bin/env bash
set -euo pipefail

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib/common.sh
source "${scriptDir}/scripts/lib/common.sh"
repoRoot="$(ryanjson_repo_root_from_source "${BASH_SOURCE[0]}" 0)"
cd "${repoRoot}"

memPlatform="${MEM_PLATFORM:-both}"
memMode="${MEM_MODE:-full}"
memStopOnFail="${MEM_STOP_ON_FAIL:-1}"
memSingleCase="${MEM_SINGLE_CASE:-1}"
memDefaultCase="${MEM_DEFAULT_CASE:-false false true}"
memStripAnsiLog="${MEM_STRIP_ANSI_LOG:-1}"

readonly memCases=(
	"header=12 align=8"
	"header=12 align=4"
	"header=8 align=8"
	"header=8 align=4"
	"header=4 align=4"
)

ryanjson_strip_ansi() {
	if command -v perl >/dev/null 2>&1; then
		perl -pe 's/\xEF\xBB\xBF//g; s/\x1b\[[0-9;]*[A-Za-z]//g'
	else
		sed -r 's/\xEF\xBB\xBF//g; s/\x1B\[[0-9;]*[A-Za-z]//g'
	fi
}

append_markdown_table_header() {
	local markdownPath="$1"
	local headerSize="$2"
	local alignSize="$3"
	local tableIndex="$4"

	if ((tableIndex > 0)); then
		printf '\n' >> "${markdownPath}"
	fi
	printf '### malloc 头部空间=%s 字节，对齐=%s 字节\n' "${headerSize}" "${alignSize}" >> "${markdownPath}"
	printf '| 用例 | 文本长度 | RyanJson 内存 | cJSON 内存 | yyjson 内存 | 相比 cJSON 节省%% | 相比 yyjson 节省%% |\n' >> "${markdownPath}"
	printf '| --- | --- | --- | --- | --- | --- | --- |\n' >> "${markdownPath}"
}

append_markdown_update_time() {
	local markdownPath="$1"
	local now=""
	now="$(date '+%Y-%m-%d %H:%M:%S')"
	printf '更新时间：%s\n\n' "${now}" >> "${markdownPath}"
}

append_markdown_title() {
	local markdownPath="$1"
	local platformLabel="$2"
	printf '# 内存占用对比（%s）\n\n' "${platformLabel}" >> "${markdownPath}"
}

append_markdown_note() {
	local markdownPath="$1"
	printf '说明：\n' >> "${markdownPath}"
	printf '%s\n' "- host 与 QEMU 结果可能不同：平台 ABI 与对齐规则差异会改变结构体布局与 padding。" >> "${markdownPath}"
	printf '%s\n' "- 即使同为 32 位，x86(i386) 与 ARM EABI 的 double/uint64_t 对齐也可能不同。" >> "${markdownPath}"
	printf '%s\n\n' "- 需要严格一致时，请以 QEMU 结果为准，或在同一 ABI/工具链下对比。" >> "${markdownPath}"
}

append_markdown_rows_from_stream() {
	local markdownPath="$1"
	awk '
		{
			gsub(/\r/, "", $0);
		}
		function pct(v) {
			if (v == "" || v == "NA") { return "-"; }
			return "**" v "%**";
		}
		function case_cn(id) {
			if (id == "mixed") { return "混合对象"; }
			if (id == "weather_object") { return "经典天气对象"; }
			if (id == "deep_array") { return "深度数组"; }
			if (id == "small_mixed") { return "小型混合对象"; }
			if (id == "small_string") { return "小型字符串对象"; }
			if (id == "compressed_business") { return "压缩业务对象"; }
			return id;
		}
		/^\[MEM\]\[COMPARE\]/ {
			id = ""; len = ""; r = ""; c = ""; y = "";
			for (i = 2; i <= NF; i++) {
				split($i, kv, "=");
				key = kv[1]; val = kv[2];
				if (key == "id") id = val;
				else if (key == "len") len = val;
				else if (key == "ryanjson") r = val;
				else if (key == "cjson") c = val;
				else if (key == "yyjson") y = val;
			}
			if (id == "") { next; }
			saveC = "NA"; saveY = "NA";
			if ((c + 0) > 0 && (r + 0) > 0) { saveC = sprintf("%.2f", 100 - (r * 100 / c)); }
			if ((y + 0) > 0 && (r + 0) > 0) { saveY = sprintf("%.2f", 100 - (r * 100 / y)); }
			printf "| %s | %s | %s | %s | %s | %s | %s |\n", \
				case_cn(id), len, r, c, y, pct(saveC), pct(saveY);
		}
	' >> "${markdownPath}"
}

run_and_capture_rows() {
	local markdownPath="$1"
	local stripAnsi="$2"
	shift 2
	local tmpRoot="${repoRoot}/localLogs/_tmp"
	local fifoPath="${tmpRoot}/memory_stream.$$"
	local cmdRc=0
	local parserPid=0

	mkdir -p "${tmpRoot}"
	rm -f "${fifoPath}"
	mkfifo "${fifoPath}"

	append_markdown_rows_from_stream "${markdownPath}" < "${fifoPath}" &
	parserPid=$!

	if [[ "${stripAnsi}" == "1" ]]; then
		"$@" 2>&1 | ryanjson_strip_ansi | tee "${fifoPath}"
	else
		"$@" 2>&1 | tee "${fifoPath}"
	fi
	cmdRc=${PIPESTATUS[0]}

	wait "${parserPid}"
	rm -f "${fifoPath}"

	return "${cmdRc}"
}

validate_inputs() {
	if [[ "${memPlatform}" != "host" && "${memPlatform}" != "qemu" && "${memPlatform}" != "both" ]]; then
		ryanjson_log_error "MEM_PLATFORM only supports host/qemu/both, current: ${memPlatform}"
		return 1
	fi
	if ! ryanjson_require_01 "MEM_STOP_ON_FAIL" "${memStopOnFail}"; then
		return 1
	fi
	if ! ryanjson_require_01 "MEM_STRIP_ANSI_LOG" "${memStripAnsiLog}"; then
		return 1
	fi
	if ! ryanjson_require_01 "MEM_SINGLE_CASE" "${memSingleCase}"; then
		return 1
	fi
	if [[ "${memSingleCase}" == "0" ]]; then
		if ! ryanjson_emit_semantic_cases "${memMode}" "MEM_MODE" >/dev/null; then
			return 1
		fi
	fi
}

print_banner() {
	ryanjson_print_banner_begin "Local Memory Compare"
	ryanjson_print_banner_kv "MEM_PLATFORM" "${memPlatform}"
	ryanjson_print_banner_kv "MEM_MODE" "${memMode}"
	ryanjson_print_banner_kv "MEM_STOP_ON_FAIL" "${memStopOnFail}"
	ryanjson_print_banner_kv "MEM_SINGLE_CASE" "${memSingleCase}"
	ryanjson_print_banner_kv_optional "MEM_DEFAULT_CASE" "${memDefaultCase}"
	ryanjson_print_banner_kv "MEM_STRIP_ANSI_LOG" "${memStripAnsiLog}"
	ryanjson_print_banner_end
}

run_host_memory() {
	local markdownPath="${repoRoot}/reports/memory/host.md"
	local rc=0
	local caseRc=0
	local entry=""
	local headerSize=0
	local alignSize=0
	local caseLabel=""
	local tableIndex=0
	local headerToken=""
	local alignToken=""

	ryanjson_init_utf8_log "${markdownPath}"
	append_markdown_title "${markdownPath}" "Host"
	append_markdown_update_time "${markdownPath}"
	append_markdown_note "${markdownPath}"

	for entry in "${memCases[@]}"; do
		read -r headerToken alignToken <<< "${entry}"
		headerSize="${headerToken#header=}"
		alignSize="${alignToken#align=}"
		caseLabel="h${headerSize}_a${alignSize}"
		append_markdown_table_header "${markdownPath}" "${headerSize}" "${alignSize}" "${tableIndex}"
		tableIndex=$((tableIndex + 1))

		export RYANJSON_TEST_ALLOC_HEADER_SIZE="${headerSize}"
		export RYANJSON_TEST_ALLOC_ALIGN_SIZE="${alignSize}"
		export RYANJSON_UNIT_ONLY_MEMORY=1
		export UNIT_MODE="${memMode}"
		export UNIT_SKIP_COV=1
		export UNIT_STOP_ON_FAIL="${memStopOnFail}"
		export XMAKE_FORCE_CLEAN="${XMAKE_FORCE_CLEAN:-0}"
		export UNIT_SYNC_ONLY="${UNIT_SYNC_ONLY:-0}"
		if [[ "${memSingleCase}" == "1" ]]; then
			export UNIT_SINGLE_CASE="${memDefaultCase}"
		else
			unset UNIT_SINGLE_CASE
		fi

		set +e
		run_and_capture_rows "${markdownPath}" "${memStripAnsiLog}" \
			bash "${repoRoot}/run_local_base.sh"
		caseRc=$?
		set -e

		if [[ "${caseRc}" -ne 0 ]]; then
			rc="${caseRc}"
			if [[ "${memStopOnFail}" == "1" ]]; then
				break
			fi
		fi
	done

	ryanjson_log_done "Memory summary (Markdown): ${markdownPath}"
	cat "${markdownPath}"
	return "${rc}"
}

run_qemu_memory() {
	local markdownPath="${repoRoot}/reports/memory/qemu.md"
	local rc=0
	local caseRc=0
	local entry=""
	local headerSize=0
	local alignSize=0
	local caseLabel=""
	local tableIndex=0
	local headerToken=""
	local alignToken=""

	ryanjson_init_utf8_log "${markdownPath}"
	append_markdown_title "${markdownPath}" "QEMU"
	append_markdown_update_time "${markdownPath}"
	append_markdown_note "${markdownPath}"

	for entry in "${memCases[@]}"; do
		read -r headerToken alignToken <<< "${entry}"
		headerSize="${headerToken#header=}"
		alignSize="${alignToken#align=}"
		caseLabel="h${headerSize}_a${alignSize}"
		append_markdown_table_header "${markdownPath}" "${headerSize}" "${alignSize}" "${tableIndex}"
		tableIndex=$((tableIndex + 1))

		export RYANJSON_TEST_ALLOC_HEADER_SIZE="${headerSize}"
		export RYANJSON_TEST_ALLOC_ALIGN_SIZE="${alignSize}"
		export RYANJSON_UNIT_ONLY_MEMORY=1
		export QEMU_MODE="${memMode}"
		export QEMU_STOP_ON_FAIL="${memStopOnFail}"
		export QEMU_SAVE_LOG=0
		export QEMU_CONSOLE_LOG=1
		export QEMU_LOG_ROOT="localLogs/qemu/memory/${caseLabel}"
		export QEMU_STRIP_ANSI_LOG="${memStripAnsiLog}"
		if [[ "${memSingleCase}" == "1" ]]; then
			export QEMU_SINGLE_CASE="${memDefaultCase}"
		else
			unset QEMU_SINGLE_CASE
		fi

		set +e
		run_and_capture_rows "${markdownPath}" "${memStripAnsiLog}" \
			bash "${repoRoot}/run_local_qemu.sh"
		caseRc=$?
		set -e

		if [[ "${caseRc}" -ne 0 ]]; then
			rc="${caseRc}"
			if [[ "${memStopOnFail}" == "1" ]]; then
				break
			fi
		fi
	done

	ryanjson_log_done "Memory summary (Markdown): ${markdownPath}"
	cat "${markdownPath}"

	if [[ "${rc}" -ne 0 ]]; then
		return "${rc}"
	fi
	return 0
}

main() {
	validate_inputs
	print_banner

	case "${memPlatform}" in
		host)
			run_host_memory
			;;
		qemu)
			run_qemu_memory
			;;
		both)
			local hostRc=0
			local qemuRc=0
			if run_host_memory; then
				hostRc=0
			else
				hostRc=$?
			fi
			if run_qemu_memory; then
				qemuRc=0
			else
				qemuRc=$?
			fi
			if [[ "${hostRc}" -ne 0 || "${qemuRc}" -ne 0 ]]; then
				return 1
			fi
			;;
	esac
}

main "$@"
