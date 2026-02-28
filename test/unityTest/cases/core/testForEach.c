#include "testBase.h"

static void testForEachEdgeCases(void)
{
	RyanJson_t item = NULL;

	// 遍历 NULL 对象 (应该安全跳过循环)
	int32_t count = 0;
	RyanJsonArrayForEach(NULL, item)
	{
		count++;
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, count, "遍历 NULL Array 应不执行循环");

	count = 0;
	RyanJsonObjectForEach(NULL, item)
	{
		count++;
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, count, "遍历 NULL Object 应不执行循环");

	// 遍历非容器对象 (应该同上)
	RyanJson_t num = RyanJsonCreateInt("num", 1);
	count = 0;
	RyanJsonArrayForEach(num, item)
	{
		count++;
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, count, "遍历非容器 Array 应不执行循环");

	count = 0;
	RyanJsonObjectForEach(num, item)
	{
		count++;
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, count, "遍历非容器 Object 应不执行循环");
	RyanJsonDelete(num);

	// 循环中断测试 (break)
	RyanJson_t arr = RyanJsonCreateArray();
	RyanJsonAddIntToArray(arr, 1);
	RyanJsonAddIntToArray(arr, 2);
	RyanJsonAddIntToArray(arr, 3);

	count = 0;
	RyanJsonArrayForEach(arr, item)
	{
		count++;
		if (RyanJsonGetIntValue(item) == 2) { break; }
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(2, count, "循环 break 测试失败");
	RyanJsonDelete(arr);
}

static void testForEachIterativeTraversals(void)
{
	char jsonstr[] = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":"
			 "{\"inter\":16,\"double\":16."
			 "89,\"string\":\"hello\","
			 "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,"
			 "16.89,16.89,16.89],"
			 "\"arrayString\":[\"hello\",\"hello\","
			 "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			 "\"double\":16.89,\"string\":"
			 "\"hello\",\"boolTrue\":true,"
			 "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			 "\"boolFalse\":false,\"null\":null}]}";

	RyanJson_t json = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "解析 Json 失败");

	RyanJson_t item = NULL;

	// 遍历 arrayDouble 数组测试
	RyanJsonArrayForEach(RyanJsonGetObjectToKey(json, "arrayDouble"), item)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDouble(item), "数组元素不是浮点数类型");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareDouble(16.89, RyanJsonGetDoubleValue(item)), "数组元素值不正确");
	}

	// 遍历 arrayInt 数组测试
	RyanJsonArrayForEach(RyanJsonGetObjectToKey(json, "arrayInt"), item)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(item), "数组元素不是整数类型");
		TEST_ASSERT_EQUAL_INT_MESSAGE(16, RyanJsonGetIntValue(item), "数组元素值不正确");
	}

	// 遍历 item 对象测试
	RyanJsonObjectForEach(RyanJsonGetObjectToKey(json, "item"), item)
	{
		TEST_ASSERT_NOT_NULL_MESSAGE(RyanJsonGetKey(item), "对象键值为空");
		char *str = RyanJsonPrint(item, 128, RyanJsonTrue, NULL);
		TEST_ASSERT_NOT_NULL_MESSAGE(str, "遍历项打印失败");
		RyanJsonFree(str);
	}

	RyanJsonDelete(json);
}

void testForEachRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testForEachEdgeCases);
	RUN_TEST(testForEachIterativeTraversals);
}
