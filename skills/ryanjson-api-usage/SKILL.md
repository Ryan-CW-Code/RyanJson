---
name: ryanjson-api-usage
description: 面向 RyanJson 公开 API 的 RT-Thread 落地技能。用于 Parse/Create/Add/Change/Replace/Detach/Delete、所有权与释放语义、以及 RyanJsonInitHooks 初始化集成。用户请求“RyanJson 怎么用”“RT-Thread 怎么集成”“失败时谁释放”时使用本技能。
---

# RyanJson API 使用技能

## 技能定位
- 面向公开 API 的“怎么用”问题，输出可直接落地的调用方案与代码片段。
- 默认面向 RT-Thread 集成场景，强调初始化、失败回退和释放责任。

## 适用与切换
- 适用：接口选型、调用顺序、所有权判断、集成模板。
- 切换到 `ryanjson-test-engineering`：当目标是补测试、补覆盖、复现崩溃。
- 切换到 `ryanjson-optimization`：当目标是内部实现、性能优化、回归门禁。

## 必读入口
- 共享基线：`../shared/ryanJsonCommon.md`
- 注释规范：统一使用 Doxygen 风格，且类型名/字段语义名/API 名保持英文（见共享基线第 9 节）
- API 快速入口：`references/quickstart.md`
- 场景模板：`references/apiPatterns.md`
- 所有权细则：`references/ownershipAndErrors.md`

## 执行入口
- 代码规范：先执行 `bash ./run_local_format.sh --check --changed`，提交前执行 `bash ./run_local_format.sh`。
- 行为回归：默认先跑 `bash ./run_local_base.sh`；涉及硬件语义（对齐/异常）再跑 `bash ./run_local_qemu.sh`；需要联动 fuzz 时执行 `bash ./run_local_ci.sh` 或 `bash ./run_local_fuzz.sh`。

## 执行流程
1. 先按 `../shared/ryanJsonCommon.md` 确认宏与语义前提。
2. 按用户场景选最小 API 组合（读取/构建/更新/替换/迁移）。
3. 给出“成功路径 + 失败路径”的最小代码，不夹带内部实现细节。
4. 明确分支级所有权：创建方、接管点、失败后释放责任。
5. 给出可验证项：返回值、日志、内存统计或泄漏检查点。

## API 专项约束
- `Get*` 先判空，再 `RyanJsonIsXXX` 判型。
- 同类型更新优先 `Change*Value`，跨类型更新使用 `ReplaceByKey/ReplaceByIndex`。
- `Add/Insert` 仅接收游离节点，禁止重复挂载。
- 传输压缩优先非格式化打印：`RyanJsonPrint(..., RyanJsonFalse, ...)` 或 `RyanJsonPrintPreallocated(..., RyanJsonFalse, ...)`。
- `RyanJsonMinify` 是文本清洗工具，不作为传输输出主路径。

## 输出格式
1. 前提与结论：宏/环境前提 + API 选型。
2. 最小代码：只用公开 API，包含失败分支。
3. 所有权清单：逐分支说明谁负责释放。
4. 验证建议：最小验证步骤与后续扩展方向。

## 参考导航
- 完整 API：`references/apiReference.md`
- hooks 初始化：`references/hooksInitPolicy.md`
- RT-Thread 示例：`references/rtThreadExamples.md`
- 集成模板：`references/integrationTemplate.md`
- 排障与故障注入：`references/pitfallsAndDebug.md`、`references/faultInjectionPlaybook.md`
- 术语：`references/terminology.md`
- 本地压缩文档：`context.md`、`apiPatterns.md`、`ownership.md`、`sop.md`
