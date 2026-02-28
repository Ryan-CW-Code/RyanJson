#include "FreeRTOS.h"
#include "RyanJsonInternal.h"
#include "testCommon.h"
#include "tlsf.h"

#if defined(RyanJsonTestPlatformQemu)
#define unitTlsfPoolSize (600U * 1024U)
#define unitTlsfPoolMin  (512U * 1024U)
#else
#define unitTlsfPoolSize (1024U * 1024U)
#endif

typedef struct
{
	tlsf_t tlsfHandle;
	void *poolBuffer;
	size_t poolSize;
} unityTestTlsfCtx_t;

static unityTestTlsfCtx_t gUnityTestTlsfCtx = {NULL, NULL, 0U};
#if defined(RyanJsonTestPlatformQemu)
static uint8_t gQemuTlsfPoolLogged = 0U;
#endif
static uint8_t gAllocSimConfigLogged = 0U;
static uint8_t gAllocSimEnabled = 0U;

uint64_t platformUptimeMs(void)
{
	return testPlatformGetUptimeMs();
}

static size_t unityTestCalcSimulatedAllocSize(size_t requestSize)
{
	const size_t headerSize = (size_t)RyanJsonTestAllocHeaderSize;
	const size_t alignSize = (size_t)RyanJsonTestAllocAlignSize;
	size_t allocSize = requestSize;

	if (0U == requestSize) { return 0U; }
	if (allocSize > (SIZE_MAX - headerSize)) { return 0U; }

	allocSize += headerSize;
	if (alignSize > 1U) { allocSize = RyanJsonAlign(allocSize, alignSize); }
	return allocSize;
}

void unityTestSetAllocSimulation(uint8_t isEnable)
{
	if (0U != isEnable) { gAllocSimEnabled = 1U; }
	else
	{
		gAllocSimEnabled = 0U;
	}
}

uint8_t unityTestGetAllocSimulation(void)
{
	return gAllocSimEnabled;
}

static bool unityTestInitTlsf(void)
{
	if (NULL == gUnityTestTlsfCtx.poolBuffer)
	{
		size_t trySize = unitTlsfPoolSize;
#if defined(RyanJsonTestPlatformQemu)
		while (trySize >= unitTlsfPoolMin)
		{
			gUnityTestTlsfCtx.poolBuffer = v_malloc(trySize);
			if (NULL != gUnityTestTlsfCtx.poolBuffer)
			{
				gUnityTestTlsfCtx.poolSize = trySize;
				break;
			}
			trySize /= 2U;
		}
#else
		gUnityTestTlsfCtx.poolBuffer = v_malloc(trySize);
		if (NULL != gUnityTestTlsfCtx.poolBuffer) { gUnityTestTlsfCtx.poolSize = trySize; }
#endif
		if ((NULL == gUnityTestTlsfCtx.poolBuffer) || (0U == gUnityTestTlsfCtx.poolSize)) { return false; }
	}

	gUnityTestTlsfCtx.tlsfHandle =
		tlsf_create_with_pool(gUnityTestTlsfCtx.poolBuffer, gUnityTestTlsfCtx.poolSize, gUnityTestTlsfCtx.poolSize);
	return (NULL != gUnityTestTlsfCtx.tlsfHandle);
}

void showMemoryInfo(void)
{
	size_t total = 0U;
	size_t used = 0U;
	size_t maxUsed = 0U;

	if (NULL == gUnityTestTlsfCtx.tlsfHandle)
	{
		testLog("%s:%d tlsf 未初始化\r\n", __FILE__, __LINE__);
		return;
	}

	tlsf_memory_info(gUnityTestTlsfCtx.tlsfHandle, &total, &used, &maxUsed);
	testLog("%s:%d tlsf used: %lu, maxUsed: %lu, total: %lu\r\n", __FILE__, __LINE__, (unsigned long)used, (unsigned long)maxUsed,
		(unsigned long)total);
}

void logTaskStackRuntimeInfoByHandle(const char *tag, const char *taskName, TaskHandle_t taskHandle)
{
	const char *safeTag = "<null>";
	const char *safeTaskName = "<null>";
	TaskStatus_t taskStatus = {0};
	UBaseType_t taskPriority = 0U;
	configSTACK_DEPTH_TYPE stackTotalWords = 0U;
	configSTACK_DEPTH_TYPE stackFreeMinWords = 0U;
	size_t stackUsedPeakWords = 0U;
	size_t stackTotalBytes = 0U;
	size_t stackFreeMinBytes = 0U;
	size_t stackUsedPeakBytes = 0U;

	if (NULL != tag) { safeTag = tag; }

	if (NULL == taskHandle)
	{
		testLog("\n[%s] 任务句柄为空，无法获取任务信息\n", safeTag);
		return;
	}

	vTaskGetInfo(taskHandle, &taskStatus, pdTRUE, eInvalid);
	if (NULL != taskName) { safeTaskName = taskName; }
	else if (NULL != taskStatus.pcTaskName) { safeTaskName = taskStatus.pcTaskName; }
	taskPriority = taskStatus.uxCurrentPriority;
	stackFreeMinWords = taskStatus.usStackHighWaterMark;

#if (portSTACK_GROWTH < 0)
	if ((NULL != taskStatus.pxStackBase) && (NULL != taskStatus.pxEndOfStack) && (taskStatus.pxEndOfStack >= taskStatus.pxStackBase))
	{
		stackTotalWords = (configSTACK_DEPTH_TYPE)(taskStatus.pxEndOfStack - taskStatus.pxStackBase + 1U);
	}
#elif (portSTACK_GROWTH > 0)
	if ((NULL != taskStatus.pxStackBase) && (NULL != taskStatus.pxEndOfStack) && (taskStatus.pxStackBase >= taskStatus.pxEndOfStack))
	{
		stackTotalWords = (configSTACK_DEPTH_TYPE)(taskStatus.pxStackBase - taskStatus.pxEndOfStack + 1U);
	}
#endif

	if (stackTotalWords >= stackFreeMinWords) { stackUsedPeakWords = stackTotalWords - stackFreeMinWords; }

	stackTotalBytes = stackTotalWords * sizeof(StackType_t);
	stackFreeMinBytes = stackFreeMinWords * sizeof(StackType_t);
	stackUsedPeakBytes = stackUsedPeakWords * sizeof(StackType_t);

	testLog("\n[%s] 任务=%s, Tick=%lu, 优先级=%lu, 栈总量=%lu(字)/%lu字节, 已用峰值=%lu(字)/%lu字节, 栈最小剩余=%lu(字)/%lu字节\n",
		safeTag, safeTaskName, (unsigned long)xTaskGetTickCount(), (unsigned long)taskPriority, (unsigned long)stackTotalWords,
		(unsigned long)stackTotalBytes, (unsigned long)stackUsedPeakWords, (unsigned long)stackUsedPeakBytes,
		(unsigned long)stackFreeMinWords, (unsigned long)stackFreeMinBytes);
}

int32_t unityTestGetUse(void)
{
	size_t total = 0U;
	size_t used = 0U;
	size_t maxUsed = 0U;

	if (NULL == gUnityTestTlsfCtx.tlsfHandle) { return 0; }
	tlsf_memory_info(gUnityTestTlsfCtx.tlsfHandle, &total, &used, &maxUsed);
	return (int32_t)used;
}

void *unityTestMalloc(size_t size)
{
	size_t allocSize = 0U;

	if (NULL == gUnityTestTlsfCtx.tlsfHandle || 0U == size) { return NULL; }

	if (0U != gAllocSimEnabled) { allocSize = unityTestCalcSimulatedAllocSize(size); }
	else
	{
		allocSize = size;
	}

	return tlsf_malloc(gUnityTestTlsfCtx.tlsfHandle, allocSize);
}

void unityTestFree(void *block)
{
	if (NULL == gUnityTestTlsfCtx.tlsfHandle || NULL == block) { return; }
	tlsf_free(gUnityTestTlsfCtx.tlsfHandle, block);
}

void *unityTestRealloc(void *block, size_t size)
{
	size_t allocSize = 0U;

	if (NULL == gUnityTestTlsfCtx.tlsfHandle) { return NULL; }
	if (0U == size) { return tlsf_realloc(gUnityTestTlsfCtx.tlsfHandle, block, 0U); }

	if (0U != gAllocSimEnabled) { allocSize = unityTestCalcSimulatedAllocSize(size); }
	else
	{
		allocSize = size;
	}

	return tlsf_realloc(gUnityTestTlsfCtx.tlsfHandle, block, allocSize);
}

void ryanJsonTestSetup(void)
{
	if (!unityTestInitTlsf())
	{
		testLog("%s:%d tlsf 初始化失败\r\n", __FILE__, __LINE__);
		return;
	}
#if defined(RyanJsonTestPlatformQemu)
	if (0U == gQemuTlsfPoolLogged)
	{
		testLog("[QEMU][MEM] tlsfPoolSize=%lu\r\n", (unsigned long)gUnityTestTlsfCtx.poolSize);
		gQemuTlsfPoolLogged = 1U;
	}
#endif
	if (0U == gAllocSimConfigLogged)
	{
		testLog("[MEM][SIM] header=%lu align=%lu\r\n", (unsigned long)RyanJsonTestAllocHeaderSize,
			(unsigned long)RyanJsonTestAllocAlignSize);
		gAllocSimConfigLogged = 1U;
	}

	xPortResetHeapMinimumEverFreeHeapSize();
	RyanJsonInitHooks(unityTestMalloc, unityTestFree, unityTestRealloc);
}

void ryanJsonTestTeardown(void)
{
	gUnityTestTlsfCtx.tlsfHandle = NULL;
}
