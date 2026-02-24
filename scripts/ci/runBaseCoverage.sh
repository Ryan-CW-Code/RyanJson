#!/bin/bash
set -euo pipefail

# 单元测试配置矩阵入口（Linux）。
# 脚本路径：scripts/ci/runBaseCoverage.sh
# 执行模式：UNIT_MODE=quick|nightly|full
#   quick: 2 组（PR 快检）
#   nightly: 4 组（strict × addAtHead，scientific 固定 true）
#   full: 8 组（三个布尔宏全组合）
# 常用参数：
#   UNIT_SKIP_COV=0|1：是否跳过覆盖率
#   UNIT_STOP_ON_FAIL=0|1：失败是否立即终止
#   XMAKE_FORCE_CLEAN=0|1：每组前是否先清理配置

unitMode="${UNIT_MODE:-full}"
unitSkipCov="${UNIT_SKIP_COV:-0}"
unitStopOnFail="${UNIT_STOP_ON_FAIL:-1}"
xmakeForceClean="${XMAKE_FORCE_CLEAN:-0}"

# 统一切到仓库根目录，避免从任意 cwd 启动时相对路径失效
scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repoRoot="$(cd "${scriptDir}/../.." && pwd)"
cd "${repoRoot}"

# 覆盖率目录固定为 coverage/unitMatrix，每次执行前清理，保证只保留最新结果
coverageRoot="coverage/unitMatrix"
rm -rf "${coverageRoot}"
profileRoot="${coverageRoot}/profiles"
mkdir -p "${profileRoot}"

declare -a caseList=()

addCase() {
  local strictKey="$1"
  local addAtHead="$2"
  local scientific="$3"
  caseList+=("${strictKey} ${addAtHead} ${scientific}")
}

# 根据模式生成组合列表
case "${unitMode}" in
  quick)
    # PR 快检：默认组合 + 对立组合
    addCase false true true
    addCase true false true
    ;;
  nightly)
    # 夜间：覆盖 strict × addAtHead 四种核心语义
    for strictKey in false true; do
      for addAtHead in false true; do
        addCase "${strictKey}" "${addAtHead}" true
      done
    done
    ;;
  full)
    # 全量：三个布尔宏全组合
    for strictKey in false true; do
      for addAtHead in false true; do
        for scientific in false true; do
          addCase "${strictKey}" "${addAtHead}" "${scientific}"
        done
      done
    done
    ;;
  *)
    echo "[错误] UNIT_MODE 仅支持 quick/nightly/full，当前值：${unitMode}"
    exit 1
    ;;
esac

totalCases="${#caseList[@]}"
caseIndex=0
failedCases=0

runCase() {
  local index="$1"
  local total="$2"
  local strictKey="$3"
  local addAtHead="$4"
  local scientific="$5"

  local caseName="strict_${strictKey}__head_${addAtHead}__sci_${scientific}"
  local profraw="${profileRoot}/${caseName}.profraw"

  echo "===================================================="
  echo "【用例 ${index}/${total}】${caseName}"
  echo "  - RyanJsonStrictObjectKeyCheck=${strictKey}"
  echo "  - RyanJsonDefaultAddAtHead=${addAtHead}"
  echo "  - RyanJsonSnprintfSupportScientific=${scientific}"
  echo "===================================================="

  export RYANJSON_STRICT_OBJECT_KEY_CHECK="${strictKey}"
  export RYANJSON_DEFAULT_ADD_AT_HEAD="${addAtHead}"
  export RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC="${scientific}"

  # 重新配置，确保宏变化进入编译命令
  # 默认走增量配置，配合第三方静态库可减少重复编译
  if [[ "${xmakeForceClean}" == "1" ]]; then
    echo "[阶段] 正在执行 xmake 配置（clean 模式）..."
    if ! xmake f -c; then
      echo "[错误] xmake 配置失败：${caseName}"
      return 1
    fi
  else
    echo "[阶段] 正在执行 xmake 配置（增量模式）..."
    if ! xmake f; then
      echo "[错误] xmake 配置失败：${caseName}"
      return 1
    fi
  fi

  echo "[阶段] 正在执行 xmake 构建（target=RyanJson）..."
  if ! xmake -b RyanJson; then
    echo "[错误] xmake 构建失败：${caseName}"
    return 1
  fi

  # 单测执行，profile 分文件隔离，避免组合间互相覆盖
  echo "[阶段] 正在运行单元测试二进制..."
  if ! LLVM_PROFILE_FILE="${profraw}" ./build/linux/x86/release/RyanJson; then
    echo "[错误] 单元测试执行失败：${caseName}"
    return 1
  fi

  # 快检模式可跳过覆盖率阶段以缩短总时长
  if [[ "${unitSkipCov}" == "1" ]]; then
    echo "[信息] UNIT_SKIP_COV=1，已跳过覆盖率生成。"
    return 0
  fi
}

# 按组合清单执行
for entry in "${caseList[@]}"; do
  caseIndex=$((caseIndex + 1))
  read -r strictKey addAtHead scientific <<< "${entry}"

  if runCase "${caseIndex}" "${totalCases}" "${strictKey}" "${addAtHead}" "${scientific}"; then
    :
  else
    failedCases=$((failedCases + 1))
    if [[ "${unitStopOnFail}" == "1" ]]; then
      echo
      echo "[错误] 用例 ${caseIndex}/${totalCases} 失败，已按 UNIT_STOP_ON_FAIL=1 提前终止。"
      exit 1
    fi
  fi
done

# 若启用覆盖率，则把矩阵中所有组合的 profraw 合并后只生成一份报告
if [[ "${unitSkipCov}" != "1" ]]; then
  if ! command -v llvm-profdata >/dev/null 2>&1; then
    echo "[错误] 未找到 llvm-profdata，无法生成覆盖率。"
    exit 1
  fi
  if ! command -v llvm-cov >/dev/null 2>&1; then
    echo "[错误] 未找到 llvm-cov，无法生成覆盖率。"
    exit 1
  fi

  shopt -s nullglob
  profrawFiles=("${profileRoot}"/*.profraw)
  shopt -u nullglob
  if [[ "${#profrawFiles[@]}" -eq 0 ]]; then
    echo "[错误] 未找到可合并的 profraw 文件。"
    exit 1
  fi

  mergedProfdata="${coverageRoot}/coverage.profdata"
  reportTxt="${coverageRoot}/report.txt"
  reportHtml="${coverageRoot}/html"

  if ! llvm-profdata merge -sparse "${profrawFiles[@]}" -o "${mergedProfdata}"; then
    echo "[错误] profraw 合并失败。"
    exit 1
  fi

  if ! llvm-cov report ./build/linux/x86/release/RyanJson \
      -instr-profile="${mergedProfdata}" \
      -show-mcdc-summary \
      -sources ./RyanJson > "${reportTxt}"; then
    echo "[错误] 文本覆盖率生成失败。"
    exit 1
  fi

  if ! llvm-cov show ./build/linux/x86/release/RyanJson \
      -instr-profile="${mergedProfdata}" \
      -format=html \
      -output-dir="${reportHtml}" \
      -show-mcdc-summary \
      -show-branches=count \
      -show-expansions \
      -show-regions \
      -show-line-counts-or-regions \
      -sources ./RyanJson; then
    echo "[错误] HTML 覆盖率生成失败。"
    exit 1
  fi
fi

echo
echo "单元测试矩阵执行完成。"
echo "执行模式：${unitMode}"
echo "总用例数：${totalCases}"
echo "失败用例数：${failedCases}"
echo "覆盖率输出目录：${coverageRoot}"
if [[ "${unitSkipCov}" != "1" ]]; then
  echo "覆盖率文本报告：${coverageRoot}/report.txt"
  echo "覆盖率HTML目录：${coverageRoot}/html"
fi

if [[ "${failedCases}" -gt 0 ]]; then
  exit 1
fi
