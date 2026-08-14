# RyanJson 任务路由

## 选择任务所有者

| 主要结果 | Skill | 先读 |
| --- | --- | --- |
| 问题模糊、仓库导航、构建/CI/报告，或任务跨多个领域 | `ryanjson-workflow` | `repository-facts.md` |
| 公开 API 选型、接入、生命周期、所有权或应用代码 | `ryanjson-api-usage` | `behavior-contracts.md` |
| 内核实现、性能、内存、宏、crash 根因或代码审查 | `ryanjson-core-engineering` | `behavior-contracts.md`、`architecture.md` |
| 单测归属、重复覆盖、fuzz 回归或验证矩阵 | `ryanjson-test-engineering` | `behavior-contracts.md` |

按用户需要的结果路由，而不是按“测试”或“crash”等表面词汇路由。API 建议附带回归时先进入 API usage；crash 根因附带回归时先进入 core engineering。

## 跨领域执行顺序

| 请求形态 | 必须顺序 |
| --- | --- |
| API 问题 + 回归 | 确认 API 契约 -> 定位既有测试归属 -> 新增最小独立测试。 |
| crash/语义缺陷 + 回归 | 复现并找出被破坏的不变量 -> 修复或解释内核路径 -> 新增确定性回归。 |
| 性能或内存优化 | 建立正确性和测量基线 -> 保持契约 -> 改目标路径 -> 跑匹配的门禁。 |
| 构建、脚本、报告或文档变动 | workflow 确认所有者 -> 改权威源 -> 运行生成器或校验器。 |
| 任务不明确 | 使用 workflow；先声明工作假设，查证后继续推进。 |

## 交接规则

- workflow 可以编排其他 skill 并交付一份分阶段结果，不得只丢出下一个文档名。
- 即使其他 skill 提供证据，领域结论仍由当前 specialist 负责。
- 交接时带上目标、证据、假设、变更文件与待验证项，不能让用户重复上下文。

## 路由样例

| 用户请求 | 所有者 | 首个动作 |
| --- | --- | --- |
| “第一次接触仓库，应该看什么、跑什么？” | Workflow | 给最短仓库路径并选出下一步 skill。 |
| “Replace 失败后谁释放 `newItem`？” | API usage | 查所有权契约和 Replace 测试。 |
| “不改 API 的情况下 Parse 变慢了。” | Core engineering | 建立测量基线并检查 Parse 路径。 |
| “这个 Add 失败测试是否重复？” | Test engineering | 搜索 test-map 与现有契约覆盖。 |
| “Replace crash，修复并补测试。” | Core engineering -> test engineering | 先定位不变量，再写回归。 |
