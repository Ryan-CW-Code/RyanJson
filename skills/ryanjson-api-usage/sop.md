# RyanJson API 工作流（压缩版）

## 1. 先定边界
1. 只使用公开 API，不展开内部实现细节。
2. 先确认宏前提与 hooks 初始化方案。
3. 明确用户目标类型：读取、构建、更新、替换、迁移。

## 2. 再给方案
1. 选择最小 API 组合。
2. 给出“成功 + 失败”双路径代码。
3. 明确所有权转移与释放责任。
4. 给出 RT-Thread 可执行验证点。

## 3. 最后交付
1. 方案结论与前提。
2. 最小代码。
3. 所有权/释放清单。
4. 验证步骤与扩展建议。

## 4. 常见漏项
- 未初始化 hooks。
- `Get*` 之前缺少判空/判型。
- 把 Replace 失败语义误当 Add/Insert。
- 未标注宏前提，导致预期错位。

## 5. 依据（仓库内）
- `RyanJson/RyanJson.c`：hooks 初始化与释放路径
- `RyanJson/RyanJsonItem.c`：Insert/Replace 失败语义
- `RyanJson/RyanJsonConfig.h`：宏前提（StrictKey / AddAtHead）
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`
