# RyanJson 术语字典（共享）

- **已验证**：有直接证据支撑（代码、测试、日志、覆盖率、回归结果）。
- **推断**：暂无直接证据，仅基于已有信息推理，输出时必须显式标注。
- **未验证**：尚未执行验证；通常等同“推断”或“待验证项”。
- **可恢复错误**：应返回 `false/NULL` 且完成回滚/释放，不应用 assert 终止流程。
- **不可恢复错误**：内部不变量或结构被破坏；启用 `RyanJsonEnableAssert` 时可触发 assert。
- **失败语义**：失败时的所有权与资源处理规则；`Add/Insert` 与 `Replace` 必须分开描述。
- **语义取证链**：`example/ -> test/unityTest/ -> test/fuzzer/`。
- **术语保留英文**：注释可用中文，但类型名/字段语义名/API 名保持英文（如 `Array`、`Object`、`strValue`、`objValue`）。

## 依据（仓库内）
- `RyanJson/RyanJson.h`：公开 API 与失败语义注释
- `RyanJson/RyanJsonItem.c`：Add/Insert/Replace 失败路径
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`
