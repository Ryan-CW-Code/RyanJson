# API 操作选型

## 按目标选择操作

| 目标 | 公开 API 路径 | 主要风险 |
| --- | --- | --- |
| 读取配置 | `Parse -> Get -> IsXXX -> GetXXXValue -> Delete` | 读取不存在或错类型 item |
| 构建上报 | `Create -> Add -> Print/PrintPreallocated -> Delete` | 忽略 Add 或输出失败 |
| 同类型改值 | `Get -> Change*Value` | 用 Change 做类型转换 |
| 字段类型变更 | `Create* -> ReplaceBy*` | Replace 失败后泄漏新 item |
| 子树迁移 | `Detach* -> Add/Insert` | Detach 后失去所有权或误判 Add 失败 |
| 紧凑 JSON 传输 | `Print(..., RyanJsonFalse, ...)` | 先格式化再 Minify |
| 清理已有 JSON 文本 | 可写缓冲上的 `Minify` | 假设自动补 NUL 终止符 |

## Parse 与读取

- `RyanJsonParse` 接收 NUL 结束字符串，成功返回调用方持有的 root，失败返回 `NULL`。
- 非 NUL 缓冲、需要精确 parse-end 或严格尾部约束时使用 `RyanJsonParseOptions`。
- Get 成功后仍要 `RyanJsonIsXXX`；不能从 key 名称推断类型。

## 构建与输出

- 用 `CreateObject`、`CreateArray` 和标量 Create API 创建游离值。
- 标量用标量 Add helper；`AddItem*` 只接收 Object 或 Array item。
- 传输直接使用非格式化 Print；固定容量输出优先 `PrintPreallocated`，避免无意义动态分配。
- `RyanJsonMinify` 是已有文本的清洗工具，不是主序列化路径。返回值等于输入长度时，调用方不能假设额外写入 NUL。

## Object key 与顺序

- `RyanJsonStrictObjectKeyCheck` 决定重复 key 预期，结论受影响时必须说明有效宏。
- `RyanJsonDefaultAddAtHead` 影响默认插入方向；测试应断言 key、size 或显式插入语义，而不是假设固定遍历顺序。
- 非严格 Object 的重复 key 查询、删除、分离和替换必须以当前实现验证。

## 公开边界

- 应用侧只使用 [RyanJson.h](../../../RyanJson/RyanJson.h) 的公开声明。
- 布局、热点性能、宏设计或 crash 根因进入 [核心工程](../../ryanjson-core-engineering/SKILL.md)。
- 回归归属与重复测试进入 [测试工程](../../ryanjson-test-engineering/SKILL.md)。
