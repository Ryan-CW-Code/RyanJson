# Gemini Skill Card

名称：`ryanjson-api-usage`

## 定位
- 面向嵌入式/RT-Thread 场景的 RyanJson 公开 API 落地技能。

## 适用场景
- 询问 RyanJson 公开 API 的正确调用方式。
- 需要 Parse/Create/Add/Change/Replace/Detach/Delete 的可运行示例。
- 需要明确失败路径、所有权和释放顺序。

## 输入建议
- 目标操作：要实现的业务 Json 流程。
- 平台约束：RT-Thread 线程模型、内存预算、是否固定缓冲。
- 验收标准：返回值、日志、内存统计、协议输出。

## 硬约束
- 任意 RyanJson API 前，必须先成功调用 `RyanJsonInitHooks`。
- 默认只讲公开 API，不展开内部实现细节。
- `Get*` 使用前必须判空并 `RyanJsonIsXXX` 判型。
- 语义不明确时按 `example/ -> test/unityTest/ -> test/fuzzer/` 取证。

## 术语口径
- 统一按 `../references/terminology.md`。
- 输出必须显式区分：已验证/推断、可恢复错误/不可恢复错误、失败语义。

## 默认提示词
使用 `$ryanjson-api-usage`，输出可落地的 RyanJson 公开 API 方案：强制 hooks 前置、明确所有权/释放路径、仅使用公开 API，并在语义不明确时按 `example -> unityTest -> fuzzer` 取证。

## 输出骨架
1. 结论与接口选型（标注已验证/推断）。
2. 最小可运行代码（RT-Thread 风格）。
3. 失败路径与所有权说明（区分 Add/Insert 与 Replace）。
4. 上板验证步骤与下一步建议。

## 依据（仓库内）
- `../references/apiReference.md`
- `../references/ownershipAndErrors.md`
- `../references/apiPatterns.md`
