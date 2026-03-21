# 集成模板（业务流程版）

## 范围
- 这是 API 技能的 **基线模板**：初始化 -> 解析/修改 -> 输出 -> 释放。
- `quickstart.md` 与 `rt-thread-examples.md` 都可基于本模板裁剪。
- 共性口径见 `../../shared/ryanJsonCommon.md`。

## 基线伪代码
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

RyanJsonBool_e appProcessJson(const char *input)
{
    RyanJson_t root = NULL;
    char *output = NULL;
    RyanJsonBool_e ok = RyanJsonFalse;

    if (RyanJsonFalse == appEnsureJsonHooks()) { goto done; }

    root = RyanJsonParse(input);
    if (NULL == root) { goto done; }

    // modify/read json here

    output = RyanJsonPrint(root, 0, RyanJsonFalse, NULL);
    if (NULL == output) { goto done; }

    ok = RyanJsonTrue;

done:
    if (NULL != output) { RyanJsonFree(output); }
    if (NULL != root) { RyanJsonDelete(root); }
    return ok;
}
```

## 裁剪建议
- RTOS：把 `malloc/free/realloc` 替换为内存池包装器。
- 高可靠：补错误码与日志等级；把 `goto done` 统一接入故障统计。
- 高吞吐：优先 `RyanJsonPrintPreallocated`，减少动态分配。

## 线程模型建议
- 推荐单 owner 任务独占 Json 树。
- 若必须多任务访问，外层用互斥锁包住完整事务（Parse->Modify->Print->Delete）。

## 最小检查单
1. hooks 初始化失败时是否中止事务。
2. Parse/Print 失败分支是否都回收资源。
3. 动态输出是否统一配对 `RyanJsonFree`。
4. 事务是否有单一出口清理点。

## 依据（仓库内）
- `RyanJson/RyanJson.c`：hooks 全局指针与 `RyanJsonInitHooks`
- `RyanJson/RyanJsonPrint.c`：`RyanJsonPrint`/`RyanJsonPrintPreallocated`
- `test/unityTest/cases/utils/print/testPrintGeneral.c`：打印与预分配边界行为
- `test/unityTest/cases/core/testReplace.c`：Replace 失败由调用方处理新节点
