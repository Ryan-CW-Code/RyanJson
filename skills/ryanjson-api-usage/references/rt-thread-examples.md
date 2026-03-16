# RT-Thread 平台示例（RyanJson）

## 范围
- 本页只放 RT-Thread 平台差异：hooks 映射、任务入口、日志与并发建议。
- 通用 API 路径见 `quickstart.md` / `integrationTemplate.md`。
- 失败语义与释放规则见 `ownershipAndErrors.md`。
- 若不是 RT-Thread 环境，可跳过本页。

## 接入建议
1. 应用初始化阶段完成 hooks 初始化（如 `INIT_APP_EXPORT`）。
2. 业务线程执行完整 Json 事务（Create/Parse -> Modify -> Print -> Delete）。
3. 推荐单 owner 线程；多线程共享时在外层加锁保护完整事务。

## 示例 1：hooks 初始化（RT-Thread 映射）
```c
#include <rtthread.h>
#include "RyanJson.h"

static void *rtJsonMalloc(size_t size) { return rt_malloc(size); }
static void rtJsonFree(void *ptr)
{
    if (RT_NULL != ptr) { rt_free(ptr); }
}
static void *rtJsonRealloc(void *ptr, size_t size) { return rt_realloc(ptr, size); }

static int32_t rtJsonInit(void)
{
    if (RyanJsonFalse == RyanJsonInitHooks(rtJsonMalloc, rtJsonFree, rtJsonRealloc))
    {
        rt_kprintf("RyanJsonInitHooks failed\n");
        return -RT_ERROR;
    }

    rt_kprintf("RyanJson hooks ready\n");
    return RT_EOK;
}
INIT_APP_EXPORT(rtJsonInit);
```

## 示例 2：线程内完整事务（Create -> Replace -> Print -> Delete）
```c
#include <stdint.h>
#include <rtthread.h>
#include "RyanJson.h"

static RyanJsonBool_e buildFreqObject(RyanJson_t *outFreqObj)
{
    RyanJson_t freqObj = RT_NULL;

    if (RT_NULL == outFreqObj)
    {
        return RyanJsonFalse;
    }

    freqObj = RyanJsonCreateObject();
    if (RT_NULL == freqObj)
    {
        return RyanJsonFalse;
    }
    if (RyanJsonFalse == RyanJsonIsDetachedItem(freqObj))
    {
        RyanJsonDelete(freqObj);
        return RyanJsonFalse;
    }

    if ((RyanJsonFalse == RyanJsonAddIntToObject(freqObj, "value", 100)) ||
        (RyanJsonFalse == RyanJsonAddStringToObject(freqObj, "unit", "Hz")))
    {
        RyanJsonDelete(freqObj);
        return RyanJsonFalse;
    }

    *outFreqObj = freqObj;
    return RyanJsonTrue;
}

static void jsonWorkerEntry(void *parameter)
{
    RyanJson_t root = RyanJsonCreateObject();
    RyanJson_t freqObj = RT_NULL;
    char outBuf[192];
    uint32_t outLen = 0U;
    (void)parameter;

    if (RT_NULL == root) { return; }

    if ((RyanJsonFalse == RyanJsonAddStringToObject(root, "dev", "imu")) ||
        (RyanJsonFalse == RyanJsonAddIntToObject(root, "freq", 100)))
    {
        goto done;
    }

    if (RyanJsonTrue == buildFreqObject(&freqObj))
    {
        if (RyanJsonFalse == RyanJsonReplaceByKey(root, "freq", freqObj))
        {
            RyanJsonDelete(freqObj); // Replace 失败：调用方负责释放
        }
    }

    if (RT_NULL != RyanJsonPrintPreallocated(root, outBuf, (uint32_t)sizeof(outBuf), RyanJsonFalse, &outLen))
    {
        rt_kprintf("json=%s\n", outBuf);
    }
    else
    {
        rt_kprintf("print failed\n");
    }

done:
    RyanJsonDelete(root);
}
```

## 示例 3：Detach 迁移子树
```c
#include <rtthread.h>
#include "RyanJson.h"

static RyanJsonBool_e detachMovePayload(RyanJson_t src, RyanJson_t dst)
{
    RyanJson_t detached = RT_NULL;

    if ((RT_NULL == src) || (RT_NULL == dst)) { return RyanJsonFalse; }

    detached = RyanJsonDetachByKey(src, "payload");
    if (RT_NULL == detached) { return RyanJsonFalse; }

    if (RyanJsonFalse == RyanJsonAddItemToObject(dst, "payload", detached))
    {
        // 当前实现下，游离节点 AddItem 失败路径由库侧清理；不做二次释放
        return RyanJsonFalse;
    }

    return RyanJsonTrue;
}
```

## 上板检查单（不依赖 unity/fuzzer）
1. hooks 初始化日志是否先于业务 Json 调用。
2. Parse/Create/Add/Replace/Print 的返回值是否都处理。
3. Replace 失败后是否没有泄漏 `newItem`。
4. 事务退出点是否统一清理（`RyanJsonDelete`）。
5. 异常输入是否返回 false/NULL 且不崩溃。

## 依据（仓库内）
- `RyanJson/RyanJson.c`：hooks 全局指针与 `RyanJsonInitHooks`
- `RyanJson/RyanJsonItem.c`：`RyanJsonAddItemToObject` / `RyanJsonReplaceByKey` / `RyanJsonDetachByKey`
- `RyanJson/RyanJsonPrint.c`：`RyanJsonPrintPreallocated`
- `test/unityTest/cases/core/testCreate.c`：AddItem/Insert 失败行为
- `test/unityTest/cases/core/testReplace.c`：Replace 失败不消费新节点
- `test/unityTest/cases/utils/testPrint.c`：预分配打印边界
