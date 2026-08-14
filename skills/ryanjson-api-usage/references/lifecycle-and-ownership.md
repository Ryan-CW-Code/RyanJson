# API 生命周期与所有权

## 事务模型

将一次 RyanJson 操作视为事务：

1. 在任何依赖分配的 RyanJson API 前初始化 hooks。
2. Parse 输入或 Create root。
3. 通过公开 API 读取或修改树。
4. 仅在需要输出时 Print。
5. 在统一退出点释放仍归调用方的所有资源。

## 调用方强制检查

- `RyanJsonInitHooks` 失败后不能继续 JSON 事务。
- `Get*` 返回非空后，必须 `RyanJsonIsXXX` 再取对应类型值。
- `Change*Value` 只用于同类型改值；跨类型改动使用 `ReplaceByKey` 或 `ReplaceByIndex`。
- Add、Insert、Replace 前确认 item 为游离节点。
- 挂子节点后若继续深层结构修改，应从 root 重新获取目标，避免复用旧容器指针。

## 所有权矩阵

| 操作 | 成功 | 失败 |
| --- | --- | --- |
| `Create*` | 调用方拥有游离 item，直到挂载或删除。 | 返回 `NULL`。 |
| `Detach*` | 调用方拥有返回 item，必须挂载或删除。 | 不转移所有权。 |
| `Insert` 与 Add-item API | 合法游离 item 转给父节点。 | 清理路径依赖具体 API 与游离状态；已挂树 item 被拒绝且原树不受破坏。不能盲目释放原指针。 |
| `ReplaceByKey/ReplaceByIndex` | 新 item 转给父节点，旧节点删除。 | 新 item 仍归调用方，必须复用或删除。 |
| `RyanJsonPrint` | 调用方以 `RyanJsonFree` 释放动态缓冲。 | 返回 `NULL`，树仍归调用方。 |
| `RyanJsonPrintPreallocated` | 调用方继续拥有传入缓冲。 | 调用方继续拥有缓冲，不能把它当作成功输出。 |

## 失败路径模板

### Replace

```c
RyanJson_t newItem = RyanJsonCreateObject();

if (NULL == newItem)
{
    return RyanJsonFalse;
}

if (RyanJsonFalse == RyanJsonReplaceByKey(root, "settings", newItem))
{
    RyanJsonDelete(newItem);
    return RyanJsonFalse;
}
```

Replace 失败不消费 `newItem`，这与 `Add/Insert` 中游离节点的库侧清理路径不同。

### Detach 后迁移

```c
RyanJson_t item = RyanJsonDetachByKey(source, "payload");

if (NULL == item)
{
    return RyanJsonFalse;
}

if (RyanJsonFalse == RyanJsonAddItemToObject(destination, "payload", item))
{
    /* 决定 item 是否仍归调用方前，先核对当前 AddItem 契约。 */
    return RyanJsonFalse;
}
```

## 证据入口

- [行为契约](../../shared/behavior-contracts.md)
- [RyanJson.h](../../../RyanJson/RyanJson.h)
- [RyanJsonItem.c](../../../RyanJson/RyanJsonItem.c)
- [testCreate.c](../../../test/unityTest/cases/core/testCreate.c)
- [testReplace.c](../../../test/unityTest/cases/core/testReplace.c)
