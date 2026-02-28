# 所有权与错误处理

## 范围
- 本页只定义公开 API 的所有权与失败语义。
- 术语口径见 `../../shared/terminology.md`。

## 所有权基线
- `RyanJsonCreate*`：成功后归调用者。
- `Add/Insert` 成功：`item` 转移给父节点。
- `Replace` 成功：新 `item` 转移给父节点，旧节点由库删除。
- `DetachByKey/DetachByIndex`：返回节点归调用者。
- `RyanJsonPrint` 返回缓冲：调用者用 `RyanJsonFree` 释放。

## 失败语义（重点）
- `RyanJsonInsert` 失败：
  - `item` 是游离节点时：失败路径由库删除该 `item`。
  - `item` 非游离节点时：直接返回 false，不删除（避免破坏原树）。
- `RyanJsonAddItemToObject` 失败（当前实现）：
  - `item` 非游离：返回 false，不删除。
  - `item` 为标量：返回 false，并删除该 `item`。
  - `item` 为容器且后续插入失败：失败路径由库侧清理（包含包装节点与其子树）。
- `ReplaceByKey/ReplaceByIndex` 失败：不消费 `item`，调用者决定复用或释放。

## 建议失败处理模板
### Add/Insert
```c
RyanJson_t item = RyanJsonCreateObject();
if ((NULL == item) || (RyanJsonFalse == RyanJsonIsDetachedItem(item))) { return RyanJsonFalse; }

if (RyanJsonFalse == RyanJsonInsert(parent, UINT32_MAX, item))
{
    // Insert 失败时，游离 item 会由库侧清理；不要盲目二次释放
    return RyanJsonFalse;
}
```

### Replace
```c
RyanJson_t newItem = RyanJsonCreateObject();
if ((NULL == newItem) || (RyanJsonFalse == RyanJsonIsDetachedItem(newItem))) { return RyanJsonFalse; }

if (RyanJsonFalse == RyanJsonReplaceByKey(parent, "k", newItem))
{
    // Replace 失败不消费 item，调用者负责释放或复用
    RyanJsonDelete(newItem);
    return RyanJsonFalse;
}
```

## 游离节点防误用
- 复用节点前先 `RyanJsonIsDetachedItem(item)`。
- 已挂树节点禁止再次 Add/Insert/Replace 到其它位置。

## 常见误区
- 把 Add/Insert 与 Replace 的失败释放语义混为一套。
- Detach 后未释放也未重新挂树。
- hooks 未初始化就开始 Parse/Create。

## 依据（仓库内）
- `RyanJson/RyanJsonItem.c`：`RyanJsonInsert` 失败 `error__` 删除 `item`
- `RyanJson/RyanJsonItem.c`：`RyanJsonAddItemToObject` 对非游离/标量/包装失败路径分支
- `RyanJson/RyanJsonItem.c`：`RyanJsonReplaceByKey/ByIndex` 失败路径返回 false，不删除新 `item`
- `test/unityTest/cases/core/testCreate.c`：Insert/AddItem 失败语义（含已挂树拒绝、标量失败）
- `test/unityTest/cases/core/testReplace.c`：Replace 失败后 `item` 仍游离并可复用/需调用方释放
