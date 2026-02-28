#ifndef RYANJSON_TEST_PLATFORM_H
#define RYANJSON_TEST_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

typedef void (*testPlatformThreadEntry_t)(void *arg);
typedef int32_t (*testPlatformLogV_t)(const char *fmt, va_list args);
typedef uint64_t (*testPlatformGetUptimeMs_t)(void);
typedef void (*testPlatformSleepMs_t)(uint32_t ms);
/* stackDepthWords 使用 FreeRTOS 栈单位（StackType_t 个数），不是字节。 */
typedef int32_t (*testPlatformRunThreadWithStackSize_t)(testPlatformThreadEntry_t entry, void *arg, size_t stackDepthWords);

typedef struct
{
	testPlatformLogV_t logV;
	testPlatformGetUptimeMs_t getUptimeMs;
	testPlatformSleepMs_t sleepMs;
	testPlatformRunThreadWithStackSize_t runThreadWithStackSize;
} testPlatformOps_t;

// 平台函数表由 runner/main.c 提供默认实现；RTOS 可在启动时覆盖。
extern testPlatformOps_t gTestPlatformOps;

static inline void setTestPlatformOps(const testPlatformOps_t *ops)
{
	if (NULL != ops) { gTestPlatformOps = *ops; }
}

static inline testPlatformOps_t *getTestPlatformOps(void)
{
	return &gTestPlatformOps;
}

static inline int32_t testLog(const char *fmt, ...)
{
	int32_t ret = 0;
	va_list args;
	testPlatformLogV_t logFunc = gTestPlatformOps.logV;
	if (NULL == logFunc || NULL == fmt) { return -1; }

	va_start(args, fmt);
	ret = logFunc(fmt, args);
	va_end(args);
	return ret;
}

static inline uint64_t testPlatformGetUptimeMs(void)
{
	testPlatformGetUptimeMs_t getUptimeMsFunc = gTestPlatformOps.getUptimeMs;
	if (NULL == getUptimeMsFunc) { return 0U; }
	return getUptimeMsFunc();
}

static inline void testPlatformSleepMs(uint32_t ms)
{
	testPlatformSleepMs_t sleepMsFunc = gTestPlatformOps.sleepMs;
	if (NULL == sleepMsFunc) { return; }
	sleepMsFunc(ms);
}

static inline int32_t testPlatformRunThreadWithStackSize(testPlatformThreadEntry_t entry, void *arg, size_t stackDepthWords)
{
	testPlatformRunThreadWithStackSize_t runThreadFunc = gTestPlatformOps.runThreadWithStackSize;
	if (NULL == runThreadFunc || NULL == entry) { return -1; }
	return runThreadFunc(entry, arg, stackDepthWords);
}

#ifdef __cplusplus
}
#endif

#endif
