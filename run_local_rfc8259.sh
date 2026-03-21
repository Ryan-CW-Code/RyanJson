#!/usr/bin/env bash
set -euo pipefail

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib/common.sh
source "${scriptDir}/scripts/lib/common.sh"
repoRoot="$(ryanjson_repo_root_from_source "${BASH_SOURCE[0]}" 0)"
cd "${repoRoot}"

rfcPlatform="${RFC_PLATFORM:-both}"
rfcMode="${RFC_MODE:-full}"
rfcStopOnFail="${RFC_STOP_ON_FAIL:-1}"
rfcSingleCase="${RFC_SINGLE_CASE:-1}"
rfcDefaultCase="${RFC_DEFAULT_CASE:-false false true}"
rfcStripAnsiLog="${RFC_STRIP_ANSI_LOG:-1}"

ryanjson_strip_ansi() {
	if command -v perl >/dev/null 2>&1; then
		perl -pe 's/\x1b\[[0-9;]*[A-Za-z]//g'
	else
		sed -r 's/\x1B\[[0-9;]*[A-Za-z]//g'
	fi
}

append_markdown_title() {
	local markdownPath="$1"
	local platformLabel="$2"
	printf '# RFC8259 测试报告（%s）\n\n' "${platformLabel}" >> "${markdownPath}"
}

append_markdown_update_time() {
	local markdownPath="$1"
	local now=""
	now="$(date '+%Y-%m-%d %H:%M:%S')"
	printf '更新时间：%s\n\n' "${now}" >> "${markdownPath}"
}

append_markdown_note() {
	local markdownPath="$1"
	local platformLabel="$2"
	local strictKey="$3"
	local addAtHead="$4"
	local scientific="$5"
	local strictDesc="关闭（允许重复 key）"
	local headDesc="尾插"
	local sciDesc="关闭"

	if [[ "${strictKey}" == "true" ]]; then
		strictDesc="开启（拒绝重复 key）"
	fi
	if [[ "${addAtHead}" == "true" ]]; then
		headDesc="头插"
	fi
	if [[ "${scientific}" == "true" ]]; then
		sciDesc="开启"
	fi

	printf '说明：\n' >> "${markdownPath}"
	printf '%s\n' "- 严格模式：${strictDesc}" >> "${markdownPath}"
	printf '%s\n' "- 插入策略：${headDesc}" >> "${markdownPath}"
	printf '%s\n' "- 科学计数法：${sciDesc}" >> "${markdownPath}"
	printf '%s\n' "- 测试集来源：test/data/rfc8259（共 319 例，已内嵌到 RFC8259 测试代码中）" >> "${markdownPath}"
	printf '%s\n' "- 统计规则：y_/n_ 用例按预期判定，i_ 用例默认计入通过；语义差异不影响通过率。" >> "${markdownPath}"
	printf '%s\n' "- Host/QEMU 均使用内嵌数据集，避免依赖文件系统。" >> "${markdownPath}"
	printf '\n' >> "${markdownPath}"
}

append_markdown_report_from_stream() {
	local markdownPath="$1"
	awk '
		BEGIN {
			split("RyanJson yyjson cJSON", expected, " ");
		}
		function pct(p, t) {
			if (t == 0) { return "-"; }
			return sprintf("%.2f%%", (p * 100.0) / t);
		}
		{
			gsub(/\r/, "", $0);
		}
		/^\[RFC8259\]\[STATS\][[:space:]]/ {
			lib = ""; p = ""; t = "";
			for (i = 1; i <= NF; i++) {
				if (index($i, "lib=") == 1) { lib = substr($i, 5); }
				else if (index($i, "pass=") == 1) { p = substr($i, 6); }
				else if (index($i, "total=") == 1) { t = substr($i, 7); }
			}
			if (lib != "" && p != "" && t != "") {
				statSeen[lib] = 1;
				pass[lib] = p;
				total[lib] = t;
			}
			next;
		}
		/^\[RFC8259\]\[FAIL\][[:space:]]/ {
			lib = ""; file = ""; expect = ""; actual = "";
			for (i = 1; i <= NF; i++) {
				if (index($i, "lib=") == 1) { lib = substr($i, 5); }
				else if (index($i, "file=") == 1) { file = substr($i, 6); }
				else if (index($i, "expect=") == 1) { expect = substr($i, 8); }
				else if (index($i, "actual=") == 1) { actual = substr($i, 8); }
			}
			if (lib == "") { lib = "未知"; }
			kind = "失败";
			if (expect == "success" && actual == "fail") { kind = "期望成功但失败"; }
			else if (expect == "fail" && actual == "success") { kind = "期望失败但成功"; }
			key = lib "\037" file "\037" expect "\037" actual "\037" kind;
			if (!(key in seenFail)) {
				seenFail[key] = 1;
				failByLib[lib]++;
				failLibs[++failCount] = lib;
				failTypes[failCount] = kind;
				failFiles[failCount] = file;
				failExpect[failCount] = expect;
				failActual[failCount] = actual;
			}
			next;
		}
		/^\[RFC8259\]\[LEAK\][[:space:]]/ {
			lib = ""; file = "";
			for (i = 1; i <= NF; i++) {
				if (index($i, "lib=") == 1) { lib = substr($i, 5); }
				else if (index($i, "file=") == 1) { file = substr($i, 6); }
			}
			if (lib == "") { lib = "未知"; }
			key = lib "\037" file "\037" "leak";
			if (!(key in seenFail)) {
				seenFail[key] = 1;
				failByLib[lib]++;
				failLibs[++failCount] = lib;
				failTypes[failCount] = "内存泄漏";
				failFiles[failCount] = file;
				failExpect[failCount] = "-";
				failActual[failCount] = "-";
			}
			next;
		}
		END {
			print "## 统计";
			print "";
			print "| 组件 | 通过 | 总数 | 通过率 |";
			print "| --- | --- | --- | --- |";
			for (i = 1; i <= 3; i++) {
				lib = expected[i];
				if (lib in statSeen) {
					printf "| %s | %s | %s | %s |\n", lib, pass[lib], total[lib], pct(pass[lib], total[lib]);
				} else {
					printf "| %s | 缺失 | 缺失 | 缺失 |\n", lib;
				}
			}
			print "";

			print "## RFC 失败汇总";
			print "";
			totalFail = 0;
			for (lib in failByLib) { totalFail += failByLib[lib]; }
			if (totalFail == 0) {
				print "无";
			} else {
				print "| 组件 | 失败数 |";
				print "| --- | --- |";
				for (i = 1; i <= 3; i++) {
					lib = expected[i];
					printf "| %s | %d |\n", lib, (lib in failByLib) ? failByLib[lib] : 0;
				}
			}

			print "";
			print "## RFC 失败记录";
			print "";
			if (totalFail == 0) {
				print "无";
			} else {
				printf "总计 %d 条：\n", totalFail;
				print "";
				print "| 来源 | 类型 | 用例 | 期望 | 实际 |";
				print "| --- | --- | --- | --- | --- |";
				for (i = 1; i <= failCount; i++) {
					file = failFiles[i];
					gsub(/\|/, "\\\\|", file);
					printf "| %s | %s | %s | %s | %s |\n", failLibs[i], failTypes[i], file, failExpect[i], failActual[i];
				}
			}
		}
	' >> "${markdownPath}"
}

run_and_capture_stream() {
	local streamPath="$1"
	local stripAnsi="$2"
	shift 2
	local tmpRoot="${repoRoot}/localLogs/_tmp"
	local cmdRc=0

	mkdir -p "${tmpRoot}"
	rm -f "${streamPath}"

	if [[ "${stripAnsi}" == "1" ]]; then
		"$@" 2>&1 | ryanjson_strip_ansi | tee "${streamPath}"
	else
		"$@" 2>&1 | tee "${streamPath}"
	fi
	cmdRc=${PIPESTATUS[0]}

	return "${cmdRc}"
}

validate_inputs() {
	if [[ "${rfcPlatform}" != "host" && "${rfcPlatform}" != "qemu" && "${rfcPlatform}" != "both" ]]; then
		ryanjson_log_error "RFC_PLATFORM only supports host/qemu/both, current: ${rfcPlatform}"
		return 1
	fi
	if ! ryanjson_require_01 "RFC_STOP_ON_FAIL" "${rfcStopOnFail}"; then
		return 1
	fi
	if ! ryanjson_require_01 "RFC_STRIP_ANSI_LOG" "${rfcStripAnsiLog}"; then
		return 1
	fi
	if ! ryanjson_require_01 "RFC_SINGLE_CASE" "${rfcSingleCase}"; then
		return 1
	fi
	if [[ "${rfcSingleCase}" == "0" ]]; then
		if ! ryanjson_emit_semantic_cases "${rfcMode}" "RFC_MODE" >/dev/null; then
			return 1
		fi
	fi
}

print_banner() {
	ryanjson_print_banner_begin "Local RFC8259 Report"
	ryanjson_print_banner_kv "RFC_PLATFORM" "${rfcPlatform}"
	ryanjson_print_banner_kv "RFC_MODE" "${rfcMode}"
	ryanjson_print_banner_kv "RFC_STOP_ON_FAIL" "${rfcStopOnFail}"
	ryanjson_print_banner_kv "RFC_SINGLE_CASE" "${rfcSingleCase}"
	ryanjson_print_banner_kv_optional "RFC_DEFAULT_CASE" "${rfcDefaultCase}"
	ryanjson_print_banner_kv "RFC_STRIP_ANSI_LOG" "${rfcStripAnsiLog}"
	ryanjson_print_banner_end
}

run_host_rfc8259() {
	local markdownPath="${repoRoot}/rfc8259ReportHost.md"
	local streamPath="${repoRoot}/localLogs/_tmp/rfc8259_stream_host.$$"
	local rc=0
	local caseRc=0
	local strictKey=""
	local addAtHead=""
	local scientific=""

	read -r strictKey addAtHead scientific <<< "${rfcDefaultCase}"

	export RYANJSON_UNIT_ONLY_RFC8259=1
	export UNIT_MODE="${rfcMode}"
	export UNIT_SKIP_COV=1
	export UNIT_STOP_ON_FAIL="${rfcStopOnFail}"
	export XMAKE_FORCE_CLEAN="${XMAKE_FORCE_CLEAN:-0}"
	export UNIT_SYNC_ONLY="${UNIT_SYNC_ONLY:-0}"
	if [[ "${rfcSingleCase}" == "1" ]]; then
		export UNIT_SINGLE_CASE="${rfcDefaultCase}"
	else
		unset UNIT_SINGLE_CASE
	fi

	set +e
	run_and_capture_stream "${streamPath}" "${rfcStripAnsiLog}" \
		bash "${repoRoot}/run_local_base.sh"
	caseRc=$?
	set -e

	if [[ "${caseRc}" -ne 0 ]]; then
		rm -f "${streamPath}"
		rc="${caseRc}"
		ryanjson_log_error "RFC8259 执行失败，未生成报告。"
		return "${rc}"
	fi

	ryanjson_init_utf8_log "${markdownPath}"
	append_markdown_title "${markdownPath}" "Host"
	append_markdown_update_time "${markdownPath}"
	append_markdown_note "${markdownPath}" "Host" "${strictKey}" "${addAtHead}" "${scientific}"
	append_markdown_report_from_stream "${markdownPath}" < "${streamPath}"
	rm -f "${streamPath}"

	ryanjson_log_done "RFC8259 report (Markdown): ${markdownPath}"
	cat "${markdownPath}"
	return "${rc}"
}

run_qemu_rfc8259() {
	local markdownPath="${repoRoot}/rfc8259ReportQemu.md"
	local streamPath="${repoRoot}/localLogs/_tmp/rfc8259_stream_qemu.$$"
	local rc=0
	local caseRc=0
	local strictKey=""
	local addAtHead=""
	local scientific=""

	read -r strictKey addAtHead scientific <<< "${rfcDefaultCase}"

	export RYANJSON_UNIT_ONLY_RFC8259=1
	export QEMU_MODE="${rfcMode}"
	export QEMU_STOP_ON_FAIL="${rfcStopOnFail}"
	export QEMU_SAVE_LOG=0
	export QEMU_CONSOLE_LOG=1
	export QEMU_STRIP_ANSI_LOG="${rfcStripAnsiLog}"
	if [[ "${rfcSingleCase}" == "1" ]]; then
		export QEMU_SINGLE_CASE="${rfcDefaultCase}"
	else
		unset QEMU_SINGLE_CASE
	fi

	set +e
	run_and_capture_stream "${streamPath}" "${rfcStripAnsiLog}" \
		bash "${repoRoot}/run_local_qemu.sh"
	caseRc=$?
	set -e

	if [[ "${caseRc}" -ne 0 ]]; then
		rm -f "${streamPath}"
		rc="${caseRc}"
		ryanjson_log_error "RFC8259 执行失败，未生成报告。"
		return "${rc}"
	fi

	ryanjson_init_utf8_log "${markdownPath}"
	append_markdown_title "${markdownPath}" "QEMU"
	append_markdown_update_time "${markdownPath}"
	append_markdown_note "${markdownPath}" "QEMU" "${strictKey}" "${addAtHead}" "${scientific}"
	append_markdown_report_from_stream "${markdownPath}" < "${streamPath}"
	rm -f "${streamPath}"

	ryanjson_log_done "RFC8259 report (Markdown): ${markdownPath}"
	cat "${markdownPath}"
	return "${rc}"
}

main() {
	validate_inputs
	print_banner

	case "${rfcPlatform}" in
		host)
			run_host_rfc8259
			;;
		qemu)
			run_qemu_rfc8259
			;;
		both)
			local hostRc=0
			local qemuRc=0
			if run_host_rfc8259; then
				hostRc=0
			else
				hostRc=$?
			fi
			if run_qemu_rfc8259; then
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
