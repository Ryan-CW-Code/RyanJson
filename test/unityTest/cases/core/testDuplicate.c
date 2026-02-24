#include "testBase.h"

static void testDuplicateEdgeCases(void)
{
	// 复制 NULL
	TEST_ASSERT_NULL_MESSAGE(RyanJsonDuplicate(NULL), "Duplicate(NULL) 应返回 NULL");

	// 深拷贝验证
	// 创建一个嵌套对象: root -> child -> val
	RyanJson_t root = RyanJsonCreateObject();
	RyanJson_t child = RyanJsonCreateObject();
	RyanJsonAddIntToObject(child, "val", 100);
	RyanJsonAddItemToObject(root, "child", child);

	// 复制整个树
	RyanJson_t rootCopy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(rootCopy);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, rootCopy), "复制后内容应一致");

	// 修改副本的深层值
	RyanJson_t childCopy = RyanJsonGetObjectToKey(rootCopy, "child");
	RyanJsonChangeIntValue(RyanJsonGetObjectToKey(childCopy, "val"), 200);

	// 验证：原件应保持 100，副本为 200
	TEST_ASSERT_EQUAL_INT_MESSAGE(100, RyanJsonGetIntValue(RyanJsonGetObjectToKey(root, "child", "val")), "修改副本不应影响原件");
	TEST_ASSERT_EQUAL_INT_MESSAGE(200, RyanJsonGetIntValue(RyanJsonGetObjectToKey(rootCopy, "child", "val")), "副本修改失效");

	RyanJsonDelete(root);
	RyanJsonDelete(rootCopy);
}

static void testDuplicateEmptyAndSpecial(void)
{
	// 复制空对象和空数组
	RyanJson_t emptyObj = RyanJsonCreateObject();
	RyanJson_t dupEmptyObj = RyanJsonDuplicate(emptyObj);
	TEST_ASSERT_NOT_NULL(dupEmptyObj);
	TEST_ASSERT_TRUE(RyanJsonIsObject(dupEmptyObj));
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetSize(dupEmptyObj));
	RyanJsonDelete(emptyObj);
	RyanJsonDelete(dupEmptyObj);

	RyanJson_t emptyArr = RyanJsonCreateArray();
	RyanJson_t dupEmptyArr = RyanJsonDuplicate(emptyArr);
	TEST_ASSERT_NOT_NULL(dupEmptyArr);
	TEST_ASSERT_TRUE(RyanJsonIsArray(dupEmptyArr));
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetSize(dupEmptyArr));
	RyanJsonDelete(emptyArr);
	RyanJsonDelete(dupEmptyArr);

	// 复制包含特殊值的对象
	RyanJson_t specialObj = RyanJsonCreateObject();
	RyanJsonAddNullToObject(specialObj, "null");
	RyanJsonAddBoolToObject(specialObj, "true", RyanJsonTrue);
	RyanJsonAddBoolToObject(specialObj, "false", RyanJsonFalse);
	RyanJsonAddStringToObject(specialObj, "emptyStr", "");

	RyanJson_t dupSpecial = RyanJsonDuplicate(specialObj);
	TEST_ASSERT_TRUE(RyanJsonCompare(specialObj, dupSpecial));
	RyanJsonDelete(specialObj);
	RyanJsonDelete(dupSpecial);
}

static void testDuplicateFullScenarios(void)
{
	RyanJson_t json, dupItem, jsonRoot = NULL;
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

	/**
	 * @brief 普通类型复制测试
	 */
	json = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "解析 Json 失败");

	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "inter"));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(dupItem, RyanJsonGetObjectToKey(json, "inter")), "普通类型复制后比较失败");
	RyanJsonDelete(dupItem);

	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "inter"));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonAddItemToObject(json, "test", dupItem), "AddItemToObject 不应接受标量 item");

	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "inter"));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonAddItemToObject(json, "test", dupItem), "AddItemToObject 不应接受标量 item");
	RyanJsonDelete(json);

	/**
	 * @brief 对象类型复制测试
	 */
	json = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "重新解析 Json 失败");

	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(dupItem, RyanJsonGetObjectToKey(json, "item")), "对象类型复制后比较失败");
	RyanJsonDelete(dupItem);

	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));
	RyanJsonAddItemToObject(json, "test", dupItem);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "item")),
				 "对象类型复制并添加后比较失败");
	RyanJsonDelete(RyanJsonDetachByKey(json, "test"));

	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));
	RyanJsonAddItemToObject(json, "test", dupItem);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "item")),
				 "对象类型复制并添加后删除再添加比较失败");
	RyanJsonDelete(json);

	/**
	 * @brief 数组类型复制测试
	 */
	json = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "重新解析 Json 失败 (数组测试)");

	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "arrayItem"));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(dupItem, RyanJsonGetObjectToKey(json, "arrayItem")), "数组类型复制后比较失败");
	RyanJsonDelete(dupItem);

	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "arrayItem"));
	RyanJsonAddItemToObject(json, "test", dupItem);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "arrayItem")),
				 "数组类型复制并添加后比较失败");
	RyanJsonDelete(RyanJsonDetachByKey(json, "test"));

	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "arrayItem"));
	RyanJsonAddItemToObject(json, "test", dupItem);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "arrayItem")),
				 "数组类型复制并添加后删除再添加比较失败");
	RyanJsonDelete(json);

	/**
	 * @brief 循环压力与内存泄漏测试
	 */
	json = RyanJsonParse(jsonstr);
	jsonRoot = RyanJsonCreateObject();
	RyanJsonAddBoolToObject(jsonRoot, "arrayItem", RyanJsonTrue);

	int32_t initialUse = vallocGetUse();
	for (uint8_t i = 0; i < 10; i++)
	{
		dupItem = RyanJsonParse(jsonstr);
		TEST_ASSERT_NOT_NULL_MESSAGE(dupItem, "循环中解析失败");

		RyanJsonReplaceByKey(jsonRoot, "arrayItem", RyanJsonDuplicate(dupItem));
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(RyanJsonGetObjectToKey(jsonRoot, "arrayItem"), dupItem), "循环中替换并比较失败");

		RyanJsonReplaceByKey(json, "arrayItem", RyanJsonDuplicate(RyanJsonGetObjectByKey(dupItem, "item")));
		TEST_ASSERT_TRUE_MESSAGE(
			RyanJsonCompare(RyanJsonGetObjectToKey(json, "arrayItem"), RyanJsonGetObjectByKey(dupItem, "item")),
			"循环中嵌套替换并比较失败");

		RyanJsonDelete(dupItem);

		if (i > 0) { TEST_ASSERT_EQUAL_INT_MESSAGE(initialUse, vallocGetUse(), "内存泄漏检测失败"); }
		initialUse = vallocGetUse();
	}

	RyanJsonDelete(json);
	RyanJsonDelete(jsonRoot);
}

static void testDuplicateMassiveStress(void)
{
	// 压力测试：大数组复制
	int32_t bigSize = 2000;
	RyanJson_t bigArr = RyanJsonCreateArray();
	for (int32_t i = 0; i < bigSize; i++)
	{
		RyanJsonAddIntToArray(bigArr, i);
	}
	RyanJson_t dupBigArr = RyanJsonDuplicate(bigArr);
	TEST_ASSERT_EQUAL_INT(bigSize, RyanJsonGetArraySize(dupBigArr));
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(dupBigArr, bigSize - 1)));
#else
	TEST_ASSERT_EQUAL_INT(bigSize - 1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(dupBigArr, bigSize - 1)));
#endif
	RyanJsonDelete(bigArr);
	RyanJsonDelete(dupBigArr);
}

void testDuplicateRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testDuplicateEdgeCases);
	RUN_TEST(testDuplicateEmptyAndSpecial);
	RUN_TEST(testDuplicateFullScenarios);
	RUN_TEST(testDuplicateMassiveStress);
}
