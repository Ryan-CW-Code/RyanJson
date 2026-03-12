#include "testBase.h"

static void testEdgeContainerOpsInsertBeyondSizeArray(void)
{
	// 复杂链路：
	// Parse(Array) -> Insert(超范围) -> Compare。
	// 目标：验证超范围插入等价尾插。
	RyanJson_t arr = RyanJsonParse("[1,2]");
	TEST_ASSERT_NOT_NULL(arr);

	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 99, RyanJsonCreateInt(NULL, 3)));
	RyanJson_t expect = RyanJsonParse("[1,2,3]");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(arr, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(arr);
}

static void testEdgeContainerOpsInsertBeyondSizeObject(void)
{
	// 复杂链路：
	// Parse(Object) -> Insert(超范围) -> 顺序校验。
	// 目标：验证超范围插入等价尾插。
	RyanJson_t obj = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 99, RyanJsonCreateInt("b", 2)));
	TEST_ASSERT_EQUAL_STRING("a", RyanJsonGetKey(RyanJsonGetObjectByIndex(obj, 0)));
	TEST_ASSERT_EQUAL_STRING("b", RyanJsonGetKey(RyanJsonGetObjectByIndex(obj, 1)));

	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsInsertAtHeadObject(void)
{
	// 复杂链路：
	// Parse(Object) -> Insert(head) -> 顺序校验。
	// 目标：验证头插对顺序的影响。
	RyanJson_t obj = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, RyanJsonCreateInt("b", 2)));
	TEST_ASSERT_EQUAL_STRING("b", RyanJsonGetKey(RyanJsonGetObjectByIndex(obj, 0)));
	TEST_ASSERT_EQUAL_STRING("a", RyanJsonGetKey(RyanJsonGetObjectByIndex(obj, 1)));

	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsInsertReorderObject(void)
{
	// 复杂链路：
	// Create(Object) -> 多次 Insert(中间) -> 顺序校验。
	// 目标：验证中间插入的顺序效果。
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, RyanJsonCreateInt("a", 1)));
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 1, RyanJsonCreateInt("b", 2)));
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 1, RyanJsonCreateInt("c", 3)));

	TEST_ASSERT_EQUAL_STRING("a", RyanJsonGetKey(RyanJsonGetObjectByIndex(obj, 0)));
	TEST_ASSERT_EQUAL_STRING("c", RyanJsonGetKey(RyanJsonGetObjectByIndex(obj, 1)));
	TEST_ASSERT_EQUAL_STRING("b", RyanJsonGetKey(RyanJsonGetObjectByIndex(obj, 2)));

	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsAddItemToObjectRejectScalarThenRecover(void)
{
	// 复杂链路：
	// Create(Object) -> AddItemToObject(标量失败) -> AddIntToObject(成功)。
	// 目标：验证 AddItemToObject 失败后对象仍可稳定复用。
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t scalar = RyanJsonCreateInt(NULL, 7);
	TEST_ASSERT_NOT_NULL(scalar);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonAddItemToObject(obj, "x", scalar), "AddItemToObject(标量) 应失败");
	TEST_ASSERT_EQUAL_UINT32(0U, RyanJsonGetSize(obj));

	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "x", 7));
	TEST_ASSERT_EQUAL_INT(7, RyanJsonGetIntValue(RyanJsonGetObjectToKey(obj, "x")));

	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsDeleteByIndexArrayOfObjects(void)
{
	// 复杂链路：
	// Parse(Array<Object>) -> DeleteByIndex -> 结构校验。
	// 目标：验证删除对象数组元素后剩余元素正确。
	RyanJson_t arr = RyanJsonParse("[{\"a\":1},{\"b\":2}]");
	TEST_ASSERT_NOT_NULL(arr);

	TEST_ASSERT_TRUE(RyanJsonDeleteByIndex(arr, 0));
	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetArraySize(arr));
	RyanJson_t item = RyanJsonGetObjectByIndex(arr, 0);
	TEST_ASSERT_TRUE(RyanJsonIsObject(item));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(item, "b"));

	RyanJsonDelete(arr);
}

static void testEdgeContainerOpsDeleteByKeyObjectWithArray(void)
{
	// 复杂链路：
	// Parse(Object) -> DeleteByKey(数组) -> Size 校验。
	// 目标：验证删除数组字段后对象结构正确。
	RyanJson_t obj = RyanJsonParse("{\"a\":[1],\"b\":2}");
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "a"));
	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetSize(obj));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(obj, "a"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "b"));

	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsDetachThenInsertBackSameIndex(void)
{
	// 复杂链路：
	// Parse(Array) -> DetachByIndex -> Insert(同索引) -> Compare。
	// 目标：验证同索引回插恢复顺序。
	RyanJson_t arr = RyanJsonParse("[1,2,3]");
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t moved = RyanJsonDetachByIndex(arr, 1);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 1, moved));

	RyanJson_t expect = RyanJsonParse("[1,2,3]");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(arr, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(arr);
}

static void testEdgeContainerOpsDetachThenInsertBackTail(void)
{
	// 复杂链路：
	// Parse(Array) -> DetachByIndex -> Insert(tail) -> Compare。
	// 目标：验证回插到尾部后的顺序。
	RyanJson_t arr = RyanJsonParse("[1,2,3]");
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t moved = RyanJsonDetachByIndex(arr, 1);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 99, moved));

	RyanJson_t expect = RyanJsonParse("[1,3,2]");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(arr, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(arr);
}

static void testEdgeContainerOpsDeleteThenInsertSameKey(void)
{
	// 复杂链路：
	// Parse(Object) -> DeleteByKey -> Insert(同 key) -> Compare。
	// 目标：验证删除后可重新插入同名 key。
	RyanJson_t obj = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "a"));
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, RyanJsonCreateInt("a", 2)));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "a")));

	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsDeleteThenInsertSameIndexArray(void)
{
	// 复杂链路：
	// Parse(Array) -> DeleteByIndex -> Insert(同索引) -> Compare。
	// 目标：验证删除后可在同索引插入新元素。
	RyanJson_t arr = RyanJsonParse("[1,2]");
	TEST_ASSERT_NOT_NULL(arr);

	TEST_ASSERT_TRUE(RyanJsonDeleteByIndex(arr, 0));
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, RyanJsonCreateInt(NULL, 9)));

	RyanJson_t expect = RyanJsonParse("[9,2]");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(arr, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(arr);
}

static void testEdgeContainerOpsInsertKeyedItemIntoArray(void)
{
	// 复杂链路：
	// Parse(Object) -> DetachByKey -> Insert(Array) -> Key 校验。
	// 目标：验证数组接受携带 key 的节点。
	RyanJson_t obj = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(obj);
	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t moved = RyanJsonDetachByKey(obj, "a");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, moved));
	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetArraySize(arr));

	RyanJson_t item = RyanJsonGetObjectByIndex(arr, 0);
	TEST_ASSERT_TRUE(RyanJsonIsKey(item));
	TEST_ASSERT_EQUAL_STRING("a", RyanJsonGetKey(item));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(item));

	RyanJsonDelete(arr);
	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsDeleteByIndexOnObject(void)
{
	// 复杂链路：
	// Parse(Object) -> DeleteByIndex -> Key 校验。
	// 目标：验证对象按索引删除语义。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2,\"c\":3}");
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonDeleteByIndex(obj, 1));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "a"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(obj, "b"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "c"));

	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsDeleteByIndexOnEmptyObject(void)
{
	// 复杂链路：
	// Create(Object) -> DeleteByIndex(空对象) -> 返回 false。
	// 目标：验证空对象删除失败路径。
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_FALSE(RyanJsonDeleteByIndex(obj, 0));
	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsDetachByIndexObjectOutOfRange(void)
{
	// 复杂链路：
	// Parse(Object) -> DetachByIndex(越界) -> 返回 NULL。
	// 目标：验证对象越界分离失败路径。
	RyanJson_t obj = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_NULL(RyanJsonDetachByIndex(obj, 2));
	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsChangeKeyOnArrayElement(void)
{
	// 复杂链路：
	// Parse(Object) -> DetachByKey -> Insert(Array) -> ChangeKey -> 校验。
	// 目标：验证数组内带 key 元素可改 key。
	RyanJson_t obj = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(obj);
	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t moved = RyanJsonDetachByKey(obj, "a");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, moved));
	TEST_ASSERT_TRUE(RyanJsonChangeKey(moved, "b"));
	TEST_ASSERT_EQUAL_STRING("b", RyanJsonGetKey(RyanJsonGetObjectByIndex(arr, 0)));

	RyanJsonDelete(arr);
	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsDetachThenDeleteByIndexArray(void)
{
	// 复杂链路：
	// Parse(Array) -> DetachByIndex -> DeleteByIndex -> Compare。
	// 目标：验证分离与删除组合后顺序正确。
	RyanJson_t arr = RyanJsonParse("[1,2,3,4]");
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t moved = RyanJsonDetachByIndex(arr, 1);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonDeleteByIndex(arr, 1));

	RyanJson_t expect = RyanJsonParse("[1,4]");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(arr, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(moved);
	RyanJsonDelete(arr);
}

void testEdgeContainerInsertDeleteRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testEdgeContainerOpsInsertBeyondSizeArray);
	RUN_TEST(testEdgeContainerOpsInsertBeyondSizeObject);
	RUN_TEST(testEdgeContainerOpsInsertAtHeadObject);
	RUN_TEST(testEdgeContainerOpsInsertReorderObject);
	RUN_TEST(testEdgeContainerOpsAddItemToObjectRejectScalarThenRecover);
	RUN_TEST(testEdgeContainerOpsDeleteByIndexArrayOfObjects);
	RUN_TEST(testEdgeContainerOpsDeleteByKeyObjectWithArray);
	RUN_TEST(testEdgeContainerOpsDetachThenInsertBackSameIndex);
	RUN_TEST(testEdgeContainerOpsDetachThenInsertBackTail);
	RUN_TEST(testEdgeContainerOpsDeleteThenInsertSameKey);
	RUN_TEST(testEdgeContainerOpsDeleteThenInsertSameIndexArray);
	RUN_TEST(testEdgeContainerOpsInsertKeyedItemIntoArray);
	RUN_TEST(testEdgeContainerOpsDeleteByIndexOnObject);
	RUN_TEST(testEdgeContainerOpsDeleteByIndexOnEmptyObject);
	RUN_TEST(testEdgeContainerOpsDetachByIndexObjectOutOfRange);
	RUN_TEST(testEdgeContainerOpsChangeKeyOnArrayElement);
	RUN_TEST(testEdgeContainerOpsDetachThenDeleteByIndexArray);
}
