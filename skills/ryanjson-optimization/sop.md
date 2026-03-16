# RyanJson 优化工作流（压缩版）

## 1. 先建基线
1. 用当前代码复现问题（正确性/性能/内存）。
2. 采集基线证据（行为+指标）。
3. 先定义验收标准再开始改动。

## 2. 实施循环
1. 做最小局部改动。
2. 先跑定向回归。
3. 再校验失败路径和宏敏感行为。
4. 迭代到目标达成为止。

## 3. 回归顺序
1. 本地常规先跑 `run_local_*`。
2. 需要细调矩阵/并发/覆盖时，直接用 `run_local_*` 传环境变量。
3. 最终门禁覆盖 unit + fuzz + 历史崩溃样本。

## 4. 交付结构
1. 改了什么，为什么改。
2. 收益与代价。
3. 已验证证据与剩余风险。
4. 回滚条件与后续建议。

## 5. 依据（仓库内）
- `xmake.lua`：`RyanJson` / `RyanJsonFuzz` 模式 target
- `run_local_base.sh`、`run_local_ci.sh`、`run_local_fuzz.sh`：本地常规入口
- `run_local_base.sh`：unit 特殊矩阵/覆盖执行链
- `run_local_fuzz.sh`：fuzz 特殊参数/覆盖执行链
- `RyanJson/RyanJsonConfig.h`：宏前提
- `test/unityTest/runner/main.c`：unit 入口模式隔离
