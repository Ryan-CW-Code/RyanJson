# 覆盖率与回归分诊

## 范围
- 本页仅覆盖“覆盖率诊断方法”。
- 用例写法与证据结构见 `testcaseTemplate.md`。
- 执行入口、模式隔离、覆盖目录口径见 `../../shared/ryanJsonCommon.md`。

## 分诊常用命令
- unit 覆盖（全矩阵）：`UNIT_MODE=full UNIT_SKIP_COV=0 bash scripts/ci/runBaseCoverage.sh`
- fuzz 覆盖（夜间预算）：`FUZZ_MODE=nightly FUZZ_SKIP_COV=0 bash scripts/ci/runCoverage.sh`

## 解读原则
- 行覆盖高不代表质量高；优先看关键分支的 true/false 是否都触发。
- 对 `RyanJsonCheckReturnFalse` 这类宏，重点看失败分支是否有证据。
- 覆盖率不能替代语义断言与泄漏检查。

## 定位流程
1. 锁定未覆盖行对应函数和前置条件。
2. 判断是否被更早分支拦截（前置校验/类型判断/宏条件）。
3. 先写确定性单测触达，再用 fuzz 扩展组合路径。
4. 复跑覆盖率，验证 true/false 双向都触发。

## 典型问题定位
- 分支只走 false：通常是输入构造不到位，或前置判断提前拦截。
- 分支只走 true：可能是防御过强或路径被随机策略短路。
- 覆盖下降但测试通过：检查是否引入不可达代码或常真/常假条件。

## 回归闭环
1. 从失败日志还原最小输入。
2. 先加确定性单测，再加 fuzzer 变体触发。
3. 复跑覆盖率，确认目标分支双向触发。
4. 固化 crash 样本到 corpus，防止回归。

## 定位技巧（当前脚本行为）
- unit 报告是多组 `profraw` 合并后的总览。
- fuzz 会先在终端彩色打印 `llvm-cov report`，再写入文本报告。
- 如果只做快速真假分支定位，可先 `UNIT_MODE=quick` 或 `FUZZ_MODE=quick` 缩短反馈周期，再切 full/nightly 做最终证据。

## 不应强求覆盖的路径
- 仅在内存破坏下可达的 assert 路径。
- 平台相关、编译选项关闭的不可达分支。
