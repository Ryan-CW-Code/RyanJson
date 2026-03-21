#include "testBase.h"

/**
 * @brief 压力与边界测试
 *
 * @note 验证 RyanJson 处理大数据量（长 String、大 Array）的能力。
 */

static void testStressLongString(void)
{
	// 测试超长 String (10KB)
	const int32_t longStrLen = 10 * 1024;
	char *longStrVal = (char *)malloc(longStrLen + 1);
	TEST_ASSERT_NOT_NULL(longStrVal);
	memset(longStrVal, 'A', longStrLen);
	longStrVal[longStrLen] = '\0';

	RyanJson_t jsonStr = RyanJsonCreateString("longStr", longStrVal);
	TEST_ASSERT_NOT_NULL(jsonStr);
	TEST_ASSERT_EQUAL_STRING_MESSAGE(longStrVal, RyanJsonGetStringValue(jsonStr), "超长 String 读取不匹配");

	RyanJsonDelete(jsonStr);
	free(longStrVal);
}

static void testStressLargeArray(void)
{
	// 测试大 Array (1000 个 Int)
	const uint32_t arraySize = 1000;
	RyanJson_t array = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(array);
	for (uint32_t i = 0; i < arraySize; i++)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddIntToArray(array, (int32_t)i), "向大 Array 添加 Int 失败");
	}

	TEST_ASSERT_EQUAL_UINT32_MESSAGE(arraySize, (uint32_t)RyanJsonGetArraySize(array), "大 Array 长度错误");

	// 校验最后一个元素
	RyanJson_t lastItem = RyanJsonGetObjectByIndex(array, (int32_t)(arraySize - 1U));
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_EQUAL_INT32_MESSAGE(0, RyanJsonGetIntValue(lastItem), "大 Array 末尾元素错误");
#else
	TEST_ASSERT_EQUAL_INT32_MESSAGE((int32_t)(arraySize - 1U), RyanJsonGetIntValue(lastItem), "大 Array 末尾元素错误");
#endif

	RyanJsonDelete(array);
}

static void testStressPrint(void)
{
	// 序列化压力测试 (无格式)
	const uint32_t arraySize = 1000;
	RyanJson_t array = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(array);
	for (uint32_t i = 0; i < arraySize; i++)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddIntToArray(array, (int32_t)i), "向大 Array 添加 Int 失败");
	}

	char *printed = RyanJsonPrint(array, 8192, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(printed, "大 Array 序列化失败");

	RyanJsonFree(printed);
	RyanJsonDelete(array);
}

static void testStressLargeArrayRoundtrip(void)
{
	const uint32_t arraySize = 2048;
	RyanJson_t array = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(array);
	for (uint32_t i = 0; i < arraySize; i++)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddIntToArray(array, (int32_t)i), "向大 Array 添加 Int 失败");
	}

	// 关键位置抽样检查，避免仅检查尾节点导致漏检
	const uint32_t sampleIndex[] = {0U, 1U, 2U, 127U, 1023U, 1536U, 2047U};
	for (uint32_t i = 0; i < sizeof(sampleIndex) / sizeof(sampleIndex[0]); i++)
	{
		uint32_t idx = sampleIndex[i];
		RyanJson_t item = RyanJsonGetObjectByIndex(array, (int32_t)idx);
		TEST_ASSERT_NOT_NULL_MESSAGE(item, "大 Array 抽样索引越界");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(item), "大 Array 抽样元素类型错误");
#if true == RyanJsonDefaultAddAtHead
		TEST_ASSERT_EQUAL_INT32_MESSAGE((int32_t)(arraySize - 1U - idx), RyanJsonGetIntValue(item), "大 Array 抽样元素值错误");
#else
		TEST_ASSERT_EQUAL_INT32_MESSAGE((int32_t)idx, RyanJsonGetIntValue(item), "大 Array 抽样元素值错误");
#endif
	}

	uint32_t printLen = 0;
	char *printed = RyanJsonPrint(array, 0, RyanJsonFalse, &printLen);
	TEST_ASSERT_NOT_NULL_MESSAGE(printed, "大 Array 往返测试：序列化失败");
	TEST_ASSERT_TRUE_MESSAGE(printLen > arraySize, "大 Array 序列化长度异常");

	RyanJson_t parsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL_MESSAGE(parsed, "大 Array 往返测试：反序列化失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(array, parsed), "大 Array 往返测试：前后 Compare 不一致");

	RyanJsonDelete(parsed);
	RyanJsonFree(printed);
	RyanJsonDelete(array);
}

static void testStressLargeStringArrayPreallocated(void)
{
	const uint32_t arraySize = 1200;
	RyanJson_t array = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(array);

	for (uint32_t i = 0; i < arraySize; i++)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddStringToArray(array, "v"), "向 String 大 Array 添加元素失败");
	}

	uint32_t expectLen = 0;
	char *expect = RyanJsonPrint(array, 0, RyanJsonFalse, &expectLen);
	TEST_ASSERT_NOT_NULL_MESSAGE(expect, "String 大 Array 序列化失败");

	char *buf = (char *)malloc((size_t)expectLen + 1U);
	TEST_ASSERT_NOT_NULL(buf);

	char *out = RyanJsonPrintPreallocated(array, buf, expectLen + 1U, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(out, "String 大 Array 预分配刚好够用应成功");
	TEST_ASSERT_EQUAL_STRING_MESSAGE(expect, out, "String 大 Array 预分配输出不一致");

	RyanJson_t parsed = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL_MESSAGE(parsed, "String 大 Array 预分配结果解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(array, parsed), "String 大 Array 预分配结果前后 Compare 不一致");

	RyanJsonDelete(parsed);
	free(buf);
	RyanJsonFree(expect);
	RyanJsonDelete(array);
}

static void testStressLargeStringArrayPreallocatedNoTerminator(void)
{
	const uint32_t arraySize = 800;
	RyanJson_t array = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(array);

	for (uint32_t i = 0; i < arraySize; i++)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddStringToArray(array, "value"), "向 String 大 Array 添加元素失败");
	}

	uint32_t expectLen = 0;
	char *expect = RyanJsonPrint(array, 0, RyanJsonFalse, &expectLen);
	TEST_ASSERT_NOT_NULL(expect);

	char *buf = (char *)malloc((size_t)expectLen);
	TEST_ASSERT_NOT_NULL(buf);

	// 少 1 字节 '\0' 空间，预分配打印应失败
	char *out = RyanJsonPrintPreallocated(array, buf, expectLen, RyanJsonFalse, NULL);
	TEST_ASSERT_NULL_MESSAGE(out, "预分配缺少 '\\0' 空间应失败");

	free(buf);
	RyanJsonFree(expect);
	RyanJsonDelete(array);
}

static void testStressPrintOomLargeArray(void)
{
	RyanJson_t array = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(array);
	for (int32_t i = 0; i < 2000; i++)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddIntToArray(array, i), "向大 Array 添加 Int 失败");
	}

	// 首次分配失败
	UNITY_TEST_OOM_BEGIN(0);
	char *printed = RyanJsonPrint(array, 0, RyanJsonFalse, NULL);
	UNITY_TEST_OOM_END();
	if (printed) { RyanJsonFree(printed); }
	TEST_ASSERT_NULL_MESSAGE(printed, "Print OOM(首次分配失败) 应返回 NULL");

	// 首次分配成功，扩容失败
	UNITY_TEST_OOM_BEGIN(1);
	printed = RyanJsonPrint(array, 1, RyanJsonFalse, NULL);
	UNITY_TEST_OOM_END();
	if (printed) { RyanJsonFree(printed); }
	TEST_ASSERT_NULL_MESSAGE(printed, "Print OOM(扩容失败) 应返回 NULL");

	RyanJsonDelete(array);
}

static void testStressLargeObjectKeyLookup(void)
{
	const uint32_t keyCount = 1500;
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);

	for (uint32_t i = 0; i < keyCount; i++)
	{
		char key[24];
		RyanJsonSnprintf(key, sizeof(key), "k%04u", (unsigned)i);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddIntToObject(obj, key, (int32_t)i), "大 Object 添加 key 失败");
	}

	TEST_ASSERT_EQUAL_UINT32_MESSAGE(keyCount, (uint32_t)RyanJsonGetSize(obj), "大 Object 大小错误");

	// 热点索引查找：头/中/尾
	const uint32_t lookupIndex[] = {0U, keyCount / 2U, keyCount - 1U};
	for (uint32_t i = 0; i < sizeof(lookupIndex) / sizeof(lookupIndex[0]); i++)
	{
		char key[24];
		uint32_t idx = lookupIndex[i];
		RyanJsonSnprintf(key, sizeof(key), "k%04u", (unsigned)idx);
		RyanJson_t item = RyanJsonGetObjectByKey(obj, key);
		TEST_ASSERT_NOT_NULL_MESSAGE(item, "大 Object key 查找失败");
		TEST_ASSERT_EQUAL_INT32_MESSAGE((int32_t)idx, RyanJsonGetIntValue(item), "大 Object key 查找值错误");
	}

	// 替换中间节点并验证
	{
		char midKey[24];
		uint32_t mid = keyCount / 2U;
		RyanJsonSnprintf(midKey, sizeof(midKey), "k%04u", (unsigned)mid);
		RyanJson_t replaceItem = RyanJsonCreateInt(midKey, -12345);
		TEST_ASSERT_NOT_NULL(replaceItem);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(obj, midKey, replaceItem), "大 Object 中间节点替换失败");
		TEST_ASSERT_EQUAL_INT32_MESSAGE(-12345, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, midKey)),
						"大 Object 中间节点替换值错误");
	}

	uint32_t len = 0;
	char *printed = RyanJsonPrint(obj, 0, RyanJsonFalse, &len);
	TEST_ASSERT_NOT_NULL_MESSAGE(printed, "大 Object 序列化失败");
	TEST_ASSERT_TRUE_MESSAGE(len > keyCount * 6U, "大 Object 序列化长度异常");

	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL_MESSAGE(roundtrip, "大 Object 往返测试：解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(obj, roundtrip), "大 Object 往返测试：前后 Compare 不一致");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(obj);
}

void testStressRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testStressLongString);
	RUN_TEST(testStressLargeArray);
	RUN_TEST(testStressPrint);
	RUN_TEST(testStressLargeArrayRoundtrip);
	RUN_TEST(testStressLargeStringArrayPreallocated);
	RUN_TEST(testStressLargeStringArrayPreallocatedNoTerminator);
	RUN_TEST(testStressPrintOomLargeArray);
	RUN_TEST(testStressLargeObjectKeyLookup);
}
