# 新手入门速览（零基础）

## 适用人群
- 第一次用 JSON/C 库，甚至不了解 RTOS 的嵌入式新手。
- 只需要“能跑通并安全释放”的最小路径。

## 先给你一个心智模型
- JSON 是一段文本，Parse 会把文本变成“树”。
- 树里每个节点叫 item。
- `Object` 是“键值对”，`Array` 是“按下标顺序”的列表。
- 读 Object 用 key（字符串），读 Array 用 index（数字）。

## 最小流程（必须顺序）
1. 初始化内存 hooks：`RyanJsonInitHooks`。
2. Parse 文本 **或** Create 新树。
3. Get/Change/Replace/Detach 做操作。
4. Print 输出字符串（可选）。
5. `RyanJsonDelete(root)` 统一释放树。

## 你不懂 RTOS 也没关系
- 如果你在 PC/裸机/其他环境，只要能提供 `malloc/free/realloc` 就能先跑通。
- 在 RTOS 上，只是把这三个函数换成系统的分配函数。
- 只要 `realloc` 语义正确（失败返回 `NULL`，不破坏旧指针），就能用。

## 新手必记 6 条
1. 每次 `Get` 前先判空，再 `RyanJsonIsXXX` 判型。
2. `Add/Insert/Replace` 只接受“游离节点”，不要重复挂树。
3. `Replace` 失败时新节点不会被库释放，调用方要管。
4. `Detach` 后必须“重挂”或“释放”，不能悬空。
5. `Print` 返回的字符串用 `RyanJsonFree` 释放。
6. 任何失败分支都要清理 `root`。

## 下一步看哪里
- 选最短可跑通的代码：`quickstart.md`
- 按场景选 API：`apiPatterns.md`
- 释放责任细节：`ownershipAndErrors.md`
- 平台集成模板：`integrationTemplate.md`
- 不确定当前问题该走哪个 skill：`../../shared/questionRouter.md`
