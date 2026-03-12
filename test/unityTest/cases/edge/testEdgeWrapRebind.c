#include "testBase.h"

static void testEdgeWrapAddItemToObjectDiscardOldKey(void)
{
	// 复杂链路：
	// Parse -> DetachByKey(带 key 的容器) -> AddItemToObject(新 key) -> Compare。
	// 目标：验证 AddItemToObject 会丢弃原 key，仅保留新 key。
	RyanJson_t root = RyanJsonParse("{\"src\":{\"old\":{\"v\":1}},\"dst\":{}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t src = RyanJsonGetObjectToKey(root, "src");
	RyanJson_t dst = RyanJsonGetObjectToKey(root, "dst");
	TEST_ASSERT_NOT_NULL(src);
	TEST_ASSERT_NOT_NULL(dst);

	RyanJson_t moved = RyanJsonDetachByKey(src, "old");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToObject(dst, "new", moved), "AddItemToObject 包装挂载失败");

	TEST_ASSERT_NOT_NULL(RyanJsonGetObjectToKey(dst, "new"));
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(dst, "old"), "包装后不应保留旧 key");
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(dst, "new", "v")));

	RyanJson_t expect = RyanJsonParse("{\"src\":{},\"dst\":{\"new\":{\"v\":1}}}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

static void testEdgeWrapAddItemToArrayDiscardOldKey(void)
{
	// 复杂链路：
	// Parse -> DetachByKey(带 key 的容器) -> AddItemToArray -> Compare/属性校验。
	// 目标：验证 AddItemToArray 会丢弃原 key，数组元素不应携带 key。
	RyanJson_t root = RyanJsonParse("{\"src\":{\"old\":{\"v\":1}},\"arr\":[]}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t src = RyanJsonGetObjectToKey(root, "src");
	RyanJson_t arr = RyanJsonGetObjectToKey(root, "arr");
	TEST_ASSERT_NOT_NULL(src);
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t moved = RyanJsonDetachByKey(src, "old");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToArray(arr, moved), "AddItemToArray 包装挂载失败");

	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetArraySize(arr));
	RyanJson_t arrItem = RyanJsonGetObjectByIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(arrItem);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsKey(arrItem), "数组元素不应携带 key");
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(arrItem, "v")));

	RyanJson_t expect = RyanJsonParse("{\"src\":{},\"arr\":[{\"v\":1}]}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

static void testEdgeWrapObjectNodeToArrayThenRebindNewKey(void)
{
	// 复杂链路：
	// Parse -> DetachByKey -> AddItemToArray -> DetachByIndex -> ChangeKey(失败) -> AddItemToObject。
	// 目标：验证对象节点经数组中转后会失去 key，必须通过 AddItemToObject 重新绑定新 key。
	RyanJson_t root = RyanJsonParse("{\"obj\":{\"k\":{\"v\":1}},\"arr\":[]}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t obj = RyanJsonGetObjectToKey(root, "obj");
	RyanJson_t arr = RyanJsonGetObjectToKey(root, "arr");
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t moved = RyanJsonDetachByKey(obj, "k");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonAddItemToArray(arr, moved));

	RyanJson_t movedBack = RyanJsonDetachByIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(movedBack);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeKey(movedBack, "k2"), "数组元素无 key，ChangeKey 应失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToObject(obj, "k2", movedBack), "AddItemToObject 重新绑定 key 失败");

	RyanJson_t expect = RyanJsonParse("{\"obj\":{\"k2\":{\"v\":1}},\"arr\":[]}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

static void testEdgeWrapArrayElementToObjectAndBackPreservesWrappedKey(void)
{
	// 复杂链路：
	// Parse -> DetachByIndex -> AddItemToObject -> DetachByKey -> Insert(Array)。
	// 目标：验证数组元素包装成对象字段后，再插回数组时会保留包装阶段生成的 key。
	RyanJson_t root = RyanJsonParse("{\"arr\":[{\"id\":\"a\"},{\"id\":\"b\"}],\"obj\":{}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t arr = RyanJsonGetObjectToKey(root, "arr");
	RyanJson_t obj = RyanJsonGetObjectToKey(root, "obj");
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t moved = RyanJsonDetachByIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToObject(obj, "first", moved), "将数组元素包装挂到对象失败");

	RyanJson_t movedBack = RyanJsonDetachByKey(obj, "first");
	TEST_ASSERT_NOT_NULL(movedBack);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(arr, 1, movedBack), "将包装节点插回数组失败");

	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(arr));
	TEST_ASSERT_EQUAL_UINT32(0U, RyanJsonGetSize(obj));

	RyanJson_t e0 = RyanJsonGetObjectByIndex(arr, 0);
	RyanJson_t e1 = RyanJsonGetObjectByIndex(arr, 1);
	TEST_ASSERT_NOT_NULL(e0);
	TEST_ASSERT_NOT_NULL(e1);
	TEST_ASSERT_EQUAL_STRING("b", RyanJsonGetStringValue(RyanJsonGetObjectToKey(e0, "id")));
	TEST_ASSERT_EQUAL_STRING("a", RyanJsonGetStringValue(RyanJsonGetObjectToKey(e1, "id")));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsKey(e1), "包装节点插回数组后应仍携带 key");

	RyanJsonDelete(root);
}

static void testEdgeWrapChangeDetachedKeyThenInsertBack(void)
{
	// 复杂链路：
	// Parse -> DetachByKey -> ChangeKey -> Insert(回插同对象) -> Compare。
	// 目标：验证分离节点改 key 后仍可稳定插回对象。
	RyanJson_t root = RyanJsonParse("{\"o\":{\"a\":1,\"b\":2}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t obj = RyanJsonGetObjectToKey(root, "o");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t moved = RyanJsonDetachByKey(obj, "a");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(moved, "c"));
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, moved));

	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(root, "o", "a"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "o", "b"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "o", "c"));

	RyanJson_t expect = RyanJsonParse("{\"o\":{\"b\":2,\"c\":1}}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

void testEdgeWrapRebindRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testEdgeWrapAddItemToObjectDiscardOldKey);
	RUN_TEST(testEdgeWrapAddItemToArrayDiscardOldKey);
	RUN_TEST(testEdgeWrapObjectNodeToArrayThenRebindNewKey);
	RUN_TEST(testEdgeWrapArrayElementToObjectAndBackPreservesWrappedKey);
	RUN_TEST(testEdgeWrapChangeDetachedKeyThenInsertBack);
}
