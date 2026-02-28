# RyanJson 核心优化工作流

## 范围
- 本页仅覆盖“优化执行流程”。
- 交付结构见 `optimizationTemplate.md`。
- 模块风险细节见 `moduleHotspots.md`。
- 执行入口、覆盖目录、脚本参数基线见 `../../shared/ryanJsonCommon.md`。

## 0) 前置条件（必须）
- 在任何业务入口、测试入口、fuzzer 入口先调用 `RyanJsonInitHooks`。
- 默认建议使用统一封装分配器（即便最终仍转发到 `malloc/free/realloc`），便于统计和故障注入。
- 未初始化 hooks 时，不执行优化结论对比（避免基线不可比）。
- 当前实现没有默认 hooks 回退（全局 hooks 初始为 `NULL`），因此该前置条件是硬约束。

## 1) 定义问题边界
- 明确是“行为错误修复”还是“性能/内存优化”。
- 明确是否允许改变历史行为（默认不允许）。
- 明确目标平台（32 位/64 位、是否 RTOS、是否启用 ASan/UBSan）。

## 2) 建立可比较基线
- 功能：跑与改动模块相关的单元测试。
- 资源：记录内存峰值、分配次数、失败路径泄漏结果。
- 稳定性：保留至少一个历史 crash 样本回归。
- 覆盖：确认目标分支 true/false 双向是否可达。

## 2.1) 回归入口策略
- 本地常规回归：先跑 `run_local_*`。
- 需要细调矩阵/并发/覆盖时，再直调 `scripts/ci/*`。

## 3) 聚焦热点模块
- `RyanJsonParse.c`：数字解析、字符串转义、Unicode 代理对、状态推进。
- `RyanJsonPrint.c`：预分配边界、append 失败路径、double 打印策略。
- `RyanJsonItem.c`：Add/Insert/Replace/Detach/Delete 的所有权与链安全。
- `RyanJson.c`：Compare 有序/无序路径、深度与回溯。

## 4) 设计最小改动
- 优先修复内存安全与语义一致性，再做微优化。
- 新防御逻辑要区分可恢复错误（返回 false/NULL）与不可恢复错误（启用 `RyanJsonEnableAssert` 时可 assert）。
- 避免引入额外常驻内存；若必须引入，先得到明确授权。

## 5) 同步测试策略
- 不新增散文件，补到现有分类测试。
- 单元测试覆盖确定性语义，fuzzer 覆盖组合路径与未知输入。
- 新增每个分支至少一个触发样本，必要时固化到 corpus。

## 6) 交付与复核
- 给出改动原因、收益、代价、兼容影响。
- 给出“已验证/未验证”边界。
- 给出剩余风险和下一轮优化建议。

## 常见反模式
- 只看覆盖率，不看语义断言。
- 通过 assert 处理用户输入错误。
- 为了覆盖率引入会改变行为的冗余逻辑。

## 依据（仓库内）
- `RyanJson/RyanJson.c`：`RyanJsonInitHooks` 与全局 hooks 初始状态
- `RyanJson/RyanJson.h`：`Add/Insert`、`Replace` 失败语义注释
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`：失败所有权与宏分支断言
- `test/fuzzer/entry.c`：fuzzer 入口与稳定性回归链路
- `../../shared/ryanJsonCommon.md`：统一执行入口与覆盖口径
