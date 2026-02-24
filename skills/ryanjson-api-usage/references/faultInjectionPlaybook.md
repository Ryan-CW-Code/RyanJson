# 故障注入与稳定性验证（主机侧可选）

## 范围
- 用于主机侧或实验环境验证失败路径。
- RT-Thread 板端资源紧张时可不执行本流程。
- 所有权口径见 `ownershipAndErrors.md`。

## 目标
- 验证分配失败场景下的可恢复性。
- 覆盖 Parse/Create/Add/Insert/Replace/Print 的失败分支。

## 注入策略
- 固定第 N 次分配失败（稳定复现）。
- 按概率失败（压力回归）。
- 按调用点标签失败（精确定位）。

## 包装器示例
```c
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "RyanJson.h"

typedef struct
{
    uint32_t allocCount;
    uint32_t failAt;
} jsonFaultState_t;

static jsonFaultState_t gFaultState = {0U, 0U};

static void *faultMalloc(size_t size)
{
    gFaultState.allocCount++;
    if ((0U != gFaultState.failAt) && (gFaultState.allocCount == gFaultState.failAt))
    {
        return NULL;
    }
    return malloc(size);
}

static void faultFree(void *ptr)
{
    free(ptr);
}

static void *faultRealloc(void *ptr, size_t size)
{
    gFaultState.allocCount++;
    if ((0U != gFaultState.failAt) && (gFaultState.allocCount == gFaultState.failAt))
    {
        return NULL;
    }
    return realloc(ptr, size);
}
```

## 建议检查点
- Parse 失败：返回 `NULL`，无泄漏。
- Add/Insert 失败：返回 false；游离 item 由库侧清理，非游离 item 失败不消费。
- Replace 失败：返回 false，item 仍由调用方持有（可继续复用或释放）。
- Print 失败：返回 `NULL`，原 Json 树仍可正常释放。

## 结果判定
- 无崩溃。
- 无泄漏。
- 返回值与所有权语义与头文件一致。

## 依据（仓库内）
- `RyanJson/RyanJsonItem.c`：`RyanJsonInsert`、`RyanJsonReplaceByKey/ByIndex`
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`
- `test/fuzzer/cases/fuzzerCreate.c`、`test/fuzzer/cases/fuzzerReplace.c`
