# 回归矩阵（测试工程）

## 范围
- 本页只定义回归维度与门禁。
- 问题描述、改动清单、证据输出见 `testcaseTemplate.md`。
- 执行口径与脚本参数基线见 `../../shared/ryanJsonCommon.md`。

## 语义回归
- Parse 成功/失败语义。
- Add/Insert/Replace/Detach/Delete 所有权语义。
- Compare 有序/乱序语义。
- Parse 默认非严格尾部语义（`RyanJsonParse`）与 strict 选项语义（`RyanJsonParseOptions`）。
- Minify 终止符语义：`ret < textLen` 写 `\0`，`ret == textLen` 不写 `\0`。
- Replace 失败语义：调用方负责复用或释放 `item`（库不消费）。

## 稳定性回归
- ASan 崩溃是否消失。
- 泄漏检测是否归零。
- 历史 crash 样本是否稳定通过。

## 覆盖率回归
- 目标分支 true/false 是否都有触达。
- 新增防御分支是否有用例覆盖。
- `RyanJsonInternalCalcLenBytes` 边界：255/256、65535/65536。
- 已达到 100% 分支覆盖的文件是否保持不回退；若回退，必须说明是误删运行期逻辑还是主动收缩手动覆盖。
- fuzz 覆盖审查时是否已对比当前工作区与暂存区/稳定基线，避免误把回退当成“自然波动”。

## 配置回归
- 严格/非严格宏下断言是否一致。
- hooks 初始化路径在各测试入口是否生效。

## 执行策略
- 本地常规：优先 `run_local_*`。
- 特殊回归：按需直调 `run_local_base.sh` / `run_local_fuzz.sh`。
- 推荐分层顺序：每轮增量修改先 `UNIT_MODE=quick`，合并前再 `UNIT_MODE=full`。
- 全量日志建议重定向到临时文件并尾部审阅（例如 `/tmp/*.log`），避免终端截断导致误判。

## 推荐验收门禁
1. 改动模块相关测试全部通过。
2. 全量 unity 通过且无泄漏。
3. fuzzer 冒烟通过并包含历史 crash 样本。
4. 覆盖审阅时保留 unit/fuzz 报告路径（见 shared 约定）。

## 依据（仓库内）
- `RyanJson/RyanJsonParse.c`：`RyanJsonParse` 默认 `requireNullTerminator=RyanJsonFalse`
- `RyanJson/RyanJson.c`：`RyanJsonMinify` 终止符写入条件
- `RyanJson/RyanJsonItem.c`：Insert/Replace 失败语义
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`、`test/unityTest/cases/utils/testUtils.c`
