# 确定性测试设计

## 测试前置

- 每个 runner 或测试组先确保 `RyanJsonInitHooks` 成功；内存统计、泄漏判断和 OOM 注入都通过 hooks。
- 先查 `RyanJsonStrictObjectKeyCheck` 和 `RyanJsonDefaultAddAtHead`。宏改变时，已有期望可能不成立。
- 复杂树优先通过 Parse 形成初始态，减少测试搭建本身引入的无关风险。

## 一个用例只证明一条核心契约

按以下顺序补齐缺口，而不是机械追求每项都出现：

1. 成功路径：证明主要可观察行为。
2. 失败路径：证明返回值、树完整性和所有权。
3. 边界路径：空容器、尺寸临界、index 头/中/尾、短长字符串或数值边界。
4. 资源路径：失败后没有泄漏、双重释放或脏链。

## 断言策略

- 返回值用 `TEST_ASSERT_TRUE/FALSE`，指针用 `TEST_ASSERT_NULL/NOT_NULL`。
- 结构期望优先通过字段验证或 `RyanJsonCompare`；不要仅检查一条容易碰巧成立的局部值。
- 失败结构操作同时断言失败返回、原树可用和 item 的预期所有权。
- 复用 `UNITY_TEST_OOM_BEGIN/END`、`UNITY_TEST_OOM_BEGIN_NO_REALLOC`、`unityTestLeakScopeBegin/End` 等公共帮助器。
- Unity 断言失败会提前返回；多步测试在关键前置断言后安排清理，不能让断言绕过资源释放。

## 归属原则

| 契约形态 | 优先目录 |
| --- | --- |
| 公共 Create、Change、Delete、Detach、Replace、Load | `test/unityTest/cases/core/` |
| 容器、Parse/Print、key/string 尺寸边界 | `test/unityTest/cases/edge/` |
| 标量解析/打印往返 | `test/unityTest/cases/equality/` |
| 深度、内存、压力 | `test/unityTest/cases/performance/` |
| 高耦合业务流程 | `test/unityTest/cases/scenario/` 或 `usage/` |
| 链表迁移、遍历与 churn 稳定性 | `test/unityTest/cases/stability/` |
| Print、Minify、内部低层 helper | `test/unityTest/cases/utils/` |

测试注释用中文解释“为何在这里测、漏掉会怎样”；API、类型和宏名保留英文。
