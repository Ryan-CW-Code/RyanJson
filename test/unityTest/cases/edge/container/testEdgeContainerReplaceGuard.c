#include "testBase.h"

static void testEdgeContainerOpsReplaceByIndexObjectWithNewKey(void)
{
	// 复杂链路：
	// Parse(Object) -> ReplaceByIndex -> Key 校验。
	// 目标：验证索引替换可更换 key。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(obj, 1, RyanJsonCreateInt("c", 3)));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "a"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "c"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(obj, "b"));

	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsReplaceByIndexArrayWithObject(void)
{
	// 复杂链路：
	// Parse(Array) -> ReplaceByIndex(对象) -> 结构校验。
	// 目标：验证数组索引可替换为对象。
	RyanJson_t arr = RyanJsonParse("[1,2]");
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "x", 1));
	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(arr, 0, obj));

	RyanJson_t item = RyanJsonGetObjectByIndex(arr, 0);
	TEST_ASSERT_TRUE(RyanJsonIsObject(item));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(item, "x")));

	RyanJsonDelete(arr);
}

static void testEdgeContainerOpsReplaceByKeyWithDetachedFromOther(void)
{
	// 复杂链路：
	// Parse -> DetachByKey -> ReplaceByKey -> Key 校验。
	// 目标：验证 ReplaceByKey 会重命名 key。
	RyanJson_t obj1 = RyanJsonParse("{\"a\":1}");
	RyanJson_t obj2 = RyanJsonParse("{\"b\":2}");
	TEST_ASSERT_NOT_NULL(obj1);
	TEST_ASSERT_NOT_NULL(obj2);

	RyanJson_t detached = RyanJsonDetachByKey(obj2, "b");
	TEST_ASSERT_NOT_NULL(detached);
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(obj1, "a", detached));

	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj1, "a"));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj1, "a")));
	TEST_ASSERT_EQUAL_UINT32(0U, RyanJsonGetSize(obj2));

	RyanJsonDelete(obj2);
	RyanJsonDelete(obj1);
}

static void testEdgeContainerOpsReplaceByKeyWithArrayContainer(void)
{
	// 复杂链路：
	// Parse -> Create(Array) -> ReplaceByKey -> 类型校验。
	// 目标：验证 ReplaceByKey 可替换为数组容器。
	RyanJson_t obj = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 2));
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(obj, "a", arr));

	RyanJson_t node = RyanJsonGetObjectByKey(obj, "a");
	TEST_ASSERT_TRUE(RyanJsonIsArray(node));
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(node));

	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsAddItemToArrayThenReplaceByIndex(void)
{
	// 复杂链路：
	// Create(Array) -> AddItemToArray -> ReplaceByIndex -> 类型校验。
	// 目标：验证替换后数组元素类型更新。
	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "x", 1));
	TEST_ASSERT_TRUE(RyanJsonAddItemToArray(arr, obj));
	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(arr, 0, RyanJsonCreateInt(NULL, 9)));

	TEST_ASSERT_TRUE(RyanJsonIsInt(RyanJsonGetObjectByIndex(arr, 0)));
	TEST_ASSERT_EQUAL_INT(9, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));

	RyanJsonDelete(arr);
}

static void testEdgeContainerOpsAddItemToObjectThenReplaceByKey(void)
{
	// 复杂链路：
	// Create(Object) -> AddItemToObject -> ReplaceByKey -> 类型校验。
	// 目标：验证替换后对象字段类型更新。
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 1));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(obj, "arr", arr));
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(obj, "arr", RyanJsonCreateObject()));

	TEST_ASSERT_TRUE(RyanJsonIsObject(RyanJsonGetObjectByKey(obj, "arr")));

	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsDetachThenReplaceByIndexSameObject(void)
{
	// 复杂链路：
	// Parse(Object) -> DetachByKey -> ReplaceByIndex -> Size 校验。
	// 目标：验证分离后再替换可正确更新结构。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t moved = RyanJsonDetachByKey(obj, "a");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(obj, 0, moved));
	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetSize(obj));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "a"));

	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsReplaceByIndexArrayOutOfRange(void)
{
	// 复杂链路：
	// Parse(Array) -> ReplaceByIndex(越界) -> item 保持游离。
	// 目标：验证越界替换失败路径。
	RyanJson_t arr = RyanJsonParse("[1]");
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t item = RyanJsonCreateInt(NULL, 9);
	TEST_ASSERT_NOT_NULL(item);
	TEST_ASSERT_FALSE(RyanJsonReplaceByIndex(arr, 2, item));
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(item));

	RyanJsonDelete(item);
	RyanJsonDelete(arr);
}

static void testEdgeContainerOpsReplaceByKeyMissing(void)
{
	// 复杂链路：
	// Parse(Object) -> ReplaceByKey(缺失) -> item 保持游离。
	// 目标：验证缺失 key 替换失败路径。
	RyanJson_t obj = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t item = RyanJsonCreateInt("x", 9);
	TEST_ASSERT_NOT_NULL(item);
	TEST_ASSERT_FALSE(RyanJsonReplaceByKey(obj, "missing", item));
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(item));

	RyanJsonDelete(item);
	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsReplaceByIndexEmptyContainerKeepsDetachedItem(void)
{
	// 复杂链路：
	// Create(空容器) -> ReplaceByIndex(失败) -> item 保持游离。
	// 目标：验证空容器替换失败不会消费 item。
	RyanJson_t arr = RyanJsonCreateArray();
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t arrItem = RyanJsonCreateInt(NULL, 1);
	TEST_ASSERT_NOT_NULL(arrItem);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByIndex(arr, 0, arrItem), "空数组 ReplaceByIndex 应失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDetachedItem(arrItem), "ReplaceByIndex 失败后 item 应保持游离");
	RyanJsonDelete(arrItem);

	RyanJson_t objItem = RyanJsonCreateInt("a", 1);
	TEST_ASSERT_NOT_NULL(objItem);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByIndex(obj, 0, objItem), "空对象 ReplaceByIndex 应失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDetachedItem(objItem), "ReplaceByIndex 失败后 item 应保持游离");
	RyanJsonDelete(objItem);

	RyanJsonDelete(obj);
	RyanJsonDelete(arr);
}

static void testEdgeContainerOpsGetObjectByIndexOnScalarReturnsNull(void)
{
	// 复杂链路：
	// Parse(标量) -> GetObjectByIndex -> NULL。
	// 目标：验证标量场景下索引获取失败。
	RyanJson_t root = RyanJsonParse("1");
	TEST_ASSERT_NOT_NULL(root);
	TEST_ASSERT_NULL(RyanJsonGetObjectByIndex(root, 0));
	RyanJsonDelete(root);
}

static void testEdgeContainerOpsGetObjectByKeyOnArrayReturnsNull(void)
{
	// 复杂链路：
	// Parse(Array) -> GetObjectByKey -> NULL。
	// 目标：验证数组场景下 key 获取失败。
	RyanJson_t arr = RyanJsonParse("[1]");
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(arr, "a"));
	RyanJsonDelete(arr);
}

static void testEdgeContainerOpsInsertKeylessIntoObjectFailsRecover(void)
{
	// 复杂链路：
	// Parse(Object) -> Insert(keyless, 失败) -> Insert(合法)。
	// 目标：验证失败后对象可继续使用。
	RyanJson_t obj = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t bad = RyanJsonCreateInt(NULL, 9);
	TEST_ASSERT_NOT_NULL(bad);
	TEST_ASSERT_FALSE(RyanJsonInsert(obj, 0, bad));

	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, RyanJsonCreateInt("b", 2)));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "a"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "b"));

	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsInsertIntoNonContainerFails(void)
{
	// 复杂链路：
	// Create(String) -> Insert(失败) -> 值不变。
	// 目标：验证非容器插入失败不会破坏自身。
	RyanJson_t str = RyanJsonCreateString("s", "v");
	TEST_ASSERT_NOT_NULL(str);

	RyanJson_t item = RyanJsonCreateInt(NULL, 1);
	TEST_ASSERT_NOT_NULL(item);
	TEST_ASSERT_FALSE(RyanJsonInsert(str, 0, item));

	TEST_ASSERT_EQUAL_STRING("v", RyanJsonGetStringValue(str));
	RyanJsonDelete(str);
}

static void testEdgeContainerOpsReplaceByIndexObjectOutOfRange(void)
{
	// 复杂链路：
	// Parse(Object) -> ReplaceByIndex(越界) -> item 保持游离。
	// 目标：验证对象越界替换失败路径。
	RyanJson_t obj = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t item = RyanJsonCreateInt("b", 2);
	TEST_ASSERT_NOT_NULL(item);
	TEST_ASSERT_FALSE(RyanJsonReplaceByIndex(obj, 2, item));
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(item));

	RyanJsonDelete(item);
	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsReplaceByIndexObjectWithKeylessItemFails(void)
{
	// 复杂链路：
	// Parse(Object) -> ReplaceByIndex(无 key 节点) -> 失败且 item 仍游离。
	// 目标：验证对象替换必须提供带 key 的节点。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t item = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(item);
	TEST_ASSERT_FALSE(RyanJsonReplaceByIndex(obj, 0, item));
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(item));

	RyanJsonDelete(item);
	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsReplaceByIndexUpdatesArrayHead(void)
{
	// 复杂链路：
	// Parse(Array) -> ReplaceByIndex(head) -> GetArrayValue 校验。
	// 目标：验证替换头元素后数组入口指针更新。
	RyanJson_t arr = RyanJsonParse("[1,2]");
	TEST_ASSERT_NOT_NULL(arr);

	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(arr, 0, RyanJsonCreateInt(NULL, 9)));
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(arr));
	TEST_ASSERT_EQUAL_INT(9, RyanJsonGetIntValue(RyanJsonGetArrayValue(arr)));
	TEST_ASSERT_EQUAL_INT(9, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));

	RyanJsonDelete(arr);
}

static void testEdgeContainerOpsReplaceByIndexUpdatesObjectHead(void)
{
	// 复杂链路：
	// Parse(Object) -> ReplaceByIndex(head) -> GetObjectValue 校验。
	// 目标：验证替换头节点后对象入口指针更新。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(obj, 0, RyanJsonCreateInt("h", 9)));
	RyanJson_t head = RyanJsonGetObjectValue(obj);
	TEST_ASSERT_NOT_NULL(head);
	TEST_ASSERT_EQUAL_STRING("h", RyanJsonGetKey(head));
	TEST_ASSERT_EQUAL_INT(9, RyanJsonGetIntValue(head));

	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsReplaceByKeyOnArrayFails(void)
{
	// 复杂链路：
	// Parse(Array) -> ReplaceByKey(失败) -> item 保持游离。
	// 目标：验证数组不可使用 ReplaceByKey。
	RyanJson_t arr = RyanJsonParse("[1]");
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t item = RyanJsonCreateInt("a", 9);
	TEST_ASSERT_NOT_NULL(item);
	TEST_ASSERT_FALSE(RyanJsonReplaceByKey(arr, "a", item));
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(item));

	RyanJsonDelete(item);
	RyanJsonDelete(arr);
}

static void testEdgeContainerOpsGetObjectByIndexEmptyObject(void)
{
	// 复杂链路：
	// Create(Object) -> GetObjectByIndex -> NULL。
	// 目标：验证空对象索引获取失败。
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_NULL(RyanJsonGetObjectByIndex(obj, 0));
	RyanJsonDelete(obj);
}

static void testEdgeContainerOpsGetObjectByIndexEmptyArray(void)
{
	// 复杂链路：
	// Create(Array) -> GetObjectByIndex -> NULL。
	// 目标：验证空数组索引获取失败。
	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_NULL(RyanJsonGetObjectByIndex(arr, 0));
	RyanJsonDelete(arr);
}

void testEdgeContainerReplaceGuardRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testEdgeContainerOpsReplaceByIndexObjectWithNewKey);
	RUN_TEST(testEdgeContainerOpsReplaceByIndexArrayWithObject);
	RUN_TEST(testEdgeContainerOpsReplaceByKeyWithDetachedFromOther);
	RUN_TEST(testEdgeContainerOpsReplaceByKeyWithArrayContainer);
	RUN_TEST(testEdgeContainerOpsAddItemToArrayThenReplaceByIndex);
	RUN_TEST(testEdgeContainerOpsAddItemToObjectThenReplaceByKey);
	RUN_TEST(testEdgeContainerOpsDetachThenReplaceByIndexSameObject);
	RUN_TEST(testEdgeContainerOpsReplaceByIndexArrayOutOfRange);
	RUN_TEST(testEdgeContainerOpsReplaceByKeyMissing);
	RUN_TEST(testEdgeContainerOpsReplaceByIndexEmptyContainerKeepsDetachedItem);
	RUN_TEST(testEdgeContainerOpsGetObjectByIndexOnScalarReturnsNull);
	RUN_TEST(testEdgeContainerOpsGetObjectByKeyOnArrayReturnsNull);
	RUN_TEST(testEdgeContainerOpsInsertKeylessIntoObjectFailsRecover);
	RUN_TEST(testEdgeContainerOpsInsertIntoNonContainerFails);
	RUN_TEST(testEdgeContainerOpsReplaceByIndexObjectOutOfRange);
	RUN_TEST(testEdgeContainerOpsReplaceByIndexObjectWithKeylessItemFails);
	RUN_TEST(testEdgeContainerOpsReplaceByIndexUpdatesArrayHead);
	RUN_TEST(testEdgeContainerOpsReplaceByIndexUpdatesObjectHead);
	RUN_TEST(testEdgeContainerOpsReplaceByKeyOnArrayFails);
	RUN_TEST(testEdgeContainerOpsGetObjectByIndexEmptyObject);
	RUN_TEST(testEdgeContainerOpsGetObjectByIndexEmptyArray);
}
