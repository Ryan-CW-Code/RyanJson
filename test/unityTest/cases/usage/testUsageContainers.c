#include "testBase.h"

static void testUsageContainerInsertAtExactSizeArray(void)
{
	// 复杂链路：
	// Parse(Array) -> Insert(index==size) -> Compare。
	// 目标：验证用户显式传入 index==size 时等价尾插。
	RyanJson_t arr = RyanJsonParse("[1,2]");
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 2, RyanJsonCreateInt(NULL, 3)));

	RyanJson_t expect = RyanJsonParse("[1,2,3]");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(arr, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(arr);
}

static void testUsageContainerInsertAtExactSizeObject(void)
{
	// 复杂链路：
	// Parse(Object) -> Insert(index==size) -> 顺序校验。
	// 目标：验证对象显式尾插的用户写法。
	RyanJson_t obj = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 1, RyanJsonCreateInt("b", 2)));

	TEST_ASSERT_EQUAL_STRING("a", RyanJsonGetKey(RyanJsonGetObjectByIndex(obj, 0)));
	TEST_ASSERT_EQUAL_STRING("b", RyanJsonGetKey(RyanJsonGetObjectByIndex(obj, 1)));

	RyanJsonDelete(obj);
}

static void testUsageContainerInsertDetachedContainerBetweenObjects(void)
{
	// 复杂链路：
	// Parse -> DetachByKey(container) -> Insert(另一对象) -> Compare。
	// 目标：验证带 key 的容器节点可按用户预期整体迁移并保留原 key。
	RyanJson_t root = RyanJsonParse("{\"left\":{\"arr\":[1,2]},\"right\":{}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t left = RyanJsonGetObjectToKey(root, "left");
	RyanJson_t right = RyanJsonGetObjectToKey(root, "right");
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(right);

	RyanJson_t moved = RyanJsonDetachByKey(left, "arr");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonInsert(right, 0, moved));

	RyanJson_t expect = RyanJsonParse("{\"left\":{},\"right\":{\"arr\":[1,2]}}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

static void testUsageContainerChangeKeyOnDetachedArrayThenInsertIntoObject(void)
{
	// 复杂链路：
	// Parse(Object) -> DetachByKey(Array) -> ChangeKey -> Insert(Object) -> Compare。
	// 目标：验证已分离数组在重命名后可直接回插为新字段。
	RyanJson_t obj = RyanJsonParse("{\"arr\":[1]}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t moved = RyanJsonDetachByKey(obj, "arr");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(moved, "arr2"));
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, moved));

	RyanJson_t expect = RyanJsonParse("{\"arr2\":[1]}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(obj);
}

static void testUsageContainerDetachNestedArrayToRoot(void)
{
	// 复杂链路：
	// Parse -> DetachByKey(子数组) -> AddItemToObject(root) -> 结果校验。
	// 目标：验证嵌套数组可直接提升为根级字段。
	RyanJson_t root = RyanJsonParse("{\"payload\":{\"items\":[1,2]},\"meta\":0}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t payload = RyanJsonGetObjectToKey(root, "payload");
	TEST_ASSERT_NOT_NULL(payload);

	RyanJson_t items = RyanJsonDetachByKey(payload, "items");
	TEST_ASSERT_NOT_NULL(items);
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, "items", items));

	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(payload, "items"));
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(RyanJsonGetObjectToKey(root, "items")));

	RyanJsonDelete(root);
}

static void testUsageContainerDetachArrayElementToObject(void)
{
	// 复杂链路：
	// Parse -> DetachByIndex -> AddItemToObject -> 结果校验。
	// 目标：验证数组元素可通过新 key 挂载到对象。
	RyanJson_t root = RyanJsonParse("{\"arr\":[{\"id\":1},{\"id\":2}],\"dst\":{}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t arr = RyanJsonGetObjectToKey(root, "arr");
	RyanJson_t dst = RyanJsonGetObjectToKey(root, "dst");
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_NOT_NULL(dst);

	RyanJson_t moved = RyanJsonDetachByIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(dst, "first", moved));

	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetArraySize(arr));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByKey(RyanJsonGetObjectByIndex(arr, 0), "id")));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(root, "dst", "first", "id")));

	RyanJsonDelete(root);
}

static void testUsageContainerDetachObjectToArrayThenInsert(void)
{
	// 复杂链路：
	// Parse -> DetachByKey(Object) -> Insert(Array) -> Compare。
	// 目标：验证对象字段可整体降级为数组元素。
	RyanJson_t root = RyanJsonParse("{\"obj\":{\"k\":1},\"arr\":[]}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t moved = RyanJsonDetachByKey(root, "obj");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonInsert(RyanJsonGetObjectToKey(root, "arr"), 0, moved));

	RyanJson_t expect = RyanJsonParse("{\"arr\":[{\"k\":1}]}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

static void testUsageContainerInsertParsedRootObjectIntoParsedRootArray(void)
{
	// 复杂链路：
	// Parse(ArrayRoot) -> Parse(ObjectRoot) -> Insert(Array, parsedRoot) -> Print -> Parse -> Compare。
	// 目标：验证独立解析出来的根对象，本身就能像游离容器一样直接并入另一份根数组文档。
	RyanJson_t arrRoot = RyanJsonParse("[{\"id\":\"old\"}]");
	RyanJson_t objRoot = RyanJsonParse("{\"id\":\"new\",\"tags\":[\"hot\"]}");
	TEST_ASSERT_NOT_NULL(arrRoot);
	TEST_ASSERT_NOT_NULL(objRoot);

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(arrRoot, 1, objRoot), "将解析出的根对象插入根数组失败");

	RyanJson_t expect = RyanJsonParse("[{\"id\":\"old\"},{\"id\":\"new\",\"tags\":[\"hot\"]}]");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(arrRoot, expect), "根对象并入根数组后的结构不符合预期");

	char *printed = RyanJsonPrint(arrRoot, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(arrRoot, roundtrip), "根对象并入根数组后的往返 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(expect);
	RyanJsonDelete(arrRoot);
}

static void testUsageContainerMoveDetachedSubtreeBetweenParsedDocs(void)
{
	// 复杂链路：
	// Parse(src) -> Parse(dst) -> DetachByKey(src 子对象) -> ChangeKey -> Insert(dst 子对象)
	// -> 双文档 Compare -> Print/Parse。
	// 目标：验证两份独立解析文档之间，可直接迁移 detached subtree，
	// 且源/目标文档都会稳定收敛到各自期望结构。
	RyanJson_t src = RyanJsonParse("{\"pool\":{\"prefs\":{\"lang\":\"zh\",\"tz\":\"UTC\"}},\"meta\":1}");
	RyanJson_t dst = RyanJsonParse("{\"user\":{\"name\":\"neo\"}}");
	TEST_ASSERT_NOT_NULL(src);
	TEST_ASSERT_NOT_NULL(dst);

	RyanJson_t pool = RyanJsonGetObjectToKey(src, "pool");
	RyanJson_t user = RyanJsonGetObjectToKey(dst, "user");
	TEST_ASSERT_NOT_NULL(pool);
	TEST_ASSERT_NOT_NULL(user);

	RyanJson_t moved = RyanJsonDetachByKey(pool, "prefs");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeKey(moved, "settings"), "重命名 detached subtree 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(user, UINT32_MAX, moved), "将 detached subtree 插入目标文档失败");

	RyanJson_t expectSrc = RyanJsonParse("{\"pool\":{},\"meta\":1}");
	RyanJson_t expectDst = RyanJsonParse("{\"user\":{\"name\":\"neo\",\"settings\":{\"lang\":\"zh\",\"tz\":\"UTC\"}}}");
	TEST_ASSERT_NOT_NULL(expectSrc);
	TEST_ASSERT_NOT_NULL(expectDst);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(src, expectSrc), "源文档迁移后的结构不符合预期");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(dst, expectDst), "目标文档迁移后的结构不符合预期");

	char *printedSrc = RyanJsonPrint(src, 128, RyanJsonFalse, NULL);
	char *printedDst = RyanJsonPrint(dst, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printedSrc);
	TEST_ASSERT_NOT_NULL(printedDst);

	RyanJson_t roundtripSrc = RyanJsonParse(printedSrc);
	RyanJson_t roundtripDst = RyanJsonParse(printedDst);
	TEST_ASSERT_NOT_NULL(roundtripSrc);
	TEST_ASSERT_NOT_NULL(roundtripDst);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(src, roundtripSrc), "源文档跨文档迁移后往返 Compare 应相等");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(dst, roundtripDst), "目标文档跨文档迁移后往返 Compare 应相等");

	RyanJsonDelete(roundtripDst);
	RyanJsonDelete(roundtripSrc);
	RyanJsonFree(printedDst);
	RyanJsonFree(printedSrc);
	RyanJsonDelete(expectDst);
	RyanJsonDelete(expectSrc);
	RyanJsonDelete(dst);
	RyanJsonDelete(src);
}

static void testUsageContainerMoveDetachedSubtreeFromParsedDocToCreatedDoc(void)
{
	// 复杂链路：
	// Parse(src) -> Create(dst root/user) -> DetachByKey(src 子对象) -> ChangeKey
	// -> Insert(created user, index==size) -> 双文档 Compare -> Print/Parse。
	// 目标：验证 parsed 文档分离出来的 detached subtree，可直接迁入 create 出来的目标文档；
	// 该链路与“parsed -> parsed”迁移不同，专门覆盖 parse/create 混合来源。
	RyanJson_t src = RyanJsonParse("{\"pool\":{\"prefs\":{\"lang\":\"zh\",\"tz\":\"UTC\"}},\"meta\":1}");
	TEST_ASSERT_NOT_NULL(src);

	RyanJson_t dst = RyanJsonCreateObject();
	RyanJson_t user = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(dst);
	TEST_ASSERT_NOT_NULL(user);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(user, "name", "neo"));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(dst, "user", user));

	RyanJson_t pool = RyanJsonGetObjectToKey(src, "pool");
	user = RyanJsonGetObjectToKey(dst, "user");
	TEST_ASSERT_NOT_NULL(pool);
	TEST_ASSERT_NOT_NULL(user);

	RyanJson_t moved = RyanJsonDetachByKey(pool, "prefs");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeKey(moved, "settings"), "重命名 detached subtree 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(user, RyanJsonGetSize(user), moved), "将 detached subtree 插入 created 文档失败");

	RyanJson_t expectSrc = RyanJsonParse("{\"pool\":{},\"meta\":1}");
	RyanJson_t expectDst = RyanJsonParse("{\"user\":{\"name\":\"neo\",\"settings\":{\"lang\":\"zh\",\"tz\":\"UTC\"}}}");
	TEST_ASSERT_NOT_NULL(expectSrc);
	TEST_ASSERT_NOT_NULL(expectDst);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(src, expectSrc), "源 parsed 文档迁移后的结构不符合预期");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(dst, expectDst), "目标 created 文档迁移后的结构不符合预期");

	char *printedSrc = RyanJsonPrint(src, 128, RyanJsonFalse, NULL);
	char *printedDst = RyanJsonPrint(dst, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printedSrc);
	TEST_ASSERT_NOT_NULL(printedDst);

	RyanJson_t roundtripSrc = RyanJsonParse(printedSrc);
	RyanJson_t roundtripDst = RyanJsonParse(printedDst);
	TEST_ASSERT_NOT_NULL(roundtripSrc);
	TEST_ASSERT_NOT_NULL(roundtripDst);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(src, roundtripSrc), "源 parsed 文档迁移到 created 文档后往返 Compare 应相等");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(dst, roundtripDst), "目标 created 文档接收 detached subtree 后往返 Compare 应相等");

	RyanJsonDelete(roundtripDst);
	RyanJsonDelete(roundtripSrc);
	RyanJsonFree(printedDst);
	RyanJsonFree(printedSrc);
	RyanJsonDelete(expectDst);
	RyanJsonDelete(expectSrc);
	RyanJsonDelete(dst);
	RyanJsonDelete(src);
}

void testUsageContainersRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testUsageContainerInsertAtExactSizeArray);
	RUN_TEST(testUsageContainerInsertAtExactSizeObject);
	RUN_TEST(testUsageContainerInsertDetachedContainerBetweenObjects);
	RUN_TEST(testUsageContainerChangeKeyOnDetachedArrayThenInsertIntoObject);
	RUN_TEST(testUsageContainerDetachNestedArrayToRoot);
	RUN_TEST(testUsageContainerDetachArrayElementToObject);
	RUN_TEST(testUsageContainerDetachObjectToArrayThenInsert);
	RUN_TEST(testUsageContainerInsertParsedRootObjectIntoParsedRootArray);
	RUN_TEST(testUsageContainerMoveDetachedSubtreeBetweenParsedDocs);
	RUN_TEST(testUsageContainerMoveDetachedSubtreeFromParsedDocToCreatedDoc);
}
