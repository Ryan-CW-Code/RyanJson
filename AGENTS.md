# RyanJson 智能体入口

## 目的
- 本文件只做导航，不复述 skills 内的规则与流程。
- 处理任务前先阅读共享基线，随后进入对应技能文档。

## Claude Skills 规范要点
- 每个 skill 必须有 `SKILL.md`，并以 YAML frontmatter 开头。
- frontmatter 必填 `name`、`description`。
- `name` 规则：仅小写字母/数字/连字符，最多 64 字符，不含 XML 标签，不含保留字 anthropic/claude。
- `description` 规则：非空，最多 1024 字符，不含 XML 标签；需同时说明“做什么”和“何时用”。
- 目录名必须与 `name` 一致。
- `SKILL.md` 正文建议包含“步骤说明 + 示例”，并保持精简（建议不超过 500 行）。
- 本仓库不采用 `.claude/skills` 路径，但上述结构与字段要求必须满足。
- 若需兼容 Claude.ai，`description` 建议控制在 200 字符以内。

## 共享基线与术语
- 共享基线：`skills/shared/ryanJsonCommon.md`
- 术语表：`skills/shared/terminology.md`
- 架构与数据结构：`skills/shared/architecture.md`

## 技能路由
- 项目概览、目录结构、构建脚本、CI 与报告路径：`skills/ryanjson-project-guide/SKILL.md`
- 涉及内部实现/数据结构：先读 `skills/shared/architecture.md`，再进入对应技能。
- API 使用、集成、所有权与示例：`skills/ryanjson-api-usage/SKILL.md`
- 核心优化、性能、内存与语义边界：`skills/ryanjson-optimization/SKILL.md`
- 单元测试工程、用例归属与覆盖策略：`skills/ryanjson-test-engineering/SKILL.md`

## Agent 入口
- 技能内的 agent 模板：`skills/*/agents/openai.yaml`

## 本地校验
- skills/agents 文档校验入口：`run_local_skills.sh`
- 仓库内 validator：`scripts/tools/validate_skills.py`
