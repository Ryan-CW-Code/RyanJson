# 脚本目录说明

## 目录结构
- `scripts/lib`：本地入口共享的 Bash 公共函数
- `scripts/setup`：本地环境准备脚本

## 共享脚本
- `scripts/lib/common.sh`
  - 统一仓库根路径解析、banner 输出、命令检查、目录清理、xmake 配置/构建辅助函数
  - 统一 `quick/nightly/full` 语义矩阵生成与 caseName 命名

## 脚本清单
- `scripts/setup/install_qemu_deps.sh`
  - 安装 `arm-none-eabi` 工具链与 `qemu-system-arm`
  - 支持 `--no-update` 跳过包索引刷新

## 根目录本地脚本
- `run_local_base.sh`
  - 本地一键跑单元测试矩阵（默认 `UNIT_MODE=full`，跳过覆盖率）
  - 自动同步 Unity runner 列表，可用 `UNIT_SYNC_ONLY=1` 仅同步不执行测试
- `run_local_ci.sh`
  - 本地一键模拟 `ci-pr`（先 unit，再 fuzz quick）
- `run_local_fuzz.sh`
  - 本地一键 fuzz；默认走 `FUZZ_RUNS=100000`，也支持显式切到 `FUZZ_MAX_TOTAL_TIME`
  - `FUZZ_ENABLE_MERGE=1` 时先 merge 原始 corpus，再直接回写原 corpus 目录
  - 默认写入 `./fuzz.log`（仅运行日志，覆盖写）
- `run_local_format.sh`
  - 本地一键格式化或 `--check` 校验仓库源码（排除 `test/externalModule` 与临时产物）
- `run_local_qemu.sh`
  - 本地一键跑 QEMU 硬件语义校验（完整 localbase 单测 + 非对齐 fault）
  - 默认 `QEMU_MEMORY=64M`，可按需通过环境变量覆盖
- `run_local_skills.sh`
  - 本地同步并校验 `skills/*`
