---
name: ryanjson-api-usage
description: RyanJson 公开 API 使用与审查。用于接口选型、调用顺序、初始化 hooks、所有权与失败释放、集成示例；用户问“怎么用/怎么集成/失败谁释放/这段 API 调用对不对”时使用。测试补强转 ryanjson-test-engineering，内部优化转 ryanjson-optimization。
---

# RyanJson API 使用技能

## 目标
- 给出可落地的公开 API 调用方案（含失败路径与释放责任）。
- 面向“JSON/嵌入式新手”也能理解的解释与最小示例。
- 不讨论内部优化与测试补强。

## 必要输入
- 运行环境（RT-Thread/裸机/其他）与相关宏配置。
- 目标场景（读取/构建/更新/替换/迁移）。
- 约束（内存/性能/代码尺寸/兼容性）。
> 若用户说不清环境，默认假设：可用 `malloc/free/realloc`、单线程、简单对象结构，并在回答中明确这些假设。

## 核心流程
1. 先读 `../shared/ryanJsonCommon.md`，确认宏与失败语义基线。
2. 若问题更像仓库导航、内部实现优化或测试归属，先按 `../shared/questionRouter.md` 分流，不强行在本技能内回答。
3. 判断用户是否“新手”：若是，先用 `references/beginnerPrimer.md` 解释术语与最小流程。
4. 用 `references/apiPatterns.md` 选择最小 API 路径。
5. 高频问题优先套用 `references/answerTemplates.md` 的回答骨架，减少遗漏。
6. 输出成功/失败路径与所有权清单。
7. 给出最小验证项（返回值/日志/内存检查点）。

## 约束（必须遵守）
- `Get*` 先判空，再 `RyanJsonIsXXX` 判型。
- 同类型更新优先 `Change*Value`，跨类型更新用 `ReplaceByKey/ReplaceByIndex`。
- `Add/Insert` 只接收游离节点，禁止重复挂载。
- `RyanJsonAddItemToObject` 成功后若继续操作子树，重新从根 `GetObjectToKey` 取容器指针。
- 传输输出优先非格式化打印：`RyanJsonPrint(..., RyanJsonFalse, ...)` 或 `RyanJsonPrintPreallocated(..., RyanJsonFalse, ...)`。
- `RyanJsonMinify` 仅做文本清洗，不作为主输出路径。

## 输出格式
1. 前提与结论：宏/环境前提 + API 选型（若有默认假设必须写明）。
2. 最小代码：只用公开 API，包含失败分支。
3. 所有权清单：逐分支说明谁负责释放。
4. 验证建议：最小验证步骤与后续扩展方向。
> 面向新手时，在第 1 段先给“5 步最小流程”，避免术语堆叠。

## 用户问题覆盖（按用户视角路由）
- 如何初始化与注入内存 hooks：`references/hooksInitPolicy.md`、`references/quickstart.md`
- 如何解析 JSON 并读取字段：`references/apiPatterns.md`（A 段）、`references/quickstart.md`
- 如何构建对象/数组并打印：`references/apiPatterns.md`（B/G 段）、`references/quickstart.md`
- 如何同类型更新或跨类型替换：`references/apiPatterns.md`（C/D 段）
- 如何插入/删除/分离节点：`references/apiPatterns.md`（E 段）、`references/ownershipAndErrors.md`
- Add/Insert/Replace 失败谁释放：`references/ownershipAndErrors.md`、`references/quickstart.md`
- 重复 key 如何处理：`references/apiPatterns.md`（F 段）、`RyanJsonStrictObjectKeyCheck`（见共享基线）
- 传输压缩输出与 Minify 区别：`references/apiPatterns.md`（G/H 段）
- RT-Thread 集成与平台差异：`references/rt-thread-examples.md`、`references/integrationTemplate.md`
- 出错/崩溃/边界排查：`references/pitfallsAndDebug.md`、`references/faultInjectionPlaybook.md`
- 高频问答骨架：`references/answerTemplates.md`
- 需要完整 API 说明：`references/apiReference.md`
- 新手术语与概念：`references/beginnerPrimer.md`
- 内部实现与数据结构问题：`../shared/architecture.md`（若超出公开 API 语义，转 `ryanjson-optimization`）

## 示例
- 用户问“RT-Thread 下如何初始化并解析 JSON？”：走本技能，给出最小可运行 API 路径与失败释放责任。
- 用户问“Add/Insert/Replace 失败谁释放？”：走本技能，列出失败路径与所有权清单。
- 用户问“如何用自定义 malloc/free 初始化？”：走本技能，给出 `RyanJsonInitHooks` 初始化顺序与失败处理。
- 用户问“如何把对象打印成紧凑字符串？”：走本技能，给出 `RyanJsonPrint(..., RyanJsonFalse, ...)` 的最小示例。
- 用户问“如何在对象里更新/替换字段类型？”：走本技能，说明 `Change*Value` 与 `ReplaceByKey` 的选择规则。
- 用户问“如何删除或分离节点？”：走本技能，给出 `Delete/Detach` 的使用与释放责任。

## 参考导航
- 共享基线：`../shared/ryanJsonCommon.md`
- 快速分流：`../shared/questionRouter.md`
- 路由样例：`../shared/routingExamples.md`
- 架构与数据结构：`../shared/architecture.md`
- 注释规范：统一用 Doxygen 风格，类型名/字段语义/API 名保持英文（见共享基线第 9 节）
- 新手入门：`references/beginnerPrimer.md`
- API 快速入口：`references/quickstart.md`
- 场景模板：`references/apiPatterns.md`
- 高频回答模板：`references/answerTemplates.md`
- 所有权细则：`references/ownershipAndErrors.md`
- 完整 API：`references/apiReference.md`
- hooks 初始化：`references/hooksInitPolicy.md`
- RT-Thread 示例：`references/rt-thread-examples.md`
- 集成模板：`references/integrationTemplate.md`
- 排障与故障注入：`references/pitfallsAndDebug.md`、`references/faultInjectionPlaybook.md`
- 术语：`references/terminology.md`
- 本地压缩文档：`context.md`、`apiPatterns.md`、`ownership.md`、`sop.md`
