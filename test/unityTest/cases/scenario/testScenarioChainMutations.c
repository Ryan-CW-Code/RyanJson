#include "testBase.h"

static void testChainScenarioDeepMoveReplaceAndRoundtrip(void)
{
	// 复杂链路：
	// Parse -> Detach/Change/Insert(嵌套) -> ReplaceByIndex -> AddItemToObject ->
	// ReplaceByKey(meta) -> Print/Parse。
	// 目标：覆盖嵌套数组/对象的多步变更与往返稳定性。
	RyanJson_t root = RyanJsonParse(
		"{\"cfg\":{\"mode\":\"a\",\"limit\":3},\"items\":[{\"id\":1,\"tags\":[\"x\",\"y\"]},{\"id\":2,\"tags\":[\"z\"]}],"
		"\"meta\":{\"ver\":1}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t items = RyanJsonGetObjectToKey(root, "items");
	TEST_ASSERT_NOT_NULL(items);

	RyanJson_t moved = RyanJsonDetachByIndex(items, 0);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(moved, "id"), 10));

	RyanJson_t tags = RyanJsonGetObjectToKey(moved, "tags");
	TEST_ASSERT_NOT_NULL(tags);
	RyanJson_t tag = RyanJsonDetachByIndex(tags, 1);
	TEST_ASSERT_NOT_NULL(tag);
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(tag, "yy"));
	TEST_ASSERT_TRUE(RyanJsonInsert(tags, 0, tag));

	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(items, 0, RyanJsonCreateInt(NULL, 99)));

	RyanJson_t cfg = RyanJsonGetObjectToKey(root, "cfg");
	TEST_ASSERT_NOT_NULL(cfg);
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(cfg, "primary", moved));

	RyanJson_t metaNew = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(metaNew);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(metaNew, "ver", 2));
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(metaNew, "note", "ok"));
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "meta", metaNew));

	RyanJson_t expect =
		RyanJsonParse("{\"cfg\":{\"mode\":\"a\",\"limit\":3,\"primary\":{\"id\":10,\"tags\":[\"yy\",\"x\"]}},\"items\":[99],"
			      "\"meta\":{\"ver\":2,\"note\":\"ok\"}}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	char *printed = RyanJsonPrint(root, 0, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

static void testChainScenarioSwapArrayObjectNodesWithMetaReplace(void)
{
	// 复杂链路：
	// Parse -> DetachByIndex(数组对象) -> ChangeKey -> Insert(对象) ->
	// DetachByKey(对象) -> ChangeKey -> Insert(数组) -> ReplaceByKey(meta)。
	// 目标：覆盖数组对象与对象字段互换、以及 meta 容器替换的组合路径。
	RyanJson_t root = RyanJsonParse("{\"obj\":{\"x\":1,\"y\":{\"id\":1}},\"arr\":[{\"id\":2},3],\"meta\":{\"flag\":true}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t obj = RyanJsonGetObjectToKey(root, "obj");
	RyanJson_t arr = RyanJsonGetObjectToKey(root, "arr");
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t fromArr = RyanJsonDetachByIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(fromArr);
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(obj, "fromArr", fromArr));
	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetArraySize(arr));

	RyanJson_t moved = RyanJsonDetachByKey(obj, "y");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(moved, "moved"));
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, moved));

	RyanJson_t metaNew = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(metaNew);
	TEST_ASSERT_TRUE(RyanJsonAddBoolToObject(metaNew, "flag", RyanJsonFalse));
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(metaNew, "tag", "swap"));
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "meta", metaNew));

	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(obj, "x")));
	RyanJson_t fromArrNode = RyanJsonGetObjectToKey(obj, "fromArr");
	TEST_ASSERT_NOT_NULL(fromArrNode);
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectToKey(fromArrNode, "id")));
	TEST_ASSERT_NULL(RyanJsonGetObjectToKey(obj, "y"));

	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(arr));
	RyanJson_t arr0 = RyanJsonGetObjectByIndex(arr, 0);
	RyanJson_t arr1 = RyanJsonGetObjectByIndex(arr, 1);
	TEST_ASSERT_TRUE(RyanJsonIsObject(arr0));
	TEST_ASSERT_TRUE(RyanJsonIsInt(arr1));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(arr0, "id")));
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(arr1));

	RyanJson_t meta = RyanJsonGetObjectToKey(root, "meta");
	TEST_ASSERT_NOT_NULL(meta);
	TEST_ASSERT_EQUAL_INT(RyanJsonFalse, RyanJsonGetBoolValue(RyanJsonGetObjectToKey(meta, "flag")));
	TEST_ASSERT_EQUAL_STRING("swap", RyanJsonGetStringValue(RyanJsonGetObjectToKey(meta, "tag")));

	RyanJsonDelete(root);
}

void testScenarioChainMutationsRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testChainScenarioDeepMoveReplaceAndRoundtrip);
	RUN_TEST(testChainScenarioSwapArrayObjectNodesWithMetaReplace);
}
