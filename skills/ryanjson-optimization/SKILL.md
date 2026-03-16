---
name: ryanjson-optimization
description: RyanJson 核心实现的正确性优先优化技能。用于 Parse/Print/Item/Compare 优化、宏语义设计、内存与性能权衡、以及 crash/覆盖率回归闭环；用户请求“核心优化”“最小内存提效”“不改 API 前提下优化”时使用。接口用法转 ryanjson-api-usage，测试补强转 ryanjson-test-engineering。
---

# RyanJson 核心优化技能

## 目标
- 在不破坏契约的前提下提升性能、内存或稳定性。
- 不讨论公开 API 的用法咨询与测试补强。

## 必要输入
- 优先级：性能/内存/稳定性/代码尺寸。
- 边界：是否允许行为变化、是否允许新增宏、RAM/ROM 预算。
- 门禁：必须通过的测试与覆盖范围。

## 核心流程
1. 先读 `../shared/ryanJsonCommon.md`，锁定宏与语义边界。
2. 建立基线：正确性、耗时、内存峰值、崩溃样本、覆盖率。
3. 设计最小改动：只改目标路径，避免顺手重构。
4. 增量验证：先定向回归，再扩到全量门禁。
5. 输出结论：收益、代价、兼容影响、剩余风险与回滚条件。

## 约束（必须遵守）
- 未授权不得改变公开 API 语义或默认行为。
- 禁止引入依赖常驻额外节点开销的侵入式提速。
- `Add/Insert/Replace` 失败语义必须与头文件和测试一致。
- 宏语义变更需同步代码、测试与文档。
- 涉及对齐/异常路径时，必须补跑 QEMU 链路确认 `UNALIGN_TRP + HardFault` 行为。

## 输出格式
1. 结论：优先级与目标达成情况（已验证/推断）。
2. 方案：文件/函数级最小改动点与原因。
3. 证据：unit、fuzz、覆盖、内存与性能数据。
4. 风险：未覆盖边界、回滚触发条件、下一步建议。

## 示例
- 用户问“解析性能慢，不改 API 前提下怎么优化？”：走本技能，给出最小改动与验证门禁。
- 用户问“内存峰值过高，允许新增宏开关”：走本技能，评估兼容影响与回归链路。

## 参考导航
- 共享基线：`../shared/ryanJsonCommon.md`
- 架构与数据结构：`../shared/architecture.md`
- 注释规范：统一用 Doxygen 风格，类型名/字段语义/API 名保持英文（见共享基线第 9 节）
- 工作流：`references/coreWorkflow.md`
- 门禁：`references/regressionGates.md`
- 输出模板：`references/optimizationTemplate.md`
- 核心结构：`references/coreArchitecture.md`
- 模块热点：`references/moduleHotspots.md`
- 配置开关：`references/configSwitches.md`
- 优化配方：`references/optimizationRecipes.md`
- 术语：`references/terminology.md`
- Gemini 对齐：`references/geminiCompat.md`
- 本地压缩文档：`architecture.md`、`pitfalls.md`、`sop.md`
