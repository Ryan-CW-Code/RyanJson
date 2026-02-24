# RyanJson 测试工作流（压缩版）

## 1. 入口与模式
1. 先确认模式（unit/fuzz）和宏前提。
2. 本地常规回归优先 `run_local_*`。
3. 特殊测试（矩阵/并发/轮次/覆盖）再直调 `scripts/ci/*`。

## 2. 补测循环
1. 锁定未覆盖分支或崩溃信号。
2. 找到阻断分支可达性的 guard。
3. 先加最小确定性 unit 用例。
4. 需要时补 fuzz 样本和触达路径。
5. 复跑并确认双向分支触达。

## 3. 泄漏/崩溃分诊
1. 按所有权、边界、结构不变量分类。
2. 关联到 API 家族（Create/Add/Replace/Detach/Change）。
3. 在失败路径补释放断言。

## 4. 交付格式
1. 根因摘要。
2. 修改的测试文件与函数。
3. 证据（模式、分支、泄漏/崩溃状态、覆盖率报告路径）。
4. 未覆盖边界与下一步补测建议。

## 5. 依据（仓库内）
- `xmake.lua`：`RyanJson` / `RyanJsonFuzz` 模式 target
- `test/unityTest/runner/main.c`：unit 模式入口隔离
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`：失败路径断言样例
- `scripts/ci/runBaseCoverage.sh`：unit 矩阵与合并覆盖率（`coverage/unitMatrix`）
- `scripts/ci/runCoverage.sh`：fuzz 覆盖率复核链路（`coverage/fuzz`）
