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
# shellcheck source=scripts/lib/common.sh
source "${scriptDir}/scripts/lib/common.sh"
repoRoot="$(ryanjson_repo_root_from_source "${BASH_SOURCE[0]}" 0)"
cd "${repoRoot}"

main() {
	ryanjson_print_banner_begin "本地 CI 启动：阶段 1/2 -> Base 单元测试"
	ryanjson_print_banner_end
	bash ./run_local_base.sh

	ryanjson_print_banner_begin "本地 CI 启动：阶段 2/2 -> Fuzz quick"
	ryanjson_print_banner_end

	: "${RYANJSON_STRICT_OBJECT_KEY_CHECK:=false}"
	: "${RYANJSON_DEFAULT_ADD_AT_HEAD:=true}"
	: "${RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC:=true}"
	: "${FUZZ_MODE:=quick}"
	: "${FUZZ_SKIP_COV:=1}"
	: "${FUZZ_RUNS:=}"
	: "${FUZZ_MAX_TOTAL_TIME:=45}"
	: "${FUZZ_WORKERS:=2}"
	: "${FUZZ_JOBS:=2}"
	: "${XMAKE_FORCE_CLEAN:=0}"

	export RYANJSON_STRICT_OBJECT_KEY_CHECK
	export RYANJSON_DEFAULT_ADD_AT_HEAD
	export RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC
	export FUZZ_MODE FUZZ_SKIP_COV FUZZ_RUNS FUZZ_MAX_TOTAL_TIME FUZZ_WORKERS FUZZ_JOBS
	export XMAKE_FORCE_CLEAN
	bash ./run_local_fuzz.sh

	ryanjson_print_banner_begin "本地 CI 执行完成"
	ryanjson_print_banner_end
}

main "$@"
