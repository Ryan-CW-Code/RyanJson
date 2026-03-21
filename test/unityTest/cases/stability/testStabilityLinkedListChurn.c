#include "testBase.h"

static void testListStabilityObjectInsertMiddleThenDeleteHead(void)
{
	// 复杂链路：
	// Parse(Object) -> Insert(middle) -> DeleteByIndex(head) -> 遍历链表校验。
	// 目标：验证 Object 中间插入与头删组合后链表稳定。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"c\":3}");
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 1, RyanJsonCreateInt("b", 2)));
	TEST_ASSERT_TRUE(RyanJsonDeleteByIndex(obj, 0));

	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetSize(obj));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "b"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "c"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(obj, "a"));

	uint32_t sizeAfter = RyanJsonGetSize(obj);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(obj);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > sizeAfter + 1U) { TEST_FAIL_MESSAGE("Object 链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(sizeAfter, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "Object 尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(obj);
}

static void testListStabilityArrayInsertMiddleThenDeleteTail(void)
{
	// 复杂链路：
	// Parse(Array) -> Insert(middle) -> DeleteByIndex(tail) -> 遍历链表校验。
	// 目标：验证 Array 中间插入与尾删组合后链表稳定。
	RyanJson_t arr = RyanJsonParse("[1,2,3]");
	TEST_ASSERT_NOT_NULL(arr);

	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 1, RyanJsonCreateInt(NULL, 9)));
	TEST_ASSERT_TRUE(RyanJsonDeleteByIndex(arr, RyanJsonGetArraySize(arr) - 1U));

	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetArraySize(arr));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));
	TEST_ASSERT_EQUAL_INT(9, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 1)));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 2)));

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

static void testListStabilityNestedObjectDetachChildToOuter(void)
{
	// 复杂链路：
	// Parse -> DetachByKey(inner) -> Insert(outer) -> 遍历 inner/outer 链表校验。
	// 目标：验证嵌套 Object 节点迁移后父子链表稳定。
	RyanJson_t root = RyanJsonParse("{\"outer\":{\"keep\":0,\"inner\":{\"x\":1}},\"slot\":{}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t outer = RyanJsonGetObjectToKey(root, "outer");
	RyanJson_t slot = RyanJsonGetObjectToKey(root, "slot");
	TEST_ASSERT_NOT_NULL(outer);
	TEST_ASSERT_NOT_NULL(slot);

	RyanJson_t inner = RyanJsonDetachByKey(outer, "inner");
	TEST_ASSERT_NOT_NULL(inner);
	TEST_ASSERT_TRUE(RyanJsonInsert(slot, 0, inner));

	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(root, "outer", "inner"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "slot", "inner"));

	// outer 链表校验
	{
		uint32_t size = RyanJsonGetSize(outer);
		uint32_t count = 0;
		RyanJson_t node = RyanJsonGetObjectValue(outer);
		RyanJson_t last = NULL;
		while (node)
		{
			count++;
			if (count > size + 1U) { TEST_FAIL_MESSAGE("outer 链表疑似形成环"); }
			last = node;
			node = RyanJsonGetNext(node);
		}
		TEST_ASSERT_EQUAL_UINT32(size, count);
		TEST_ASSERT_NOT_NULL(last);
		TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "outer 尾节点 GetNext 应返回 NULL");
	}

	// slot 链表校验
	{
		uint32_t size = RyanJsonGetSize(slot);
		uint32_t count = 0;
		RyanJson_t node = RyanJsonGetObjectValue(slot);
		RyanJson_t last = NULL;
		while (node)
		{
			count++;
			if (count > size + 1U) { TEST_FAIL_MESSAGE("slot 链表疑似形成环"); }
			last = node;
			node = RyanJsonGetNext(node);
		}
		TEST_ASSERT_EQUAL_UINT32(size, count);
		TEST_ASSERT_NOT_NULL(last);
		TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "slot 尾节点 GetNext 应返回 NULL");
	}

	RyanJsonDelete(root);
}

static void testListStabilityNestedArrayMoveBetweenInnerArrays(void)
{
	// 复杂链路：
	// Parse -> DetachByIndex(innerA) -> Insert(innerB) -> 遍历链表校验。
	// 目标：验证内层 Array 元素跨 Array 迁移后链表稳定。
	RyanJson_t root = RyanJsonParse("{\"a\":[[1,2],[3,4]]}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t outer = RyanJsonGetObjectToKey(root, "a");
	RyanJson_t innerA = RyanJsonGetObjectByIndex(outer, 0);
	RyanJson_t innerB = RyanJsonGetObjectByIndex(outer, 1);
	TEST_ASSERT_NOT_NULL(innerA);
	TEST_ASSERT_NOT_NULL(innerB);

	RyanJson_t moved = RyanJsonDetachByIndex(innerA, 1);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonInsert(innerB, 0, moved));

	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetArraySize(innerA));
	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetArraySize(innerB));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(innerA, 0)));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(innerB, 0)));
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(innerB, 1)));
	TEST_ASSERT_EQUAL_INT(4, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(innerB, 2)));

	{
		uint32_t size = RyanJsonGetArraySize(innerA);
		uint32_t count = 0;
		RyanJson_t node = RyanJsonGetObjectValue(innerA);
		RyanJson_t last = NULL;
		while (node)
		{
			count++;
			if (count > size + 1U) { TEST_FAIL_MESSAGE("innerA 链表疑似形成环"); }
			last = node;
			node = RyanJsonGetNext(node);
		}
		TEST_ASSERT_EQUAL_UINT32(size, count);
		TEST_ASSERT_NOT_NULL(last);
		TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "innerA 尾节点 GetNext 应返回 NULL");
	}

	{
		uint32_t size = RyanJsonGetArraySize(innerB);
		uint32_t count = 0;
		RyanJson_t node = RyanJsonGetObjectValue(innerB);
		RyanJson_t last = NULL;
		while (node)
		{
			count++;
			if (count > size + 1U) { TEST_FAIL_MESSAGE("innerB 链表疑似形成环"); }
			last = node;
			node = RyanJsonGetNext(node);
		}
		TEST_ASSERT_EQUAL_UINT32(size, count);
		TEST_ASSERT_NOT_NULL(last);
		TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "innerB 尾节点 GetNext 应返回 NULL");
	}

	RyanJsonDelete(root);
}

static void testListStabilityReplaceByIndexObjectWithDetached(void)
{
	// 复杂链路：
	// Parse -> DetachByKey -> ReplaceByIndex(新节点) -> 遍历链表校验。
	// 目标：验证 Object 经历分离后再替换索引的链表稳定性。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2,\"c\":3}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t moved = RyanJsonDetachByKey(obj, "a");
	TEST_ASSERT_NOT_NULL(moved);
	uint32_t size = RyanJsonGetSize(obj);
	TEST_ASSERT_TRUE_MESSAGE(size > 0U, "Object size 异常");
	uint32_t replaceIndex = size - 1U;
	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(obj, replaceIndex, RyanJsonCreateInt("a", 9)));
	RyanJsonDelete(moved);

	TEST_ASSERT_EQUAL_UINT32(size, RyanJsonGetSize(obj));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "a"));
	// 替换的是指定索引，避免依赖 Object 顺序，直接校验索引处 key/value。
	RyanJson_t replaced = RyanJsonGetObjectByIndex(obj, replaceIndex);
	TEST_ASSERT_NOT_NULL(replaced);
	TEST_ASSERT_EQUAL_STRING("a", RyanJsonGetKey(replaced));
	TEST_ASSERT_EQUAL_INT(9, RyanJsonGetIntValue(replaced));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "b") || RyanJsonHasObjectByKey(obj, "c"));

	uint32_t sizeAfter = RyanJsonGetSize(obj);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(obj);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > sizeAfter + 1U) { TEST_FAIL_MESSAGE("Object 链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(sizeAfter, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "Object 尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(obj);
}

static void testListStabilityDeleteByKeyDuplicateThenTraverse(void)
{
	// 复杂链路：
	// Parse(重复 key) -> DeleteByKey -> 遍历链表校验。
	// 目标：验证重复 key 删除后链表稳定。
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
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetSize(obj));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "a"));

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

static void testListStabilityArrayChurnInsertDetachReplace(void)
{
	// 复杂链路：
	// Parse(Array) -> Insert -> Detach -> Replace -> 遍历链表校验。
	// 目标：验证多次 Array 变更后链表稳定。
	RyanJson_t arr = RyanJsonParse("[1,2,3]");
	TEST_ASSERT_NOT_NULL(arr);

	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 1, RyanJsonCreateInt(NULL, 9)));
	RyanJson_t moved = RyanJsonDetachByIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(arr, 1, moved));

	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetArraySize(arr));
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

static void testListStabilityObjectChurnReplaceInsertDetach(void)
{
	// 复杂链路：
	// Parse(Object) -> ReplaceByKey -> Insert -> DetachByKey -> 遍历链表校验。
	// 目标：验证多次 Object 变更后链表稳定。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(obj, "a", RyanJsonCreateInt("a", 9)));
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, RyanJsonCreateInt("c", 3)));
	RyanJson_t moved = RyanJsonDetachByKey(obj, "b");
	TEST_ASSERT_NOT_NULL(moved);
	RyanJsonDelete(moved);

	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetSize(obj));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "a"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "c"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(obj, "b"));

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

static void testListStabilityGetNextAfterNestedDetachInsert(void)
{
	// 复杂链路：
	// Parse -> DetachByKey(嵌套) -> Insert(嵌套) -> 遍历链表校验。
	// 目标：验证嵌套 Object 内的 GetNext 关系稳定。
	RyanJson_t root = RyanJsonParse("{\"o\":{\"a\":1,\"b\":2,\"c\":3}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t obj = RyanJsonGetObjectToKey(root, "o");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t moved = RyanJsonDetachByKey(obj, "b");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, moved));

	uint32_t size = RyanJsonGetSize(obj);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(obj);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > size + 1U) { TEST_FAIL_MESSAGE("嵌套 Object 链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(size, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "嵌套 Object 尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(root);
}

static void testListStabilityArrayInsertHeadThenDetachMiddle(void)
{
	// 复杂链路：
	// Parse(Array) -> Insert(head) -> DetachByIndex(middle) -> 遍历链表校验。
	// 目标：验证 Array 头插后再分离中间节点的链表稳定性。
	RyanJson_t arr = RyanJsonParse("[1,2,3]");
	TEST_ASSERT_NOT_NULL(arr);

	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, RyanJsonCreateInt(NULL, 9)));
	RyanJson_t moved = RyanJsonDetachByIndex(arr, 1);
	TEST_ASSERT_NOT_NULL(moved);
	RyanJsonDelete(moved);

	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetArraySize(arr));
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

static void testListStabilityArrayDetachAllThenRebuild(void)
{
	// 复杂链路：
	// Parse(Array) -> DetachByIndex(全部) -> Insert(重建) -> 遍历链表校验。
	// 目标：验证 Array 清空后重建链表稳定。
	RyanJson_t arr = RyanJsonParse("[1,2,3]");
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t a = RyanJsonDetachByIndex(arr, 0);
	RyanJson_t b = RyanJsonDetachByIndex(arr, 0);
	RyanJson_t c = RyanJsonDetachByIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(a);
	TEST_ASSERT_NOT_NULL(b);
	TEST_ASSERT_NOT_NULL(c);
	TEST_ASSERT_EQUAL_UINT32(0U, RyanJsonGetArraySize(arr));

	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, c));
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, b));
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, a));

	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetArraySize(arr));
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

void testStabilityLinkedListChurnRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testListStabilityObjectInsertMiddleThenDeleteHead);
	RUN_TEST(testListStabilityArrayInsertMiddleThenDeleteTail);
	RUN_TEST(testListStabilityNestedObjectDetachChildToOuter);
	RUN_TEST(testListStabilityNestedArrayMoveBetweenInnerArrays);
	RUN_TEST(testListStabilityReplaceByIndexObjectWithDetached);
	RUN_TEST(testListStabilityDeleteByKeyDuplicateThenTraverse);
	RUN_TEST(testListStabilityArrayChurnInsertDetachReplace);
	RUN_TEST(testListStabilityObjectChurnReplaceInsertDetach);
	RUN_TEST(testListStabilityGetNextAfterNestedDetachInsert);
	RUN_TEST(testListStabilityArrayInsertHeadThenDetachMiddle);
	RUN_TEST(testListStabilityArrayDetachAllThenRebuild);
}
