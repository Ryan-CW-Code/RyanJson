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
cd "${scriptDir}"

unitMode="${UNIT_MODE:-full}"
unitSkipCov="${UNIT_SKIP_COV:-1}"
unitStopOnFail="${UNIT_STOP_ON_FAIL:-1}"
xmakeForceClean="${XMAKE_FORCE_CLEAN:-0}"

echo "===================================================="
echo "本地 Base 启动（单元测试矩阵）"
echo "  - UNIT_MODE=${unitMode}"
echo "  - UNIT_SKIP_COV=${unitSkipCov}"
echo "  - UNIT_STOP_ON_FAIL=${unitStopOnFail}"
echo "  - XMAKE_FORCE_CLEAN=${xmakeForceClean}"
echo "===================================================="

UNIT_MODE="${unitMode}" \
UNIT_SKIP_COV="${unitSkipCov}" \
UNIT_STOP_ON_FAIL="${unitStopOnFail}" \
XMAKE_FORCE_CLEAN="${xmakeForceClean}" \
bash ./scripts/ci/runBaseCoverage.sh
