#ifndef RYANJSON_TEST_COMMON_H
#define RYANJSON_TEST_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <inttypes.h>
#include "FreeRTOS.h"
#include "task.h"
#include "unity.h"
#include "valloc.h"
#include "RyanJson.h"
#include "cJSON.h"
#include "yyjson.h"
#include "testPlatform.h"

#ifndef RyanJsonTestAllocHeaderSize
#define RyanJsonTestAllocHeaderSize RyanJsonMallocHeaderSize
#endif

#ifndef RyanJsonTestAllocAlignSize
#define RyanJsonTestAllocAlignSize RyanJsonMallocAlign
#endif

static inline void delay(uint32_t ms)
{
	testPlatformSleepMs(ms);
}

#define getArraySize(arr) ((int32_t)(sizeof(arr) / sizeof((arr)[0])))
// 定义枚举类型
extern void *unityTestMalloc(size_t size);
extern void unityTestFree(void *block);
extern void *unityTestRealloc(void *block, size_t size);
extern void unityTestSetAllocSimulation(uint8_t isEnable);
extern uint8_t unityTestGetAllocSimulation(void);
extern void unityTestOomBegin(int32_t failAfter, RyanJsonBool_e disableRealloc);
extern void unityTestOomEnd(void);
extern int32_t unityTestGetUse(void);
extern void showMemoryInfo(void);
extern void logTaskStackRuntimeInfoByHandle(const char *tag, const char *taskName, TaskHandle_t taskHandle);
// 定义结构体类型
extern uint64_t platformUptimeMs(void);

extern RyanJsonBool_e RyanJsonExample(void);
extern RyanJsonBool_e RyanJsonTestFun(void);

extern void ryanJsonTestSetup(void);
extern void ryanJsonTestTeardown(void);

#define UNITY_TEST_OOM_BEGIN(failAfter)            unityTestOomBegin((failAfter), RyanJsonFalse)
#define UNITY_TEST_OOM_BEGIN_NO_REALLOC(failAfter) unityTestOomBegin((failAfter), RyanJsonTrue)
#define UNITY_TEST_OOM_END()                       unityTestOomEnd()

typedef struct
{
	int32_t baseline;
} unityTestLeakScope_t;

static inline unityTestLeakScope_t unityTestLeakScopeBegin(void)
{
	unityTestLeakScope_t scope = {unityTestGetUse()};
	return scope;
}

static inline void unityTestLeakScopeEnd(unityTestLeakScope_t scope, const char *msg)
{
	TEST_ASSERT_EQUAL_INT_MESSAGE(scope.baseline, unityTestGetUse(), msg);
}

#ifdef __cplusplus
}
#endif

#endif
