#include "testBase.h"

static void testAccessorPathGuardKeyTraversalStopsAtArrayNode(void)
{
	// 覆盖 key 路径在中途遇到 Array 节点时应立即停止：
	// - GetObjectByKeys 返回 NULL；
	// - HasObjectToKey 返回 false；
	// - 邻近合法路径不受影响。
	const char *jsonText = "{\"cfg\":{\"list\":[{\"id\":1}],\"obj\":{\"ok\":2}}}";
	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "array 边界样本解析失败");

	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "cfg", "list", "id", NULL), "Object key 路径在 Array 节点处应停止");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonHasObjectToKey(root, "cfg", "list", "id"), "HasObjectToKey 在 Array 节点处应为 false");

	RyanJson_t list = RyanJsonGetObjectByKeys(root, "cfg", "list", NULL);
	TEST_ASSERT_NOT_NULL(list);
	TEST_ASSERT_TRUE(RyanJsonIsArray(list));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(RyanJsonGetObjectToIndex(list, 0), "id")));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByKeys(root, "cfg", "obj", "ok", NULL)));

	RyanJsonDelete(root);
}

static void testAccessorPathGuardIndexTraversalStopsAfterObjectPathAtScalarLeaf(void)
{
	// 覆盖 index 路径可穿过 Object 容器，但在最终落到标量叶子后必须停止。
	const char *jsonText = "[{\"obj\":{\"k\":1}},{\"arr\":[10]}]";
	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "object/index 链路样本解析失败");

	TEST_ASSERT_TRUE(RyanJsonHasObjectToIndex(root, 0, 0, 0));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonHasObjectToIndex(root, 0, 0, 0, 0), "标量叶子后继续按 index 下钻应为 false");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByIndexs(root, 0, 0, 0, 0, UINT32_MAX), "Array index 路径在标量叶子后应停止");

	RyanJson_t first = RyanJsonGetObjectByIndex(root, 0);
	RyanJson_t second = RyanJsonGetObjectByIndex(root, 1);
	TEST_ASSERT_NOT_NULL(first);
	TEST_ASSERT_NOT_NULL(second);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(RyanJsonGetObjectToKey(first, "obj"), "k")));
	TEST_ASSERT_EQUAL_INT(10, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(second, "arr"), 0)));

	RyanJsonDelete(root);
}

static void testAccessorPathGuardKeyTraversalStopsAtScalarFamily(void)
{
	// 覆盖 key 路径在 Bool/Double/String/Null 叶子上继续下钻时的守护语义。
	RyanJson_t root = RyanJsonParse("{\"b\":false,\"d\":1.25,\"s\":\"x\",\"n\":null}");
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "scalar key guard 样本解析失败");

	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "b", "x", NULL), "bool 叶子继续下钻应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "d", "x", NULL), "double 叶子继续下钻应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "s", "x", NULL), "string 叶子继续下钻应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "n", "x", NULL), "null 叶子继续下钻应返回 NULL");

	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(root, "b", "x"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(root, "d", "x"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(root, "s", "x"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(root, "n", "x"));

	RyanJsonDelete(root);
}

static void testAccessorPathGuardIndexTraversalStopsAtScalarFamily(void)
{
	// 覆盖 index 路径在 Bool/Double/String/Null 元素上继续下钻时的守护语义。
	RyanJson_t root = RyanJsonParse("[false,1.25,\"x\",null]");
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "scalar index guard 样本解析失败");

	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByIndexs(root, 0, 0, UINT32_MAX), "bool 元素继续下钻应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByIndexs(root, 1, 0, UINT32_MAX), "double 元素继续下钻应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByIndexs(root, 2, 0, UINT32_MAX), "string 元素继续下钻应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByIndexs(root, 3, 0, UINT32_MAX), "null 元素继续下钻应返回 NULL");

	TEST_ASSERT_FALSE(RyanJsonHasObjectToIndex(root, 0, 0));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToIndex(root, 1, 0));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToIndex(root, 2, 0));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToIndex(root, 3, 0));

	RyanJsonDelete(root);
}

static void testAccessorPathGuardMismatchDoesNotPoisonSubsequentLookups(void)
{
	// 复杂链路：先走两条错误路径，再验证同一文档中的合法路径仍可稳定命中。
	const char *jsonText = "{\"left\":[{\"name\":\"a\"}],\"right\":{\"items\":[10,20]}}";
	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "mismatch 恢复样本解析失败");

	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "left", "name", NULL), "先经过 Array 再取 key 应失败");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByIndexs(RyanJsonGetObjectByKey(root, "right"), 0, 0, 0, UINT32_MAX),
				 "先经过 Object 容器再落到标量后继续取 index 应失败");

	RyanJson_t left = RyanJsonGetObjectByKey(root, "left");
	RyanJson_t rightItems = RyanJsonGetObjectByKeys(root, "right", "items", NULL);
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(rightItems);
	TEST_ASSERT_EQUAL_STRING("a", RyanJsonGetStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectByIndex(left, 0), "name")));
	TEST_ASSERT_EQUAL_INT(20, RyanJsonGetIntValue(RyanJsonGetObjectByIndexs(rightItems, 1, UINT32_MAX)));

	RyanJsonDelete(root);
}

void testAccessorPathGuardRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testAccessorPathGuardKeyTraversalStopsAtArrayNode);
	RUN_TEST(testAccessorPathGuardIndexTraversalStopsAfterObjectPathAtScalarLeaf);
	RUN_TEST(testAccessorPathGuardKeyTraversalStopsAtScalarFamily);
	RUN_TEST(testAccessorPathGuardIndexTraversalStopsAtScalarFamily);
	RUN_TEST(testAccessorPathGuardMismatchDoesNotPoisonSubsequentLookups);
}
