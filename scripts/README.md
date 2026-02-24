# 脚本目录说明

## 目录结构
- `scripts/ci`：CI 与覆盖率相关核心脚本
- `scripts/setup`：本地环境准备脚本

## 脚本清单
- `scripts/ci/runBaseCoverage.sh`
  - 单元测试矩阵入口（`quick/nightly/full`）
  - 支持 `UNIT_SKIP_COV`、`UNIT_STOP_ON_FAIL`、`XMAKE_FORCE_CLEAN`
- `scripts/ci/runCoverage.sh`
  - Fuzzer 入口（`quick/nightly/full`）
  - 支持 `FUZZ_RUNS/FUZZ_MAX_TOTAL_TIME`、`FUZZ_WORKERS/JOBS`、`XMAKE_FORCE_CLEAN`
- `scripts/setup/install_qemu_deps.sh`
  - 安装 `arm-none-eabi` 工具链与 `qemu-system-arm`
  - 支持 `--no-update` 跳过包索引刷新

## 根目录本地脚本
- `run_local_base.sh`
  - 本地一键跑单元测试矩阵（默认 full）
- `run_local_ci.sh`
  - 本地一键模拟 `ci-pr`（先 unit，再 fuzz quick）
- `run_local_fuzz.sh`
  - 本地一键 fuzz
- `run_local_qemu.sh`
  - 本地一键跑 QEMU 硬件语义校验（完整 localbase 单测 + 非对齐 fault）
  - 默认 `QEMU_MEMORY=64M`，可按需通过环境变量覆盖
