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
- `run_local_memory.sh`
  - 本地一键运行内存对比用例（仅 memory runner），支持 host / qemu
  - 默认仅跑单配置（`MEM_SINGLE_CASE=1`），默认语义：`MEM_DEFAULT_CASE="false false true"`
  - 默认同时生成 host + qemu 两份报告（`MEM_PLATFORM=both`）
  - 默认清理 ANSI 颜色：`MEM_STRIP_ANSI_LOG=1`（如需保留颜色设为 0）
  - 固定跑 4 组模拟分配参数：
    - header=12 align=4
    - header=8 align=8
    - header=8 align=4
    - header=4 align=4
  - 结构化输出 `[MEM][COMPARE]`
  - 仅输出 Markdown 汇总（`reports/memory/`）：
    - `reports/memory/host.md`
    - `reports/memory/qemu.md`
- `run_local_rfc8259.sh`
  - 本地一键生成 RFC8259 报告（仅 RFC runner），支持 host / qemu
  - 默认仅跑单配置（`RFC_SINGLE_CASE=1`），默认语义：`RFC_DEFAULT_CASE="false false true"`
  - 默认同时生成 host + qemu 两份报告（`RFC_PLATFORM=both`）
  - 默认清理 ANSI 颜色：`RFC_STRIP_ANSI_LOG=1`（如需保留颜色设为 0）
  - Host/QEMU 均使用内嵌 RFC8259 数据集（避免依赖文件系统）
  - 仅输出 Markdown 汇总（`reports/rfc8259/`）：
    - `reports/rfc8259/host.md`
    - `reports/rfc8259/qemu.md`
- `scripts/tools/gen_rfc8259_embedded.py`
  - 生成 RFC8259 内嵌数据文件（`test/unityTest/cases/RFC8259/rfc8259Embedded.*`）
  - 当 `test/data/rfc8259` 更新时需要重新生成
- `run_local_qemu.sh`
  - 本地一键跑 QEMU 硬件语义校验（完整 localbase 单测 + 非对齐 fault）
  - 默认 `QEMU_MEMORY=64M`，可按需通过环境变量覆盖
  - 日志目录默认 `localLogs/qemu`（已在 `.gitignore` 中忽略）
- `run_local_skills.sh`
  - 本地同步并校验 `skills/*`
  - 同时校验 `skills/evals/cases/*.json` 路由评测样本
- `scripts/tools/validate_skills.py`
  - 校验 skills frontmatter、agents/openai.yaml、AGENTS 路由与 Markdown 明确路径引用
- `scripts/tools/validate_skill_cases.py`
  - 校验 `skills/evals/cases/*.json` 的格式、路由目标、引用路径与覆盖分布
