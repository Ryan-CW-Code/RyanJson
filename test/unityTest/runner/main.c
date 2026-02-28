#include "FreeRTOS.h"
#include "RyanJson.h"
#include "task.h"
#include "../../common/testCommon.h"
#include "testBase.h"

#include <errno.h>
#include <stdlib.h>
#include <time.h>

#if defined(RyanJsonTestPlatformQemu)
#include "qemuPlatform.h"
#endif

#if defined(RyanJsonFreeRtosHeap4)
void vApplicationGetRandomHeapCanary(portPOINTER_SIZE_TYPE *heapCanary)
{
	portPOINTER_SIZE_TYPE seed = (portPOINTER_SIZE_TYPE)(uintptr_t)&vApplicationGetRandomHeapCanary;

	if (NULL == heapCanary) { return; }
#if defined(RyanJsonTestPlatformQemu)
	seed ^= (portPOINTER_SIZE_TYPE)xTaskGetTickCount();
#else
	struct timespec ts = {0};
	if (0 == clock_gettime(CLOCK_MONOTONIC, &ts))
	{
		seed ^= (portPOINTER_SIZE_TYPE)(uintptr_t)ts.tv_sec;
		seed ^= (portPOINTER_SIZE_TYPE)(uintptr_t)ts.tv_nsec;
	}
#endif
	seed ^= (portPOINTER_SIZE_TYPE)(uintptr_t)heapCanary;
	if (0U == seed) { seed = (portPOINTER_SIZE_TYPE)0xA5A5A5A5U; }
	*heapCanary = seed;
}
#endif

static int32_t defaultTestPlatformLogV(const char *fmt, va_list args)
{
	if (NULL == fmt) { return -1; }
#if defined(RyanJsonTestPlatformQemu)
	char logBuffer[1024];
	int32_t logLen = (int32_t)vsnprintf(logBuffer, sizeof(logBuffer), fmt, args);
	if (logLen > 0) { qemuUartWrite(logBuffer); }
	return logLen;
#else
	return vprintf(fmt, args);
#endif
}

static uint64_t defaultTestPlatformGetUptimeMs(void)
{
	return (uint64_t)xTaskGetTickCount() * (uint64_t)portTICK_PERIOD_MS;
}

static void defaultTestPlatformSleepMs(uint32_t ms)
{
	if (0U == ms) { return; }
	vTaskDelay(pdMS_TO_TICKS(ms));
}

typedef struct
{
	testPlatformThreadEntry_t threadEntry;
	void *threadArg;
	TaskHandle_t waitTask;
} testPlatformThreadCtx_t;

static void defaultTestPlatformThreadTask(void *arg)
{
	testPlatformThreadCtx_t *threadCtx = (testPlatformThreadCtx_t *)arg;

	if (NULL != threadCtx && NULL != threadCtx->threadEntry) { threadCtx->threadEntry(threadCtx->threadArg); }
	if (NULL != threadCtx && NULL != threadCtx->waitTask) { xTaskNotifyGive(threadCtx->waitTask); }
	vTaskDelete(NULL);
}

static int32_t defaultTestPlatformRunThreadWithStackSize(testPlatformThreadEntry_t entry, void *arg, size_t stackDepthWords)
{
	BaseType_t createRet = pdFAIL;
	TaskHandle_t waitTask = xTaskGetCurrentTaskHandle();
	configSTACK_DEPTH_TYPE taskStackDepth = 0U;
	testPlatformThreadCtx_t threadCtx = {0};

	if (NULL == entry || NULL == waitTask) { return EINVAL; }

	if (0U == stackDepthWords) { taskStackDepth = (configSTACK_DEPTH_TYPE)configMINIMAL_STACK_SIZE; }
	else
	{
		if (stackDepthWords < (size_t)configMINIMAL_STACK_SIZE) { stackDepthWords = (size_t)configMINIMAL_STACK_SIZE; }
		if (stackDepthWords > (size_t)(~(configSTACK_DEPTH_TYPE)0U)) { stackDepthWords = (size_t)(~(configSTACK_DEPTH_TYPE)0U); }
		taskStackDepth = (configSTACK_DEPTH_TYPE)stackDepthWords;
	}

	threadCtx.threadEntry = entry;
	threadCtx.threadArg = arg;
	threadCtx.waitTask = waitTask;

	createRet = xTaskCreate(defaultTestPlatformThreadTask, "unitCase", taskStackDepth, &threadCtx,
				(UBaseType_t)(configMAX_PRIORITIES - 3U), NULL);
	if (pdPASS != createRet) { return ENOMEM; }

	(void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	return 0;
}

static int32_t gUnitTestResult = 1;

#if defined(RyanJsonTestPlatformQemu)
static uint32_t qemuLoadWordViaAsm(const void *address)
{
	uint32_t value = 0U;
	__asm volatile("ldr %0, [%1]" : "=r"(value) : "r"(address));
	return value;
}

static void qemuForceUnalignedFaultViaAsm(const void *address)
{
	/* LDRD on unaligned address should trap on Cortex-M models that implement it. */
	__asm volatile("ldrd r2, r3, [%0]" : : "r"(address) : "r2", "r3", "memory");
}

static void qemuRunPostUnitAlignmentCheck(void) __attribute__((noreturn));
static void qemuRunPostUnitAlignmentCheck(void)
{
	uint32_t alignedWords[2] = {0x12345678UL, 0xAABBCCDDUL};
	uint32_t readBack = qemuLoadWordViaAsm((const void *)&alignedWords[1]);
	uint8_t raw[8] __attribute__((aligned(4))) = {0x11U, 0x22U, 0x33U, 0x44U, 0x55U, 0x66U, 0x77U, 0x88U};
	void const *unalignedAddr = (void const *)(raw + 1U);

	if (readBack != alignedWords[1])
	{
		testLog("[QEMU][ALIGN] aligned_access FAIL read=0x%08lx expected=0x%08lx\n", (unsigned long)readBack,
			(unsigned long)alignedWords[1]);
		qemuRequestExit(1);
	}

	testLog("[QEMU][ALIGN] aligned_access PASS read=0x%08lx\n", (unsigned long)readBack);
	qemuSetExpectUnalignedFault(true);
	testLog("[QEMU][ALIGN] unaligned_access TRIGGER addr=0x%08lx\n", (unsigned long)(uintptr_t)unalignedAddr);
	qemuForceUnalignedFaultViaAsm(unalignedAddr);

#if defined(RyanJsonQemuSoftUnalignedTrap)
	if ((((uintptr_t)unalignedAddr) & 0x3U) != 0U)
	{
		testLog("[QEMU][ALIGN] fallback_soft_trap\n");
		testLog("[QEMU][HARDFAULT] fallback_soft_trap_no_hw_fault addr=0x%08lx\n", (unsigned long)(uintptr_t)unalignedAddr);
		testLog("[QEMU][RESULT] EXPECTED_UNALIGNED_FAULT fallbackAddr=0x%08lx\n", (unsigned long)(uintptr_t)unalignedAddr);
		qemuRequestExit(0);
	}
#endif

	qemuSetExpectUnalignedFault(false);
	testLog("[QEMU][ALIGN] unaligned_access did not fault\n");
	testLog("[QEMU][RESULT] UNIT_FAIL code=%ld\n", (long)1L);
	qemuRequestExit(1);
}
#endif

void vAssertCalled(const char *file, int32_t line)
{
	testLog("\n[FreeRTOS ASSERT] %s:%ld\n", (NULL != file) ? file : "<null>", (long)line);
	abort();
}

void vApplicationMallocFailedHook(void)
{
	testLog("\n[FreeRTOS] Malloc Failed Hook\n");
	vAssertCalled(__FILE__, (int32_t)__LINE__);
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *taskName)
{
	(void)task;
	testLog("\n[FreeRTOS] Stack Overflow Hook: %s\n", (NULL != taskName) ? taskName : "<null>");
	vAssertCalled(__FILE__, (int32_t)__LINE__);
}

testPlatformOps_t gTestPlatformOps = {
	.logV = defaultTestPlatformLogV,
	.getUptimeMs = defaultTestPlatformGetUptimeMs,
	.sleepMs = defaultTestPlatformSleepMs,
	.runThreadWithStackSize = defaultTestPlatformRunThreadWithStackSize,
};

static int32_t baselineUsed = 0;

void setUp(void)
{
	ryanJsonTestSetup();
	baselineUsed = unityTestGetUse();
}

void tearDown(void)
{
	int32_t used = unityTestGetUse();
	if (used != baselineUsed)
	{
		testLog("\n\033[1;31m[MEMORY LEAK] Test '%s' leaked %d bytes!\033[0m\n", Unity.CurrentTestName, used - baselineUsed);
		showMemoryInfo();
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(baselineUsed, used, "Memory Leak Detected");

	ryanJsonTestTeardown();
}

void testRyanJsonExample(void)
{
	TEST_ASSERT_EQUAL(RyanJsonTrue, RyanJsonExample());
	RyanJsonInitHooks(unityTestMalloc, unityTestFree, unityTestRealloc);
}

static int32_t runAllUnitTests(void)
{
	UnityBegin(__FILE__);

	RUN_TEST(testRyanJsonExample);

	testChangeRunner();
	testCompareRunner();
	testCreateRunner();
	testDeleteRunner();
	testDetachRunner();
	testDuplicateRunner();
	testForEachRunner();
	testLoadSuccessRunner();
	testLoadFailureRunner();
	testReplaceRunner();

	testEqualityBoolRunner();
	testEqualityDoubleRunner();
	testEqualityIntRunner();
	testEqualityStringRunner();

	testUtilsRunner();
	testRobustRunner();
	testPrintRunner();
	testStressRunner();
#if !defined(RyanJsonTestPlatformQemu)
	testRfc8259Runner();
#endif
	testMemoryRunner();
	testDeepRecursionRunner();

	return UnityEnd();
}

static void logUnitTaskRuntimeInfo(void)
{
	TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();
	logTaskStackRuntimeInfoByHandle("unitMain", NULL, currentTask);
}

static void unitTestTask(void *arg)
{
	(void)arg;
	gUnitTestResult = runAllUnitTests();
	logUnitTaskRuntimeInfo();

#if defined(RyanJsonTestPlatformQemu)
	if (0 == gUnitTestResult)
	{
		testLog("[QEMU][RESULT] UNIT_PASS code=%ld tick=%lu\n", (long)gUnitTestResult, (unsigned long)xTaskGetTickCount());
		qemuRunPostUnitAlignmentCheck();
	}

	testLog("[QEMU][RESULT] UNIT_FAIL code=%ld\n", (long)gUnitTestResult);
	qemuRequestExit((0 == gUnitTestResult) ? 1 : gUnitTestResult);
#else
	vTaskEndScheduler();
	vTaskDelete(NULL);
#endif
}

#ifndef isEnableFuzzer
int32_t main(void)
{
	BaseType_t createRet = xTaskCreate(unitTestTask,                            // 任务入口函数
					   "unitMain",                              // 任务名称
					   4096,                                    // 任务栈大小
					   NULL,                                    // 任务参数
					   (UBaseType_t)(configMAX_PRIORITIES - 2), // 任务优先级
					   NULL                                     // 不需要任务句柄
	);
	if (pdPASS != createRet)
	{
		testLog("[FreeRTOS] xTaskCreate(unitMain) failed\n");
		return 1;
	}

	vTaskStartScheduler();
	return gUnitTestResult;
}
#endif
