# Fuzzer 测试补充策略

## 范围
- 本页只保留 Fuzzer 特有策略（输入扰动、种子、崩溃处理）。
- 通用交付结构见 `testcaseTemplate.md`。
- 单元测试差异规则见 `unityPlaybook.md`。
- 执行入口、覆盖目录、脚本默认值口径见 `../../shared/ryanJsonCommon.md`。

## 前置初始化（必须）
- 进入 `LLVMFuzzerTestOneInput` 后，先确保 `RyanJsonInitHooks` 已初始化。
- 所有随机创建/替换/打印路径都要走 hooks 分配器。
- 需要故障注入时，通过 hooks 驱动，不直接改核心 API 逻辑。
- fuzz 构建使用专用 target：`xmake -b RyanJsonFuzz`（该目标内置 `isEnableFuzzer` + `-fsanitize=fuzzer`）。
- 覆盖链路统一使用 `scripts/ci/runCoverage.sh`（fuzzer 执行 + `llvm-profdata` + `llvm-cov`）。
- 本地常规优先 `./run_local_fuzz.sh`；只有特殊测试（自定义参数）才直调 `scripts/ci/runCoverage.sh`。

## 脚本参数约定（`scripts/ci/runCoverage.sh`）
- 模式预算：`quick/nightly/full` 分别对应短/中/长预算（具体默认值以 `scripts/ci/runCoverage.sh` 为准）。
- 次数优先级：
  - 设置 `FUZZ_RUNS` 时按固定轮次执行（覆盖 `FUZZ_MAX_TOTAL_TIME`）
  - 未设置 `FUZZ_RUNS` 时按 `FUZZ_MAX_TOTAL_TIME`（或模式默认值）执行
- 关键透传参数：
  - `FUZZ_TIMEOUT`、`FUZZ_MAX_LEN`、`FUZZ_VERBOSITY`
  - `FUZZ_CORPUS_DIR`、`FUZZ_DICT_PATH`
  - `FUZZ_EXTRA_ARGS`
- 构建策略：
  - 默认增量：`XMAKE_FORCE_CLEAN=0`
  - 强制清理：`XMAKE_FORCE_CLEAN=1`（仅在怀疑缓存污染时使用）

## 目标
- 让随机输入触达防御分支，不只跑通 happy path。
- 保证 fuzzer 断言不与可配置语义冲突。
- 在覆盖率提升和热路径成本之间保持平衡，避免为了追 100% 把每轮 fuzz 变成手工回归脚本。

## 热路径 / SelfTest 分工
- 热路径只保留与当前输入强相关的行为，例如：
  - parse 成功后的 print/get/modify/create 等运行期链路
  - 当前节点值参与断言的 Add* / Change* / typed array builder 成功路径
  - 与当前输入结构直接相关的递归遍历、替换、删除、分离
- `SelfTestOnce` 只保留以下两类：
  - 与输入无关的固定 guard（空指针、非法 hooks 组合、固定 contract）
  - 运行期 harness 结构上难以稳定构造的内部状态或极端边界
- 若某条路径“数学上可由 fuzz 命中”，默认不放 self-test；先考虑：
  - 增加 runs
  - 扩 corpus
  - 调整运行期触发点，但仍留在热路径

## 手动覆盖准入规则
- 先看覆盖报告中未命中的具体 branch，再定位源文件与行号。
- 再判断该 branch 是：
  - 结构上难以稳定命中
  - 预算下难以合理命中
  - 其实运行期可达，只是当前用例被误删或降频过度
- 只有前两类才允许加最小 self-test；第三类应恢复运行期 fuzz 逻辑，而不是新增手动覆盖。
- 若某文件历史上已到 100% 分支覆盖，修改后必须优先与基线或暂存区对比，解释覆盖下降原因。

## 输入策略
- 结构扰动：对象/数组嵌套、重复 key、空 key、混合类型。
- 数值扰动：超长整数、小数尾长、指数边界、非法符号。
- 字符串扰动：转义截断、非法 Unicode、代理对不完整。
- 状态扰动：parse 成功后执行 create/replace/detach/delete 组合。

## 覆盖回归建议顺序
1. 先跑默认 `./run_local_fuzz.sh`，以当前默认 100000 次为基线。
2. 看 `test/fuzzer/coverage/report.txt`，确认是总覆盖下降还是单文件回退。
3. 再看 HTML branch 明细，找出具体未命中的 true/false 分支。
4. 对比当前工作区与暂存区/上一个稳定基线，确认是不是误删了运行期逻辑。
5. 只有确认运行期逻辑无法合理覆盖后，再提议最小 self-test。

## 断言策略
- 避免对“可能合法也可能非法”的输入写硬断言。
- 对宏控制行为使用条件断言。
- 崩溃优先级高于语义偏差；先修内存安全，再修预期一致性。
- 所有权断言要区分：Add/Insert 失败（游离 item 由库侧清理）与 Replace 失败（item 不消费）。
- `fuzzTestWithMemFail` 只用于稳定验证后置状态，不用于把整个运行期路径偷搬进 self-test。

## 覆盖策略
- 为目标分支准备定向种子（不仅依赖随机变异）。
- 将历史 crash 文件固化到 corpus。
- 对新增防御分支记录触达证据（日志或覆盖率）。
- merge corpus 时默认就地更新原目录；只有在用户要求隔离输出时才额外保留临时目录概念。

## 与单元测试分工
- 单元测试负责精确语义与回归基准。
- fuzzer 负责发现未知组合路径与内存错误。

## 依据（仓库内）
- `test/fuzzer/entry.c`：`LLVMFuzzerTestOneInput` 中 hooks 初始化与断言
- `test/fuzzer/utils/fuzzerSelfTest.c`：一次性自检的归属边界
- `xmake.lua`：`target("RyanJsonFuzz")` 的 fuzz 宏与 `-fsanitize=fuzzer` 配置
- `scripts/ci/runCoverage.sh`：fuzz 覆盖执行链与参数默认值
- `test/fuzzer/cases/fuzzerCreate.c`：Add/Insert 失败与游离态分支
- `test/fuzzer/cases/fuzzerParse.c`：超长数字溢出与大输入预算控制
- `test/fuzzer/cases/fuzzerReplace.c`：Replace 失败不消费 `item`
