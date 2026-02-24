#!/bin/bash
set -euo pipefail

# 本地一键 clang-format。
# 默认行为：格式化仓库内受管源码（C/C++ 头源文件）。
# 可选参数：
#   --check    仅检查，不修改文件（不符合时返回非 0）
#   --changed  仅处理当前改动文件（默认处理全部受管源码）

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${scriptDir}"

mode="write"   # write | check
scope="all"    # all | changed

while (($# > 0)); do
	case "$1" in
	--check)
		mode="check"
		;;
	--changed)
		scope="changed"
		;;
	-h | --help)
		echo "用法: bash ./run_local_format.sh [--check] [--changed]"
		exit 0
		;;
	*)
		echo "[错误] 未知参数: $1"
		echo "用法: bash ./run_local_format.sh [--check] [--changed]"
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

allFiles=()
if [[ "${scope}" == "all" ]]; then
	mapfile -d '' allFiles < <(git ls-files -z -- '*.c' '*.h' '*.cc' '*.cpp' '*.hpp')
else
	mapfile -d '' allFiles < <(git diff --name-only -z -- '*.c' '*.h' '*.cc' '*.cpp' '*.hpp')
fi

files=()
for f in "${allFiles[@]}"; do
	case "${f}" in
	test/externalModule/* | build/* | coverage/* | .xmake/* | test/fuzzer/corpus/*) ;;
	*)
		files+=("${f}")
		;;
	esac
done

echo "===================================================="
echo "本地格式化启动"
echo "  - formatter=${formatter}"
echo "  - mode=${mode}"
echo "  - scope=${scope}"
echo "  - files=${#files[@]}"
echo "===================================================="

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

