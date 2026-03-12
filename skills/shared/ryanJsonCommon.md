# RyanJson 共性规则

## 1. 范围
- 本文件是以下三个技能的共性约束基线：
  - `skills/ryanjson-api-usage`
  - `skills/ryanjson-optimization`
  - `skills/ryanjson-test-engineering`
- 各技能的领域细节仍以各自 `references/` 文档为准。
- 统一术语字典：`terminology.md`。

## 2. 仓库事实（必须遵守）
- 主机侧主入口是 `xmake`。
- 当前仓库通过 target 区分模式：
  - `RyanJson`：默认 unit 目标（不注入 libFuzzer main）
  - `RyanJsonFuzz`：专用 fuzz 目标（启 `isEnableFuzzer` + `-fsanitize=fuzzer`）
  - `RyanJsonQemu` / `RyanJsonQemuCm3`：QEMU 目标（FreeRTOS Cortex-M，含非对齐访问 fault 校验）
- QEMU 非对齐基线（默认约束）：
  - 运行时开启 `SCB->CCR.UNALIGN_TRP=1`，以硬件语义捕获非对齐访问。
  - 不在 QEMU 目标上启用 `-mno-unaligned-access`，避免编译器辅助对齐掩盖真实语义。
  - `YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS=1` 作为第三方库局部防御开关保留，除非用户明确要求调整。
- 覆盖脚本分工：
  - `scripts/ci/runBaseCoverage.sh`：单元测试矩阵（`quick=2` / `nightly=4` / `full=8`）
  - `scripts/ci/runCoverage.sh`：fuzzer 执行与覆盖率生成
- Unity runner 列表自动生成：`scripts/ci/checkUnityRunnerList.sh` 生成 `test/unityTest/runner/test_list.inc`，由 `scripts/ci/runBaseCoverage.sh` 调用；不要手改 `test_list.inc`，新增/删除 runner 后运行脚本或 `run_local_base.sh`。
- 本地便捷入口在仓库根目录：
  - `run_local_base.sh`：本地一键 unit 矩阵
  - `run_local_qemu.sh`：本地一键 QEMU 矩阵（默认 full，覆盖 localbase 用例并校验对齐异常）
  - `run_local_ci.sh`：本地模拟 `ci-pr`（unit + quick fuzz）
  - `run_local_fuzz.sh`：本地固定 6 并发 fuzz
  - 默认值摘要：
    - `run_local_base.sh`：`UNIT_MODE=full`、`UNIT_SKIP_COV=1`
    - `run_local_qemu.sh`：`QEMU_MODE=full`、`QEMU_STOP_ON_FAIL=1`
    - `run_local_ci.sh`：full unit + quick fuzz（`FUZZ_SKIP_COV=1`）
    - `run_local_fuzz.sh`：`FUZZ_RUNS=10000`、`FUZZ_WORKERS/JOBS=6`
  - `run_local_qemu.sh` 默认保留 ANSI 颜色输出；仅在用户明确要求“去色/净化日志”时再剥离控制符。
- 覆盖率目录固定且每次执行前清理（仅保留最新结果）：
  - unit：`coverage/unitMatrix`（`report.txt` + `html/`）
  - fuzz：`coverage/fuzz`（`report.txt` + `html/`）
- `Makefile` 为历史辅助，不是当前主流程。
- `SConscript` 主要用于 RT-Thread 软件包集成。

### 2.1 依据（仓库内）
- `xmake.lua`：`target("RyanJson")` 与 `target("RyanJsonFuzz")` 的模式分离
- `xmake.lua`：`RyanJsonFuzz` 中 `add_defines("isEnableFuzzer")` 与 `-fsanitize=fuzzer`
- `xmake.lua`：`RyanJson` 为链接兼容补入 `test/fuzzer/utils/fuzzerDriver.c`、`fuzzerMemory.c`（不启用 fuzz sanitizer）
- `test/unityTest/runner/main.c`：`#ifndef isEnableFuzzer` 包裹 Unity `main`
- `scripts/ci/runBaseCoverage.sh`：unit 矩阵执行与合并覆盖率
- `scripts/ci/runCoverage.sh`：构建/执行 `RyanJsonFuzz` + `llvm-cov`
- `scripts/ci/checkUnityRunnerList.sh`：自动生成 `test/unityTest/runner/test_list.inc`
- `run_local_base.sh`、`run_local_qemu.sh`、`run_local_ci.sh`、`run_local_fuzz.sh`：本地入口封装
- `test/qemu/platform/qemuStartup.c`：`SCB->CCR.UNALIGN_TRP=1` 的运行时设置与启动日志
- `xmake.lua`：QEMU target 保留 `YYJSON_DISABLE_UNALIGNED_MEMORY_ACCESS=1`，不启用 `-mno-unaligned-access`
- `run_local_qemu.sh`：QEMU 流清洗默认保留 ANSI `ESC`（颜色输出）

## 3. 语义取证链（不确定时必须执行）
1. `example/`
2. `test/unityTest/`
3. `test/fuzzer/`
- 取证不完整时，结论必须标注为“推断”。

## 3.1 证据优先（回答必须可追溯）
- 涉及语义细节时（如 `Print/Minify`、`Add/Insert/Replace` 失败所有权、宏分支行为），优先依据当前源码与单元测试。
- 默认证据顺序：
  1. 头文件与实现（`RyanJson/*.h`、`RyanJson/*.c`）
  2. 单元测试（`test/unityTest/`）
  3. 模糊测试（`test/fuzzer/`）
- 输出建议中应给出至少一个可追溯依据路径；没有依据时必须明确写“推断”。

## 4. 宏基线检查（回答前必须做）
- 必查当前源码中的：
  - `RyanJsonStrictObjectKeyCheck`
  - `RyanJsonDefaultAddAtHead`
- 当前仓库默认值（仅作初始参考，最终以当前源码为准）：
  - `RyanJsonStrictObjectKeyCheck=false`
  - `RyanJsonDefaultAddAtHead=false`

## 5. 所有权共识（统一口径）
- `Create*` 成功：节点归调用方，直到挂载或显式释放。
- `Detach*` 成功：返回节点归调用方。
- `Add/Insert` 与 `Replace` 的失败语义不得混用。
- 当前实现中：
  - `Add/Insert` 失败时，游离 `item` 走库侧清理；非游离 `item` 返回失败但不释放（保护原树）。
  - `Replace` 失败不消费新节点，调用方需复用或释放。
- 对边界语义有疑问时，必须回查当前头文件与测试。

### 5.1 依据（仓库内）
- `RyanJson/RyanJsonItem.c`：`RyanJsonInsert` 失败语义（游离 `item` 走 `error__` 删除，非游离 `item` 早返回不删除）
- `RyanJson/RyanJsonItem.c`：`RyanJsonReplaceByKey/ByIndex` 失败路径返回 false，不删除新 `item`
- `test/unityTest/cases/core/testCreate.c`：已挂树节点被拒绝插入
- `test/unityTest/cases/core/testReplace.c`：Replace 失败后 `item` 仍可复用并由调用方释放

## 6. 测试模式契约
- unit：走 Unity `main` 入口，不允许 fuzz sanitizer 注入 `main`。
- qemu：走 `RyanJsonQemu*` 目标，校验 FreeRTOS 调度与非对齐访问 fault 语义。
- fuzz：按 fuzz 宏/编译参数构建，使用 `scripts/ci/runCoverage.sh` 进行覆盖链路。
- 禁止在同一条执行建议中混写 unit/qemu/fuzz 的前提。
- 推荐命令：
  - unit：`xmake -b RyanJson`
  - qemu：`xmake -b RyanJsonQemu`
  - fuzz：`xmake -b RyanJsonFuzz`

### 6.1 脚本调用建议（优先）
- 本地优先：直接运行根目录入口
  - `./run_local_base.sh`
  - `./run_local_qemu.sh`
  - `./run_local_ci.sh`
  - `./run_local_fuzz.sh`
- CI/细粒度调参时再直调 `scripts/ci/*`
  - unit 快检：`UNIT_MODE=quick UNIT_SKIP_COV=1 bash scripts/ci/runBaseCoverage.sh`
  - unit 全矩阵：`UNIT_MODE=full bash scripts/ci/runBaseCoverage.sh`
  - fuzz 快检：`FUZZ_MODE=quick FUZZ_SKIP_COV=1 bash scripts/ci/runCoverage.sh`
  - fuzz 覆盖：`FUZZ_MODE=nightly FUZZ_SKIP_COV=0 bash scripts/ci/runCoverage.sh`
- `XMAKE_FORCE_CLEAN=1` 仅在怀疑配置缓存污染时启用；默认增量模式更快。
- `scripts/ci/runCoverage.sh` 会先把 `llvm-cov report --use-color` 输出到终端，再写入 `coverage/fuzz/report.txt`。
- RFC8259 用例列表通过目录扫描（`readdir`）获取，不再维护 `rfc8259_filelist.inc`。

## 7. 输出契约
- 输出必须显式区分：
  - 已验证事实
  - 推断结论
  - 可恢复错误/不可恢复错误
- 代码建议必须包含：
  - 前置条件
  - 成功路径
  - 失败路径
  - 所有权与释放责任

## 8. 代码与审查基线
- 类型统一 `stdint`，命名统一小驼峰。
- 示例中避免魔法数字，优先使用命名常量。
- 三目运算符（`?:`）可用于“无副作用、单行、明显更清晰”的场景；复杂分支统一使用 `if/else`。
- 禁止把未验证内部行为当作确定事实输出。
- API 使用类问题默认限制在公开接口，除非用户明确要求内部实现。

## 9. 注释规范（Doxygen）
- 配置宏、公开接口、关键内部函数的新增/改动注释，统一使用 Doxygen 风格。
- 首行必须使用 `@brief` 描述“这是什么 + 做什么”。
- 存在边界或默认行为时，补 `@note`；涉及原理或公式时，补 `@details`。
- 禁止使用序号式注释前缀：如 `1.`、`2)`、`3.1`、`A.`、`Stage 1:`；改用语义化短句标题。
- 参数/返回值注释按需使用：
  - 函数参数：`@param`
  - 返回值：`@return`
- 允许短注释使用 `//`；关键配置、公开接口、复杂逻辑优先使用 Doxygen 块注释。
- Doxygen 注释至少三行，不使用单行 `/** @brief ... */` 形式。
- 注释语言可中文优先，但“类型名 / 字段语义名 / API 名”保持英文原样，不做中文化。
- 强制保留英文示例：
  - 类型名：`Array`、`Object`、`Null`、`Bool`、`Int`、`Double`
  - 字段语义名：`strValue`、`intValue`、`doubleValue`、`boolValue`、`objValue`
  - API/宏名：`Add/Insert/Replace/Detach/Delete`、`RyanJsonAddPosition`、`RyanJsonInlineStringSize`
- 若需中文解释，采用“英文术语 + 中文说明”形式，例如：`strValue（字符串载荷）`。
- 推荐模板：

```c
/**
 * @brief 说明对象与作用。
 * @note 说明默认行为或限制条件。
 * @details 说明推导过程、公式或实现约束（可选）。
 */
```
