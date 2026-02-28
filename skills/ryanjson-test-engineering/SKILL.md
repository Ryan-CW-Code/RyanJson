---
name: ryanjson-test-engineering
description: 面向 RyanJson 单元测试与模糊测试的覆盖工程技能。用于按现有分类补充边界/失败路径/所有权断言、修复泄漏与崩溃回归、并提升关键分支触达质量。用户请求“扩展单测”“补覆盖率”“fuzzer 崩溃回归”时使用本技能。
---

# RyanJson 测试工程技能

## 技能定位
- 面向 unit/fuzz 的测试补强与回归闭环。
- 重点是失败路径、边界条件、所有权语义和覆盖率触达质量。

## 必读入口
- 共享基线：`../shared/ryanJsonCommon.md`
- 注释规范：统一使用 Doxygen 风格，且类型名/字段语义名/API 名保持英文（见共享基线第 9 节）
- 断言与分层：`references/unityPlaybook.md`、`references/fuzzerPlaybook.md`
- 分诊与回归：`references/coverageTriage.md`、`references/regressionMatrix.md`
- 输出模板：`references/testcaseTemplate.md`

## 执行入口
- 代码规范：先执行 `bash ./run_local_format.sh --check --changed`，提交前执行 `bash ./run_local_format.sh`。
- 本地常规回归：优先双链路 `run_local_*`。
  - `run_local_base.sh`：默认 full unit（跳过覆盖）
  - `run_local_qemu.sh`：默认 full QEMU 矩阵（完整 localbase 单测 + 非对齐 fault 校验）
  - `run_local_ci.sh`：默认 full unit + quick fuzz
  - `run_local_fuzz.sh`：默认 6 并发 + 固定轮次
- 特殊测试再直调：
  - unit：`scripts/ci/runBaseCoverage.sh`
  - fuzz：`scripts/ci/runCoverage.sh`

## 执行流程
1. 先按 `../shared/ryanJsonCommon.md` 确认宏前提和模式边界。
2. 从日志判定问题类型：崩溃、泄漏、断言失败、分支未命中。
3. 先补最小确定性 unit 用例，再补 fuzz 触达样本。
4. 回归时先走本地入口，必要时用 `scripts/ci/*` 细调参数。
5. 输出证据：分支触达、泄漏状态、崩溃状态、覆盖率路径。

## 测试专项约束
- 不新建零散文件，优先补到现有分类。
- unit/fuzz 必须隔离，禁止把两种模式写成同一条混合建议。
- `Add/Insert` 与 `Replace` 的失败所有权必须分开断言。
- 覆盖率目录为固定路径且每次会清理旧结果，不假设保留历史目录。
- QEMU 链路不再有 basic scope；默认与 localbase 用例集一致（RFC8259 仍留 Linux 快速链路）。
- RFC8259 文件集通过目录扫描（`readdir`）获取，不依赖 `rfc8259_filelist.inc`。

## 输出格式
1. 问题与目标：失败现象 + 目标分支。
2. 改动清单：文件、用例函数、场景意图。
3. 验证证据：执行模式、命中分支、泄漏/崩溃结果、覆盖率产物。
4. 风险与下一步：未覆盖边界与后续补测建议。

## 参考导航
- 架构检查点：`references/coreArchitectureCheckpoints.md`
- 断言策略：`references/assertPolicy.md`
- 术语：`references/terminology.md`
- Gemini 对齐：`references/geminiCompat.md`
- 本地压缩文档：`testArchitecture.md`、`context.md`、`sop.md`
