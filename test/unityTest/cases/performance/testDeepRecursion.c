#define _GNU_SOURCE
#include "testBase.h"
#include "FreeRTOS.h"
#include "task.h"

// 使用轻量 xorshift 保持生成过程可复现且开销更低
static uint32_t gDeepRandState = 0x9E3779B9U;

typedef enum
{
	deepJsonModeMixed = 0,
	deepJsonModeObjectOnly = 1,
	deepJsonModeArrayOnly = 2
} deepJsonMode_e;

static deepJsonMode_e gDeepJsonMode = deepJsonModeMixed;
static int32_t gDeepJsonDepth = 10000;
static const size_t gDeepStackThreadWords = 512;

static const char *deepJsonModeName(deepJsonMode_e mode)
{
	if (deepJsonModeObjectOnly == mode) { return "object-only"; }
	if (deepJsonModeArrayOnly == mode) { return "array-only"; }
	return "mixed";
}

// 辅助：生成 0 到 max-1 的随机数
static inline uint32_t randomRange(uint32_t max)
{
	if (0U == max) { return 0U; }

	gDeepRandState ^= gDeepRandState << 13;
	gDeepRandState ^= gDeepRandState >> 17;
	gDeepRandState ^= gDeepRandState << 5;
	return gDeepRandState % max;
}

/**
 * @brief 生成深度嵌套的 Json 字符串
 *
 * @param depth 目标深度
 * @param outSize 输出生成的字符串长度
 * @return char* 动态分配的字符串，需要调用者 free
 */
static char *generateDeepJson(int32_t depth, size_t *outSize, deepJsonMode_e mode)
{
	/**
	 * @brief 生成缓冲区预估策略
	 *
	 * 每层结构平均约 4 字节，再叠加少量噪声负载与位图开销，使用 `depth * 6 + 1024`
	 * 可在控制内存占用的同时，显著降低深层 Json 生成过程中的扩容/越界风险。
	 */

	size_t bufCap = depth * 6 + 1024;
	char *jsonStr = (char *)malloc(bufCap);

	// 使用位图替代类型栈，减少辅助空间占用
	// bit=1 表示对象层，bit=0 表示数组层
	uint8_t *typeBitset = (uint8_t *)calloc((depth + 7) / 8, 1);

	if (!jsonStr || !typeBitset)
	{
		if (jsonStr) { free(jsonStr); }
		if (typeBitset) { free(typeBitset); }
		return NULL;
	}

	char *ptr = jsonStr;

	for (int32_t i = 0; i < depth; i++)
	{
		// 随机决定当前层类型：对象或数组
		bool isObject = false;
		if (deepJsonModeObjectOnly == mode) { isObject = true; }
		else if (deepJsonModeArrayOnly == mode) { isObject = false; }
		else
		{
			isObject = randomRange(2);
		}

		// 记录类型到位图
		if (isObject) { typeBitset[i / 8] |= (1 << (i % 8)); }

		if (isObject)
		{
			*ptr++ = '{';

			// 稀疏噪声注入：仅 5% 概率插入额外字段，兼顾覆盖率与内存占用
			if (deepJsonModeMixed == mode && randomRange(100) < 5)
			{
				// 随机生成短 key，避免与核心 key "n" 冲突
				char key = (char)('a' + randomRange(13)); // a-m
				int32_t type = (int32_t)randomRange(4);

				// 生成简短负载
				if (type == 0)
				{
					ptr += sprintf(ptr, "\"%c\":%d,", key, randomRange(9)); // int32_t
				}
				else if (type == 1)
				{
					ptr += sprintf(ptr, "\"%c\":true,", key); // bool
				}
				else if (type == 2)
				{
					ptr += sprintf(ptr, "\"%c\":null,", key); // null
				}
				else
				{
					ptr += sprintf(ptr, "\"%c\":\"s\",", key); // string
				}
			}

			// 核心嵌套 key："n"（next）
			memcpy(ptr, "\"n\":", 4);
			ptr += 4;
		}
		else // 数组层
		{
			*ptr++ = '[';

			// 稀疏噪声注入
			if (deepJsonModeMixed == mode && randomRange(100) < 5)
			{
				int32_t type = (int32_t)randomRange(4);
				if (type == 0) { ptr += sprintf(ptr, "%d,", randomRange(9)); }
				else if (type == 1) { memcpy(ptr, "false,", 6); }
				else if (type == 2) { memcpy(ptr, "null,", 5); }
				else
				{
					memcpy(ptr, "\"v\",", 4);
				}
			}
		}

		// 简单的越界保护
		if ((size_t)(ptr - jsonStr) >= bufCap - 128)
		{
			testLog("警告：Json 生成缓冲区在深度 %d 处接近耗尽\n", i);
			break;
		}
	}

	// 最内层终点
	memcpy(ptr, "\"end\"", 5);
	ptr += 5;

	// 回溯闭合：倒序读取位图并补齐 ] / }
	for (int32_t i = depth - 1; i >= 0; i--)
	{
		int32_t isObject = (typeBitset[i / 8] >> (i % 8)) & 1;
		if (isObject) { *ptr++ = '}'; }
		else
		{
			*ptr++ = ']';
		}
	}
	*ptr = '\0';

	if (outSize) { *outSize = (size_t)(ptr - jsonStr); }

	free(typeBitset); // 释放位图
	return jsonStr;
}

/**
 * @brief 深度递归与栈占用压力测试
 *
 * 在 16KB 栈环境中验证 Parse、Print、Duplicate、Delete 等核心路径是否以迭代方式工作，
 * 并在深层嵌套数据下避免栈溢出风险。
 */
static void deepStackTask(void)
{
	const int32_t depth = gDeepJsonDepth;
	const deepJsonMode_e mode = gDeepJsonMode;
	size_t jsonLen = 0;
	const char *modeName = deepJsonModeName(mode);

	gDeepRandState = 0x9E3779B9U ^ (uint32_t)depth ^ (((uint32_t)mode + 1U) << 24);
	testLog("正在生成深度 %d 的测试 Json（mode=%s）...\n", depth, modeName);
	char *jsonStr = generateDeepJson(depth, &jsonLen, mode);
	TEST_ASSERT_NOT_NULL(jsonStr);

	// 执行 Minify（压缩，迭代路径验证）
	testLog("正在运行 Minify...\n");
	RyanJsonMinify(jsonStr, (int32_t)jsonLen);

	// 执行 Parse（解析，迭代路径验证）
	testLog("正在运行 Parse，深度 %d ...\n", depth);
	RyanJson_t json = RyanJsonParse(jsonStr);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "深层嵌套解析失败 (返回了 NULL)");
	free(jsonStr);

	// 执行 Duplicate（复制，迭代路径验证）
	testLog("正在运行 Duplicate...\n");
	RyanJson_t dup = RyanJsonDuplicate(json);
	TEST_ASSERT_NOT_NULL_MESSAGE(dup, "深层嵌套复制失败 (返回了 NULL)");

	// 执行 Compare（比较，迭代路径验证）
	testLog("正在运行 Compare...\n");
	RyanJsonBool_e eq = RyanJsonCompare(json, dup);
	TEST_ASSERT_TRUE_MESSAGE(eq, "深层嵌套比较失败 (结果不相等)");
	RyanJsonDelete(dup);

	// 执行 Print（打印，迭代路径验证）
	testLog("正在运行 Print (无格式化)...\n");
	uint32_t len = 0;
	char *text = RyanJsonPrint(json, 1024, RyanJsonFalse, &len);
	TEST_ASSERT_NOT_NULL_MESSAGE(text, "深层嵌套打印失败 (返回了 NULL)");
	TEST_ASSERT_TRUE_MESSAGE(len == jsonLen, "打印输出长度异常");
	RyanJsonFree(text);

	// 格式化打印
	// 占用堆太大，不够用
	// printf("正在运行 Print (格式化)...\n");
	// text = RyanJsonPrint(json, 1024, RyanJsonTrue, &len);
	// TEST_ASSERT_NOT_NULL_MESSAGE(text, "深层嵌套格式化打印失败");
	// RyanJsonFree(text);

	// GetSize（统计节点数量，迭代路径验证）
	testLog("正在运行 GetSize...\n");
	uint32_t size = RyanJsonGetSize(json);
	TEST_ASSERT_TRUE(size > 0);

	// 深层遍历与修改测试
	testLog("正在运行深度遍历...\n");
	RyanJson_t current = json;
	RyanJson_t parent = NULL;
	int32_t actualDepth = 0;

	// 逐层向下遍历到最深节点
	while (current)
	{
		if (RyanJsonIsObject(current))
		{
			parent = current;
			current = RyanJsonGetObjectByKey(current, "n");
		}
		else if (RyanJsonIsArray(current))
		{
			// 对于本测试生成的结构，下一个层级（或负载）总是数组最后一个元素。
			// 旧逻辑只查找 Object/Array，末层为 "end"（String/Number）时会少算 1 层。
			RyanJson_t child = RyanJsonGetObjectValue(current);
			if (child)
			{
				while (RyanJsonGetNext(child))
				{
					child = RyanJsonGetNext(child);
				}
				// child 此时为最后一个节点（容器或负载）
				parent = current;
				current = child;
			}
			else
			{
				// 生成器理论上不会产出空数组，这里保留防御性分支
				current = NULL;
			}
		}
		else
		{
			break; // 已到达负载节点
		}

		if (current) { actualDepth++; }
	}

	testLog("实际遍历深度: %d (预期 %d)\n", actualDepth, depth);
	TEST_ASSERT_TRUE_MESSAGE(actualDepth == depth, "遍历深度严重偏离预期");
	TEST_ASSERT_NOT_NULL(parent);

	// 深层插入
	testLog("正在运行深层插入...\n");
	RyanJsonBool_e insertOk = RyanJsonFalse;
	if (RyanJsonIsObject(parent)) { insertOk = RyanJsonAddStringToObject(parent, "inserted_key", "val"); }
	else if (RyanJsonIsArray(parent))
	{
		RyanJson_t newItem = RyanJsonCreateString(NULL, "val");
		insertOk = RyanJsonInsert(parent, 0, newItem);
		if (RyanJsonFalse == insertOk) { RyanJsonDelete(newItem); }
	}
	TEST_ASSERT_TRUE_MESSAGE(insertOk, "Deep Insert 失败");

	// 深层替换
	testLog("正在运行深层替换...\n");
	RyanJson_t replaceItem = RyanJsonCreateInt("replaced", 999);
	if (RyanJsonIsObject(parent))
	{
		// RyanJsonReplaceByKey 会替换 key 为 "inserted_key" 的节点
		if (RyanJsonFalse == RyanJsonReplaceByKey(parent, "inserted_key", replaceItem))
		{
			RyanJsonDelete(replaceItem);
			replaceItem = NULL;
		}
	}
	else if (RyanJsonIsArray(parent))
	{
		if (RyanJsonFalse == RyanJsonReplaceByIndex(parent, 0, replaceItem))
		{
			RyanJsonDelete(replaceItem);
			replaceItem = NULL;
		}
	}
	else
	{
		RyanJsonDelete(replaceItem);
		replaceItem = NULL;
	}

	// 深层分离
	testLog("正在运行深层分离...\n");
	RyanJson_t detached = NULL;
	if (RyanJsonIsObject(parent)) { detached = RyanJsonDetachByKey(parent, "inserted_key"); }
	else if (RyanJsonIsArray(parent)) { detached = RyanJsonDetachByIndex(parent, 0); }

	if (detached) { RyanJsonDelete(detached); }
	else
	{
		testLog("警告：Detach 返回了 NULL。\n");
	}

	// 删除整棵 Json 树
	testLog("正在运行 Delete Json...\n");
	RyanJsonDelete(json);
}

typedef struct
{
	int32_t depth;
	deepJsonMode_e mode;
	uint8_t isTestProtectPassed;
} deepStressThreadCtx_t;

static void deepStressThreadTask(void *arg)
{
	deepStressThreadCtx_t *threadCtx = (deepStressThreadCtx_t *)arg;
	const char *modeName = "mixed";
	TaskHandle_t currentTask = NULL;
	char taskName[64] = {0};

	if (NULL == threadCtx) { return; }
	threadCtx->isTestProtectPassed = 0U;
	gDeepJsonDepth = threadCtx->depth;
	gDeepJsonMode = threadCtx->mode;

	if (TEST_PROTECT())
	{
		deepStackTask();
		threadCtx->isTestProtectPassed = 1U;
	}

	modeName = deepJsonModeName(threadCtx->mode);
	testLog("[deep] 用例参数: 模式=%s, 深度=%ld\n", modeName, (long)threadCtx->depth);
	(void)snprintf(taskName, sizeof(taskName), "deep-%s", modeName);
	currentTask = xTaskGetCurrentTaskHandle();
	logTaskStackRuntimeInfoByHandle("deep", taskName, currentTask);
}

static void runDeepStackUsageStressCase(int32_t depth, deepJsonMode_e mode)
{
	int32_t runRet = 0;
	deepStressThreadCtx_t threadCtx = {0};

	threadCtx.depth = depth;
	threadCtx.mode = mode;
	runRet = testPlatformRunThreadWithStackSize(deepStressThreadTask, &threadCtx, gDeepStackThreadWords);
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, runRet, "deep 测试线程启动失败");
	TEST_ASSERT_TRUE_MESSAGE(0U != threadCtx.isTestProtectPassed, "deep 测试线程执行失败");
}

static void testDeepStackUsageStress(void)
{
	runDeepStackUsageStressCase(10000, deepJsonModeMixed);
}

static void testDeepStackUsageObjectOnlyStress(void)
{
	runDeepStackUsageStressCase(10000, deepJsonModeObjectOnly);
}

static void testDeepStackUsageArrayOnlyStress(void)
{
	runDeepStackUsageStressCase(10000, deepJsonModeArrayOnly);
}

void testDeepRecursionRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testDeepStackUsageStress);
	RUN_TEST(testDeepStackUsageObjectOnlyStress);
	RUN_TEST(testDeepStackUsageArrayOnlyStress);
}
