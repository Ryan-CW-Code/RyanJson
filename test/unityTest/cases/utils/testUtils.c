
#include "testBase.h"
/**
 * @brief Json 公共校验辅助函数
 */

void printJsonDebug(RyanJson_t json)
{
	char *str = RyanJsonPrint(json, 1024, RyanJsonTrue, NULL);
	testLog("aa %s\r\n", str);
	RyanJsonFree(str);
}

void rootNodeCheckTest(RyanJson_t json)
{
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "根节点检查：输入 Json 为空");

	// 校验整数字段
	RyanJson_t inter = RyanJsonGetObjectToKey(json, "inter");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(inter), "字段 'inter' 不是整数类型");
	TEST_ASSERT_EQUAL_INT_MESSAGE(16, RyanJsonGetIntValue(inter), "字段 'inter' 值不正确");

	// 校验浮点数字段
	RyanJson_t dbl = RyanJsonGetObjectToKey(json, "double");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDouble(dbl), "字段 'double' 不是浮点数类型");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareDouble(RyanJsonGetDoubleValue(dbl), 16.89), "字段 'double' 值不正确");

	// 校验字符串字段
	RyanJson_t str = RyanJsonGetObjectToKey(json, "string");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(str), "字段 'string' 不是字符串类型");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("hello", RyanJsonGetStringValue(str), "字段 'string' 值不正确");

	// 校验布尔字段（true）
	RyanJson_t bTrue = RyanJsonGetObjectToKey(json, "boolTrue");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(bTrue), "字段 'boolTrue' 不是布尔类型");
	TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonTrue, RyanJsonGetBoolValue(bTrue), "字段 'boolTrue' 值不正确");

	// 校验布尔字段（false）
	RyanJson_t bFalse = RyanJsonGetObjectToKey(json, "boolFalse");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(bFalse), "字段 'boolFalse' 不是布尔类型");
	TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonFalse, RyanJsonGetBoolValue(bFalse), "字段 'boolFalse' 值不正确");

	// 校验 null 字段
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsNull(RyanJsonGetObjectToKey(json, "null")), "字段 'null' 不是 Null 类型");
}

void itemNodeCheckTest(RyanJson_t json)
{
	RyanJson_t item = RyanJsonGetObjectToKey(json, "item");
	TEST_ASSERT_NOT_NULL_MESSAGE(item, "字段 'item' 不存在");
	rootNodeCheckTest(item);
}

void arrayNodeCheckTest(RyanJson_t json, RyanJsonBool_e isReversed)
{
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayInt")), "arrayInt 不是数组");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayDouble")), "arrayDouble 不是数组");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayString")), "arrayString 不是数组");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsArray(RyanJsonGetObjectToKey(json, "array")), "array 不是数组");

	// 校验混合数组 array：[16, 16.89, "hello", true, false, null]
	RyanJson_t array = RyanJsonGetObjectToKey(json, "array");
	TEST_ASSERT_EQUAL_INT_MESSAGE(6, RyanJsonGetSize(array), "混合数组长度不正确");

	if (isReversed)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsNull(RyanJsonGetObjectByIndex(array, 0)), "混合数组[0]不是 Null");

		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(RyanJsonGetObjectByIndex(array, 1)), "混合数组[1]不是布尔值");
		TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonFalse, RyanJsonGetBoolValue(RyanJsonGetObjectByIndex(array, 1)), "混合数组[1]值错误");

		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(RyanJsonGetObjectByIndex(array, 2)), "混合数组[2]不是布尔值");
		TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonTrue, RyanJsonGetBoolValue(RyanJsonGetObjectByIndex(array, 2)), "混合数组[2]值错误");

		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(RyanJsonGetObjectByIndex(array, 3)), "混合数组[3]不是字符串");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("hello", RyanJsonGetStringValue(RyanJsonGetObjectByIndex(array, 3)), "混合数组[3]值错误");

		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDouble(RyanJsonGetObjectByIndex(array, 4)), "混合数组[4]不是浮点数");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareDouble(RyanJsonGetDoubleValue(RyanJsonGetObjectByIndex(array, 4)), 16.89),
					 "混合数组[4]值错误");

		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(RyanJsonGetObjectByIndex(array, 5)), "混合数组[5]不是整数");
		TEST_ASSERT_EQUAL_INT_MESSAGE(16, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(array, 5)), "混合数组[5]值错误");
	}
	else
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(RyanJsonGetObjectByIndex(array, 0)), "混合数组[0]不是整数");
		TEST_ASSERT_EQUAL_INT_MESSAGE(16, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(array, 0)), "混合数组[0]值错误");

		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDouble(RyanJsonGetObjectByIndex(array, 1)), "混合数组[1]不是浮点数");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareDouble(RyanJsonGetDoubleValue(RyanJsonGetObjectByIndex(array, 1)), 16.89),
					 "混合数组[1]值错误");

		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(RyanJsonGetObjectByIndex(array, 2)), "混合数组[2]不是字符串");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("hello", RyanJsonGetStringValue(RyanJsonGetObjectByIndex(array, 2)), "混合数组[2]值错误");

		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(RyanJsonGetObjectByIndex(array, 3)), "混合数组[3]不是布尔值");
		TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonTrue, RyanJsonGetBoolValue(RyanJsonGetObjectByIndex(array, 3)), "混合数组[3]值错误");

		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(RyanJsonGetObjectByIndex(array, 4)), "混合数组[4]不是布尔值");
		TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonFalse, RyanJsonGetBoolValue(RyanJsonGetObjectByIndex(array, 4)), "混合数组[4]值错误");

		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsNull(RyanJsonGetObjectByIndex(array, 5)), "混合数组[5]不是 Null");
	}

	// 校验强类型数组
	RyanJson_t arrayInt = RyanJsonGetObjectToKey(json, "arrayInt");
	for (int32_t i = 0; i < RyanJsonGetSize(arrayInt); i++)
	{
		RyanJson_t item = RyanJsonGetObjectByIndex(arrayInt, i);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(item), "arrayInt 元素不是整数");
		TEST_ASSERT_EQUAL_INT_MESSAGE(16, RyanJsonGetIntValue(item), "arrayInt 元素值错误");
	}

	RyanJson_t arrayDouble = RyanJsonGetObjectToKey(json, "arrayDouble");
	for (int32_t i = 0; i < RyanJsonGetSize(arrayDouble); i++)
	{
		RyanJson_t item = RyanJsonGetObjectByIndex(arrayDouble, i);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDouble(item), "arrayDouble 元素不是浮点数");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareDouble(RyanJsonGetDoubleValue(item), 16.89), "arrayDouble 元素值错误");
	}

	RyanJson_t arrayString = RyanJsonGetObjectToKey(json, "arrayString");
	for (int32_t i = 0; i < RyanJsonGetSize(arrayString); i++)
	{
		RyanJson_t item = RyanJsonGetObjectByIndex(arrayString, i);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(item), "arrayString 元素不是字符串");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("hello", RyanJsonGetStringValue(item), "arrayString 元素值错误");
	}
}

void arrayItemNodeCheckTest(RyanJson_t json)
{
	RyanJson_t arrayItem = RyanJsonGetObjectToKey(json, "arrayItem");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsArray(arrayItem), "arrayItem 不是数组");
	TEST_ASSERT_EQUAL_INT_MESSAGE(2, RyanJsonGetSize(arrayItem), "arrayItem 长度错误");

	rootNodeCheckTest(RyanJsonGetObjectToIndex(arrayItem, 0));
	rootNodeCheckTest(RyanJsonGetObjectToIndex(arrayItem, 1));
}

void testCheckRootEx(RyanJson_t pJson, RyanJsonBool_e isReversed)
{
	rootNodeCheckTest(pJson);
	itemNodeCheckTest(pJson);
	arrayNodeCheckTest(pJson, isReversed);
	arrayItemNodeCheckTest(pJson);
}

void testCheckRoot(RyanJson_t pJson)
{
	testCheckRootEx(pJson, RyanJsonFalse);
}

static void testUtilsBasic(void)
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
	TEST_ASSERT_NOT_NULL(json);

	// 调用全部校验辅助函数
	testCheckRoot(json);

	RyanJsonDelete(json);
}

static void testUtilsMinifyNoExtraSpace(void)
{
	char buf[4];
	buf[0] = 'a';
	buf[1] = 'b';
	buf[2] = 'c';
	buf[3] = 'X'; // 哨兵：不应被覆盖

	uint32_t count = RyanJsonMinify(buf, 3);
	TEST_ASSERT_EQUAL_UINT32(3, count);
	TEST_ASSERT_EQUAL_UINT8('a', (uint8_t)buf[0]);
	TEST_ASSERT_EQUAL_UINT8('b', (uint8_t)buf[1]);
	TEST_ASSERT_EQUAL_UINT8('c', (uint8_t)buf[2]);
	TEST_ASSERT_EQUAL_UINT8('X', (uint8_t)buf[3]);
}

static void testUtilsMinifyWriteTerminatorWhenSpaceRemain(void)
{
	/*
	 * textLen=4 时，压缩后 "abc" 长度为 3：
	 * - 返回值应为 3
	 * - 因为 3 < 4，函数应在 buf[3] 写入 '\0'
	 * - buf[4] 是哨兵，必须保持不变
	 */
	char buf[5];
	buf[0] = 'a';
	buf[1] = ' ';
	buf[2] = 'b';
	buf[3] = 'c';
	buf[4] = '#';

	uint32_t count = RyanJsonMinify(buf, 4);
	TEST_ASSERT_EQUAL_UINT32(3, count);
	TEST_ASSERT_EQUAL_UINT8('a', (uint8_t)buf[0]);
	TEST_ASSERT_EQUAL_UINT8('b', (uint8_t)buf[1]);
	TEST_ASSERT_EQUAL_UINT8('c', (uint8_t)buf[2]);
	TEST_ASSERT_EQUAL_UINT8('\0', (uint8_t)buf[3]);
	TEST_ASSERT_EQUAL_UINT8('#', (uint8_t)buf[4]);
}

static void testUtilsVarargsPathHelpers(void)
{
	RyanJson_t root = RyanJsonCreateObject();

	RyanJson_t objA = RyanJsonCreateObject();
	RyanJson_t objB = RyanJsonCreateObject();
	RyanJsonAddIntToObject(objB, "c", 3);
	RyanJsonAddItemToObject(objA, "b", objB);
	RyanJsonAddItemToObject(root, "a", objA);

	RyanJson_t c = RyanJsonGetObjectToKey(root, "a", "b", "c");
	TEST_ASSERT_NOT_NULL_MESSAGE(c, "GetObjectToKey 多级路径失败");
	TEST_ASSERT_EQUAL_INT_MESSAGE(3, RyanJsonGetIntValue(c), "多级路径取值错误");

	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(root, "a", "missing"), "不存在的路径应返回 NULL");

	RyanJson_t arr = RyanJsonCreateArray();
	RyanJson_t sub = RyanJsonCreateArray();
	RyanJsonAddIntToArray(sub, 7);
	RyanJsonAddItemToArray(arr, sub);
	RyanJsonAddItemToObject(root, "arr", arr);

	RyanJson_t arrNode = RyanJsonGetObjectToKey(root, "arr");
	RyanJson_t v = RyanJsonGetObjectToIndex(arrNode, 0, 0);
	TEST_ASSERT_NOT_NULL_MESSAGE(v, "GetObjectToIndex 多级索引失败");
	TEST_ASSERT_EQUAL_INT_MESSAGE(7, RyanJsonGetIntValue(v), "多级索引取值错误");

	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToIndex(arrNode, 1), "越界索引应返回 NULL");

	RyanJsonDelete(root);
}

void testUtilsRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testUtilsBasic);
	RUN_TEST(testUtilsMinifyNoExtraSpace);
	RUN_TEST(testUtilsMinifyWriteTerminatorWhenSpaceRemain);
	RUN_TEST(testUtilsVarargsPathHelpers);
}
