#!/bin/bash
set -euo pipefail

# 本地一键 clang-format。
# 默认行为：格式化仓库内受管源码（C/C++ 头源文件，含未追踪新增文件）。
# 可选参数：
#   --check    仅检查，不修改文件（不符合时返回非 0）

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib/common.sh
source "${scriptDir}/scripts/lib/common.sh"
repoRoot="$(ryanjson_repo_root_from_source "${BASH_SOURCE[0]}" 0)"
cd "${repoRoot}"

ryanjson_detect_clang_format() {
	# 选择可用的 clang-format 可执行文件
	if command -v clang-format >/dev/null 2>&1; then
		echo "clang-format"
		return 0
	fi
	if command -v clang-format-21 >/dev/null 2>&1; then
		echo "clang-format-21"
		return 0
	fi
	if command -v clang-format-20 >/dev/null 2>&1; then
		echo "clang-format-20"
		return 0
	fi

	echo ""
	return 1
}

ryanjson_collect_format_files() {
	# 收集受管源文件，过滤第三方与构建产物
	local -a files=()
	local -a candidates=()

	mapfile -d '' candidates < <(git ls-files -z --cached --others --exclude-standard -- '*.c' '*.h' '*.cc' '*.cpp' '*.hpp')
	for f in "${candidates[@]}"; do
		if [[ ! -f "${f}" ]]; then
			continue
		fi
		case "${f}" in
		test/externalModule/* | build/* | coverage/* | localLogs/* | .xmake/* | test/fuzzer/corpus/*)
			continue
			;;
		esac
		files+=("${f}")
	done

	printf '%s\n' "${files[@]}"
}

ryanjson_format_main() {
	# 参数解析：--check / --help
	local mode="write"

	while (($# > 0)); do
		case "$1" in
		--check)
			mode="check"
			;;
		-h | --help)
			echo "用法: bash ./run_local_format.sh [--check]"
			return 0
			;;
		*)
			ryanjson_log_error "未知参数: $1"
			echo "用法: bash ./run_local_format.sh [--check]"
			return 2
			;;
		esac
		shift
	done

	# 选择格式化工具
	local formatter=""
	if ! formatter="$(ryanjson_detect_clang_format)"; then
		ryanjson_log_error "未找到 clang-format，可执行文件名尝试过: clang-format / clang-format-21 / clang-format-20"
		return 127
	fi

	# 收集文件列表
	local -a files=()
	mapfile -t files < <(ryanjson_collect_format_files)

	ryanjson_print_banner_begin "本地格式化启动"
	ryanjson_print_banner_kv "formatter" "${formatter}"
	ryanjson_print_banner_kv "mode" "${mode}"
	ryanjson_print_banner_kv "files" "${#files[@]}"
	ryanjson_print_banner_end

	# 无文件时直接返回
	if [[ ${#files[@]} -eq 0 ]]; then
		ryanjson_log_info "没有可处理的源码文件"
		return 0
	fi

	# 根据模式执行
	if [[ "${mode}" == "check" ]]; then
		if printf '%s\0' "${files[@]}" | xargs -0 "${formatter}" --dry-run --Werror; then
			ryanjson_log_done "clang-format 检查通过"
		else
			ryanjson_log_error "存在不符合 .clang-format 的文件，请执行: bash ./run_local_format.sh"
			return 1
		fi
	else
		printf '%s\0' "${files[@]}" | xargs -0 "${formatter}" -i
		ryanjson_log_done "clang-format 已应用到 ${#files[@]} 个文件"
	fi
}

main() {
	ryanjson_format_main "$@"
}

main "$@"
