#include "testBase.h"

static void testListStabilityArrayDeleteHeadTailThenInsert(void)
{
	// 复杂链路：
	// Parse(Array) -> DeleteByIndex(head/tail) -> Insert(head/tail) -> 遍历链表校验。
	// 目标：验证 Array 头尾删插后链表稳定。
	RyanJson_t arr = RyanJsonParse("[1,2,3,4,5]");
	TEST_ASSERT_NOT_NULL(arr);

	TEST_ASSERT_TRUE(RyanJsonDeleteByIndex(arr, 0));
	TEST_ASSERT_TRUE(RyanJsonDeleteByIndex(arr, RyanJsonGetArraySize(arr) - 1U));
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, RyanJsonCreateInt(NULL, 9)));
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 99, RyanJsonCreateInt(NULL, 8)));

	TEST_ASSERT_EQUAL_UINT32(5U, RyanJsonGetArraySize(arr));
	TEST_ASSERT_EQUAL_INT(9, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 1)));
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 2)));
	TEST_ASSERT_EQUAL_INT(4, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 3)));
	TEST_ASSERT_EQUAL_INT(8, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 4)));

	uint32_t size = RyanJsonGetArraySize(arr);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(arr);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > size + 1U) { TEST_FAIL_MESSAGE("Array 链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(size, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "Array 尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(arr);
}

static void testListStabilityArrayLargeChurnNoCycle(void)
{
	// 复杂链路：
	// Create(Array) -> 批量 Insert -> Detach/Insert -> 遍历链表校验。
	// 目标：验证较多节点下链表无环且尾节点正确。
	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);

	for (int32_t i = 0; i < 20; i++)
	{
		TEST_ASSERT_TRUE(RyanJsonInsert(arr, 99, RyanJsonCreateInt(NULL, i)));
	}

	RyanJson_t moved1 = RyanJsonDetachByIndex(arr, 5);
	RyanJson_t moved2 = RyanJsonDetachByIndex(arr, 10);
	TEST_ASSERT_NOT_NULL(moved1);
	TEST_ASSERT_NOT_NULL(moved2);
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, moved1));
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 99, moved2));

	uint32_t size = RyanJsonGetArraySize(arr);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(arr);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > size + 1U) { TEST_FAIL_MESSAGE("Array 链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(size, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "Array 尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(arr);
}

static void testListStabilityObjectDeleteHeadTailThenInsert(void)
{
	// 复杂链路：
	// Parse(Object) -> DeleteByKey(head/tail) -> Insert(head/tail) -> 遍历链表校验。
	// 目标：验证 Object 头尾删插后链表稳定。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2,\"c\":3,\"d\":4}");
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "a"));
	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "d"));
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, RyanJsonCreateInt("x", 9)));
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 99, RyanJsonCreateInt("y", 8)));

	TEST_ASSERT_EQUAL_UINT32(4U, RyanJsonGetSize(obj));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "b"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "c"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "x"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "y"));

	uint32_t size = RyanJsonGetSize(obj);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(obj);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > size + 1U) { TEST_FAIL_MESSAGE("Object 链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(size, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "Object 尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(obj);
}

static void testListStabilityObjectDetachThenReplaceAndInsert(void)
{
	// 复杂链路：
	// Parse(Object) -> DetachByKey -> ReplaceByKey -> Insert -> 遍历链表校验。
	// 目标：验证 Object 分离/替换/插入后的链表稳定。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2,\"c\":3}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t moved = RyanJsonDetachByKey(obj, "b");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(obj, "a", RyanJsonCreateInt("a", 9)));
	TEST_ASSERT_TRUE(RyanJsonChangeKey(moved, "d"));
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 99, moved));

	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetSize(obj));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "a"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "c"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "d"));

	uint32_t size = RyanJsonGetSize(obj);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(obj);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > size + 1U) { TEST_FAIL_MESSAGE("Object 链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(size, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "Object 尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(obj);
}

static void testListStabilityObjectMultiReplaceByIndex(void)
{
	// 复杂链路：
	// Parse(Object) -> ReplaceByIndex(多次) -> 遍历链表校验。
	// 目标：验证多次索引替换后链表稳定。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2,\"c\":3}");
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(obj, 0, RyanJsonCreateInt("x", 9)));
	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(obj, 1, RyanJsonCreateInt("y", 8)));

	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetSize(obj));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "x"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "y"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "c"));

	uint32_t size = RyanJsonGetSize(obj);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(obj);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > size + 1U) { TEST_FAIL_MESSAGE("Object 链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(size, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "Object 尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(obj);
}

static void testListStabilityObjectDuplicateKeyDeleteLoop(void)
{
	// 复杂链路：
	// Parse(重复 key) -> DeleteByKey(多次) -> 遍历链表校验。
	// 目标：验证重复 key 删除后的链表稳定。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"a\":2,\"b\":3}");
	if (NULL == obj)
	{
		// strict 控制组：无重复 key。
		RyanJson_t control = RyanJsonParse("{\"a\":1,\"b\":3}");
		TEST_ASSERT_NOT_NULL(control);
		TEST_ASSERT_TRUE(RyanJsonDeleteByKey(control, "a"));
		TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetSize(control));
		RyanJsonDelete(control);
		return;
	}

	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "a"));
	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "a"));
	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetSize(obj));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "b"));

	uint32_t size = RyanJsonGetSize(obj);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(obj);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > size + 1U) { TEST_FAIL_MESSAGE("Object 链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(size, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "Object 尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(obj);
}

void testStabilityLinkedListStressRunner(void)
{
	RUN_TEST(testListStabilityArrayDeleteHeadTailThenInsert);
	RUN_TEST(testListStabilityArrayLargeChurnNoCycle);
	RUN_TEST(testListStabilityObjectDeleteHeadTailThenInsert);
	RUN_TEST(testListStabilityObjectDetachThenReplaceAndInsert);
	RUN_TEST(testListStabilityObjectMultiReplaceByIndex);
	RUN_TEST(testListStabilityObjectDuplicateKeyDeleteLoop);
}
