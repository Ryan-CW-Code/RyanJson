#include "testBase.h"

static void testDetachEdgeCases(void)
{
	// 分离接口输入 NULL
	TEST_ASSERT_NULL_MESSAGE(RyanJsonDetachByKey(NULL, "key"), "DetachByKey(NULL, key) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonDetachByIndex(NULL, 0), "DetachByIndex(NULL, 0) 应返回 NULL");

	// 分离不存在的元素
	RyanJson_t obj = RyanJsonCreateObject();
	RyanJsonAddIntToObject(obj, "a", 1);

	TEST_ASSERT_NULL_MESSAGE(RyanJsonDetachByKey(obj, "non_existent"), "DetachByKey(不存在) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonDetachByIndex(obj, 99), "DetachByIndex(越界) 应返回 NULL");

	// 从非容器节点分离
	RyanJson_t val = RyanJsonCreateString("str", "value");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonDetachByKey(val, "a"), "从 String DetachByKey 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonDetachByIndex(val, 0), "从 String DetachByIndex 应返回 NULL");

	RyanJsonDelete(obj);
	RyanJsonDelete(val);
}

static void testDetachDuplicateKey(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "dup", 1));
#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonAddIntToObject(obj, "dup", 2), "严格模式下对象不应允许重复 key");
#else
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddIntToObject(obj, "dup", 2), "非严格模式下对象应允许重复 key");
#endif

	RyanJson_t only = RyanJsonGetObjectByKey(obj, "dup");
	TEST_ASSERT_NOT_NULL(only);
#if true == RyanJsonDefaultAddAtHead && false == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(only));
#else
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(only));
#endif

	RyanJson_t detached = RyanJsonDetachByKey(obj, "dup");
	TEST_ASSERT_NOT_NULL(detached);
#if true == RyanJsonDefaultAddAtHead && false == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(detached));
#else
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(detached));
#endif
	RyanJsonDelete(detached);

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(obj, "dup"));
#else
	RyanJson_t second = RyanJsonGetObjectByKey(obj, "dup");
	TEST_ASSERT_NOT_NULL(second);
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(second));
#else
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(second));
#endif
	RyanJson_t detached2 = RyanJsonDetachByKey(obj, "dup");
	TEST_ASSERT_NOT_NULL(detached2);
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(detached2));
#else
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(detached2));
#endif
	RyanJsonDelete(detached2);
	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(obj, "dup"));
#endif

	RyanJsonDelete(obj);
}

static void testDetachCrossObject(void)
{
	// 从一个 Object 分离节点并迁移到另一个 Object
	RyanJson_t obj1 = RyanJsonCreateObject();
	RyanJsonAddIntToObject(obj1, "move_me", 100);

	RyanJson_t obj2 = RyanJsonCreateObject();

	RyanJson_t item = RyanJsonDetachByKey(obj1, "move_me");
	TEST_ASSERT_NOT_NULL(item);
	TEST_ASSERT_EQUAL_INT(100, RyanJsonGetIntValue(item));

	// 迁移到 obj2
	// 注意：RyanJsonAddItemToObject 会创建包装节点，可能引入额外层级。
	// 这里使用 ChangeKey + Insert，表达“移动并重命名”的语义。
	RyanJsonChangeKey(item, "moved");
	RyanJsonInsert(obj2, UINT32_MAX, item);

	TEST_ASSERT_EQUAL_INT(100, RyanJsonGetIntValue(RyanJsonGetObjectToKey(obj2, "moved")));

	RyanJsonDelete(obj1);
	RyanJsonDelete(obj2);
}

static void testDetachReInsert(void)
{
	// 分离 -> 修改 -> 重新插入
	RyanJson_t arr = RyanJsonCreateArray();
	RyanJsonAddIntToArray(arr, 10);

	RyanJson_t item = RyanJsonDetachByIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(item);
	TEST_ASSERT_EQUAL_INT(10, RyanJsonGetIntValue(item));

	RyanJsonAddIntToArray(arr, 20); // 当前数组为 [20]

	// 使用 RyanJsonInsert 直接插入，避免 RyanJsonAddItemToArray 的包装行为
	RyanJsonInsert(arr, UINT32_MAX, item); // 当前数组为 [20, 10]

	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetArraySize(arr));
	TEST_ASSERT_EQUAL_INT(20, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));
	TEST_ASSERT_EQUAL_INT(10, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 1)));

	RyanJsonDelete(arr);
}

static void testDetachSingleNodeAndReuse(void)
{
	// Object：单节点分离后应变为空 Object，并可重新插回
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "only", 11));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(obj));

	RyanJson_t detachedObjItem = RyanJsonDetachByKey(obj, "only");
	TEST_ASSERT_NOT_NULL(detachedObjItem);
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(detachedObjItem));
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetSize(obj));
	TEST_ASSERT_NULL(RyanJsonGetObjectValue(obj));
	TEST_ASSERT_NULL(RyanJsonDetachByKey(obj, "only"));

	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, detachedObjItem));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(obj));
	TEST_ASSERT_EQUAL_INT(11, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "only")));

	// Array：单节点分离后应变为空 Array，并可重新插回
	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 22));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(arr));

	RyanJson_t detachedArrItem = RyanJsonDetachByIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(detachedArrItem);
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(detachedArrItem));
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetSize(arr));
	TEST_ASSERT_NULL(RyanJsonGetObjectValue(arr));
	TEST_ASSERT_NULL(RyanJsonDetachByIndex(arr, 0));

	TEST_ASSERT_TRUE(RyanJsonInsert(arr, UINT32_MAX, detachedArrItem));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(arr));
	TEST_ASSERT_EQUAL_INT(22, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));

	RyanJsonDelete(obj);
	RyanJsonDelete(arr);
}

static void testDetachTailAndMiddleThenAppend(void)
{
	// Object：先分离尾节点，再分离中间节点，最后追加新节点，验证链表修复正确
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "a", 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "b", 2));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "c", 3));

	RyanJson_t detachedTail = RyanJsonDetachByKey(obj, "c");
	TEST_ASSERT_NOT_NULL(detachedTail);
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(detachedTail));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetSize(obj));
	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(obj, "c"));
	RyanJsonDelete(detachedTail);

	RyanJson_t detachedMiddle = RyanJsonDetachByKey(obj, "b");
	TEST_ASSERT_NOT_NULL(detachedMiddle);
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(detachedMiddle));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(obj));
	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(obj, "b"));
	RyanJsonDelete(detachedMiddle);

	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "d", 4));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetSize(obj));
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_EQUAL_INT(4, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(obj, 0)));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(obj, 1)));
#else
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(obj, 0)));
	TEST_ASSERT_EQUAL_INT(4, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(obj, 1)));
#endif

	// Array：同样覆盖“尾/中分离后再追加”的路径
	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 10));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 20));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 30));

	RyanJson_t arrTail = RyanJsonDetachByIndex(arr, 2);
	TEST_ASSERT_NOT_NULL(arrTail);
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(arrTail));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetSize(arr));
	RyanJsonDelete(arrTail);

	RyanJson_t arrMiddle = RyanJsonDetachByIndex(arr, 1);
	TEST_ASSERT_NOT_NULL(arrMiddle);
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(arrMiddle));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(arr));
	RyanJsonDelete(arrMiddle);

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

static void testDetachStandardOperations(void)
{

	char jsonstr[] =
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":"
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],"
		"\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],"
		"\"array\":[16,16.89,\"hello\",true,false,null],"
		"\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{"
		"\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,"
		"\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,"
		"\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}],"
		"\"string2222\":\"hello\"}";

	RyanJson_t json = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "解析 Json 失败");

	/**
	 * @brief 对象子项分离测试（头、中、尾）
	 */
	{
		// 头部（第一个 key：inter）
		RyanJson_t detached = RyanJsonDetachByIndex(json, 0);
		TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离头部项 inter 失败");
		RyanJsonDelete(detached);
		TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(json, "inter"), "分离后 inter 依然存在");

		// 中间（double）
		detached = RyanJsonDetachByKey(json, "double");
		TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离中间项 double 失败");
		RyanJsonDelete(detached);
		TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(json, "double"), "分离后 double 依然存在");

		// 尾部（最后一个 key：string2222）
		uint32_t lastIndex = RyanJsonGetSize(json) - 1;
		detached = RyanJsonDetachByIndex(json, lastIndex);
		TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离尾部项 string2222 失败");
		RyanJsonDelete(detached);
		TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(json, "string2222"), "分离后 string2222 依然存在");
	}

	/**
	 * @brief 数组元素分离测试 (arrayInt / arrayDouble / arrayString)
	 */
	{
		RyanJson_t arrInt = RyanJsonGetObjectByKey(json, "arrayInt");
		TEST_ASSERT_NOT_NULL_MESSAGE(arrInt, "获取 arrayInt 失败");
		uint32_t size = RyanJsonGetSize(arrInt);
		RyanJson_t detached = RyanJsonDetachByIndex(arrInt, 0);
		TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离 arrayInt 头部项失败");
		RyanJsonDelete(detached);
		TEST_ASSERT_EQUAL_INT_MESSAGE(size - 1, RyanJsonGetSize(arrInt), "分离 arrayInt 头部后长度未减少");

		// 中间
		size = RyanJsonGetSize(arrInt);
		detached = RyanJsonDetachByIndex(arrInt, 1);
		TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离 arrayInt 中间项失败");
		RyanJsonDelete(detached);
		TEST_ASSERT_EQUAL_INT_MESSAGE(size - 1, RyanJsonGetSize(arrInt), "分离 arrayInt 中间项后长度未减少");

		// 尾部
		size = RyanJsonGetSize(arrInt);
		detached = RyanJsonDetachByIndex(arrInt, size - 1);
		TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离 arrayInt 尾部项失败");
		RyanJsonDelete(detached);
		TEST_ASSERT_EQUAL_INT_MESSAGE(size - 1, RyanJsonGetSize(arrInt), "分离 arrayInt 尾部后长度未减少");
	}

	{
		RyanJson_t arrDouble = RyanJsonGetObjectByKey(json, "arrayDouble");
		TEST_ASSERT_NOT_NULL_MESSAGE(arrDouble, "获取 arrayDouble 失败");
		uint32_t size = RyanJsonGetSize(arrDouble);
		RyanJsonDelete(RyanJsonDetachByIndex(arrDouble, 0));
		TEST_ASSERT_EQUAL_INT_MESSAGE(size - 1, RyanJsonGetSize(arrDouble), "分离 arrayDouble 头部后长度未减少");
	}

	{
		RyanJson_t arrString = RyanJsonGetObjectByKey(json, "arrayString");
		TEST_ASSERT_NOT_NULL_MESSAGE(arrString, "获取 arrayString 失败");
		uint32_t size = RyanJsonGetSize(arrString);
		RyanJsonDelete(RyanJsonDetachByIndex(arrString, size - 1));
		TEST_ASSERT_EQUAL_INT_MESSAGE(size - 1, RyanJsonGetSize(arrString), "分离 arrayString 尾部后长度未减少");
	}

	/**
	 * @brief 嵌套对象分离测试 (item)
	 */
	{
		RyanJson_t detached = RyanJsonDetachByKey(json, "item");
		TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离嵌套对象 item 失败");
		RyanJsonDelete(detached);
		TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(json, "item"), "分离后 item 依然存在");
	}

	/**
	 * @brief 数组对象元素分离测试 (arrayItem 头、中、尾)
	 */
	{
		RyanJson_t arr = RyanJsonGetObjectByKey(json, "arrayItem");
		TEST_ASSERT_NOT_NULL_MESSAGE(arr, "获取 arrayItem 失败");

		uint32_t size = RyanJsonGetSize(arr);
		// 头部
		RyanJson_t detached = RyanJsonDetachByIndex(arr, 0);
		TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离对象数组头部项失败");
		RyanJsonDelete(detached);
		TEST_ASSERT_EQUAL_INT_MESSAGE(size - 1, RyanJsonGetSize(arr), "分离对象数组头部后长度未减少");

		// 中间
		size = RyanJsonGetSize(arr);
		detached = RyanJsonDetachByIndex(arr, 1);
		TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离对象数组中间项失败");
		RyanJsonDelete(detached);
		TEST_ASSERT_EQUAL_INT_MESSAGE(size - 1, RyanJsonGetSize(arr), "分离对象数组中间后长度未减少");

		// 尾部
		size = RyanJsonGetSize(arr);
		detached = RyanJsonDetachByIndex(arr, RyanJsonGetSize(arr) - 1);
		TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离对象数组尾部项失败");
		RyanJsonDelete(detached);
		TEST_ASSERT_EQUAL_INT_MESSAGE(size - 1, RyanJsonGetSize(arr), "分离对象数组尾部后长度未减少");
	}

	/**
	 * @brief 特殊类型分离测试（null / bool）
	 */
	{
		RyanJson_t detached = RyanJsonDetachByKey(json, "null");
		TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离 null 失败");
		RyanJsonDelete(detached);
		TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(json, "null"), "分离后 null 依然存在");

		detached = RyanJsonDetachByKey(json, "boolTrue");
		TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离 boolTrue 失败");
		RyanJsonDelete(detached);
		TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(json, "boolTrue"), "分离后 boolTrue 依然存在");
	}

	RyanJsonDelete(json);
}

static void testDetachMassiveItemsStress(void)
{
	// 循环创建并分离节点，验证内存稳定
	int32_t count = 100;
	RyanJson_t arr = RyanJsonCreateArray();
	for (int32_t i = 0; i < count; i++)
	{
		RyanJsonAddIntToArray(arr, i);
	}

	// 这里采用“每次分离 index=0”的策略，避免索引迁移带来的复杂性
	for (int32_t i = 0; i < count; i++)
	{
		RyanJson_t item = RyanJsonDetachByIndex(arr, 0);
		TEST_ASSERT_NOT_NULL(item);
#if true == RyanJsonDefaultAddAtHead
		TEST_ASSERT_EQUAL_INT(count - 1 - i, RyanJsonGetIntValue(item));
#else
		TEST_ASSERT_EQUAL_INT(i, RyanJsonGetIntValue(item));
#endif
		RyanJsonDelete(item);
	}
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetArraySize(arr));
	RyanJsonDelete(arr);

	// 分离刚添加的对象字段（字段类型为 String）
	RyanJson_t obj = RyanJsonCreateObject();
	// 使用标准 AddString 宏：会创建带 key 的 String 节点并插入
	RyanJsonAddStringToObject(obj, "sub", "v");

	RyanJson_t detached = RyanJsonDetachByKey(obj, "sub");
	TEST_ASSERT_NOT_NULL(detached);
	TEST_ASSERT_EQUAL_STRING("v", RyanJsonGetStringValue(detached));

	// 再次分离应返回 NULL
	TEST_ASSERT_NULL(RyanJsonDetachByKey(obj, "sub"));

	RyanJsonDelete(detached);
	RyanJsonDelete(obj);
}

void testDetachRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testDetachEdgeCases);
	RUN_TEST(testDetachDuplicateKey);
	RUN_TEST(testDetachCrossObject);
	RUN_TEST(testDetachReInsert);
	RUN_TEST(testDetachSingleNodeAndReuse);
	RUN_TEST(testDetachTailAndMiddleThenAppend);
	RUN_TEST(testDetachStandardOperations);
	RUN_TEST(testDetachMassiveItemsStress);
}
