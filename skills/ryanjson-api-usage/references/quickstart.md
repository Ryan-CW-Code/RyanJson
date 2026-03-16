# RyanJson 快速上手（RT-Thread/C）

## 范围
- 本文提供“最小可跑通”的公开 API 路径。
- 完整业务模板见 `integrationTemplate.md`。
- 平台差异实现见 `rt-thread-examples.md`。
- 失败所有权细则见 `ownershipAndErrors.md`。

## 新手提示
- 不懂 RTOS 也没关系：先用 `malloc/free/realloc` 跑通，再做平台适配。
- 不熟 JSON 术语：先读 `beginnerPrimer.md`。

## 入口条件（必须）
1. 任何 Json API 前先完成 `RyanJsonInitHooks`。
2. 回答或示例中标明宏前提：
   - `RyanJsonStrictObjectKeyCheck`
   - `RyanJsonDefaultAddAtHead`
3. 每条失败分支都要显式释放（`RyanJsonDelete` / `RyanJsonFree`）。

## 示例 1：初始化 hooks（一次性）
```c
#include <stdlib.h>
#include "RyanJson.h"

static RyanJsonBool_e appEnsureJsonHooks(void)
{
    static RyanJsonBool_e inited = RyanJsonFalse;
    if (RyanJsonTrue == inited) { return RyanJsonTrue; }

    inited = RyanJsonInitHooks(malloc, free, realloc);
    return inited;
}
```

## 示例 2：Parse + Get（安全读取）
```c
#include <stdint.h>
#include "RyanJson.h"

static RyanJsonBool_e readAge(const char *jsonText, int32_t *outAge)
{
    RyanJson_t root = NULL;
    RyanJson_t ageItem = NULL;

    if ((NULL == jsonText) || (NULL == outAge)) { return RyanJsonFalse; }
    if (RyanJsonFalse == appEnsureJsonHooks()) { return RyanJsonFalse; }

    root = RyanJsonParse(jsonText);
    if (NULL == root) { return RyanJsonFalse; }

    ageItem = RyanJsonGetObjectByKey(root, "age");
    if ((NULL == ageItem) || (RyanJsonFalse == RyanJsonIsInt(ageItem)))
    {
        RyanJsonDelete(root);
        return RyanJsonFalse;
    }

    *outAge = RyanJsonGetIntValue(ageItem);
    RyanJsonDelete(root);
    return RyanJsonTrue;
}
```

## 示例 3：Create + Add + PrintPreallocated（非格式化输出）
```c
#include <stdint.h>
#include "RyanJson.h"

static RyanJsonBool_e buildReport(char *out, uint32_t outCap, uint32_t *outLen)
{
    RyanJson_t root = NULL;

    if ((NULL == out) || (NULL == outLen) || (0U == outCap)) { return RyanJsonFalse; }

    root = RyanJsonCreateObject();
    if (NULL == root) { return RyanJsonFalse; }

    if ((RyanJsonFalse == RyanJsonAddStringToObject(root, "name", "ryan")) ||
        (RyanJsonFalse == RyanJsonAddIntToObject(root, "age", 18)))
    {
        RyanJsonDelete(root);
        return RyanJsonFalse;
    }

    if (NULL == RyanJsonPrintPreallocated(root, out, outCap, RyanJsonFalse, outLen))
    {
        RyanJsonDelete(root);
        return RyanJsonFalse;
    }

    RyanJsonDelete(root);
    return RyanJsonTrue;
}
```

## 示例 4：Replace 跨类型切换（失败不消费 newItem）
```c
#include "RyanJson.h"

static RyanJsonBool_e replaceFreqAsObject(RyanJson_t root)
{
    RyanJson_t newFreq = NULL;

    if (NULL == root) { return RyanJsonFalse; }

    newFreq = RyanJsonCreateObject();
    if (NULL == newFreq) { return RyanJsonFalse; }
    if (RyanJsonFalse == RyanJsonIsDetachedItem(newFreq))
    {
        RyanJsonDelete(newFreq);
        return RyanJsonFalse;
    }

    if ((RyanJsonFalse == RyanJsonAddIntToObject(newFreq, "value", 100)) ||
        (RyanJsonFalse == RyanJsonAddStringToObject(newFreq, "unit", "Hz")))
    {
        RyanJsonDelete(newFreq);
        return RyanJsonFalse;
    }

    if (RyanJsonFalse == RyanJsonReplaceByKey(root, "freq", newFreq))
    {
        RyanJsonDelete(newFreq); // Replace 失败：调用方负责释放
        return RyanJsonFalse;
    }

    return RyanJsonTrue;
}
```

## 示例 5：Detach 后重挂或释放
```c
#include "RyanJson.h"

static RyanJsonBool_e movePayload(RyanJson_t src, RyanJson_t dst)
{
    RyanJson_t detached = NULL;

    if ((NULL == src) || (NULL == dst)) { return RyanJsonFalse; }

    detached = RyanJsonDetachByKey(src, "payload");
    if (NULL == detached) { return RyanJsonFalse; }

    if (RyanJsonFalse == RyanJsonAddItemToObject(dst, "payload", detached))
    {
        // 当前实现下，对游离节点 AddItem 失败路径由库侧清理；不要二次释放 detached
        return RyanJsonFalse;
    }

    return RyanJsonTrue;
}
```

## 最小检查单
1. hooks 初始化是否先于 Parse/Create。
2. Get 前是否判空 + 判型。
3. Replace 失败后是否由调用方处理 `newItem`。
4. 动态打印返回值是否配对 `RyanJsonFree`。
5. 退出分支是否都做了 `RyanJsonDelete`。

## 依据（仓库内）
- `RyanJson/RyanJson.c`：`RyanJsonInitHooks`、`RyanJsonDelete`
- `RyanJson/RyanJsonItem.c`：`RyanJsonReplaceByKey`、`RyanJsonDetachByKey`、`RyanJsonAddItemToObject`
- `test/unityTest/cases/core/testReplace.c`：Replace 失败不消费 `item`
- `test/unityTest/cases/core/testDetach.c`、`test/unityTest/cases/core/testCreate.c`：Detach/AddItem 失败路径
- `test/unityTest/cases/utils/testPrint.c`：非格式化打印与 preallocated 边界
