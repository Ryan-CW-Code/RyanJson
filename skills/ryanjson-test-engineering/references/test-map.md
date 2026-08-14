# RyanJson 用例地图

本地图只描述测试的唯一职责。新增或整理测试时，以此定位归属并防止语义重复。

## 运行基础

- `test/unityTest/common/`：hooks、泄漏/OOM 统计、Unity 辅助与平台适配。
- `test/unityTest/runner/main.c`：Unity 入口。
- `test/unityTest/runner/test_list.inc`：自动生成的 runner 清单，禁止手改。
- `test/unityTest/cases/RFC8259/`：内嵌 RFC8259 样本、执行器和辅助比较。

## 主要目录

| 目录 | 唯一职责 | 代表文件 |
| --- | --- | --- |
| `core/` | 核心公开 API 的成功、失败与所有权 | `testCreate.c`、`testReplace.c`、`testDetach.c`、`testLoad*.c` |
| `core/accessor/` | 访问器、路径守护与结构修改后的路径恢复 | `testAccessor*.c` |
| `core/compare/` | Compare、CompareOnlyKey、double 容差和重复 key 比较 | `testCompare*.c` |
| `core/key/` | key 转义、重复、mutation 与 numeric-like key | `testKey*.c` |
| `core/standard/` | 标准 JSON 值、文档流与预处理契约 | `testStandard*.c` |
| `edge/` | key/string 存储边界与 wrap/rebind | `testEdge*.c` |
| `edge/container/` | Insert/Delete/Detach/Replace 的容器守护与恢复 | `testEdgeContainer*.c` |
| `edge/parseprint/` | ParseOptions、Minify slice、Print 容量/样式边界 | `testEdge*.c` |
| `equality/` | Bool/Int/Double/String 的解析-打印往返 | `testEquality*.c` |
| `performance/` | 深层嵌套、内存和大规模压力 | `testDeepRecursion.c`、`testMemory.c`、`testStress.c` |
| `scenario/` | 无法被单 API 文件证明的高耦合业务链路 | `testScenario*.c` |
| `stability/` | 线索链表迁移、churn、遍历和大规模结构稳定性 | `testStabilityLinkedList*.c` |
| `usage/` | 用户可见 document recipe 与真实结构流转 | `testUsage*.c` |
| `utils/` | Print、Minify 和内部 helper 契约 | `testUtils.c`、`testInternalApis.c`、`utils/print/*.c` |

## 高频归属

- `Add/Insert` 创建、游离态与失败所有权：`core/testCreate.c`；复杂容器 guard 再看 `edge/container/`。
- `Replace` 成功/失败、新 item 所有权与 key 重写：`core/testReplace.c`。
- `Detach` 与重挂：`core/testDetach.c`；复杂迁移/链表稳定性分别归 `scenario/` 或 `stability/`。
- Parse 成功/失败：`core/testLoadSuccess.c`、`core/testLoadFailure.c`；严格尾部和 end 指针边界归 `edge/parseprint/`。
- Print 主契约：`utils/print/testPrintGeneral.c`；style 专属语义归 `testPrintStyle.c`。
- Minify NUL 终止与参数守护：`utils/testUtils.c`；slice 与注释边界归 `edge/parseprint/testEdgeMinifySlices.c`。
- 内部布局和低层 helper：`utils/testInternalApis.c`，不与公开 API 用例混放。

文件责任变动时必须同步更新本页。
