#include "testBase.h"

static void testListStabilityArrayDetachHeadAppend(void)
{
	// 复杂链路：
	// Parse(Array) -> DetachByIndex(head) -> Insert(tail) -> 遍历链表校验。
	// 目标：验证数组节点线索化链表在头删尾插后仍稳定。
	RyanJson_t arr = RyanJsonParse("[1,2,3]");
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t detached = RyanJsonDetachByIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(detached);
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(detached));
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(detached), "游离数组节点 next 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonInternalGetParent(detached), "游离数组节点 parent 应为 NULL");
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 99, detached));

	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetArraySize(arr));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 1)));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 2)));
	TEST_ASSERT_EQUAL_PTR_MESSAGE(arr, RyanJsonInternalGetParent(detached), "回插后的数组节点 parent 应重新绑定");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(detached, RyanJsonGetObjectByIndex(arr, 2), "尾插后的数组末元素应为回插节点");

	uint32_t size = RyanJsonGetArraySize(arr);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(arr);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > size + 1U) { TEST_FAIL_MESSAGE("数组链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(size, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "数组尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(arr);
}

static void testListStabilityObjectReplaceLastThenTraverse(void)
{
	// 复杂链路：
	// Parse(Object) -> ReplaceByKey(尾节点) -> 遍历链表校验。
	// 目标：验证对象尾节点替换后链表依旧稳定。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2,\"c\":3}");
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(obj, "c", RyanJsonCreateInt("c", 9)));
	TEST_ASSERT_EQUAL_INT(9, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "c")));

	uint32_t size = RyanJsonGetSize(obj);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(obj);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > size + 1U) { TEST_FAIL_MESSAGE("对象链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(size, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "对象尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(obj);
}

static void testListStabilityObjectDetachReinsertAtHead(void)
{
	// 复杂链路：
	// Parse(Object) -> DetachByKey(middle) -> Insert(head) -> 遍历链表校验。
	// 目标：验证对象中间节点迁移后链表稳定。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2,\"c\":3}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t moved = RyanJsonDetachByKey(obj, "b");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(moved));
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(moved), "游离对象节点 next 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonInternalGetParent(moved), "游离对象节点 parent 应为 NULL");
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, moved));

	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "a"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "b"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "c"));
	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetSize(obj));
	TEST_ASSERT_EQUAL_PTR_MESSAGE(obj, RyanJsonInternalGetParent(moved), "回插后的对象节点 parent 应重新绑定");

	uint32_t size = RyanJsonGetSize(obj);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(obj);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > size + 1U) { TEST_FAIL_MESSAGE("对象链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(size, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "对象尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(obj);
}

static void testListStabilityArrayReplaceMiddleRebindsChain(void)
{
	// 复杂链路：
	// Parse(Array) -> ReplaceByIndex(middle) -> sibling/parent 校验。
	// 目标：验证 public Replace 会把新节点重新接入正确链路，而不是仅修改值。
	RyanJson_t arr = RyanJsonParse("[1,2,3]");
	TEST_ASSERT_NOT_NULL(arr);

	RyanJson_t replacement = RyanJsonCreateInt(NULL, 9);
	TEST_ASSERT_NOT_NULL(replacement);
	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(arr, 1, replacement));

	RyanJson_t head = RyanJsonGetObjectByIndex(arr, 0);
	RyanJson_t middle = RyanJsonGetObjectByIndex(arr, 1);
	RyanJson_t tail = RyanJsonGetObjectByIndex(arr, 2);
	TEST_ASSERT_NOT_NULL(head);
	TEST_ASSERT_NOT_NULL(middle);
	TEST_ASSERT_NOT_NULL(tail);
	TEST_ASSERT_EQUAL_PTR_MESSAGE(replacement, middle, "Replace 后中间节点应为 replacement");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(middle, RyanJsonGetNext(head), "Replace 后 head.next 应指向新节点");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(tail, RyanJsonGetNext(middle), "Replace 后新节点 next 应连接尾节点");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(tail), "Replace 后尾节点 next 应返回 NULL");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(arr, RyanJsonInternalGetParent(middle), "Replace 后新节点 parent 应为数组");

	RyanJsonDelete(arr);
}

static void testListStabilityDetachAllThenRebuild(void)
{
	// 复杂链路：
	// Parse(Object) -> DetachByKey(全部) -> Insert(重建) -> 遍历链表校验。
	// 目标：验证对象被清空后重建仍保持链表稳定。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2,\"c\":3}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t a = RyanJsonDetachByKey(obj, "a");
	RyanJson_t b = RyanJsonDetachByKey(obj, "b");
	RyanJson_t c = RyanJsonDetachByKey(obj, "c");
	TEST_ASSERT_NOT_NULL(a);
	TEST_ASSERT_NOT_NULL(b);
	TEST_ASSERT_NOT_NULL(c);
	TEST_ASSERT_EQUAL_UINT32(0U, RyanJsonGetSize(obj));

	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(a));
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(b));
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(c));

	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, c));
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, b));
	TEST_ASSERT_TRUE(RyanJsonInsert(obj, 0, a));

	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "a"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "b"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "c"));
	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetSize(obj));

	uint32_t size = RyanJsonGetSize(obj);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(obj);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > size + 1U) { TEST_FAIL_MESSAGE("对象链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(size, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "对象尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(obj);
}

static void testListStabilityArrayMoveBetweenParents(void)
{
	// 复杂链路：
	// Parse(root) -> DetachByIndex(arrA) -> Insert(arrB) -> 遍历链表校验。
	// 目标：验证数组元素跨父节点迁移后链表稳定。
	RyanJson_t root = RyanJsonParse("{\"a\":[1,2],\"b\":[3,4]}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t arrA = RyanJsonGetObjectToKey(root, "a");
	RyanJson_t arrB = RyanJsonGetObjectToKey(root, "b");
	TEST_ASSERT_NOT_NULL(arrA);
	TEST_ASSERT_NOT_NULL(arrB);

	RyanJson_t moved = RyanJsonDetachByIndex(arrA, 1);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonInsert(arrB, 0, moved));

	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetArraySize(arrA));
	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetArraySize(arrB));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arrA, 0)));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arrB, 0)));
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arrB, 1)));
	TEST_ASSERT_EQUAL_INT(4, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arrB, 2)));

	// arrA 链表校验
	{
		uint32_t size = RyanJsonGetArraySize(arrA);
		uint32_t count = 0;
		RyanJson_t node = RyanJsonGetObjectValue(arrA);
		RyanJson_t last = NULL;
		while (node)
		{
			count++;
			if (count > size + 1U) { TEST_FAIL_MESSAGE("arrA 链表疑似形成环"); }
			last = node;
			node = RyanJsonGetNext(node);
		}
		TEST_ASSERT_EQUAL_UINT32(size, count);
		TEST_ASSERT_NOT_NULL(last);
		TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "arrA 尾节点 GetNext 应返回 NULL");
	}

	// arrB 链表校验
	{
		uint32_t size = RyanJsonGetArraySize(arrB);
		uint32_t count = 0;
		RyanJson_t node = RyanJsonGetObjectValue(arrB);
		RyanJson_t last = NULL;
		while (node)
		{
			count++;
			if (count > size + 1U) { TEST_FAIL_MESSAGE("arrB 链表疑似形成环"); }
			last = node;
			node = RyanJsonGetNext(node);
		}
		TEST_ASSERT_EQUAL_UINT32(size, count);
		TEST_ASSERT_NOT_NULL(last);
		TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "arrB 尾节点 GetNext 应返回 NULL");
	}

	RyanJsonDelete(root);
}

static void testListStabilityWrappedContainerChildTraversal(void)
{
	// 复杂链路：
	// Create(child) -> AddItemToObject(包装) -> 子链表遍历校验。
	// 目标：验证包装后的子容器链表稳定。
	RyanJson_t root = RyanJsonCreateObject();
	RyanJson_t child = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(root);
	TEST_ASSERT_NOT_NULL(child);

	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(child, "x", 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(child, "y", 2));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, "wrap", child));

	RyanJson_t wrap = RyanJsonGetObjectToKey(root, "wrap");
	TEST_ASSERT_NOT_NULL(wrap);
	TEST_ASSERT_TRUE(RyanJsonIsObject(wrap));
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetSize(wrap));

	uint32_t size = RyanJsonGetSize(wrap);
	uint32_t count = 0;
	RyanJson_t node = RyanJsonGetObjectValue(wrap);
	RyanJson_t last = NULL;
	while (node)
	{
		count++;
		if (count > size + 1U) { TEST_FAIL_MESSAGE("包装子链表疑似形成环"); }
		last = node;
		node = RyanJsonGetNext(node);
	}
	TEST_ASSERT_EQUAL_UINT32(size, count);
	TEST_ASSERT_NOT_NULL(last);
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(last), "包装子链表尾节点 GetNext 应返回 NULL");

	RyanJsonDelete(root);
}

static void testListStabilityDuplicateAfterChurn(void)
{
	// 复杂链路：
	// Parse -> DetachByIndex -> AddItemToObject(包装) -> ReplaceByKey -> Duplicate -> Compare。
	// 目标：验证变更后链表仍可被 Duplicate 正确复制。
	RyanJson_t root = RyanJsonParse("{\"arr\":[{\"v\":1},{\"v\":2}],\"obj\":{\"a\":1}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t arr = RyanJsonGetObjectToKey(root, "arr");
	RyanJson_t moved = RyanJsonDetachByIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, "moved", moved));
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "obj", RyanJsonCreateString("obj", "flat")));

	RyanJson_t dup = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(dup);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, dup));

	RyanJsonDelete(dup);
	RyanJsonDelete(root);
}

static void testListStabilityPrintAfterChurn(void)
{
	// 复杂链路：
	// Parse -> DetachByKey -> AddItemToObject -> Insert(Array) -> Print/Parse -> Compare。
	// 目标：验证多次结构变更后打印链路稳定。
	RyanJson_t root = RyanJsonParse("{\"a\":[1,2],\"b\":{\"x\":1}}");
	TEST_ASSERT_NOT_NULL(root);

	// 直接迁移容器节点，避免 AddItemToObject 处理标量失败
	RyanJson_t moved = RyanJsonDetachByKey(root, "b");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, "moved", moved));

	RyanJson_t arr = RyanJsonGetObjectToKey(root, "a");
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 1, RyanJsonCreateInt(NULL, 9)));

	char *printed = RyanJsonPrint(root, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

void testStabilityLinkedListBasicRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testListStabilityArrayDetachHeadAppend);
	RUN_TEST(testListStabilityObjectReplaceLastThenTraverse);
	RUN_TEST(testListStabilityObjectDetachReinsertAtHead);
	RUN_TEST(testListStabilityArrayReplaceMiddleRebindsChain);
	RUN_TEST(testListStabilityDetachAllThenRebuild);
	RUN_TEST(testListStabilityArrayMoveBetweenParents);
	RUN_TEST(testListStabilityWrappedContainerChildTraversal);
	RUN_TEST(testListStabilityDuplicateAfterChurn);
	RUN_TEST(testListStabilityPrintAfterChurn);
}
