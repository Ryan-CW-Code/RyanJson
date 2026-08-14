---
name: ryanjson-test-engineering
description: 设计、审查、实现并验证 RyanJson 的确定性 unit 测试、去重治理、用例归属、test-map、fuzz 回归和验证矩阵。用于“该测什么、放在哪里、是否重复、怎样把缺陷变成最小回归”等测试工程问题。
---

# RyanJson 测试工程

用最小、确定且不重复的证据固定契约。测试的职责是证明独立行为，不是堆叠相似输入或替代根因分析。

## 阅读顺序

1. 阅读 [行为契约](../shared/behavior-contracts.md)。
2. 阅读 [测试设计](references/test-design.md) 和 [用例地图](references/test-map.md)。
3. 根据任务加载 [回归与覆盖](references/regression-and-coverage.md) 或 [fuzz 回归](references/fuzz-regression.md)。
4. 内部结构敏感问题再读 [内部架构](../shared/architecture.md)。

## 补测循环

1. 写明要证明的一条契约和触发它的最小前置状态。
2. 在目标目录与全树搜索 API、关键输入、断言和同类失败路径，判断现有覆盖是否已证明同一契约。
3. 按 test-map 选择唯一归属文件。发现重复时合并、搬运或删除，不能并存。
4. 为成功、失败、边界和所有权分别选择必要的最小断言；复杂初始树优先用 Parse 固化。
5. 检查宏敏感性、runner 注册和资源清理，运行最窄 runner。
6. 根据行为影响扩大 unit、QEMU、fuzz 或覆盖验证，并记录确切证据。

## 强制规则

- 新增或改动的 unit 测试不得与现有测试证明同一功能语义；字面量、断言写法或链路长短不同不构成独立契约。
- unit 证明确定性行为；fuzz 只在任务明确涉及异常输入、历史 crash 或稳定性时进入。
- 优先扩展既有归属文件，不能为单一断言新建零散文件；单个测试文件不超过 1000 行。
- `test/unityTest/runner/test_list.inc` 由 `run_local_base.sh` 生成，禁止手改。
- `RyanJsonDefaultAddAtHead` 会影响顺序；断言优先使用 key、size 或显式插入语义。
- `Add/Insert` 与 `Replace` 的失败所有权必须用不同断言证明。

## 参考资料

- [测试设计](references/test-design.md)
- [用例地图](references/test-map.md)
- [回归与覆盖](references/regression-and-coverage.md)
- [fuzz 回归](references/fuzz-regression.md)

## 交接

- API 契约未明确时，先进入 [API 使用](../ryanjson-api-usage/SKILL.md)。
- crash 根因、内部不变量、性能或布局风险未明确时，先进入 [核心工程](../ryanjson-core-engineering/SKILL.md)。
