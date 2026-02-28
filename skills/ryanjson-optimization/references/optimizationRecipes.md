# 优化策略库（可直接复用）

## 范围
- 本页只保留“可复用配方”。
- 执行顺序见 `coreWorkflow.md`。
- 交付结构见 `optimizationTemplate.md`。

## 配方 1：先修语义再提速
适用：崩溃、泄漏、误判与性能问题同时存在。

步骤：
1. 先修内存安全/语义一致性。
2. 用最小输入补单测回归。
3. 再做性能微调（减少重复查找、减少无效分支）。

## 配方 2：零额外内存优化
适用：用户明确“不能增加额外内存”。

优先手段：
- 减少重复计算与重复遍历。
- 改善分支顺序，提高常见路径命中。
- 缩短失败路径，减少临时对象生命周期。

避免：
- 引入缓存表或哈希索引（除非授权）。

## 配方 3：Compare 逻辑优化
适用：对象比较高频、深层结构较多。

优先检查：
- 同序快路径是否正确。
- 乱序路径是否 fallback 正确。
- 深度上限和回溯开销是否可控。

## 配方 4：Print 失败路径稳健化
适用：预分配失败、append 失败、double 特殊值场景。

要点：
- 刚好够用边界（`<=`）必须有单测。
- 内部缓冲失败必须释放并返回 NULL。
- `inf/nan` 输出策略需与规范一致并有断言。

## 配方 5：Parse 数值边界防护
适用：覆盖指数溢出、超长小数、非法格式。

要点：
- 分阶段防溢出（整数、小数、指数分别保护）。
- 避免前置判断吞掉目标分支。
- 用单测 + fuzz 双路径验证。

## 当前项目冻结项（默认不改）
以下是当前已确认的项目决策，除非用户明确要求，否则不要主动推动改动：
- `RyanJsonParse` 默认保持非严格尾部语义（不改为 strict default）。
- Compare 相关“纯性能优化”（不改变行为）当前优先级低，默认只做正确性修复。
- 字符串 `\uXXXX` 预扫长度的进一步内存微优化（更复杂的精算）当前不做。

补充约束：
- Compare 的结构破坏路径按“内部不变量损坏”处理；启用 `RyanJsonEnableAssert` 时可走 assert。

## 依据（仓库内）
- `RyanJson/RyanJsonParse.c`：Parse 默认尾部语义与数值路径
- `RyanJson/RyanJsonPrint.c`、`test/unityTest/cases/utils/testPrint.c`：Print 预分配边界与失败路径
- `RyanJson/RyanJson.c`：Minify 行为与 Compare 入口
- `test/unityTest/cases/core/testReplace.c`、`test/unityTest/cases/core/testCreate.c`：核心失败语义回归
