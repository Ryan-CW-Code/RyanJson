# RyanJson 文档、报告与 AI 路由

## 范围
- 用于回答“README、报告、skills、AGENTS 分别管什么，文件现在在哪”。
- 不展开具体 API/优化/测试细节。

## 对外文档入口
- `README.md`：项目介绍、能力摘要、脚本入口、报告链接。
- `example/`：最小示例。
- `reports/memory/host.md`、`reports/memory/qemu.md`：内存对比报告。
- `reports/rfc8259/host.md`、`reports/rfc8259/qemu.md`：RFC8259 报告。

## AI 入口
- `AGENTS.md`：总入口与技能路由。
- `skills/shared/ryanJsonCommon.md`：共享基线。
- `skills/shared/questionRouter.md`：模糊问题先分流。
- `skills/shared/routingExamples.md`：边界问题与真实提问路由样例。
- `skills/shared/architecture.md`：内部结构与实现事实。
- `skills/shared/terminology.md`：统一术语。

## 技能路由
- `skills/ryanjson-project-guide/SKILL.md`：仓库导航、构建脚本、CI、报告路径。
- `skills/ryanjson-api-usage/SKILL.md`：公开 API 使用、集成与失败语义。
- `skills/ryanjson-optimization/SKILL.md`：核心实现优化与语义边界。
- `skills/ryanjson-test-engineering/SKILL.md`：单元测试工程与去重治理。

## 报告生成事实
- `run_local_memory.sh` 生成 `reports/memory/*.md`
- `run_local_rfc8259.sh` 生成 `reports/rfc8259/*.md`
- 这些 Markdown 是脚本生成且被 README 引用的仓库文档，不按临时文件处理。

## 依据（仓库内）
- `README.md`
- `AGENTS.md`
- `scripts/README.md`
- `run_local_memory.sh`
- `run_local_rfc8259.sh`
- `reports/memory/host.md`
- `reports/memory/qemu.md`
- `reports/rfc8259/host.md`
- `reports/rfc8259/qemu.md`
