# RyanJson 核心架构（压缩版）

## 1. 作用
- 提供优化前必须掌握的结构心智，避免“改对性能、改坏正确性”。
- 详细结构说明见 `references/coreArchitecture.md`。

## 2. 节点与 Payload 心智
- RyanJson 节点是紧凑模型，运行行为依赖内部负载布局。
- 优化时不能按“传统大而全节点结构”做假设。
- 未经授权不要引入会增加常驻节点开销的结构改动。

## 3. 链接与遍历约束
- 结构编辑（`Detach/Replace/Delete`）必须保持遍历不变量。
- 任何“简化链接重接”的改动都必须配套回归覆盖。

## 4. 字符串路径约束
- 字符串处理存在短路径与动态路径切换。
- 短长路径切换的更新逻辑必须保证无泄漏。

## 5. 分配器基线
- 内部分配依赖 hooks；未对齐 hooks 的优化验证结果无效。

## 6. 优化守则
1. 先正确再提速。
2. 只做最小必要改动。
3. 先验证失败路径，再验证热点路径。

## 7. 依据（仓库内）
- `RyanJson/RyanJson.c`：hooks 全局指针与初始化
- `RyanJson/RyanJsonItem.c`：Detach/Replace/Delete 链与所有权路径
- `RyanJson/RyanJsonConfig.h`：宏前提（StrictKey / AddAtHead）
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`
