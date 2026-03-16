# RyanJson 术语字典（共享）

- **已验证**：有直接证据支撑（代码、测试、日志、覆盖率、回归结果）。
- **推断**：暂无直接证据，仅基于已有信息推理，输出时必须显式标注。
- **未验证**：尚未执行验证；通常等同“推断”或“待验证项”。
- **可恢复错误**：应返回 `false/NULL` 且完成回滚/释放，不应用 assert 终止流程。
- **不可恢复错误**：内部不变量或结构被破坏；启用 `RyanJsonEnableAssert` 时可触发 assert。
- **失败语义**：失败时的所有权与资源处理规则；`Add/Insert` 与 `Replace` 必须分开描述。
- **语义取证链**：`example/ -> test/unityTest/ -> test/fuzzer/`。
- **术语保留英文**：注释可用中文，但类型名/字段语义名/API 名保持英文（如 `Array`、`Object`、`strValue`、`objValue`）。
- **线索化链表**：同层兄弟用 `next` 串联，最后一个兄弟的 `next` 指向父节点，用 `IsLast` 标志区分。
- **IsLast**：节点为同层最后一个兄弟的标志位；对外 `RyanJsonGetNext` 会返回 NULL。
- **payload**：紧跟节点结构体后的动态载荷区，用于存放 key、strValue、number 或 children 指针等。
- **内联/指针模式**：key/strValue 在节点内联区存储或在堆上存储的两种策略，由 `flag` 位控制。
- **keyLenField**：key 长度字段本身的字节数编码（1/2/4 字节），由 `flag` 位控制。
- **RyanJsonInlineStringSize**：内联 key/strValue 的阈值字节数，用于决定是否走内联存储。

## 依据（仓库内）
- `RyanJson/RyanJson.h`：公开 API 与失败语义注释
- `RyanJson/RyanJsonItem.c`：Add/Insert/Replace 失败路径
- `RyanJson/RyanJsonUtils.c`：线索化链表与字符串存储模式
- `RyanJson/RyanJsonInternal.h`：内联阈值与 keyLenField 计算
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`
