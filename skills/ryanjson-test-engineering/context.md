# RyanJson 测试语境（压缩版）

## 1. 作用
- 统一测试侧断言边界与宏敏感规则，减少假阳性。

## 2. 测试前必查
1. 当前宏值：
   - `RyanJsonStrictObjectKeyCheck`
   - `RyanJsonDefaultAddAtHead`
2. 当前模式：unit 或 fuzz。
3. 执行入口与覆盖目录口径：见 `../shared/ryanJsonCommon.md`。

## 3. 断言边界
- 业务/输入错误：断言可恢复结果（false/NULL）。
- 内存破坏/不变量损坏：可视为致命行为；启用 `RyanJsonEnableAssert` 时可进入 assert 路径。

## 4. 高风险覆盖区
- Parse 失败回滚。
- Replace 失败所有权清理。
- AddAtHead 相关索引断言。
- 字符串边界切换。

## 5. 质量检查单
1. true/false 分支是否双向覆盖。
2. 失败分支是否断言了所有权。
3. 测试说明是否标明模式前提。

## 6. 依据（仓库内）
- `xmake.lua`：`RyanJson`（unit）与 `RyanJsonFuzz`（fuzz）目标分离
- `test/unityTest/runner/main.c`：`#ifndef isEnableFuzzer` 的 unit `main` 路径
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`：失败分支所有权断言
- `../shared/ryanJsonCommon.md`：统一执行入口与覆盖口径
