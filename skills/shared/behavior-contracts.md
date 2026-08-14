# RyanJson 行为契约

## 证据标准

- 语义先以当前公开头文件和实现为准，再查 unit，最后查 fuzz；示例仅用于理解，不能覆盖源码事实。
- 证据链不完整的结论必须标为推断。
- 回答语义问题时至少给出一个支撑结论的源码或测试路径。

## 配置前提

- 声称语义、添加顺序敏感测试前，先读 `RyanJsonStrictObjectKeyCheck` 与 `RyanJsonDefaultAddAtHead` 的当前值。当前默认值是 `false`，但源码才是权威。
- 应用侧在调用依赖分配的 RyanJson API 前，必须确保 `RyanJsonInitHooks` 成功。
- 未获明确授权，不得改变公开默认值或 API 语义；若获得授权，必须有兼容性分析、测试和文档更新。

## 所有权

| 操作 | 成功 | 失败 |
| --- | --- | --- |
| `Create*` | 调用方拥有游离节点，直到挂载或删除。 | 调用方不获得节点。 |
| `Detach*` | 调用方拥有返回节点，必须挂载或删除。 | 不转移所有权。 |
| `Add/Insert` | 合法游离 item 的所有权转给父节点。 | 游离 item 走库侧清理路径；已挂树 item 被拒绝且原树不受破坏。给调用建议前必须核对具体 API。 |
| `ReplaceByKey/ReplaceByIndex` | 新 item 转给父节点，旧节点由库删除。 | 新 item 不被消费；调用方必须复用或删除。 |
| `RyanJsonPrint` | 调用方拥有返回缓冲并以 `RyanJsonFree` 释放。 | 不返回输出缓冲。 |

不得将 `Add/Insert` 的失败清理规则套用到 `Replace`，这是审查与测试的强制检查点。

## 模式隔离

- unit 通过 Unity main 验证确定性契约。
- QEMU 验证 FreeRTOS 与硬件敏感语义，包括非对齐访问 fault。
- fuzz 使用专用 target 验证异常和广泛输入的稳健性。
- 不能把三种模式的命令或断言说成同一项覆盖。

## 代码与文档规则

- 新 C 代码使用 `stdint` 类型、小驼峰命名和命名常量。
- 改公开接口、配置宏或关键内部逻辑时使用 Doxygen 块注释：首行 `@brief`，边界/默认行为加 `@note`，复杂原理加 `@details`。
- 中文说明中保留 RyanJson 类型、字段、API 与宏的英文名称。
- 代码建议必须给出前置条件、成功路径、失败路径、所有权与释放责任。

## 证据入口

- [RyanJsonConfig.h](../../RyanJson/RyanJsonConfig.h)
- [RyanJsonItem.c](../../RyanJson/RyanJsonItem.c)
- [testCreate.c](../../test/unityTest/cases/core/testCreate.c)
- [testReplace.c](../../test/unityTest/cases/core/testReplace.c)
