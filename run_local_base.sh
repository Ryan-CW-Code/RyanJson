#!/bin/bash
set -euo pipefail

# 本地一键 Base（单元测试矩阵）。
# 用途：直接调用 scripts/ci/runBaseCoverage.sh，免去手工拼参数。
# 默认值：
#   UNIT_MODE=full（8 组配置全覆盖）
#   UNIT_SKIP_COV=1（跳过覆盖率，提速）
#   UNIT_STOP_ON_FAIL=1（首个失败立即退出）
#   XMAKE_FORCE_CLEAN=0（增量配置，减少重编译）
# 以上参数都可用同名环境变量临时覆盖。

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib/common.sh
source "${scriptDir}/scripts/lib/common.sh"
repoRoot="$(ryanjson_repo_root_from_source "${BASH_SOURCE[0]}" 0)"
cd "${repoRoot}"

unitMode="${UNIT_MODE:-full}"
unitSkipCov="${UNIT_SKIP_COV:-1}"
unitStopOnFail="${UNIT_STOP_ON_FAIL:-1}"
xmakeForceClean="${XMAKE_FORCE_CLEAN:-0}"

ryanjson_print_banner_begin "本地 Base 启动（单元测试矩阵）"
ryanjson_print_banner_kv "UNIT_MODE" "${unitMode}"
ryanjson_print_banner_kv "UNIT_SKIP_COV" "${unitSkipCov}"
ryanjson_print_banner_kv "UNIT_STOP_ON_FAIL" "${unitStopOnFail}"
ryanjson_print_banner_kv "XMAKE_FORCE_CLEAN" "${xmakeForceClean}"
ryanjson_print_banner_end

UNIT_MODE="${unitMode}" \
UNIT_SKIP_COV="${unitSkipCov}" \
UNIT_STOP_ON_FAIL="${unitStopOnFail}" \
XMAKE_FORCE_CLEAN="${xmakeForceClean}" \
bash ./scripts/ci/runBaseCoverage.sh
