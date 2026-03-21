---
name: ryanjson-test-engineering
description: RyanJson 单元测试工程与审查。用于补单测、去重审查、用例归属调整、test-map 更新与回归策略；用户问“这类契约该测在哪/现有测试是否重复/如何补最小回归”时使用。仅在明确要求时处理 fuzz 回归或崩溃复现。接口选型转 ryanjson-api-usage，内部优化转 ryanjson-optimization。
---

# RyanJson 测试工程技能

## 目标
- 构建确定性 unit 测试，覆盖特殊条件下的纯契约与失败语义。
- 避免矩阵式重复与“换字面量”堆叠。

## 必要输入
- 目标 API 或契约（Parse/Print/Item/Compare/Utils 等）。
- 期望行为或已知回归。
- 若是 fuzz 回归：最小复现样本与日志。

## 核心流程
1. 先读 `../shared/ryanJsonCommon.md`，确认宏与失败语义基线。
2. 查 `references/test-map.md` 定位归属，先做去重扫描。
3. 选唯一契约写最小确定性测试；复杂链路优先 `Parse` 固化结构。
4. 变更职责或文件归属时同步更新 `references/test-map.md`。
5. 仅在用户明确要求时进入 fuzz 流程（见 `references/fuzzerPlaybook.md`）。
6. 回归脚本入口按共享基线执行。

## 去重检查清单
- 在目标目录与全树用 `rg -n` 搜索 API/字面量，确认无同语义覆盖。
- RFC8259 与 `cases/equality/*` 作为基线套件，不做去重。
- 若冲突，合并到正确归属或删除重复。

## 约束（必须遵守）
- 最高准则：新增/改动单元测试绝对不允许与现有测试形成功能语义重复；即使断言写法不同、字面量不同、链路长短不同，只要证明的是同一契约，就必须合并、搬运到正确归属文件，或直接删除。
- unit 与 fuzz 必须隔离，fuzz 仅在明确要求时介入。
- 不新建零散文件，单文件不超过 1000 行。
- 测试注释用中文，类型名/API 名保持英文。
- 复杂链路优先用 `Parse` 固定初始结构。
- `RyanJsonAddItemToObject` 成功后若继续操作子容器，需从 root 重新 `GetObjectToKey` 取回指针再用。
- `RyanJsonDefaultAddAtHead` 影响插入顺序，断言优先用 key/size。
- 非严格模式允许重复 key，`CompareOnlyKey` 必须校验重复次数。
- `test/unityTest/runner/test_list.inc` 由 `run_local_base.sh` 自动生成，禁止手改；新增/删除 runner 后运行脚本或 `./run_local_base.sh`。

## 输出格式
1. 目标与契约：明确测试证明的行为。
2. 改动清单：文件、用例函数、场景意图。
3. 验证证据：执行模式与结果。
4. 风险与下一步：仍缺失的独立契约。

## 示例
- 用户问“Add/Insert 失败语义缺少单测”：走本技能，补最小确定性测试并去重。
- 用户问“fuzz 崩溃想转为 unit 回归”：走本技能，按复现样本收敛成单测（仅在明确要求时）。

## 参考导航
- 共享基线：`../shared/ryanJsonCommon.md`
- 架构与数据结构：`../shared/architecture.md`
- 注释规范：公共文档注释按共享基线执行；测试内部注释优先用中文说明场景/契约，类型名与 API 名保持英文（见共享基线第 9 节）
- 断言与分层：`references/unityPlaybook.md`
- 覆盖率分诊：`references/coverageTriage.md`
- fuzz 补测：`references/fuzzerPlaybook.md`
- 分诊与回归：`references/regressionMatrix.md`
- 输出模板：`references/testcaseTemplate.md`
- 架构检查点：`references/coreArchitectureCheckpoints.md`
- 断言策略：`references/assertPolicy.md`
- 术语：`references/terminology.md`
- 用例地图：`references/test-map.md`
- 本地压缩文档：`testArchitecture.md`、`context.md`、`sop.md`
