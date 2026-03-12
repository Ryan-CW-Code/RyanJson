#!/bin/bash
set -euo pipefail

# Fuzzer 入口脚本（Linux）。
# 脚本路径：scripts/ci/runCoverage.sh
# 执行模式：FUZZ_MODE=quick|nightly|full
#   quick/nightly/full：仅决定默认 workers/jobs（2/2、4/4、6/6）
# 常用参数：
#   FUZZ_SKIP_COV=0|1：是否跳过覆盖率
#   FUZZ_RUNS=<N>：固定迭代次数；默认 100000
#   FUZZ_MAX_TOTAL_TIME=<秒>：显式总时间预算；仅在未设置 FUZZ_RUNS 时生效
#   FUZZ_WORKERS / FUZZ_JOBS：并行 worker/job 数
#   FUZZ_TIMEOUT / FUZZ_MAX_LEN / FUZZ_VERBOSITY：透传给 libFuzzer
#   FUZZ_RSS_LIMIT_MB / FUZZ_MALLOC_LIMIT_MB：内存预算（透传给 libFuzzer）
#   FUZZ_CORPUS_DIR / FUZZ_DICT_PATH：corpus 与字典路径
#   FUZZ_ENABLE_MERGE=0|1：是否先将输入 corpus merge 到临时目录，再回写原 corpus 目录
#   FUZZ_EXTRA_ARGS：附加 libFuzzer 参数（空格分割）
#   XMAKE_FORCE_CLEAN=0|1：是否在配置前先清理

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/common.sh
source "${scriptDir}/../lib/common.sh"
# shellcheck source=../lib/semantic_matrix.sh
source "${scriptDir}/../lib/semantic_matrix.sh"
repoRoot="$(ryanjson_repo_root_from_source "${BASH_SOURCE[0]}" 2)"
cd "${repoRoot}"

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
fuzzEnableMerge="${FUZZ_ENABLE_MERGE:-0}"
xmakeForceClean="${XMAKE_FORCE_CLEAN:-0}"
defaultRuns=100000

ryanjson_require_cmd xmake

case "${fuzzMode}" in
  quick)
    defaultWorkers=2
    defaultJobs=2
    ;;
  nightly)
    defaultWorkers=4
    defaultJobs=4
    ;;
  full)
    defaultWorkers=6
    defaultJobs=6
    ;;
  *)
    echo "[错误] FUZZ_MODE 仅支持 quick/nightly/full，当前值：${fuzzMode}"
    exit 1
    ;;
esac

case "${fuzzEnableMerge}" in
  0|1)
    ;;
  *)
    echo "[错误] FUZZ_ENABLE_MERGE 仅支持 0/1，当前值：${fuzzEnableMerge}"
    exit 1
    ;;
esac

if [[ -z "${fuzzWorkers}" ]]; then
  fuzzWorkers="${defaultWorkers}"
fi
if [[ -z "${fuzzJobs}" ]]; then
  fuzzJobs="${defaultJobs}"
fi
if [[ -z "${fuzzRuns}" && -z "${fuzzMaxTotalTime}" ]]; then
  fuzzRuns="${defaultRuns}"
fi

strictKey="${RYANJSON_STRICT_OBJECT_KEY_CHECK:-false}"
addAtHead="${RYANJSON_DEFAULT_ADD_AT_HEAD:-true}"
scientific="${RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC:-true}"
caseName="$(ryanjson_semantic_case_name "${strictKey}" "${addAtHead}" "${scientific}")"

coverageRoot="test/fuzzer/coverage"
profileRoot="${coverageRoot}/profiles"
ryanjson_prepare_clean_dir "${coverageRoot}"
mkdir -p "${profileRoot}"

profraw="${profileRoot}/coverage.profraw"
profdata="${coverageRoot}/coverage.profdata"
reportTxt="${coverageRoot}/report.txt"
reportHtml="${coverageRoot}/html"

ryanjson_print_banner_begin "Fuzzer 执行配置"
ryanjson_print_banner_kv "模式" "${fuzzMode}"
ryanjson_print_banner_kv "配置" "${caseName}"
ryanjson_print_banner_kv "Corpus" "${fuzzCorpusDir}"
ryanjson_print_banner_kv "字典" "${fuzzDictPath}"
ryanjson_print_banner_kv "merge" "${fuzzEnableMerge}"
if [[ "${fuzzEnableMerge}" == "1" ]]; then
  ryanjson_print_banner_kv "merged_corpus" "${fuzzCorpusDir} (in-place)"
fi
ryanjson_print_banner_kv "timeout" "${fuzzTimeout}"
ryanjson_print_banner_kv "max_len" "${fuzzMaxLen}"
ryanjson_print_banner_kv "workers/jobs" "${fuzzWorkers}/${fuzzJobs}"
if [[ -n "${fuzzRssLimitMb}" ]]; then
  ryanjson_print_banner_kv "rss_limit_mb" "${fuzzRssLimitMb}"
fi
if [[ -n "${fuzzMallocLimitMb}" ]]; then
  ryanjson_print_banner_kv "malloc_limit_mb" "${fuzzMallocLimitMb}"
fi
if [[ -n "${fuzzRuns}" ]]; then
  ryanjson_print_banner_kv "runs" "${fuzzRuns}"
else
  ryanjson_print_banner_kv "max_total_time" "${fuzzMaxTotalTime}s"
fi
ryanjson_print_banner_end

ryanjson_run_xmake_config "${xmakeForceClean}" "${caseName}"
ryanjson_run_xmake_build "RyanJsonFuzz" "${caseName}"
echo "[信息] xmake 构建完成（target=RyanJsonFuzz）"

if [[ ! -d "${fuzzCorpusDir}" ]]; then
  mkdir -p "${fuzzCorpusDir}"
  echo "[信息] Corpus 目录不存在，已自动创建：${fuzzCorpusDir}"
fi

actualFuzzCorpusDir="${fuzzCorpusDir}"
if [[ "${fuzzEnableMerge}" == "1" ]]; then
  mergeTempDir="${coverageRoot}/merged-corpus.tmp"

  ryanjson_prepare_clean_dir "${mergeTempDir}"
  echo "[阶段] 正在 merge corpus..."
  ./build/linux/x86/release/RyanJsonFuzz -merge=1 "${mergeTempDir}" "${fuzzCorpusDir}"

  rm -rf "${fuzzCorpusDir}"
  mv "${mergeTempDir}" "${fuzzCorpusDir}"
  actualFuzzCorpusDir="${fuzzCorpusDir}"
  echo "[信息] merge 完成，后续 fuzz 使用：${actualFuzzCorpusDir}"
fi

declare -a dictArgs=()
if [[ -f "${fuzzDictPath}" ]]; then
  dictArgs+=("-dict=${fuzzDictPath}")
else
  echo "[警告] 未找到字典文件 ${fuzzDictPath}，已跳过 -dict 参数。"
fi

declare -a fuzzArgs=()
fuzzArgs+=("${actualFuzzCorpusDir}")
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
if [[ -n "${FUZZ_EXTRA_ARGS:-}" ]]; then
  read -r -a extraArgs <<< "${FUZZ_EXTRA_ARGS}"
  fuzzArgs+=("${extraArgs[@]}")
fi

echo "[阶段] 正在运行 fuzz 二进制..."
LLVM_PROFILE_FILE="${profraw}" ./build/linux/x86/release/RyanJsonFuzz "${fuzzArgs[@]}"

if [[ "${fuzzSkipCov}" == "1" ]]; then
  echo "[信息] FUZZ_SKIP_COV=1，已跳过覆盖率生成。"
  echo "输出目录：${coverageRoot}"
  exit 0
fi

if ! command -v llvm-profdata >/dev/null 2>&1; then
  echo "[错误] 未找到 llvm-profdata，无法生成覆盖率。"
  exit 1
fi
if ! command -v llvm-cov >/dev/null 2>&1; then
  echo "[错误] 未找到 llvm-cov，无法生成覆盖率。"
  exit 1
fi

llvm-profdata merge -sparse "${profraw}" -o "${profdata}"

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
