# RyanJson 构建、脚本与 CI 入口

## 范围
- 用于回答“怎么构建、怎么跑本地验证、CI 在跑什么”。
- 具体 API 语义与测试断言细节转对应 skill。

## 构建目标
- `RyanJson`：默认 unit 目标。
- `RyanJsonFuzz`：fuzz 目标。
- `RyanJsonQemu` / `RyanJsonQemuCm3`：QEMU 目标。
- 当前主流程以 `xmake` 为主，不走历史 `Makefile`。

## 根目录本地脚本
- `./run_local_base.sh`：unit 矩阵。
- `./run_local_fuzz.sh`：fuzz 与覆盖率链路。
- `./run_local_qemu.sh`：QEMU 语义验证。
- `./run_local_ci.sh`：本地模拟 `ci-pr`。
- `./run_local_format.sh --check`：格式检查。
- `./run_local_skills.sh --validate-only`：skills/agents 校验。
- `./run_local_memory.sh`：生成内存报告。
- `./run_local_rfc8259.sh`：生成 RFC8259 报告。

## 变更场景建议
- 仅改 unit 用例：先 `./run_local_base.sh`
- 解析/打印/内存路径改动：`./run_local_base.sh` + `./run_local_fuzz.sh`
- 对齐、HardFault、嵌入式语义：`./run_local_qemu.sh`
- 提交前对齐 CI：`./run_local_ci.sh`
- skills 文档或 agent 元数据调整：`./run_local_skills.sh --validate-only`

## CI 摘要
- `skills-lint`：执行 `bash ./run_local_skills.sh --validate-only`
- `unit-full`：执行 full unit 矩阵
- `fuzz-quick-default`：执行 quick fuzz

## 依据（仓库内）
- `xmake.lua`
- `.github/workflows/ci-pr.yml`
- `scripts/README.md`
- `run_local_base.sh`
- `run_local_fuzz.sh`
- `run_local_qemu.sh`
- `run_local_ci.sh`
- `run_local_skills.sh`
- `run_local_memory.sh`
- `run_local_rfc8259.sh`
