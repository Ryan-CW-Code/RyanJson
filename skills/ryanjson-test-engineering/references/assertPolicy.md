# Assert 使用策略

## 范围
- 仅用于不可恢复的内部不变量破坏。
- 不用于用户可构造输入导致的常规错误。
- 术语口径见 `../../shared/terminology.md`。

## 判定规则
- 输入错误可恢复：返回 false/NULL，并保证资源回收。
- 内存破坏或结构破坏不可恢复：允许 assert（启用 `RyanJsonEnableAssert` 时）。
- Compare 内部对象链/Key 不变量属于结构破坏范畴，可按 assert 路径处理，不要求业务输入可达覆盖。

## 测试策略
- 单元测试验证“可恢复错误”返回路径。
- fuzzer 避免把业务预期写成 assert。
- 对 assert 路径，重点确认是否误分类。

## 常见误用
- 用 assert 拦截重复 key、非法数字、非法字符串输入。
- 以 assert 代替失败路径释放逻辑。

## 依据（仓库内）
- `RyanJson/RyanJson.h`：断言宏与公开语义注释
- `RyanJson/RyanJsonItem.c`：失败路径与可恢复错误返回
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`
