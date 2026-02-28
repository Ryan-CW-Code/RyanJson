#include "testBase.h"

static void testDeleteEdgeCases(void)
{
	// 删除 NULL（应安全返回）
	RyanJsonDelete(NULL);

	// RyanJsonDeleteByKey 参数校验
	RyanJson_t obj = RyanJsonCreateObject();
	RyanJsonAddIntToObject(obj, "a", 1);

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonDeleteByKey(NULL, "a"), "DeleteByKey(NULL, key) 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonDeleteByKey(obj, NULL), "DeleteByKey(obj, NULL) 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonDeleteByKey(obj, "non_existent"), "DeleteByKey(不存在的key) 应返回 False");

	// RyanJsonDeleteByIndex 参数校验
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonDeleteByIndex(NULL, 0), "DeleteByIndex(NULL, 0) 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonDeleteByIndex(obj, 999), "DeleteByIndex(越界) 应返回 False");

	// 从非容器类型删除
	RyanJson_t num = RyanJsonCreateInt("num", 123);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonDeleteByKey(num, "a"), "从非容器 DeleteByKey 应返回 False"); // 内部 Detach 会做类型与存在性检查
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonDeleteByIndex(num, 0), "从非容器 DeleteByIndex 应返回 False");

	RyanJsonDelete(obj);
	RyanJsonDelete(num);
}

static void testDeleteMassiveItemsStress(void)
{
	// 连续删除直到空（Array）
	RyanJson_t arr = RyanJsonCreateArray();
	for (int32_t i = 0; i < 100; i++)
	{
		RyanJsonAddIntToArray(arr, i);
	}
	TEST_ASSERT_EQUAL_INT(100, RyanJsonGetSize(arr));

	// 从头部连续删除
	while (RyanJsonGetSize(arr) > 0)
	{
		TEST_ASSERT_TRUE(RyanJsonDeleteByIndex(arr, 0));
	}
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetSize(arr));

	// 空数组删除
	TEST_ASSERT_FALSE(RyanJsonDeleteByIndex(arr, 0));
	RyanJsonDelete(arr);

	// 连续删除直到空（Object）
	RyanJson_t obj = RyanJsonCreateObject();
	char key[16];
	for (int32_t i = 0; i < 100; i++)
	{
		snprintf(key, sizeof(key), "%d", i);
		RyanJsonAddIntToObject(obj, key, i);
	}
	TEST_ASSERT_EQUAL_INT(100, RyanJsonGetSize(obj));

	// 从头部连续删除（Object 也是链表，ByIndex=0 有效）
	while (RyanJsonGetSize(obj) > 0)
	{
		TEST_ASSERT_TRUE(RyanJsonDeleteByIndex(obj, 0));
	}
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetSize(obj));

	// 对象按 key 全量删除
	for (int32_t i = 0; i < 100; i++)
	{
		snprintf(key, sizeof(key), "%d", i);
		RyanJsonAddIntToObject(obj, key, i);
	}
	for (int32_t i = 0; i < 100; i++)
	{
		snprintf(key, sizeof(key), "%d", i);
		TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, key));
	}
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetSize(obj));

	// 空 Object 删除不存在的 key
	TEST_ASSERT_FALSE(RyanJsonDeleteByKey(obj, "any"));
	RyanJsonDelete(obj);
}

static void testDeleteSingleNodeAndReuse(void)
{
	// Object：删除唯一节点后应保持可复用
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "a", 1));
	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "a"));
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetSize(obj));
	TEST_ASSERT_NULL(RyanJsonGetObjectValue(obj));
	TEST_ASSERT_FALSE(RyanJsonDeleteByKey(obj, "a"));

	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(obj, "b", "ok"));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(obj));
	TEST_ASSERT_EQUAL_STRING("ok", RyanJsonGetStringValue(RyanJsonGetObjectByKey(obj, "b")));

	// Array：删除唯一节点后应保持可复用
	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 9));
	TEST_ASSERT_TRUE(RyanJsonDeleteByIndex(arr, 0));
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetSize(arr));
	TEST_ASSERT_NULL(RyanJsonGetObjectValue(arr));
	TEST_ASSERT_FALSE(RyanJsonDeleteByIndex(arr, 0));

	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 10));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(arr));
	TEST_ASSERT_EQUAL_INT(10, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));

	RyanJsonDelete(arr);
	RyanJsonDelete(obj);
}

static void testDeleteTailAndMiddleThenAppend(void)
{
	// Object：删除尾和中间节点后再次追加，验证链表仍可正常维护
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "a", 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "b", 2));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "c", 3));

	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "c"));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetSize(obj));
	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "b"));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(obj));

	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "d", 4));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetSize(obj));
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_EQUAL_INT(4, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(obj, 0)));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(obj, 1)));
#else
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(obj, 0)));
	TEST_ASSERT_EQUAL_INT(4, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(obj, 1)));
#endif

	// Array：同样覆盖“删尾/删中后再追加”路径
	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 10));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 20));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 30));

	TEST_ASSERT_TRUE(RyanJsonDeleteByIndex(arr, 2));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetSize(arr));
	TEST_ASSERT_TRUE(RyanJsonDeleteByIndex(arr, 1));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(arr));

	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 40));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetSize(arr));
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_EQUAL_INT(40, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));
	TEST_ASSERT_EQUAL_INT(30, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 1)));
#else
	TEST_ASSERT_EQUAL_INT(10, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));
	TEST_ASSERT_EQUAL_INT(40, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 1)));
#endif

	RyanJsonDelete(arr);
	RyanJsonDelete(obj);
}

static void testDeleteStandardOperations(void)
{

	// 保持原始 jsonStr，不做修改
	char jsonstr[] =
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":"
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],"
		"\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],"
		"\"array\":[16,16.89,\"hello\",true,false,null],"
		"\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}],"
		"\"string2222\":\"hello\"}";

	RyanJson_t json = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "解析 Json 失败");

	/**
	 * @brief 删除对象中的节点（头、中、尾）
	 */
	// 删除中间节点（double）
	RyanJsonDeleteByKey(json, "double");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(json, "double"), "删除中间节点 double 失败");

	// 删除头部节点（inter）
	RyanJsonDeleteByIndex(json, 0);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(json, "inter"), "删除头部节点 inter 失败");

	// 删除尾部节点（string2222）
	uint32_t lastIndex = RyanJsonGetSize(json) - 1;
	RyanJsonDeleteByIndex(json, lastIndex);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(json, "string2222"), "删除尾部节点 string2222 失败");

	/**
	 * @brief 删除数组中的元素（arrayInt）
	 */
	RyanJson_t array = RyanJsonGetObjectToKey(json, "arrayInt");
	TEST_ASSERT_NOT_NULL_MESSAGE(array, "获取 arrayInt 失败");

	// 删除数组首位
	RyanJsonDeleteByIndex(array, 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(4, RyanJsonGetSize(array), "删除数组首位后长度错误");

	// 删除数组中间元素
	RyanJsonDeleteByIndex(array, 1);
	TEST_ASSERT_EQUAL_INT_MESSAGE(3, RyanJsonGetSize(array), "删除数组中间元素后长度错误");

	// 删除数组尾部元素
	lastIndex = RyanJsonGetSize(array) - 1;
	RyanJsonDeleteByIndex(array, lastIndex);
	TEST_ASSERT_EQUAL_INT_MESSAGE(2, RyanJsonGetSize(array), "删除数组尾部元素后长度错误");

	/**
	 * @brief 深层嵌套删除（item）
	 */
	RyanJsonDeleteByKey(json, "item");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(json, "item"), "删除嵌套对象 item 失败");

	/**
	 * @brief 数组对象元素删除（arrayItem）
	 */
	RyanJson_t arrObj = RyanJsonGetObjectToKey(json, "arrayItem");
	TEST_ASSERT_NOT_NULL_MESSAGE(arrObj, "获取 arrayItem 失败");

	// 删除第一个对象
	RyanJsonDeleteByIndex(arrObj, 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, RyanJsonGetSize(arrObj), "删除数组首个对象后长度错误");

	// 删除最后一个对象
	RyanJsonDeleteByIndex(arrObj, 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, RyanJsonGetSize(arrObj), "删除数组最后一个对象后长度错误");

	/**
	 * @brief 特殊类型删除（null / bool）
	 */
	RyanJsonDeleteByKey(json, "null");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(json, "null"), "删除 null 节点失败");

	RyanJsonDeleteByKey(json, "boolTrue");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(json, "boolTrue"), "删除 boolTrue 节点失败");

	RyanJsonDeleteByKey(json, "boolFalse");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(json, "boolFalse"), "删除 boolFalse 节点失败");

	/**
	 * @brief 异常路径覆盖（健壮性）
	 */
	RyanJsonDeleteByKey(json, "non_exist"); // 删除不存在的 key
	RyanJsonDeleteByIndex(NULL, 0);         // 在 NULL 上操作

	RyanJsonDelete(json);
}

void testDeleteRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testDeleteEdgeCases);
	RUN_TEST(testDeleteSingleNodeAndReuse);
	RUN_TEST(testDeleteTailAndMiddleThenAppend);
	RUN_TEST(testDeleteStandardOperations);
	RUN_TEST(testDeleteMassiveItemsStress);
}
