#include "testBase.h"

static void testAccessorMutationIndexPathAfterArrayChurn(void)
{
	// 复杂链路：
	// Parse(二维数组) -> DetachByIndex -> Insert -> GetObjectToIndex。
	// 目标：验证索引路径在数组重排后仍可正确命中。
	RyanJson_t root = RyanJsonParse("[[1,2],[3,4]]");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t moved = RyanJsonDetachByIndex(root, 0);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonInsert(root, 1, moved));

	RyanJson_t v1 = RyanJsonGetObjectToIndex(root, 1, 0);
	RyanJson_t v2 = RyanJsonGetObjectToIndex(root, 0, 1);
	TEST_ASSERT_NOT_NULL(v1);
	TEST_ASSERT_NOT_NULL(v2);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(v1));
	TEST_ASSERT_EQUAL_INT(4, RyanJsonGetIntValue(v2));

	RyanJsonDelete(root);
}

static void testAccessorMutationKeyPathAfterTypeReplace(void)
{
	// 复杂链路：
	// Parse -> GetObjectToKey(失败) -> ReplaceByKey(标量->对象) -> GetObjectToKey。
	// 目标：验证类型替换后路径查找恢复可用。
	RyanJson_t root = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(root);

	TEST_ASSERT_NULL(RyanJsonGetObjectToKey(root, "a", "b"));

	RyanJson_t newObj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(newObj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(newObj, "b", 2));
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "a", newObj));

	RyanJson_t node = RyanJsonGetObjectToKey(root, "a", "b");
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(node));

	RyanJsonDelete(root);
}

static void testAccessorMutationGetObjectByIndexsAfterChurn(void)
{
	// 复杂链路：
	// Parse(三层数组) -> DetachByIndex -> ChangeIntValue -> Insert -> GetObjectByIndexs。
	// 目标：验证多级 varargs 索引路径在数组重排后仍正确命中。
	RyanJson_t root = RyanJsonParse("[[[1,2],[3,4]],[[5,6],[7,8]]]");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t moved = RyanJsonDetachByIndex(root, 0);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectByIndexs(moved, 1, 0, UINT32_MAX), 30));
	TEST_ASSERT_TRUE(RyanJsonInsert(root, 1, moved));

	RyanJson_t v1 = RyanJsonGetObjectByIndexs(root, 1, 1, 0, UINT32_MAX);
	RyanJson_t v2 = RyanJsonGetObjectByIndexs(root, 0, 0, 1, UINT32_MAX);
	TEST_ASSERT_NOT_NULL(v1);
	TEST_ASSERT_NOT_NULL(v2);
	TEST_ASSERT_EQUAL_INT(30, RyanJsonGetIntValue(v1));
	TEST_ASSERT_EQUAL_INT(6, RyanJsonGetIntValue(v2));

	RyanJson_t expect = RyanJsonParse("[[[5,6],[7,8]],[[1,2],[30,4]]]");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

static void testAccessorMutationGetObjectByKeyAfterRepeatedInsertDelete(void)
{
	// 复杂链路：
	// Create(Object) -> Insert/Delete 交错 -> GetObjectByKey。
	// 目标：验证多次增删后 key 查找稳定。
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, RyanJsonCreateInt("a", 1)));
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 1, RyanJsonCreateInt("b", 2)));
	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "a"));
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, RyanJsonCreateInt("c", 3)));

	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(obj, "a"));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "b")));
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "c")));

	RyanJsonDelete(obj);
}

static void testAccessorMutationConsistencyAfterReplaceAndDetach(void)
{
	// 复杂链路：
	// Parse -> GetObjectByKeys -> DetachByIndex -> AddItemToObject -> ReplaceByKey。
	// 目标：验证路径访问和 Has 宏在 churn 后一致。
	RyanJson_t root = RyanJsonParse("{\"meta\":{\"arr\":[{\"id\":\"a\"},{\"id\":\"b\"}],\"flag\":true}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t arr = RyanJsonGetObjectByKeys(root, "meta", "arr", NULL);
	TEST_ASSERT_NOT_NULL(arr);
	RyanJson_t picked = RyanJsonDetachByIndex(arr, 1);
	TEST_ASSERT_NOT_NULL(picked);
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(RyanJsonGetObjectToKey(root, "meta"), "picked", picked));
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(RyanJsonGetObjectToKey(root, "meta"), "flag", RyanJsonCreateBool("flag", RyanJsonFalse)));

	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "meta", "picked", "id"));
	TEST_ASSERT_FALSE(RyanJsonGetBoolValue(RyanJsonGetObjectToKey(root, "meta", "flag")));
	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetArraySize(arr));

	RyanJsonDelete(root);
}

static void testAccessorMutationDeepGetObjectByIndexsAfterChurn(void)
{
	// 复杂链路：
	// Parse -> GetObjectByIndexs 深路径 -> Change -> Detach -> Compare(期望)。
	// 目标：验证深路径访问在数组 churn 后仍稳定。
	RyanJson_t root = RyanJsonParse("{\"root\":[{\"child\":[{\"v\":1},{\"v\":2}]}]}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t topArr = RyanJsonGetObjectToKey(root, "root");
	RyanJson_t topObj = RyanJsonGetObjectByIndexs(topArr, 0, UINT32_MAX);
	RyanJson_t childArr = RyanJsonGetObjectToKey(topObj, "child");
	RyanJson_t child1 = RyanJsonGetObjectByIndexs(childArr, 1, UINT32_MAX);
	TEST_ASSERT_NOT_NULL(child1);
	TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(child1, "v"), 9));

	RyanJson_t removed = RyanJsonDetachByIndex(childArr, 0);
	TEST_ASSERT_NOT_NULL(removed);
	RyanJsonDelete(removed);

	RyanJson_t expect = RyanJsonParse("{\"root\":[{\"child\":[{\"v\":9}]}]}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

void testAccessorMutationPathsRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testAccessorMutationIndexPathAfterArrayChurn);
	RUN_TEST(testAccessorMutationKeyPathAfterTypeReplace);
	RUN_TEST(testAccessorMutationGetObjectByIndexsAfterChurn);
	RUN_TEST(testAccessorMutationDeepGetObjectByIndexsAfterChurn);
	RUN_TEST(testAccessorMutationConsistencyAfterReplaceAndDetach);
	RUN_TEST(testAccessorMutationGetObjectByKeyAfterRepeatedInsertDelete);
}
