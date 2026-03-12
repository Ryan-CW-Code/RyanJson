---
name: ryanjson-api-usage
description: RyanJson 公开 API 使用与 RT-Thread 集成指南。用于接口选型、调用顺序、所有权与失败释放、以及 RyanJsonInitHooks 初始化集成；用户问“怎么用/怎么集成/失败谁释放/示例代码”时使用。测试补强转 ryanjson-test-engineering，内部优化转 ryanjson-optimization。
---

# RyanJson API 使用技能

## 目标
- 给出可落地的公开 API 调用方案（含失败路径与释放责任）。
- 不讨论内部优化与测试补强。

## 必要输入
- 运行环境（RT-Thread/裸机/其他）与相关宏配置。
- 目标场景（读取/构建/更新/替换/迁移）。
- 约束（内存/性能/代码尺寸/兼容性）。

## 核心流程
1. 先读 `../shared/ryanJsonCommon.md`，确认宏与失败语义基线。
2. 用 `references/apiPatterns.md` 选择最小 API 路径。
3. 输出成功/失败路径与所有权清单。
4. 给出最小验证项（返回值/日志/内存检查点）。

## 约束（必须遵守）
- `Get*` 先判空，再 `RyanJsonIsXXX` 判型。
- 同类型更新优先 `Change*Value`，跨类型更新用 `ReplaceByKey/ReplaceByIndex`。
- `Add/Insert` 只接收游离节点，禁止重复挂载。
- `RyanJsonAddItemToObject` 成功后若继续操作子树，重新从根 `GetObjectToKey` 取容器指针。
- 传输输出优先非格式化打印：`RyanJsonPrint(..., RyanJsonFalse, ...)` 或 `RyanJsonPrintPreallocated(..., RyanJsonFalse, ...)`。
- `RyanJsonMinify` 仅做文本清洗，不作为主输出路径。

## 输出格式
1. 前提与结论：宏/环境前提 + API 选型。
2. 最小代码：只用公开 API，包含失败分支。
3. 所有权清单：逐分支说明谁负责释放。
4. 验证建议：最小验证步骤与后续扩展方向。

## 参考导航
- 共享基线：`../shared/ryanJsonCommon.md`
- 注释规范：统一用 Doxygen 风格，类型名/字段语义/API 名保持英文（见共享基线第 9 节）
- API 快速入口：`references/quickstart.md`
- 场景模板：`references/apiPatterns.md`
- 所有权细则：`references/ownershipAndErrors.md`
- 完整 API：`references/apiReference.md`
- hooks 初始化：`references/hooksInitPolicy.md`
- RT-Thread 示例：`references/rtThreadExamples.md`
- 集成模板：`references/integrationTemplate.md`
- 排障与故障注入：`references/pitfallsAndDebug.md`、`references/faultInjectionPlaybook.md`
- 术语：`references/terminology.md`
- 本地压缩文档：`context.md`、`apiPatterns.md`、`ownership.md`、`sop.md`
