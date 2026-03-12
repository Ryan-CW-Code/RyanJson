# 脚本目录说明

## 目录结构
- `scripts/ci`：CI 与覆盖率相关核心脚本
- `scripts/lib`：本地入口与 CI 共享的 Bash 公共函数
- `scripts/setup`：本地环境准备脚本

## 共享脚本
- `scripts/lib/common.sh`
  - 统一仓库根路径解析、banner 输出、命令检查、目录清理、xmake 配置/构建辅助函数
- `scripts/lib/semantic_matrix.sh`
  - 统一 `quick/nightly/full` 语义矩阵生成与 caseName 命名

## 脚本清单
- `scripts/ci/runBaseCoverage.sh`
  - 单元测试矩阵入口（`quick/nightly/full`）
  - 支持 `UNIT_SKIP_COV`、`UNIT_STOP_ON_FAIL`、`XMAKE_FORCE_CLEAN`
  - 自动调用 `scripts/ci/checkUnityRunnerList.sh` 更新 runner 列表
- `scripts/ci/runCoverage.sh`
  - Fuzzer 入口（`quick/nightly/full`）
  - 默认走 `FUZZ_RUNS=100000`；也支持显式切到 `FUZZ_MAX_TOTAL_TIME`
  - 支持 `FUZZ_WORKERS/JOBS`、`FUZZ_ENABLE_MERGE`、`XMAKE_FORCE_CLEAN`
- `scripts/ci/checkUnityRunnerList.sh`
  - 扫描 `test/unityTest/cases` 下所有 `*Runner` 实现并更新 `test_list.inc`
- `scripts/setup/install_qemu_deps.sh`
  - 安装 `arm-none-eabi` 工具链与 `qemu-system-arm`
  - 支持 `--no-update` 跳过包索引刷新

## 根目录本地脚本
- `run_local_base.sh`
  - 本地一键跑单元测试矩阵（默认 `UNIT_MODE=full`，跳过覆盖率）
- `run_local_ci.sh`
  - 本地一键模拟 `ci-pr`（先 unit，再 fuzz quick）
- `run_local_fuzz.sh`
  - 本地一键 fuzz；默认走 `FUZZ_RUNS=100000`，也支持显式切到 `FUZZ_MAX_TOTAL_TIME`
  - `FUZZ_ENABLE_MERGE=1` 时先 merge 原始 corpus，再直接回写原 corpus 目录
- `run_local_format.sh`
  - 本地一键格式化或 `--check` 校验仓库源码（排除 `test/externalModule` 与临时产物）
- `run_local_qemu.sh`
  - 本地一键跑 QEMU 硬件语义校验（完整 localbase 单测 + 非对齐 fault）
  - 默认 `QEMU_MEMORY=64M`，可按需通过环境变量覆盖
- `run_local_skills.sh`
  - 本地同步并校验 `skills/*`
