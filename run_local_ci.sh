#!/bin/bash
set -euo pipefail

# 本地一键 CI（模拟 ci-pr）。
# 执行顺序：
#   先跑 full 单元矩阵（8 组），再跑 quick fuzz（1 组默认语义）。
# 默认参数与 ci-pr.yml 对齐：
#   unit: UNIT_MODE=full, UNIT_SKIP_COV=1
#   fuzz: FUZZ_MODE=quick, FUZZ_SKIP_COV=1, FUZZ_MAX_TOTAL_TIME=45, workers/jobs=2
# fuzz 阶段参数可用同名环境变量临时覆盖。

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${scriptDir}"

echo "===================================================="
echo "本地 CI 启动：阶段 1/2 -> Base 单元测试"
echo "===================================================="
bash ./run_local_base.sh

echo "===================================================="
echo "本地 CI 启动：阶段 2/2 -> Fuzz quick"
echo "===================================================="

RYANJSON_STRICT_OBJECT_KEY_CHECK="${RYANJSON_STRICT_OBJECT_KEY_CHECK:-false}" \
RYANJSON_DEFAULT_ADD_AT_HEAD="${RYANJSON_DEFAULT_ADD_AT_HEAD:-true}" \
RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC="${RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC:-true}" \
FUZZ_MODE="${FUZZ_MODE:-quick}" \
FUZZ_SKIP_COV="${FUZZ_SKIP_COV:-1}" \
FUZZ_MAX_TOTAL_TIME="${FUZZ_MAX_TOTAL_TIME:-45}" \
FUZZ_WORKERS="${FUZZ_WORKERS:-2}" \
FUZZ_JOBS="${FUZZ_JOBS:-2}" \
XMAKE_FORCE_CLEAN="${XMAKE_FORCE_CLEAN:-0}" \
bash ./scripts/ci/runCoverage.sh

echo "===================================================="
echo "本地 CI 执行完成"
echo "===================================================="
