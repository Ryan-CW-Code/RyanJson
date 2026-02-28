# 回归门禁与验收模板

## 范围
- 本页只定义门禁标准与阻塞条件。
- 任务结论写法见 `optimizationTemplate.md`。
- 执行入口与脚本参数基线见 `../../shared/ryanJsonCommon.md`。

## 门禁级别
- Gate-0：改动模块相关单测全部通过。
- Gate-1：全量 unity 通过且无内存泄漏。
- Gate-2：fuzzer 冒烟通过（含历史 crash 样本）。
- Gate-3：关键覆盖分支双向触达。

## 执行入口约定
- 本地常规门禁：优先 `run_local_*`。
- 特殊门禁场景（覆盖率、矩阵细调、并发细调）再直调 `scripts/ci/*`。

## 必验项清单
- Parse：非法输入返回 NULL，无崩溃。
- Item：Add/Insert/Replace/Detach/Delete 所有权一致。
- Print：动态与预分配模式行为一致。
- Compare：有序/乱序路径符合预期。
- 宏开关：严格/非严格语义与文档一致。

## 结果记录模板
- 改动范围：
- 通过的门禁：
- 失败的门禁：
- 是否阻塞合入：
- 临时豁免原因：
- 后续修复计划：

## 回滚触发条件
- 出现 P0 崩溃或内存破坏。
- 语义回归影响现有 API 合同。
- 无法在限制周期内补齐测试证据。

## 依据（仓库内）
- `test/unityTest/runner/main.c`、`test/unityTest/common/testCommon.c`：unity 入口与泄漏检测门禁
- `test/fuzzer/entry.c`：fuzzer 冒烟与回归链路
- `../../shared/ryanJsonCommon.md`：统一执行入口与覆盖口径
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`：Item 所有权回归基础用例
