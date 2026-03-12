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

static void testDuplicateCrossContainerMoveIsolationChain(void)
{
	// 复杂链路：
	// Parse -> Duplicate(work) -> DetachByKey/DetachByIndex -> Change -> Insert/AddItemToObject
	// -> ReplaceByKey -> Compare(期望文档) -> 校验 original 隔离性。
	// 目标：
	// 1) 验证副本上的跨容器迁移不会污染原树；
	// 2) 验证“分离+改写+重挂载”后结构可稳定收敛到期望文档；
	// 3) 验证复杂链路后依旧可 Print/Parse 往返。
	const char *source =
		"{\"left\":{\"node\":{\"v\":1},\"keep\":2},\"right\":[{\"id\":\"a\"},{\"id\":\"b\"}],\"map\":{},\"meta\":{\"ver\":1}}";
	const char *expectText = "{\"left\":{\"keep\":\"k\",\"fromRight\":{\"id\":\"b\"}},\"right\":[{\"id\":\"a\"}],\"map\":{\"node2\":{"
				 "\"v\":9}},\"meta\":{\"ver\":2}}";

	RyanJson_t original = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL_MESSAGE(original, "Duplicate 迁移样本解析失败");
	RyanJson_t work = RyanJsonDuplicate(original);
	TEST_ASSERT_NOT_NULL_MESSAGE(work, "Duplicate 迁移样本拷贝失败");

	RyanJson_t left = RyanJsonGetObjectToKey(work, "left");
	RyanJson_t right = RyanJsonGetObjectToKey(work, "right");
	RyanJson_t map = RyanJsonGetObjectToKey(work, "map");
	RyanJson_t meta = RyanJsonGetObjectToKey(work, "meta");
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(right);
	TEST_ASSERT_NOT_NULL(map);
	TEST_ASSERT_NOT_NULL(meta);

	RyanJson_t movedNode = RyanJsonDetachByKey(left, "node");
	TEST_ASSERT_NOT_NULL_MESSAGE(movedNode, "分离 left.node 失败");
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(movedNode));
	TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(movedNode, "v"), 9));
	TEST_ASSERT_TRUE(RyanJsonChangeKey(movedNode, "node2"));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(map, 0, movedNode), "插入 map.node2 失败");

	RyanJson_t movedRight = RyanJsonDetachByIndex(right, 1);
	TEST_ASSERT_NOT_NULL_MESSAGE(movedRight, "分离 right[1] 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToObject(left, "fromRight", movedRight), "将分离数组对象挂载到 left.fromRight 失败");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(left, "keep", RyanJsonCreateString("keep", "k")), "替换 left.keep 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(meta, "ver", RyanJsonCreateInt("ver", 2)), "替换 meta.ver 失败");

	RyanJson_t expect = RyanJsonParse(expectText);
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(work, expect), "副本迁移链路结果与期望文档不一致");

	// 原树隔离性校验：不应受到 work 上的链路变更影响。
	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(original, "left", "node", "v"));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(original, "left", "node", "v")));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectToKey(original, "left", "keep")));
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(RyanJsonGetObjectToKey(original, "right")));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(original, "meta", "ver")));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(original, work), "修改后的副本不应与原树相等");

	char *printed = RyanJsonPrint(work, 192, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(work, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(expect);
	RyanJsonDelete(work);
	RyanJsonDelete(original);
}

static void testDuplicateDetachedNodeDualAttachWithoutAlias(void)
{
	// 复杂链路：
	// Parse -> DetachByKey -> Duplicate(detached) -> Change 两份副本不同值/不同 key
	// -> Insert(Object) 双挂载 -> Compare(期望文档)。
	// 目标：
	// 1) 验证 detached 节点可被 Duplicate；
	// 2) 验证 duplicate 后两份节点无别名，可独立修改；
	// 3) 验证双挂载后的结构和值符合预期。
	const char *source = "{\"obj\":{\"a\":{\"v\":1}},\"meta\":0}";
	const char *expectText = "{\"obj\":{\"a2\":{\"v\":2},\"a3\":{\"v\":3}},\"meta\":0}";

	RyanJson_t root = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "detached duplicate 样本解析失败");
	RyanJson_t obj = RyanJsonGetObjectToKey(root, "obj");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t detached = RyanJsonDetachByKey(obj, "a");
	TEST_ASSERT_NOT_NULL_MESSAGE(detached, "分离 obj.a 失败");
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(detached));

	RyanJson_t detachedCopy = RyanJsonDuplicate(detached);
	TEST_ASSERT_NOT_NULL_MESSAGE(detachedCopy, "Duplicate(detached) 失败");

	TEST_ASSERT_TRUE(RyanJsonChangeKey(detached, "a2"));
	TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(detached, "v"), 2));
	TEST_ASSERT_TRUE(RyanJsonChangeKey(detachedCopy, "a3"));
	TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(detachedCopy, "v"), 3));

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(obj, 0, detached), "插入 obj.a2 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(obj, 1, detachedCopy), "插入 obj.a3 失败");

	TEST_ASSERT_NULL(RyanJsonGetObjectToKey(obj, "a"));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectToKey(obj, "a2", "v")));
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(RyanJsonGetObjectToKey(obj, "a3", "v")));

	RyanJson_t expect = RyanJsonParse(expectText);
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, expect), "detached duplicate 双挂载结果与期望文档不一致");

	char *printed = RyanJsonPrint(root, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

static void testDuplicateOomRecoveryAndSourceImmutability(void)
{
	// 复杂链路：
	// Parse -> Duplicate(OOM失败) -> 恢复 hooks -> Duplicate(成功) -> 修改副本
	// -> 校验原树不变。
	// 目标：
	// 1) 验证 Duplicate 失败不会破坏源树；
	// 2) 验证 OOM 恢复后 Duplicate 能继续成功；
	// 3) 验证副本修改与原树隔离。
	const char *source = "{\"cfg\":{\"mode\":\"a\",\"retry\":1},\"arr\":[1,2]}";
	RyanJson_t root = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "OOM duplicate 样本解析失败");

	RyanJson_t baseline = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL_MESSAGE(baseline, "OOM duplicate 基线快照失败");

	UNITY_TEST_OOM_BEGIN(0);
	RyanJson_t dupFail = RyanJsonDuplicate(root);
	UNITY_TEST_OOM_END();
	TEST_ASSERT_NULL_MESSAGE(dupFail, "OOM 注入下 Duplicate 应失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, baseline), "Duplicate 失败后源树语义不应改变");

	RyanJson_t dupOk = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL_MESSAGE(dupOk, "恢复 hooks 后 Duplicate 应成功");

	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(RyanJsonGetObjectToKey(dupOk, "cfg", "mode"), "b"));
	TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(dupOk, "cfg", "retry"), 9));
	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(dupOk, "arr"), 1, RyanJsonCreateInt(NULL, 7)));

	TEST_ASSERT_EQUAL_STRING("a", RyanJsonGetStringValue(RyanJsonGetObjectToKey(root, "cfg", "mode")));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(root, "cfg", "retry")));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(root, "arr"), 1)));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(root, dupOk), "修改后的副本不应与原树相等");

	RyanJsonDelete(dupOk);
	RyanJsonDelete(baseline);
	RyanJsonDelete(root);
}

void testDuplicateRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testDuplicateEdgeCases);
	RUN_TEST(testDuplicateEmptyAndSpecial);
	RUN_TEST(testDuplicateFullScenarios);
	RUN_TEST(testDuplicateMassiveStress);
	RUN_TEST(testDuplicateCrossContainerMoveIsolationChain);
	RUN_TEST(testDuplicateDetachedNodeDualAttachWithoutAlias);
	RUN_TEST(testDuplicateOomRecoveryAndSourceImmutability);
}
