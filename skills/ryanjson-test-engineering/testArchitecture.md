# RyanJson 测试架构（压缩版）

## 1. 分层
### 1.1 Unity 层（`test/unityTest/`）
- 验证可确定契约行为。
- 使用显式断言和分支目标用例。

### 1.2 Fuzzer 层（`test/fuzzer/`）
- 验证随机/异常输入下的稳健性。
- 核心目标是无崩溃/无泄漏/无 UB，而非固定值断言。

## 2. 放置规则
- 优先扩展现有分类文件。
- 非必要不新增零散测试文件。
- 分类保持稳定：core/equality/utils/performance/fuzzer。

## 2.1 执行层（脚本职责）
- 脚本入口、模式参数、覆盖率目录口径统一见 `../shared/ryanJsonCommon.md`。
- 本页仅强调职责差异：unit 负责确定性语义，fuzz 负责随机路径稳健性。

## 3. 新增测试流程
1. 选定目标分类文件。
2. 添加最小可复现用例。
3. 必要时注册 runner 入口。
4. 在正确模式下验证。
5. 复核覆盖率与所有权断言。

## 4. Fuzzer 流程
1. 增加可复现崩溃种子/corpus。
2. 扩展对应 fuzz case 路径。
3. 复跑确认历史崩溃不再复现。
4. 若有分支目标，再同步覆盖率复核。

## 5. 模式隔离规则
- unit 与 fuzz 的入口和假设不同。
- 禁止把两种模式写成同一条混合执行建议。
- 推荐构建命令：unit 用 `xmake -b RyanJson`，fuzz 用 `xmake -b RyanJsonFuzz`。

## 6. 依据（仓库内）
- `test/unityTest/runner/main.c`：`#ifndef isEnableFuzzer` 的 unit 入口
- `test/fuzzer/entry.c`：`LLVMFuzzerTestOneInput` 入口
- `xmake.lua`：`RyanJson` / `RyanJsonFuzz` 目标配置
- `../shared/ryanJsonCommon.md`：统一执行入口与覆盖口径
