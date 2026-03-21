#include "testBase.h"

static void testReplaceEdgeCases(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	RyanJsonAddIntToObject(obj, "a", 1);

	// Replace NULL 参数
	RyanJson_t newItem = RyanJsonCreateInt("b", 2);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByKey(NULL, "key", newItem), "ReplaceByKey(NULL, ...) 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByKey(obj, NULL, newItem), "ReplaceByKey(..., key=NULL, ...) 应返回 False");

	// 注意：如果 Replace 失败，newItem 及其内存由谁负责？
	// 如果 Replace 函数返回 False 且没有接管 item，调用者需要释放 item。
	// RyanJsonReplaceByKey 检查参数失败时直接返回，不触碰 item。
	// 所以这里我们需要手动释放 newItem 以避免内存泄漏。
	RyanJsonDelete(newItem);

	newItem = RyanJsonCreateInt("b", 2);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByKey(obj, "a", NULL), "ReplaceByKey(..., item=NULL) 应返回 False");
	// newItem 在这里没被传入成功，所以需要手动释放
	RyanJsonDelete(newItem);

	// Replace 不存在的 Key (应失败，而不是添加)
	newItem = RyanJsonCreateInt("b", 2);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByKey(obj, "non_existent", newItem), "ReplaceByKey(不存在的Key) 应返回 False");
	// 同样，替换失败，需释放 newItem
	RyanJsonDelete(newItem);

	// Replace Index 越界
	newItem = RyanJsonCreateInt("b", 2);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByIndex(obj, 99, newItem), "ReplaceByIndex(越界) 应返回 False");
	RyanJsonDelete(newItem);

	// ReplaceByIndex 在 Object 上的重复 key 行为由严格模式控制
	RyanJsonAddIntToObject(obj, "b", 2);
	newItem = RyanJsonCreateInt("a", 9);
#if true == RyanJsonDefaultAddAtHead
	uint32_t replaceIndex = 0; // 头插模式下，后加的 "b" 位于索引 0
#else
	uint32_t replaceIndex = 1; // 尾插模式下，后加的 "b" 位于索引 1
#endif

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByIndex(obj, replaceIndex, newItem),
				  "严格模式下 ReplaceByIndex(Object) 重复 key 应返回 False");
	RyanJsonDelete(newItem);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "a")));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "b")));
#else
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByIndex(obj, replaceIndex, newItem),
				 "非严格模式下 ReplaceByIndex(Object) 重复 key 应返回 True");
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_EQUAL_INT(9, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "a")));
#else
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "a")));
#endif
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKey(obj, "b"), "非严格模式替换后 b 应被替换掉");
#endif

	// ReplaceByIndex 在 Object 上使用相同 key 替换应成功
	// 该用例覆盖冲突检查中的 item == skipItem 分支（应跳过被替换节点本身）
	newItem = RyanJsonCreateInt("b", 99);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByIndex(obj, replaceIndex, newItem), "ReplaceByIndex(Object) 同 key 替换应成功");
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "a")));
	TEST_ASSERT_EQUAL_INT(99, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "b")));

	// 对非容器操作
	RyanJson_t num = RyanJsonCreateInt("num", 1);
	newItem = RyanJsonCreateInt("val", 2);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByKey(num, "any", newItem), "对非容器 ReplaceByKey 应返回 False");
	RyanJsonDelete(newItem);

	RyanJsonDelete(obj);
	RyanJsonDelete(num);
}

static void testReplaceSelfCheck(void)
{
	// 测试：尝试将节点替换为其自身（RyanJsonReplaceByKey/Index 至少应保持稳定，不发生崩溃）。
	// 但 API 语义通常要求 newItem 是新建节点，且不属于任何树（或来自其他位置的 Detach）。
	// 因此这里不直接传入“树内同一节点”，而是使用语义等价的替换场景做覆盖。

	// 测试：替换为“较重”的新节点（大 Array）
	RyanJson_t obj = RyanJsonCreateObject();
	RyanJsonAddIntToObject(obj, "k", 1);

	RyanJson_t bigArr = RyanJsonCreateArray();
	for (int32_t i = 0; i < 100; i++)
	{
		RyanJsonAddIntToArray(bigArr, i);
	}

	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(obj, "k", bigArr));
	TEST_ASSERT_EQUAL_INT(100, RyanJsonGetArraySize(RyanJsonGetObjectByKey(obj, "k")));

	RyanJsonDelete(obj);
}

static void testReplaceRejectAttachedItem(void)
{
	RyanJson_t obj1 = RyanJsonCreateObject();
	RyanJson_t obj2 = RyanJsonCreateObject();
	RyanJsonAddIntToObject(obj1, "a", 1);
	RyanJsonAddIntToObject(obj2, "b", 2);

	RyanJson_t attachedObjItem = RyanJsonGetObjectByKey(obj1, "a");
	TEST_ASSERT_NOT_NULL(attachedObjItem);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByKey(obj2, "b", attachedObjItem), "已挂树的 item 不应作为 ReplaceByKey 参数");
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj2, "b")));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj1, "a")));

	RyanJson_t arr1 = RyanJsonCreateArray();
	RyanJson_t arr2 = RyanJsonCreateArray();
	RyanJsonAddIntToArray(arr1, 10);
	RyanJsonAddIntToArray(arr2, 20);

	RyanJson_t attachedArrItem = RyanJsonGetObjectByIndex(arr1, 0);
	TEST_ASSERT_NOT_NULL(attachedArrItem);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByIndex(arr2, 0, attachedArrItem), "已挂树的 item 不应作为 ReplaceByIndex 参数");
	TEST_ASSERT_EQUAL_INT(20, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr2, 0)));
	TEST_ASSERT_EQUAL_INT(10, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr1, 0)));

	RyanJsonDelete(obj1);
	RyanJsonDelete(obj2);
	RyanJsonDelete(arr1);
	RyanJsonDelete(arr2);
}

static void testReplaceFailureKeepsItemOwnership(void)
{
	// ReplaceByIndex 失败后，item 应仍由调用方持有
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "a", 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "b", 2));

	RyanJson_t conflictItem = RyanJsonCreateInt("a", 9);
	TEST_ASSERT_NOT_NULL(conflictItem);
#if true == RyanJsonStrictObjectKeyCheck
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_FALSE(RyanJsonReplaceByIndex(obj, 0, conflictItem));
#else
	TEST_ASSERT_FALSE(RyanJsonReplaceByIndex(obj, 1, conflictItem));
#endif
#else
	TEST_ASSERT_FALSE(RyanJsonReplaceByIndex(obj, 99, conflictItem));
#endif
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDetachedItem(conflictItem), "ReplaceByIndex 失败后 item 不应被消费");

	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, conflictItem));
	TEST_ASSERT_EQUAL_INT(9, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));

	// ReplaceByKey 失败（目标 key 不存在）后，item 也应可复用
	RyanJson_t notFoundItem = RyanJsonCreateString("tmp", "v");
	TEST_ASSERT_NOT_NULL(notFoundItem);
	TEST_ASSERT_FALSE(RyanJsonReplaceByKey(obj, "not_exist", notFoundItem));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDetachedItem(notFoundItem), "ReplaceByKey 失败后 item 不应被消费");

	TEST_ASSERT_TRUE(RyanJsonInsert(arr, UINT32_MAX, notFoundItem));
	TEST_ASSERT_EQUAL_STRING("v", RyanJsonGetStringValue(RyanJsonGetObjectByIndex(arr, 1)));

	RyanJsonDelete(arr);
	RyanJsonDelete(obj);
}

static void testReplaceFailureCallerMustDeleteItem(void)
{
	/*
	 * 约定验证：
	 * Replace 失败时库不会释放 item，调用方必须手动释放。
	 */
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "a", 1));

	// ReplaceByKey 失败：目标 key 不存在
	RyanJson_t replaceByKeyItem = RyanJsonCreateString("tmp", "v");
	TEST_ASSERT_NOT_NULL(replaceByKeyItem);
	TEST_ASSERT_FALSE(RyanJsonReplaceByKey(obj, "not_exist", replaceByKeyItem));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDetachedItem(replaceByKeyItem), "ReplaceByKey 失败后 item 应保持游离");
	RyanJsonDelete(replaceByKeyItem); // 调用方主动释放

	// ReplaceByIndex 失败：索引越界
	RyanJson_t replaceByIndexItem = RyanJsonCreateInt("a", 9);
	TEST_ASSERT_NOT_NULL(replaceByIndexItem);
	TEST_ASSERT_FALSE(RyanJsonReplaceByIndex(obj, 99, replaceByIndexItem));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDetachedItem(replaceByIndexItem), "ReplaceByIndex 失败后 item 应保持游离");
	RyanJsonDelete(replaceByIndexItem); // 调用方主动释放

	// 原树应保持不变
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "a")));

	RyanJsonDelete(obj);
}

static void testReplaceKeyRewriteAndWrapPaths(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "a", 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "b", 2));

	// 无 key 的容器替换 Object 字段：应自动包装成 key="a"
	RyanJson_t noKeyContainer = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(noKeyContainer);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(noKeyContainer, "x", 7));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(obj, "a", noKeyContainer), "ReplaceByKey(无 key 容器) 应成功");

	RyanJson_t aNode = RyanJsonGetObjectByKey(obj, "a");
	TEST_ASSERT_NOT_NULL(aNode);
	TEST_ASSERT_TRUE(RyanJsonIsObject(aNode));
	TEST_ASSERT_EQUAL_INT(7, RyanJsonGetIntValue(RyanJsonGetObjectByKey(aNode, "x")));

	// 无 key 的标量替换 Object 字段：当前实现会包装为 key="a" 的 Object，并把标量作为唯一子节点
	RyanJson_t noKeyScalar = RyanJsonCreateInt(NULL, 123);
	TEST_ASSERT_NOT_NULL(noKeyScalar);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(obj, "a", noKeyScalar), "ReplaceByKey(无 key 标量) 应成功");
	aNode = RyanJsonGetObjectByKey(obj, "a");
	TEST_ASSERT_NOT_NULL(aNode);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsObject(aNode), "无 key 标量替换后应包装为 Object 节点");
	TEST_ASSERT_EQUAL_UINT32(1, RyanJsonGetSize(aNode));
	TEST_ASSERT_EQUAL_INT(123, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(aNode, 0)));

	// 不同 key 的节点替换：应重命名为目标 key="b"
	RyanJson_t diffKeyItem = RyanJsonCreateInt("temp", 88);
	TEST_ASSERT_NOT_NULL(diffKeyItem);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(obj, "b", diffKeyItem), "ReplaceByKey(不同 key) 应成功");
	TEST_ASSERT_EQUAL_INT(88, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "b")));
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKey(obj, "temp"), "ReplaceByKey 后不应残留旧 key");

	RyanJsonDelete(obj);
}

static void testReplaceStandardOperations(void)
{

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
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "解析基础 Json 失败");

	// Array 替换测试：arrayInt 头部
	RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 0, RyanJsonCreateString(NULL, "arrayIntHead"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 0);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 arrayInt[0] 不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("arrayIntHead", RyanJsonGetStringValue(v), "替换后的 arrayInt[0] 值错误");
	}

	// Array 替换测试：arrayInt 尾部
	{
		RyanJson_t arr = RyanJsonGetObjectToKey(json, "arrayInt");
		uint32_t last = RyanJsonGetSize(arr) - 1;
		RyanJsonReplaceByIndex(arr, last, RyanJsonCreateString(NULL, "arrayIntTail"));
		RyanJson_t v = RyanJsonGetObjectToIndex(arr, last);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 arrayInt 尾部不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("arrayIntTail", RyanJsonGetStringValue(v), "替换后的 arrayInt 尾部值错误");
	}

	// Array 中 Object 替换测试：arrayItem[0]
	RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 0, RyanJsonCreateString(NULL, "arrayItem0"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 0);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 arrayItem[0] 不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("arrayItem0", RyanJsonGetStringValue(v), "替换后的 arrayItem[0] 值错误");
	}

	// Object 字段替换：inter -> 999
	RyanJsonReplaceByKey(json, "inter", RyanJsonCreateInt("inter", 999));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "inter");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(v), "替换后的 inter 不是 Int");
		TEST_ASSERT_EQUAL_INT_MESSAGE(999, RyanJsonGetIntValue(v), "替换后的 inter 值错误");
	}

	// Object 字段替换：Double -> 123.45
	RyanJsonReplaceByKey(json, "double", RyanJsonCreateDouble("double", 123.45));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "double");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDouble(v), "替换后的 double 不是 Double");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareDouble(123.45, RyanJsonGetDoubleValue(v)), "替换后的 double 值错误");
	}

	// Object 字段替换：String -> "newString"
	RyanJsonReplaceByKey(json, "string", RyanJsonCreateString("string", "newString"));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "string");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 string 不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("newString", RyanJsonGetStringValue(v), "替换后的 string 值错误");
	}

	// Object 字段替换：boolFalse -> true
	RyanJsonReplaceByKey(json, "boolFalse", RyanJsonCreateBool("boolFalse", RyanJsonTrue));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "boolFalse");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(v), "替换后的 boolFalse 不是 Bool 值");
		TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonTrue, RyanJsonGetBoolValue(v), "替换后的 boolFalse 值错误");
	}

	// Array 替换：arrayString 中间元素 -> "headString"
	RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayString"), 1, RyanJsonCreateString(NULL, "headString"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayString"), 1);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 arrayString[1] 不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("headString", RyanJsonGetStringValue(v), "替换后的 arrayString[1] 值错误");
	}
	// Array 项替换测试：arrayString 尾部
	{
		RyanJson_t arr = RyanJsonGetObjectToKey(json, "arrayString");
		uint32_t last = RyanJsonGetSize(arr) - 1;
		RyanJsonReplaceByIndex(arr, last, RyanJsonCreateString(NULL, "tailString"));
		RyanJson_t v = RyanJsonGetObjectToIndex(arr, last);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 arrayString 尾部不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("tailString", RyanJsonGetStringValue(v), "替换后的 arrayString 尾部值错误");
	}

	// Array 中 Object 替换：arrayItem 尾部 -> "arrayItemTail"
	{
		RyanJson_t arr = RyanJsonGetObjectToKey(json, "arrayItem");
		uint32_t last = RyanJsonGetSize(arr) - 1;
		RyanJsonReplaceByIndex(arr, last, RyanJsonCreateString(NULL, "arrayItemTail"));
		RyanJson_t v = RyanJsonGetObjectToIndex(arr, last);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 arrayItem 尾部不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("arrayItemTail", RyanJsonGetStringValue(v), "替换后的 arrayItem 尾部值错误");
	}

	// 嵌套 Object 替换：item.inter -> 111
	RyanJsonReplaceByKey(RyanJsonGetObjectToKey(json, "item"), "inter", RyanJsonCreateInt("inter", 111));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(RyanJsonGetObjectToKey(json, "item"), "inter");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(v), "替换后的 item.inter 不是 Int");
		TEST_ASSERT_EQUAL_INT_MESSAGE(111, RyanJsonGetIntValue(v), "替换后的 item.inter 值错误");
	}

	// 嵌套 Object 替换：item.string -> "nestedReplace"
	RyanJsonReplaceByKey(RyanJsonGetObjectToKey(json, "item"), "string", RyanJsonCreateString("string", "nestedReplace"));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(RyanJsonGetObjectToKey(json, "item"), "string");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 item.string 不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("nestedReplace", RyanJsonGetStringValue(v), "替换后的 item.string 值错误");
	}

	// 混合 Array 替换测试
	RyanJson_t mixArr = RyanJsonGetObjectToKey(json, "array");

	// Int -> "intReplaced"
	RyanJsonReplaceByIndex(mixArr, 0, RyanJsonCreateString(NULL, "intReplaced"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(mixArr, 0);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 array[0] 不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("intReplaced", RyanJsonGetStringValue(v), "替换后的 array[0] 值错误");
	}

	// Double -> "doubleReplaced"
	RyanJsonReplaceByIndex(mixArr, 1, RyanJsonCreateString(NULL, "doubleReplaced"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(mixArr, 1);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 array[1] 不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("doubleReplaced", RyanJsonGetStringValue(v), "替换后的 array[1] 值错误");
	}
	// String -> "stringReplaced"
	RyanJsonReplaceByIndex(mixArr, 2, RyanJsonCreateString(NULL, "stringReplaced"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(mixArr, 2);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 array[2] 不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("stringReplaced", RyanJsonGetStringValue(v), "替换后的 array[2] 值错误");
	}
	// Bool -> "boolReplaced"
	RyanJsonReplaceByIndex(mixArr, 3, RyanJsonCreateString(NULL, "boolReplaced"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(mixArr, 3);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 array[3] 不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("boolReplaced", RyanJsonGetStringValue(v), "替换后的 array[3] 值错误");
	}

	// Null -> "nullReplaced"
	RyanJsonReplaceByIndex(mixArr, 5, RyanJsonCreateString(NULL, "nullReplaced"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(mixArr, 5);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 array[5] 不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("nullReplaced", RyanJsonGetStringValue(v), "替换后的 array[5] 值错误");
	}

	// 替换整个 Array 项：arrayString -> "arrayStringRenamed"
	RyanJsonReplaceByKey(json, "arrayString", RyanJsonCreateString("arrayString", "arrayStringRenamed"));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "arrayString");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 arrayString 不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("arrayStringRenamed", RyanJsonGetStringValue(v), "替换后的 arrayString 值错误");
	}

	// 修改 Array 节点为 Object 节点：arrayDouble -> duplicate(item)
	RyanJson_t duplicateJson = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));
	RyanJsonReplaceByKey(json, "arrayDouble", duplicateJson);
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "arrayDouble");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsObject(v), "替换后的 arrayDouble 不是 Object 类型");
	}

	// 替换 String 字段：string2222 -> "world"
	RyanJsonReplaceByKey(json, "string2222", RyanJsonCreateString("string2222", "world"));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "string2222");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 string2222 不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("world", RyanJsonGetStringValue(v), "替换后的 string2222 值错误");
	}

	// 替换 boolValue：boolTrue -> false
	RyanJsonReplaceByKey(json, "boolTrue", RyanJsonCreateBool("boolTrue", RyanJsonFalse));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "boolTrue");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(v), "替换后的 boolTrue 不是 Bool 值");
		TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonFalse, RyanJsonGetBoolValue(v), "替换后的 boolTrue 值错误");
	}

	// 替换 Null 为 String
	RyanJsonReplaceByKey(json, "null", RyanJsonCreateString("null", "notNull"));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "null");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(v), "替换后的 null 不是 String");
		TEST_ASSERT_EQUAL_STRING_MESSAGE("notNull", RyanJsonGetStringValue(v), "替换后的 null 值错误");
	}

	char *str = RyanJsonPrint(json, 1024, RyanJsonTrue, NULL);
	RyanJsonFree(str);
	RyanJsonDelete(json);
}

static void testReplaceTypeSwitchingStress(void)
{
	RyanJson_t root = RyanJsonCreateObject();
	RyanJsonAddIntToObject(root, "k", 1);

	// 疯狂类型切换
	// 高频类型切换
	// Int -> String
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "k", RyanJsonCreateString("k", "s")));
	TEST_ASSERT_TRUE(RyanJsonIsString(RyanJsonGetObjectByKey(root, "k")));

	// String -> Array
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "k", RyanJsonCreateArray()));
	TEST_ASSERT_TRUE(RyanJsonIsArray(RyanJsonGetObjectByKey(root, "k")));

	// Array 节点替换为 Object 节点
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "k", RyanJsonCreateObject()));
	TEST_ASSERT_TRUE(RyanJsonIsObject(RyanJsonGetObjectByKey(root, "k")));

	// Object 节点替换为 Bool 节点
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "k", RyanJsonCreateBool("k", RyanJsonTrue)));
	TEST_ASSERT_TRUE(RyanJsonIsBool(RyanJsonGetObjectByKey(root, "k")));

	// Bool -> Null
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "k", RyanJsonCreateNull("k")));
	TEST_ASSERT_TRUE(RyanJsonIsNull(RyanJsonGetObjectByKey(root, "k")));

	// 自身替换（模拟）：实际替换为副本节点
	RyanJson_t nullNode = RyanJsonGetObjectByKey(root, "k");
	RyanJson_t dupNull = RyanJsonDuplicate(nullNode);
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "k", dupNull));
	TEST_ASSERT_TRUE(RyanJsonIsNull(RyanJsonGetObjectByKey(root, "k")));

	RyanJsonDelete(root);
}

static void testReplaceFailureRecoveryChainWithExpectedDocument(void)
{
	// 复杂链路：
	// Parse -> ReplaceByKey(失败) -> 复用失败 item 成功 ReplaceByKey
	// -> ReplaceByIndex(成功/失败交错) -> ReplaceByKey(嵌套 Object) -> Compare(期望文档)。
	// 目标：
	// - 验证 Replace 失败不会错误消费 item；
	// - 验证失败后复用同一 item 进行后续替换可稳定成功；
	// - 验证 Object 替换与 Array 替换交错后语义仍可收敛到期望结果。
	const char *source = "{\"meta\":{\"v\":1},\"arr\":[{\"id\":\"a\"},{\"id\":\"b\"}],\"flag\":true}";
	const char *expectText = "{\"meta\":{\"v\":2},\"arr\":[{\"id\":\"a\"},{\"id\":\"bb\",\"done\":true}],\"flag\":\"x\"}";

	RyanJson_t root = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "Replace 链路样本解析失败");
	RyanJson_t meta = RyanJsonGetObjectToKey(root, "meta");
	RyanJson_t arr = RyanJsonGetObjectToKey(root, "arr");
	TEST_ASSERT_NOT_NULL(meta);
	TEST_ASSERT_NOT_NULL(arr);

	// 失败路径：不存在 key 的替换失败，item 应保持 detached。
	RyanJson_t failThenReuse = RyanJsonCreateString("temp", "x");
	TEST_ASSERT_NOT_NULL(failThenReuse);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByKey(root, "missing", failThenReuse), "ReplaceByKey(不存在 key) 应失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDetachedItem(failThenReuse), "失败后 failThenReuse 应保持 detached");

	// 成功路径：复用同一 item 替换 flag（会自动改写 key）。
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(root, "flag", failThenReuse), "复用失败 item 替换 flag 应成功");
	TEST_ASSERT_EQUAL_STRING("x", RyanJsonGetStringValue(RyanJsonGetObjectToKey(root, "flag")));

	// Array 替换成功路径。
	RyanJson_t arrReplace = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(arrReplace);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(arrReplace, "id", "bb"));
	TEST_ASSERT_TRUE(RyanJsonAddBoolToObject(arrReplace, "done", RyanJsonTrue));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByIndex(arr, 1, arrReplace), "ReplaceByIndex(arr,1) 应成功");

	// Array 越界失败路径：失败后 item 仍应由调用方释放。
	RyanJson_t outRange = RyanJsonCreateInt(NULL, 7);
	TEST_ASSERT_NOT_NULL(outRange);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByIndex(arr, 99, outRange), "ReplaceByIndex(arr,99) 应失败");
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(outRange));
	RyanJsonDelete(outRange);

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(meta, "v", RyanJsonCreateInt("v", 2)), "ReplaceByKey(meta.v) 应成功");

	RyanJson_t expect = RyanJsonParse(expectText);
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, expect), "Replace 链路最终结果与期望文档不一致");

	char *printed = RyanJsonPrint(root, 192, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

static void testReplaceByIndexArrayWithDetachedItemFromOtherArray(void)
{
	// Parse(arr1/arr2) -> DetachByIndex(arr2) -> ReplaceByIndex(arr1) -> Compare。
	// 目标：验证 ReplaceByIndex(Array) 可接收来自另一 Array 的游离节点。
	RyanJson_t arr1 = RyanJsonParse("[1,2,3]");
	RyanJson_t arr2 = RyanJsonParse("[4,5,6]");
	TEST_ASSERT_NOT_NULL(arr1);
	TEST_ASSERT_NOT_NULL(arr2);

	RyanJson_t moved = RyanJsonDetachByIndex(arr2, 1);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByIndex(arr1, 0, moved), "跨 Array ReplaceByIndex 应成功");

	RyanJson_t expect1 = RyanJsonParse("[5,2,3]");
	RyanJson_t expect2 = RyanJsonParse("[4,6]");
	TEST_ASSERT_NOT_NULL(expect1);
	TEST_ASSERT_NOT_NULL(expect2);
	TEST_ASSERT_TRUE(RyanJsonCompare(arr1, expect1));
	TEST_ASSERT_TRUE(RyanJsonCompare(arr2, expect2));

	RyanJsonDelete(expect2);
	RyanJsonDelete(expect1);
	RyanJsonDelete(arr2);
	RyanJsonDelete(arr1);
}

static void testReplaceByIndexObjectWithDetachedRenamedItem(void)
{
	// Parse(Object) -> DetachByKey -> ChangeKey -> ReplaceByIndex(Object)。
	// 目标：验证按索引替换 Object 字段时会保留游离节点的新 key 与索引位置。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2,\"c\":3}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t moved = RyanJsonDetachByKey(obj, "b");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(moved, "d"));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByIndex(obj, 0, moved), "ReplaceByIndex(Object) 应成功");

	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetSize(obj));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(obj, "a"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(obj, "b"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "c"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "d"));
	TEST_ASSERT_EQUAL_STRING("d", RyanJsonGetKey(RyanJsonGetObjectByIndex(obj, 0)));
	TEST_ASSERT_EQUAL_STRING("c", RyanJsonGetKey(RyanJsonGetObjectByIndex(obj, 1)));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "d")));

	RyanJsonDelete(obj);
}

static void testReplaceByKeyReuseDetachedSiblingAfterRename(void)
{
	// Parse(Object) -> DetachByKey -> ChangeKey -> ReplaceByKey(同名目标)。
	// 目标：验证来自同一 Object 的游离兄弟节点可直接复用为 ReplaceByKey 参数。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t moved = RyanJsonDetachByKey(obj, "b");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(moved, "a"));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(obj, "a", moved), "复用游离兄弟节点 ReplaceByKey 应成功");

	RyanJson_t expect = RyanJsonParse("{\"a\":2}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(obj);
}

static void testReplaceNestedObjectThenDetachChildToParent(void)
{
	// Parse -> ReplaceByKey(新 Object) -> DetachByKey(子节点) -> ChangeKey -> Insert(父 Object)。
	// 目标：验证 ReplaceByKey 引入的新 Object 仍可继续分离子节点并回插到父 Object。
	RyanJson_t root = RyanJsonParse("{\"a\":{\"x\":1},\"b\":2}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t newObj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(newObj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(newObj, "x", 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(newObj, "y", 2));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(root, "a", newObj), "ReplaceByKey(root.a) 应成功");

	RyanJson_t aNode = RyanJsonGetObjectByKey(root, "a");
	TEST_ASSERT_NOT_NULL(aNode);
	RyanJson_t detached = RyanJsonDetachByKey(aNode, "x");
	TEST_ASSERT_NOT_NULL(detached);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(detached, "x2"));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(root, UINT32_MAX, detached), "将分离子节点插回父 Object 应成功");

	RyanJson_t expect = RyanJsonParse("{\"a\":{\"y\":2},\"b\":2,\"x2\":1}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

void testReplaceRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testReplaceEdgeCases);
	RUN_TEST(testReplaceSelfCheck);
	RUN_TEST(testReplaceRejectAttachedItem);
	RUN_TEST(testReplaceFailureKeepsItemOwnership);
	RUN_TEST(testReplaceFailureCallerMustDeleteItem);
	RUN_TEST(testReplaceKeyRewriteAndWrapPaths);
	RUN_TEST(testReplaceByIndexArrayWithDetachedItemFromOtherArray);
	RUN_TEST(testReplaceByIndexObjectWithDetachedRenamedItem);
	RUN_TEST(testReplaceByKeyReuseDetachedSiblingAfterRename);
	RUN_TEST(testReplaceNestedObjectThenDetachChildToParent);
	RUN_TEST(testReplaceStandardOperations);
	RUN_TEST(testReplaceTypeSwitchingStress);
	RUN_TEST(testReplaceFailureRecoveryChainWithExpectedDocument);
}
