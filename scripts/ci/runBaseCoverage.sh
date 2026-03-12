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

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/common.sh
source "${scriptDir}/../lib/common.sh"
# shellcheck source=../lib/semantic_matrix.sh
source "${scriptDir}/../lib/semantic_matrix.sh"
repoRoot="$(ryanjson_repo_root_from_source "${BASH_SOURCE[0]}" 2)"
cd "${repoRoot}"

unitMode="${UNIT_MODE:-full}"
unitSkipCov="${UNIT_SKIP_COV:-0}"
unitStopOnFail="${UNIT_STOP_ON_FAIL:-1}"
xmakeForceClean="${XMAKE_FORCE_CLEAN:-0}"

ryanjson_require_cmd xmake

if [[ -f "./scripts/ci/checkUnityRunnerList.sh" ]]; then
  echo "[阶段] 正在校验 Unity runner 列表..."
  bash ./scripts/ci/checkUnityRunnerList.sh
fi

coverageRoot="coverage/unitMatrix"
profileRoot="${coverageRoot}/profiles"
ryanjson_prepare_clean_dir "${coverageRoot}"
mkdir -p "${profileRoot}"

caseText="$(ryanjson_emit_semantic_cases "${unitMode}" "UNIT_MODE")"
mapfile -t caseList <<< "${caseText}"

totalCases="${#caseList[@]}"
caseIndex=0
failedCases=0

runCase() {
  local index="$1"
  local total="$2"
  local strictKey="$3"
  local addAtHead="$4"
  local scientific="$5"

  local caseName=""
  local profraw=""
  caseName="$(ryanjson_semantic_case_name "${strictKey}" "${addAtHead}" "${scientific}")"
  profraw="${profileRoot}/${caseName}.profraw"

  echo "===================================================="
  echo "【用例 ${index}/${total}】${caseName}"
  echo "  - RyanJsonStrictObjectKeyCheck=${strictKey}"
  echo "  - RyanJsonDefaultAddAtHead=${addAtHead}"
  echo "  - RyanJsonSnprintfSupportScientific=${scientific}"
  echo "===================================================="

  export RYANJSON_STRICT_OBJECT_KEY_CHECK="${strictKey}"
  export RYANJSON_DEFAULT_ADD_AT_HEAD="${addAtHead}"
  export RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC="${scientific}"

  if ! ryanjson_run_xmake_config "${xmakeForceClean}" "${caseName}"; then
    return 1
  fi
  if ! ryanjson_run_xmake_build "RyanJson" "${caseName}"; then
    return 1
  fi

  echo "[阶段] 正在运行单元测试二进制..."
  if ! LLVM_PROFILE_FILE="${profraw}" ./build/linux/x86/release/RyanJson; then
    echo "[错误] 单元测试执行失败：${caseName}"
    return 1
  fi

  if [[ "${unitSkipCov}" == "1" ]]; then
    echo "[信息] UNIT_SKIP_COV=1，已跳过覆盖率生成。"
    return 0
  fi
}

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
