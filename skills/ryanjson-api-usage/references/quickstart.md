# RyanJson 快速上手（RT-Thread/C）

## 范围
- 本文提供“最小可跑通”的公开 API 路径。
- 完整业务模板见 `integrationTemplate.md`。
- 平台差异实现见 `rt-thread-examples.md`。
- 失败所有权细则见 `ownershipAndErrors.md`。

## 先选你的目标
- 只想先跑通 Parse + Get：直接看“示例 2”。
- 想创建对象并输出紧凑 JSON：直接看“示例 3”。
- 想做跨类型替换或子树迁移：直接看“示例 4 / 示例 5”。
- 如果你还不确定自己是不是该看这页：先看 `../../shared/questionRouter.md`。

## 新手提示
- 不懂 RTOS 也没关系：先用 `malloc/free/realloc` 跑通，再做平台适配。
- 不熟 JSON 术语：先读 `beginnerPrimer.md`。
- 第一次接触仓库：先补看 `../../ryanjson-project-guide/references/entryScenarios.md`。

## 30 秒心智模型
- `InitHooks`：先把分配器接进去。
- `Parse/Create`：得到一棵 Json 树。
- `Get/Add/Replace/Detach`：在树上读取或改结构。
- `Print`：需要输出时再转成字符串。
- `Delete`：事务结束统一释放 root。

## 入口条件（必须）
1. 任何 Json API 前先完成 `RyanJsonInitHooks`。
2. 回答或示例中标明宏前提：
   - `RyanJsonStrictObjectKeyCheck`
   - `RyanJsonDefaultAddAtHead`
3. 每条失败分支都要显式释放（`RyanJsonDelete` / `RyanJsonFree`）。

## 三条最短路径
### 路径 A：读取字段
1. `RyanJsonInitHooks`
2. `RyanJsonParse`
3. `RyanJsonGetObjectByKey`
4. `RyanJsonIsXXX` + `RyanJsonGetXXXValue`
5. `RyanJsonDelete(root)`

### 路径 B：创建并输出
1. `RyanJsonInitHooks`
2. `RyanJsonCreateObject`
3. `RyanJsonAdd*ToObject`
4. `RyanJsonPrintPreallocated(..., RyanJsonFalse, ...)`
5. `RyanJsonDelete(root)`

### 路径 C：替换或迁移子树
1. 先创建新节点或先 `Detach`
2. `RyanJsonReplaceByKey/ByIndex` 或 `RyanJsonAddItem*`
3. 明确失败分支的所有权
4. 结束时释放仍归调用方的节点

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
    if (RyanJsonFalse == appEnsureJsonHooks()) { return RyanJsonFalse; }

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
    if (RyanJsonFalse == appEnsureJsonHooks()) { return RyanJsonFalse; }

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
    if (RyanJsonFalse == appEnsureJsonHooks()) { return RyanJsonFalse; }

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

## 遇到这些情况时不要停在本页
- 想看平台接入模板：转 `integrationTemplate.md`
- 想看 RT-Thread 差异：转 `rt-thread-examples.md`
- 想确认失败谁释放：转 `ownershipAndErrors.md`
- 想排查 crash、性能或内部实现风险：转 `../../shared/questionRouter.md`，通常进入 `../../ryanjson-optimization/SKILL.md`

## 依据（仓库内）
- `RyanJson/RyanJson.c`：`RyanJsonInitHooks`、`RyanJsonDelete`
- `RyanJson/RyanJsonItem.c`：`RyanJsonReplaceByKey`、`RyanJsonDetachByKey`、`RyanJsonAddItemToObject`
- `test/unityTest/cases/core/testReplace.c`：Replace 失败不消费 `item`
- `test/unityTest/cases/core/testDetach.c`、`test/unityTest/cases/core/testCreate.c`：Detach/AddItem 失败路径
- `test/unityTest/cases/utils/print/testPrintGeneral.c`：非格式化打印与 preallocated 边界
