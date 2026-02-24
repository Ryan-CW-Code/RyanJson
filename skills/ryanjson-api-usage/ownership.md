# RyanJson 所有权矩阵（压缩版）

## 1. 作用
- 统一调用方与库之间的所有权口径，减少泄漏与误删。
- 术语口径见 `../shared/terminology.md`。

## 2. 核心规则
### 2.1 Create 系列
- `RyanJsonCreate*` 成功返回后，节点归调用方。
- 调用方必须挂载或删除该节点。

### 2.2 Detach 系列
- `RyanJsonDetach*` 成功返回后，节点归调用方。
- 调用方必须重新挂载或删除该节点。

### 2.3 Replace 系列
- 当前实现中，`ReplaceByKey/ByIndex` 失败不消费 `newItem`。
- 调用方必须复用或 `RyanJsonDelete(newItem)`。

### 2.4 Print 系列
- `RyanJsonPrint` 返回动态字符串，调用方必须 `RyanJsonFree`。
- `RyanJsonPrintPreallocated` 使用调用方缓冲，不额外释放输出缓冲。

## 3. 关键提醒
- `Add/Insert` 与 `Replace` 的失败语义不能混用。
- 边界行为不确定时，必须回查当前头文件与测试。
- 语义依据：`RyanJson/RyanJsonItem.c`、`test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`。

## 4. 失败分支检查单
1. 每个创建对象是否在所有分支都有明确归属。
2. API 失败时是否说明了所有权是否转移。
3. 每块动态内存是否“且仅”释放一次。
