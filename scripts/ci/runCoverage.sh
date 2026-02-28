#!/bin/bash
set -euo pipefail

# Fuzzer 入口脚本（Linux）。
# 脚本路径：scripts/ci/runCoverage.sh
# 执行模式：FUZZ_MODE=quick|nightly|full
#   quick: PR 快检（默认 60s）
#   nightly: 夜间巡检（默认 300s）
#   full: 全量压测（默认 900s）
# 常用参数：
#   FUZZ_SKIP_COV=0|1：是否跳过覆盖率
#   FUZZ_RUNS=<N>：固定迭代次数（优先于 max_total_time）
#   FUZZ_MAX_TOTAL_TIME=<秒>：总时间预算
#   FUZZ_WORKERS / FUZZ_JOBS：并行 worker/job 数
#   FUZZ_TIMEOUT / FUZZ_MAX_LEN / FUZZ_VERBOSITY：透传给 libFuzzer
#   FUZZ_RSS_LIMIT_MB / FUZZ_MALLOC_LIMIT_MB：内存预算（透传给 libFuzzer）
#   FUZZ_CORPUS_DIR / FUZZ_DICT_PATH：corpus 与字典路径
#   FUZZ_EXTRA_ARGS：附加 libFuzzer 参数（空格分割）
#   XMAKE_FORCE_CLEAN=0|1：是否在配置前先清理

fuzzMode="${FUZZ_MODE:-quick}"
fuzzSkipCov="${FUZZ_SKIP_COV:-0}"
fuzzTimeout="${FUZZ_TIMEOUT:-4}"
fuzzMaxLen="${FUZZ_MAX_LEN:-8192}"
fuzzVerbosity="${FUZZ_VERBOSITY:-0}"
fuzzCorpusDir="${FUZZ_CORPUS_DIR:-./test/fuzzer/corpus}"
fuzzDictPath="${FUZZ_DICT_PATH:-./test/fuzzer/RyanJsonFuzzer.dict}"
fuzzRuns="${FUZZ_RUNS:-}"
fuzzMaxTotalTime="${FUZZ_MAX_TOTAL_TIME:-}"
fuzzWorkers="${FUZZ_WORKERS:-}"
fuzzJobs="${FUZZ_JOBS:-}"
fuzzRssLimitMb="${FUZZ_RSS_LIMIT_MB:-}"
fuzzMallocLimitMb="${FUZZ_MALLOC_LIMIT_MB:-}"
xmakeForceClean="${XMAKE_FORCE_CLEAN:-0}"

# 统一切到仓库根目录，避免从任意 cwd 启动时相对路径失效
scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repoRoot="$(cd "${scriptDir}/../.." && pwd)"
cd "${repoRoot}"

# 按模式设置默认预算（可被环境变量覆盖）
case "${fuzzMode}" in
  quick)
    defaultMaxTotalTime=60
    defaultWorkers=2
    defaultJobs=2
    ;;
  nightly)
    defaultMaxTotalTime=300
    defaultWorkers=4
    defaultJobs=4
    ;;
  full)
    defaultMaxTotalTime=900
    defaultWorkers=6
    defaultJobs=6
    ;;
  *)
    echo "[错误] FUZZ_MODE 仅支持 quick/nightly/full，当前值：${fuzzMode}"
    exit 1
    ;;
esac

# 如果用户没显式指定并行参数，则使用模式默认值
if [[ -z "${fuzzWorkers}" ]]; then
  fuzzWorkers="${defaultWorkers}"
fi
if [[ -z "${fuzzJobs}" ]]; then
  fuzzJobs="${defaultJobs}"
fi

# 如果没设置 FUZZ_RUNS，则走 max_total_time 模式
if [[ -z "${fuzzRuns}" ]]; then
  if [[ -z "${fuzzMaxTotalTime}" ]]; then
    fuzzMaxTotalTime="${defaultMaxTotalTime}"
  fi
fi

strictKey="${RYANJSON_STRICT_OBJECT_KEY_CHECK:-false}"
addAtHead="${RYANJSON_DEFAULT_ADD_AT_HEAD:-true}"
scientific="${RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC:-true}"
caseName="strict_${strictKey}__head_${addAtHead}__sci_${scientific}"

# 覆盖率目录固定为 coverage/fuzz，每次执行前清理，保证只保留最新结果
coverageRoot="test/fuzzer/coverage"
rm -rf "${coverageRoot}"
profileRoot="${coverageRoot}/profiles"
mkdir -p "${profileRoot}"

profraw="${profileRoot}/coverage.profraw"
profdata="${coverageRoot}/coverage.profdata"
reportTxt="${coverageRoot}/report.txt"
reportHtml="${coverageRoot}/html"

echo "===================================================="
echo "Fuzzer 执行配置"
echo "  - 模式: ${fuzzMode}"
echo "  - 配置: ${caseName}"
echo "  - Corpus: ${fuzzCorpusDir}"
echo "  - 字典: ${fuzzDictPath}"
echo "  - timeout: ${fuzzTimeout}"
echo "  - max_len: ${fuzzMaxLen}"
echo "  - workers/jobs: ${fuzzWorkers}/${fuzzJobs}"
if [[ -n "${fuzzRssLimitMb}" ]]; then
  echo "  - rss_limit_mb: ${fuzzRssLimitMb}"
fi
if [[ -n "${fuzzMallocLimitMb}" ]]; then
  echo "  - malloc_limit_mb: ${fuzzMallocLimitMb}"
fi
if [[ -n "${fuzzRuns}" ]]; then
  echo "  - runs: ${fuzzRuns}"
else
  echo "  - max_total_time: ${fuzzMaxTotalTime}s"
fi
echo "===================================================="

# 重新配置，确保宏变化进入编译命令
if [[ "${xmakeForceClean}" == "1" ]]; then
  echo "[阶段] 正在执行 xmake 配置（clean 模式）..."
  xmake f -c
else
  echo "[阶段] 正在执行 xmake 配置（增量模式）..."
  xmake f
fi
echo "[阶段] 正在执行 xmake 构建（target=RyanJsonFuzz）..."
xmake -b RyanJsonFuzz
echo "[信息] xmake 构建完成（target=RyanJsonFuzz）"

# corpus 不存在时自动创建，方便首次运行
if [[ ! -d "${fuzzCorpusDir}" ]]; then
  mkdir -p "${fuzzCorpusDir}"
  echo "[信息] Corpus 目录不存在，已自动创建：${fuzzCorpusDir}"
fi

# 字典文件可选：存在就启用，不存在就跳过
declare -a dictArgs=()
if [[ -f "${fuzzDictPath}" ]]; then
  dictArgs+=("-dict=${fuzzDictPath}")
else
  echo "[警告] 未找到字典文件 ${fuzzDictPath}，已跳过 -dict 参数。"
fi

# 组装 libFuzzer 参数数组，避免字符串拼接导致转义问题
declare -a fuzzArgs=()
fuzzArgs+=("${fuzzCorpusDir}")
fuzzArgs+=("${dictArgs[@]}")
fuzzArgs+=("-timeout=${fuzzTimeout}")
fuzzArgs+=("-verbosity=${fuzzVerbosity}")
fuzzArgs+=("-max_len=${fuzzMaxLen}")
fuzzArgs+=("-workers=${fuzzWorkers}")
fuzzArgs+=("-jobs=${fuzzJobs}")
if [[ -n "${fuzzRssLimitMb}" ]]; then
  fuzzArgs+=("-rss_limit_mb=${fuzzRssLimitMb}")
fi
if [[ -n "${fuzzMallocLimitMb}" ]]; then
  fuzzArgs+=("-malloc_limit_mb=${fuzzMallocLimitMb}")
fi

if [[ -n "${fuzzRuns}" ]]; then
  fuzzArgs+=("-runs=${fuzzRuns}")
else
  fuzzArgs+=("-max_total_time=${fuzzMaxTotalTime}")
fi

# 允许 CI 透传少量临时参数，例如 -rss_limit_mb=4096
if [[ -n "${FUZZ_EXTRA_ARGS:-}" ]]; then
  # shellcheck disable=SC2206
  extraArgs=( ${FUZZ_EXTRA_ARGS} )
  fuzzArgs+=("${extraArgs[@]}")
fi

# 运行 fuzz。使用独立 profile 文件，避免多次执行互相覆盖
echo "[阶段] 正在运行 fuzz 二进制..."
LLVM_PROFILE_FILE="${profraw}" ./build/linux/x86/release/RyanJsonFuzz "${fuzzArgs[@]}"

# 快检模式可跳过覆盖率阶段以缩短总时长
if [[ "${fuzzSkipCov}" == "1" ]]; then
  echo "[信息] FUZZ_SKIP_COV=1，已跳过覆盖率生成。"
  echo "输出目录：${coverageRoot}"
  exit 0
fi

# 覆盖率工具检查：未安装时给出明确错误
if ! command -v llvm-profdata >/dev/null 2>&1; then
  echo "[错误] 未找到 llvm-profdata，无法生成覆盖率。"
  exit 1
fi
if ! command -v llvm-cov >/dev/null 2>&1; then
  echo "[错误] 未找到 llvm-cov，无法生成覆盖率。"
  exit 1
fi

# 合并 profile 数据
llvm-profdata merge -sparse "${profraw}" -o "${profdata}"

# 文本汇总覆盖率：
# 先原样打印到终端（尽量保留颜色）
# 再单独写入文件，便于归档
echo "---------------- 覆盖率摘要（llvm-cov report） ----------------"
llvm-cov report ./build/linux/x86/release/RyanJsonFuzz \
  -instr-profile="${profdata}" \
  -show-mcdc-summary \
  --use-color \
  -sources ./RyanJson

llvm-cov report ./build/linux/x86/release/RyanJsonFuzz \
  -instr-profile="${profdata}" \
  -show-mcdc-summary \
  -sources ./RyanJson > "${reportTxt}"

# HTML 详细覆盖率（用于本地/制品追踪）
llvm-cov show ./build/linux/x86/release/RyanJsonFuzz \
  -instr-profile="${profdata}" \
  -format=html \
  -output-dir="${reportHtml}" \
  -show-mcdc-summary \
  -show-branches=count \
  -show-expansions \
  -show-regions \
  -show-line-counts-or-regions \
  -sources ./RyanJson

echo "[完成] Fuzzer 执行结束，覆盖率已生成。"
echo "输出目录：${coverageRoot}"
echo "覆盖率文本报告：${reportTxt}"
echo "覆盖率HTML目录：${reportHtml}"
