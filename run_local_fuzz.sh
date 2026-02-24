#!/bin/bash
set -euo pipefail

# 本地一键 Fuzz（默认低内存并发，可通过环境变量覆盖）。
# 用途：封装常用 fuzz 参数，避免每次手工传 FUZZ_WORKERS/FUZZ_JOBS/FUZZ_RSS_LIMIT_MB。
# 默认行为：
#   workers/jobs 默认 2/2（可覆盖）
#   默认 rss_limit_mb=2048（可覆盖）
#   按轮次执行（默认 FUZZ_RUNS=10000000，不走 max_total_time）
#   默认模式 nightly，默认生成覆盖率
# 可覆盖参数：
#   FUZZ_MODE/FUZZ_SKIP_COV/FUZZ_RUNS/FUZZ_TIMEOUT/FUZZ_MAX_LEN
#   FUZZ_WORKERS/FUZZ_JOBS/FUZZ_RSS_LIMIT_MB/FUZZ_MALLOC_LIMIT_MB
# 三个语义宏默认值（可覆盖）：
#   RYANJSON_STRICT_OBJECT_KEY_CHECK=false
#   RYANJSON_DEFAULT_ADD_AT_HEAD=false
#   RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC=false

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${scriptDir}"

fuzzMode="${FUZZ_MODE:-nightly}"
fuzzSkipCov="${FUZZ_SKIP_COV:-0}"
fuzzRuns="${FUZZ_RUNS:-10000000}"
fuzzTimeout="${FUZZ_TIMEOUT:-4}"
fuzzMaxLen="${FUZZ_MAX_LEN:-8192}"
fuzzWorkers="${FUZZ_WORKERS:-3}"
fuzzJobs="${FUZZ_JOBS:-9}"
fuzzRssLimitMb="${FUZZ_RSS_LIMIT_MB:-4096}"
fuzzMallocLimitMb="${FUZZ_MALLOC_LIMIT_MB:-}"

export RYANJSON_STRICT_OBJECT_KEY_CHECK="${RYANJSON_STRICT_OBJECT_KEY_CHECK:-false}"
export RYANJSON_DEFAULT_ADD_AT_HEAD="${RYANJSON_DEFAULT_ADD_AT_HEAD:-false}"
export RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC="${RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC:-false}"

echo "===================================================="
echo "本地 Fuzz 启动"
echo "  - FUZZ_MODE=${fuzzMode}"
echo "  - FUZZ_SKIP_COV=${fuzzSkipCov}"
echo "  - FUZZ_RUNS=${fuzzRuns}"
echo "  - FUZZ_TIMEOUT=${fuzzTimeout}"
echo "  - FUZZ_MAX_LEN=${fuzzMaxLen}"
echo "  - FUZZ_WORKERS=${fuzzWorkers}"
echo "  - FUZZ_JOBS=${fuzzJobs}"
if [[ -n "${fuzzRssLimitMb}" ]]; then
  echo "  - FUZZ_RSS_LIMIT_MB=${fuzzRssLimitMb}"
fi
if [[ -n "${fuzzMallocLimitMb}" ]]; then
  echo "  - FUZZ_MALLOC_LIMIT_MB=${fuzzMallocLimitMb}"
fi
echo "  - RYANJSON_STRICT_OBJECT_KEY_CHECK=${RYANJSON_STRICT_OBJECT_KEY_CHECK}"
echo "  - RYANJSON_DEFAULT_ADD_AT_HEAD=${RYANJSON_DEFAULT_ADD_AT_HEAD}"
echo "  - RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC=${RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC}"
echo "===================================================="

FUZZ_MODE="${fuzzMode}" \
FUZZ_SKIP_COV="${fuzzSkipCov}" \
FUZZ_RUNS="${fuzzRuns}" \
FUZZ_TIMEOUT="${fuzzTimeout}" \
FUZZ_MAX_LEN="${fuzzMaxLen}" \
FUZZ_WORKERS="${fuzzWorkers}" \
FUZZ_JOBS="${fuzzJobs}" \
FUZZ_RSS_LIMIT_MB="${fuzzRssLimitMb}" \
FUZZ_MALLOC_LIMIT_MB="${fuzzMallocLimitMb}" \
bash ./scripts/ci/runCoverage.sh
