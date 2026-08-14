---
name: ryanjson-core-engineering
description: 分析、审查、修改并验证 RyanJson 内部实现，包括 Parse、Print、Item、Compare、内存、性能、crash 根因、配置宏与兼容性边界。用于内核代码变更或风险判断，不用于纯公开 API 用法或测试归属决策。
---

# RyanJson 核心工程

修复或优化实现时保持公开契约。任何布局、所有权或宏改动，在证据证明前都视为行为变更。

## 必需上下文

1. 阅读 [行为契约](../shared/behavior-contracts.md)。
2. 改表示、遍历、分配、Parse、Print 或 Compare 前阅读 [内部架构](../shared/architecture.md)。
3. 选择验证方案前阅读 [仓库事实](../shared/repository-facts.md)。
4. 按问题加载下方目标 reference，不默认加载无关配方。

## 调查与变更循环

1. 明确目标是正确性、crash 诊断、性能、内存、代码尺寸还是宏语义，并声明是否允许改公开行为。
2. 复现问题或建立基线，记录输入、配置、环境、可观察结果和测量方法。
3. 查公开契约、责任实现路径及 unit/fuzz/QEMU 证据，写出本次必须保持的不变量。
4. 设计保持不变量的最小改动；未经授权，拒绝以增加常驻每节点开销换取提速。
5. 实现改动；行为、布局、宏或基准解释变化时，同一次更新测试和文档。
6. 先跑定向回归，再按影响扩大 unit、QEMU、fuzz、覆盖率或测量门禁。
7. 交付已验证结果、代价、兼容影响、剩余风险和可执行回滚条件。

## 强制检查

- `Add/Insert` 与 `Replace` 的失败行为必须分开。
- 不能假设最后兄弟节点的 `next` 指向同层节点。
- 改字符串或分配尺寸时，必须覆盖内联与指针存储两条路径。
- 改动或测试顺序敏感行为前，检查 `RyanJsonStrictObjectKeyCheck` 与 `RyanJsonDefaultAddAtHead`。
- 涉及对齐、异常或 FreeRTOS 语义时，必须跑 QEMU 并保持 `UNALIGN_TRP` 语义。

## 常见排查

- Parse 失败或变慢：先把扫描、字符串、数字、插入和回滚拆开，固定最小输入和配置，再建立定向契约。
- Print 容量或格式异常：覆盖恰好容纳、少一个字节、空值、嵌套和数值边界；返回成功前不能使用输出缓冲。
- 字符串或分配异常：覆盖 inline/pointer 双向切换、分配失败和 Delete；通过内部访问器判断 payload 偏移。
- 结构性 crash：在操作前后检查 parent、sibling、`IsLast`；根因明确后才固化为确定性回归。

## 参考资料

- [架构与热点](references/architecture-and-hotspots.md)：模块责任、不变量与审查点。
- [工作流与门禁](references/workflow-and-gates.md)：测量、回归选择和交付证据。
- [配置](references/configuration.md)：宏与兼容性分析。

## 交接

- 应用调用方如何使用公开 API，进入 [API 使用](../ryanjson-api-usage/SKILL.md)。
- 已确认契约后的回归归属或重复测试分析，进入 [测试工程](../ryanjson-test-engineering/SKILL.md)。
