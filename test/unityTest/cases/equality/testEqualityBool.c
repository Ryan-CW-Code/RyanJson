#include "testBase.h"

void testEqualityBoolEdgeCases(void)
{
	// NULL 输入
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsBool(NULL), "RyanJsonIsBool(NULL) 应返回 false");

	// 类型混淆测试
	RyanJson_t num = RyanJsonCreateInt("num", 123);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsBool(num), "RyanJsonIsBool(Int) 应返回 false");
	RyanJsonDelete(num);

	RyanJson_t str = RyanJsonCreateString("str", "true");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsBool(str), "RyanJsonIsBool(String) 应返回 false");
	RyanJsonDelete(str);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsBool(obj), "RyanJsonIsBool(Object) 应返回 false");
	RyanJsonDelete(obj);

	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsBool(arr), "RyanJsonIsBool(Array) 应返回 false");
	RyanJsonDelete(arr);

	RyanJson_t nullNode = RyanJsonCreateNull("null");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsBool(nullNode), "RyanJsonIsBool(Null) 应返回 false");
	RyanJsonDelete(nullNode);
}

/**
 * @brief boolValue 基础一致性测试
 */
static void testEqualityBoolBasic(void)
{
	// 测试 true
	{
		const char *jsonBoolStr = "{\"bool\":true}";
		RyanJson_t jsonRoot = RyanJsonParse(jsonBoolStr);
		TEST_ASSERT_NOT_NULL_MESSAGE(jsonRoot, "解析包含 true 的 Json 失败");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(RyanJsonGetObjectToKey(jsonRoot, "bool")), "字段 'bool' 不是 Bool 类型");
		TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonTrue, RyanJsonGetBoolValue(RyanJsonGetObjectToKey(jsonRoot, "bool")),
					      "字段 'bool' 的值不是 true");

		// 往返校验（序列化 -> 解析 -> 再验证）
		char *serializedStr = RyanJsonPrint(jsonRoot, 64, RyanJsonFalse, NULL);
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);
		TEST_ASSERT_NOT_NULL_MESSAGE(roundtripJson, "往返测试：重新解析失败");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(RyanJsonGetObjectToKey(roundtripJson, "bool")), "往返测试：字段 'bool' 类型错误");
		TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonTrue, RyanJsonGetBoolValue(RyanJsonGetObjectToKey(roundtripJson, "bool")),
					      "往返测试：字段 'bool' 的值错误");

		RyanJsonDelete(roundtripJson);
	}

	// 测试 false
	{
		const char *jsonBoolStr = "{\"bool\":false}";
		RyanJson_t jsonRoot = RyanJsonParse(jsonBoolStr);
		TEST_ASSERT_NOT_NULL_MESSAGE(jsonRoot, "解析包含 false 的 Json 失败");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(RyanJsonGetObjectToKey(jsonRoot, "bool")), "字段 'bool' 不是 Bool 类型");
		TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonFalse, RyanJsonGetBoolValue(RyanJsonGetObjectToKey(jsonRoot, "bool")),
					      "字段 'bool' 的值不是 false");

		// 往返测试
		char *serializedStr = RyanJsonPrint(jsonRoot, 64, RyanJsonFalse, NULL);
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);
		TEST_ASSERT_NOT_NULL_MESSAGE(roundtripJson, "往返测试：重新解析失败");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(RyanJsonGetObjectToKey(roundtripJson, "bool")), "往返测试：字段 'bool' 类型错误");
		TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonFalse, RyanJsonGetBoolValue(RyanJsonGetObjectToKey(roundtripJson, "bool")),
					      "往返测试：字段 'bool' 的值错误");

		RyanJsonDelete(roundtripJson);
	}
}

/**
 * @brief BoolArray 一致性测试
 */
static void testEqualityBoolArray(void)
{
	const char *jsonArrayStr = "[true, false, true, false]";
	RyanJson_t jsonRoot = RyanJsonParse(jsonArrayStr);
	TEST_ASSERT_NOT_NULL_MESSAGE(jsonRoot, "解析 BoolArray 失败");
	TEST_ASSERT_EQUAL_INT_MESSAGE(4, RyanJsonGetArraySize(jsonRoot), "Array 大小不正确");

	RyanJsonBool_e expected[] = {RyanJsonTrue, RyanJsonFalse, RyanJsonTrue, RyanJsonFalse};
	int32_t idx = 0;
	RyanJson_t item = NULL;
	RyanJsonArrayForEach(jsonRoot, item)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(item), "Array 元素不是 Bool 类型");
		TEST_ASSERT_EQUAL_INT_MESSAGE(expected[idx], RyanJsonGetBoolValue(item), "Array 元素值不匹配");
		idx++;
	}

	// 往返测试
	char *serializedStr = RyanJsonPrint(jsonRoot, 64, RyanJsonFalse, NULL);
	RyanJsonDelete(jsonRoot);

	RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
	RyanJsonFree(serializedStr);
	TEST_ASSERT_NOT_NULL_MESSAGE(roundtripJson, "往返测试：重新解析 Array 失败");
	TEST_ASSERT_EQUAL_INT_MESSAGE(4, RyanJsonGetArraySize(roundtripJson), "往返测试：Array 大小不正确");

	idx = 0;
	RyanJsonArrayForEach(roundtripJson, item)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(item), "往返测试：Array 元素不是 Bool 类型");
		TEST_ASSERT_EQUAL_INT_MESSAGE(expected[idx], RyanJsonGetBoolValue(item), "往返测试：Array 元素值不匹配");
		idx++;
	}

	RyanJsonDelete(roundtripJson);
}

void testEqualityBoolRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testEqualityBoolEdgeCases);
	RUN_TEST(testEqualityBoolBasic);
	RUN_TEST(testEqualityBoolArray);
}
