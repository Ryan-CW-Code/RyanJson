# 核心配置与兼容性

## 语义宏

| 宏 | 作用 | 必须复核 |
| --- | --- | --- |
| `RyanJsonStrictObjectKeyCheck` | 控制 Object 重复 key 预期，包括 Parse 和 Compare | 重复 key 测试、公开文档和兼容性 |
| `RyanJsonDefaultAddAtHead` | 控制 Add 默认插入方向 | 顺序/索引断言与应用排序假设 |
| `RyanJsonInlineStringSize` | 决定短字符串内联阈值 | 分配大小、偏移逻辑、边界测试和内存测量 |
| `RyanJsonMallocHeaderSize` / `RyanJsonMallocAlign` | 描述分配器开销与对齐 | 布局假设、内存报告解释和目标行为 |
| `RyanJsonDoubleBufferSize` / `RyanJsonSnprintfSupportScientific` | 影响 double 打印缓冲与格式策略 | Print 边界和格式测试 |

## 变更规则

- 结论前读取当前预处理定义和调用点。
- 即使函数签名不变，默认值变化也是公开兼容性变化。
- 兼容性不明确时优先新增显式 opt-in 宏，而不是改默认值。
- 宏语义变化时更新 [行为契约](../../shared/behavior-contracts.md)、[架构](../../shared/architecture.md)、相关测试和维护映射。
