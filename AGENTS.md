# RyanJson AI Skills

## 目的

- 本文件是仓库内 AI 工作的唯一总入口。
- 本文件只负责路由与共性纪律，不重复 API、内部实现、测试或构建事实。
- 所有技能均使用带 YAML frontmatter 的通用 `SKILL.md`；禁止新增平台专有 agent 配置。

## 工作入口

1. 先按 [任务路由](skills/shared/routing.md) 判断所有者。
2. 行动前完整阅读选中的 `SKILL.md`。
3. 只加载该 skill 指定的共享事实和参考资料。
4. 对实现语义先查当前源码和测试，再下结论。
5. 完成请求后按风险运行相应验证。

| 请求目标 | 主 skill |
| --- | --- |
| 问题模糊、仓库导航、构建/CI/报告，或跨多个领域的任务 | [ryanjson-workflow](skills/ryanjson-workflow/SKILL.md) |
| 公开 API 选型、接入、生命周期、所有权或应用侧代码 | [ryanjson-api-usage](skills/ryanjson-api-usage/SKILL.md) |
| 内核实现、性能、内存、宏、crash 根因或代码审查 | [ryanjson-core-engineering](skills/ryanjson-core-engineering/SKILL.md) |
| 单测归属、重复覆盖、fuzz 回归或验证矩阵 | [ryanjson-test-engineering](skills/ryanjson-test-engineering/SKILL.md) |

## 共享事实

- [任务路由与跨域编排](skills/shared/routing.md)
- [仓库事实与验证入口](skills/shared/repository-facts.md)
- [行为契约](skills/shared/behavior-contracts.md)
- [内部架构](skills/shared/architecture.md)

## 工程纪律

- 以公开头文件、实现、unit 测试、fuzz 测试为证据链；没有直接证据的结论必须标为推断。
- `Add/Insert` 与 `Replace` 的失败所有权必须分开处理。
- unit、QEMU、fuzz 的前提和验证结论不得混写。
- 新增 C 代码使用 `stdint` 类型和小驼峰命名；改动公开接口、配置宏或关键内部逻辑时使用 Doxygen 注释。
- `test/unityTest/runner/test_list.inc` 由脚本生成，禁止手改；通过 `run_local_base.sh` 更新。

## 知识维护

- 公开 API、核心表示、配置语义、构建/测试脚本或测试归属变动时，同一次修改更新对应的权威 skill 文档。
- 每条领域事实只允许一个权威文档。其他位置只链接，不复制为摘要、SOP、context 或 prompt。
