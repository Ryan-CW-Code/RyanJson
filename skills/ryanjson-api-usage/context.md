# RyanJson API 语境（压缩版）

## 1. 作用
- 提供 API 解答的最小安全语境，防止跨宏/跨模式误答。
- 执行与模式共性口径见 `../shared/ryanJsonCommon.md`。
- 术语口径见 `../shared/terminology.md`。

## 2. 回答前必须检查
- 当前源码中的：
  - `RyanJsonStrictObjectKeyCheck`
  - `RyanJsonDefaultAddAtHead`
- 当前仓库默认值（仅参考）：
  - `RyanJsonStrictObjectKeyCheck=false`
  - `RyanJsonDefaultAddAtHead=false`

## 3. hooks 基线
- `RyanJsonInitHooks` 必须在任意 RyanJson API 之前执行。
- hooks 初始化失败时必须立即中止 Json 路径。

## 4. API 安全基线
- 读取路径：先判空，再判型，再取值。
- 更新路径：同类型用 `Change*Value`，跨类型用 `ReplaceBy*`。
- 结构路径：`Detach` 后必须重新挂载或显式释放。

## 5. 错误级别口径
- 可恢复错误：输入非法、key 不存在、类型不符等，返回 false/NULL。
- 不可恢复错误：内存破坏、双重释放、结构不变量损坏；在启用 `RyanJsonEnableAssert` 时可触发 assert。

## 6. 输出检查单
1. 标明宏前提。
2. 区分已验证/推断。
3. 给出失败分支与释放动作。

## 7. 依据（仓库内）
- `RyanJson/RyanJsonConfig.h`：`RyanJsonStrictObjectKeyCheck`、`RyanJsonDefaultAddAtHead` 默认值
- `RyanJson/RyanJson.c`：hooks 全局指针与 `RyanJsonInitHooks`
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`：失败所有权语义
