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

mode="write"

while (($# > 0)); do
	case "$1" in
	--check)
		mode="check"
		;;
	-h | --help)
		echo "用法: bash ./run_local_format.sh [--check]"
		exit 0
		;;
	*)
		echo "[错误] 未知参数: $1"
		echo "用法: bash ./run_local_format.sh [--check]"
		exit 2
		;;
	esac
	shift
done

if command -v clang-format >/dev/null 2>&1; then
	formatter="clang-format"
elif command -v clang-format-21 >/dev/null 2>&1; then
	formatter="clang-format-21"
elif command -v clang-format-20 >/dev/null 2>&1; then
	formatter="clang-format-20"
else
	echo "[错误] 未找到 clang-format，可执行文件名尝试过: clang-format / clang-format-21 / clang-format-20"
	exit 127
fi

declare -a files=()
mapfile -d '' candidates < <(git ls-files -z --cached --others --exclude-standard -- '*.c' '*.h' '*.cc' '*.cpp' '*.hpp')
for f in "${candidates[@]}"; do
	if [[ ! -f "${f}" ]]; then
		continue
	fi
	case "${f}" in
	test/externalModule/* | build/* | coverage/* | .xmake/* | test/fuzzer/corpus/*)
		continue
		;;
	esac
	files+=("${f}")
done

ryanjson_print_banner_begin "本地格式化启动"
ryanjson_print_banner_kv "formatter" "${formatter}"
ryanjson_print_banner_kv "mode" "${mode}"
ryanjson_print_banner_kv "files" "${#files[@]}"
ryanjson_print_banner_end

if [[ ${#files[@]} -eq 0 ]]; then
	echo "[信息] 没有可处理的源码文件"
	exit 0
fi

if [[ "${mode}" == "check" ]]; then
	if printf '%s\0' "${files[@]}" | xargs -0 "${formatter}" --dry-run --Werror; then
		echo "[完成] clang-format 检查通过"
	else
		echo "[失败] 存在不符合 .clang-format 的文件，请执行: bash ./run_local_format.sh"
		exit 1
	fi
else
	printf '%s\0' "${files[@]}" | xargs -0 "${formatter}" -i
	echo "[完成] clang-format 已应用到 ${#files[@]} 个文件"
fi
