# RyanJson 优化语境（压缩版）

## 1. 作用
- 帮助用户和 AI 快速判断“这是实现优化问题、行为修复问题，还是其实该转别的 skill”。
- 回答前先锁定语义边界、回归门禁和收益目标。

## 2. 首屏先判断什么
1. 这是性能、内存、稳定性，还是 crash 根因问题。
2. 是否允许改默认行为或新增宏。
3. 当前问题是不是其实属于 API 用法或测试归属。

## 3. 回答前必须覆盖
1. 已验证事实与推断结论要分开。
2. 先给最小改动方向，再给门禁和风险。
3. 涉及 crash、对齐、所有权时要明确相关回归入口。

## 4. 第一次进入本 skill 时先看哪里
- 问题还模糊：`references/entryScenarios.md`
- 想知道整体执行顺序：`references/coreWorkflow.md`
- 想知道能不能合入：`references/regressionGates.md`
- 想知道哪里最容易踩坑：`pitfalls.md`

## 5. 输出检查单
1. 有没有改变公开语义。
2. 有没有给出最小验证链路。
3. 有没有说明剩余风险与回滚条件。

## 6. 依据（仓库内）
- `references/coreWorkflow.md`
- `references/regressionGates.md`
- `pitfalls.md`
- `../shared/questionRouter.md`
