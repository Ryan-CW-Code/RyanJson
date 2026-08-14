# 核心架构与热点

## 模块责任

| 模块 | 负责内容 | 审查重点 |
| --- | --- | --- |
| `RyanJson.c` | hooks、Delete、Duplicate、Compare、Minify | 生命周期、遍历闭合、比较语义 |
| `RyanJsonParse.c` | 迭代解析与回滚 | 输入边界、半树清理、严格 key |
| `RyanJsonPrint.c` | 迭代序列化与输出容量 | 精确容量、数字格式、失败清理 |
| `RyanJsonItem.c` | Create、Add、Insert、Replace、Detach、Delete | 线索结构与失败所有权 |
| `RyanJsonUtils.c` | 布局访问、链表辅助、字符串修改 | payload 偏移、内联/指针切换、分配对齐 |
| `RyanJsonConfig.h` / `RyanJsonInternal.h` | 公开语义与表示控制 | 默认兼容性与布局假设 |

## 改动前必须命名的不变量

- 每个兄弟链只有一个 `IsLast` 节点，它的 `next` 指向父节点。
- 对外遍历必须使用 `RyanJsonGetNext`；直接访问 `next` 属于表示敏感内部行为。
- 节点分配大小和值偏移必须同时符合 key、类型和字符串存储模式。
- 指针字符串替换全过程只有一个所有者，旧缓冲仅在新值成功后释放。
- 结构操作失败后旧树仍有效，且严格遵守失败所有权。
- Parse、Print、Delete、Duplicate、Compare 的主路径保持迭代，不依赖递归栈。

## 取证入口

- 从 [RyanJson.h](../../../RyanJson/RyanJson.h) 的公开声明开始追实现调用链。
- 新增回归前先搜索对应 `test/unityTest/cases/` 分类。
- 在 [test/fuzzer/](../../../test/fuzzer/) 搜索异常输入与既有稳定性用例。
- 布局事实以 [共享架构](../../shared/architecture.md) 为准；不变量变化时更新该文档。
