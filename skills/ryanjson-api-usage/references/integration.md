# 应用与 RT-Thread 接入

## 初始化

在应用 JSON 工作前只初始化一次 hooks。分配器三元组必须遵守 malloc/free/realloc 语义，尤其是 realloc 失败不能破坏旧分配。

```c
#include <stdlib.h>
#include "RyanJson.h"

static RyanJsonBool_e appEnsureJsonHooks(void)
{
    static RyanJsonBool_e initialized = RyanJsonFalse;

    if (RyanJsonTrue == initialized)
    {
        return RyanJsonTrue;
    }

    initialized = RyanJsonInitHooks(malloc, free, realloc);
    return initialized;
}
```

RT-Thread 环境在应用初始化阶段或 JSON owner 线程启动前，将 hooks 映射到 `rt_malloc`、`rt_free`、`rt_realloc`。初始化失败必须可观察并阻断之后的 JSON 工作。

## 事务模板

```c
RyanJsonBool_e appProcessJson(const char *input)
{
    RyanJson_t root = NULL;
    char *output = NULL;
    RyanJsonBool_e ok = RyanJsonFalse;

    if ((NULL == input) || (RyanJsonFalse == appEnsureJsonHooks()))
    {
        return RyanJsonFalse;
    }

    root = RyanJsonParse(input);
    if (NULL == root)
    {
        goto done;
    }

    /* 只通过公开 API 读取或修改 root。 */

    output = RyanJsonPrint(root, 0U, RyanJsonFalse, NULL);
    if (NULL == output)
    {
        goto done;
    }

    ok = RyanJsonTrue;

done:
    if (NULL != output)
    {
        RyanJsonFree(output);
    }
    if (NULL != root)
    {
        RyanJsonDelete(root);
    }
    return ok;
}
```

固定应用缓冲使用 `RyanJsonPrintPreallocated`，并以当前 [RyanJson.h](../../../RyanJson/RyanJson.h) 为函数签名权威来源。

## 并发

- 推荐一个 owner 任务独占完整 `Parse/Create -> modify -> Print -> Delete` 事务。
- 多任务共享同一树时，外层锁覆盖整个事务；库的生命周期规则不替代应用并发策略。
- 错误上报应在树所有权路径之外，保证清理确定。

## 部署检查

- hooks 先于首次 Parse 或 Create。
- 每个公开调用都检查返回值。
- Replace 失败会删除或复用新 item。
- 动态 Print 缓冲使用 `RyanJsonFree`；预分配缓冲不由 RyanJson 释放。
- 非法输入返回可恢复错误而不 crash。
