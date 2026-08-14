---
name: ryanjson-api-usage
description: 通过公开 API 集成和使用 RyanJson，涵盖 API 选型、hooks 初始化、解析、构建、更新、输出、所有权、错误处理以及 RT-Thread 或应用侧接入。用于调用方代码与公开 API 问题，不用于内部实现改动或测试归属决策。
---

# RyanJson API 使用

交付具备明确生命周期、完整失败路径和正确所有权的调用方代码。

## 阅读顺序

1. 阅读 [行为契约](../shared/behavior-contracts.md)。
2. 所有结构修改或清理问题都阅读 [生命周期与所有权](references/lifecycle-and-ownership.md)。
3. 根据目标在 [操作选型](references/operations.md) 选择 API 路径。
4. 只有任务需要时才加载 [应用与 RT-Thread 接入](references/integration.md) 或 [诊断与故障注入](references/diagnostics.md)。公开声明以 `RyanJson/RyanJson.h` 为准。

## 工作方法

1. 确认应用环境、分配器 hooks、输入/输出缓冲规则、线程模型以及影响语义的宏值。
2. 选择最小公开 API 序列。除非任务明确跨界，应用代码不使用内部实现细节。
3. 编写或审查成功路径和每个失败返回，逐分支写清节点与输出缓冲归属。
4. 读取值时先判 `NULL`，再 `RyanJsonIsXXX`；Add、Insert、Replace 前确认结构节点游离。
5. 用最近的示例或测试验证场景。只有证据指向内部缺陷或缺回归时才升级交接。

## 强制规则

- Parse、Create 或其他依赖分配的 API 前，`RyanJsonInitHooks` 必须成功。
- 同类型更新用 `Change*Value`；类型转换用 `ReplaceByKey` 或 `ReplaceByIndex`。
- Replace 失败后新 item 仍归调用方，禁止套用 `Add/Insert` 的失败清理规则。
- Detach 会把所有权转给调用方；必须重新挂载或删除返回 item。
- 紧凑传输直接用非格式化 Print；只对已有的可写 JSON 文本使用 Minify。
- 示例和测试保持当前宏控制的行为；不能假设重复 key 或插入顺序。

## 参考资料

- [生命周期与所有权](references/lifecycle-and-ownership.md)
- [操作选型](references/operations.md)
- [应用与 RT-Thread 接入](references/integration.md)
- [诊断与故障注入](references/diagnostics.md)

## 交接

- crash、布局、宏重设计或性能问题进入 [核心工程](../ryanjson-core-engineering/SKILL.md)。
- 测试归属、重复覆盖或构造回归的请求进入 [测试工程](../ryanjson-test-engineering/SKILL.md)。
