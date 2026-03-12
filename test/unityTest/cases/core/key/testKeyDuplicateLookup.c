#include "testBase.h"

static void testKeyDuplicateCreatedByChangeKeyLookupDetach(void)
{
	// 覆盖 ChangeKey 在 non-strict 下制造重复 key 后的查询/分离语义。
#if true == RyanJsonStrictObjectKeyCheck
	// strict 模式下不允许制造重复 key，本用例只做控制组退出。
	RyanJson_t control = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL(control);
	RyanJsonDelete(control);
	return;
#else
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"b\":2,\"c\":3}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t bNode = RyanJsonGetObjectByKey(obj, "b");
	TEST_ASSERT_NOT_NULL(bNode);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(bNode, "a"));
	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetSize(obj));

	RyanJson_t first = RyanJsonGetObjectByKey(obj, "a");
	TEST_ASSERT_NOT_NULL(first);
	int32_t firstVal = RyanJsonGetIntValue(first);
	TEST_ASSERT_TRUE_MESSAGE(firstVal == 1 || firstVal == 2, "重复 key 首次命中值不在预期集合");

	RyanJson_t removed = RyanJsonDetachByKey(obj, "a");
	TEST_ASSERT_NOT_NULL(removed);
	TEST_ASSERT_EQUAL_INT_MESSAGE(firstVal, RyanJsonGetIntValue(removed), "GetObjectByKey 与 DetachByKey 应命中同一节点");

	RyanJson_t next = RyanJsonGetObjectByKey(obj, "a");
	TEST_ASSERT_NOT_NULL(next);
	int32_t nextVal = RyanJsonGetIntValue(next);
	TEST_ASSERT_TRUE_MESSAGE((nextVal == 1 && firstVal == 2) || (nextVal == 2 && firstVal == 1),
				 "分离首个重复 key 后，应命中另一个不同值节点");

	RyanJsonDelete(removed);
	RyanJsonDelete(obj);
#endif
}

static void testKeyDuplicateEscapedVsUtf8LookupDetach(void)
{
	// 覆盖转义 key 与 UTF-8 直写 key 的重复语义：
	// strict 模式下应拒绝；非 strict 下验证 GetObjectByKey/DetachByKey 命中一致。
	const char *utf8Key = "\xE4\xB8\xAD";
#if true == RyanJsonStrictObjectKeyCheck
	RyanJson_t obj = RyanJsonParse("{\"\\u4E2D\":1,\"\xE4\xB8\xAD\":2}");
	TEST_ASSERT_NULL_MESSAGE(obj, "strict 模式下转义/UTF-8 等价 key 重复应解析失败");
#else
	RyanJson_t obj = RyanJsonParse("{\"\\u4E2D\":1,\"\xE4\xB8\xAD\":2}");
	TEST_ASSERT_NOT_NULL_MESSAGE(obj, "non-strict 模式下转义/UTF-8 等价 key 重复应解析成功");

	RyanJson_t first = RyanJsonGetObjectByKey(obj, utf8Key);
	TEST_ASSERT_NOT_NULL(first);
	int32_t firstVal = RyanJsonGetIntValue(first);
	TEST_ASSERT_TRUE_MESSAGE(firstVal == 1 || firstVal == 2, "重复 key 首次命中值不在预期集合");

	RyanJson_t removed = RyanJsonDetachByKey(obj, utf8Key);
	TEST_ASSERT_NOT_NULL(removed);
	TEST_ASSERT_EQUAL_INT_MESSAGE(firstVal, RyanJsonGetIntValue(removed), "GetObjectByKey 与 DetachByKey 应命中同一节点");

	RyanJson_t next = RyanJsonGetObjectByKey(obj, utf8Key);
	TEST_ASSERT_NOT_NULL(next);
	int32_t nextVal = RyanJsonGetIntValue(next);
	TEST_ASSERT_TRUE_MESSAGE((nextVal == 1 && firstVal == 2) || (nextVal == 2 && firstVal == 1),
				 "分离首个重复 key 后，应命中另一个不同值节点");
	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetSize(obj));

	RyanJsonDelete(removed);
	RyanJsonDelete(obj);
#endif
}

static void testKeyDuplicateGetObjectByKeyAndDetachNext(void)
{
	// 覆盖重复 key 下 GetObjectByKey 与 DetachByKey 的协同行为。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"a\":2}");
	if (NULL == obj)
	{
		// strict 模式下重复 key 会解析失败，走控制组确保用例仍可执行。
		RyanJson_t control = RyanJsonParse("{\"a\":1,\"b\":2}");
		TEST_ASSERT_NOT_NULL(control);
		TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByKey(control, "a")));
		RyanJsonDelete(control);
		return;
	}

	RyanJson_t first = RyanJsonGetObjectByKey(obj, "a");
	TEST_ASSERT_NOT_NULL(first);
	int32_t firstVal = RyanJsonGetIntValue(first);
	TEST_ASSERT_TRUE_MESSAGE(firstVal == 1 || firstVal == 2, "重复 key 首次命中值不在预期集合");

	RyanJson_t removed = RyanJsonDetachByKey(obj, "a");
	TEST_ASSERT_NOT_NULL(removed);
	int32_t removedVal = RyanJsonGetIntValue(removed);
	TEST_ASSERT_EQUAL_INT_MESSAGE(firstVal, removedVal, "GetObjectByKey 与 DetachByKey 应命中同一节点");

	RyanJson_t next = RyanJsonGetObjectByKey(obj, "a");
	TEST_ASSERT_NOT_NULL(next);
	int32_t nextVal = RyanJsonGetIntValue(next);
	TEST_ASSERT_TRUE_MESSAGE((nextVal == 1 && firstVal == 2) || (nextVal == 2 && firstVal == 1),
				 "分离首个重复 key 后，应命中另一个不同值节点");
	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetSize(obj));

	RyanJsonDelete(removed);
	RyanJsonDelete(obj);
}

static void testKeyDuplicateDeleteByKeyTwiceRemovesAll(void)
{
	// 覆盖重复 key 下连续 DeleteByKey 的清空路径。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"a\":2}");
	if (NULL == obj)
	{
		// strict 控制组：无重复 key 时第二次删除应返回 False。
		RyanJson_t control = RyanJsonParse("{\"a\":1}");
		TEST_ASSERT_NOT_NULL(control);
		TEST_ASSERT_TRUE(RyanJsonDeleteByKey(control, "a"));
		TEST_ASSERT_FALSE(RyanJsonDeleteByKey(control, "a"));
		RyanJsonDelete(control);
		return;
	}

	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "a"));
	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "a"));
	TEST_ASSERT_EQUAL_UINT32(0U, RyanJsonGetSize(obj));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(obj, "a"));

	RyanJsonDelete(obj);
}

static void testKeyDuplicateDeleteByKeyReducesTraversalCount(void)
{
	// 复杂链路：
	// Parse(重复 key) -> 统计 key 次数 -> DeleteByKey -> 再统计 -> Compare。
	// 目标：验证 DeleteByKey 在非严格模式下只删除一个重复 key，
	//       且删除后对象仍可继续稳定使用。
	RyanJson_t obj = RyanJsonParse("{\"a\":1,\"a\":2,\"b\":3}");
	if (NULL == obj)
	{
		// strict 模式降级：验证单 key 删除语义
		obj = RyanJsonParse("{\"a\":1,\"b\":3}");
		TEST_ASSERT_NOT_NULL(obj);
		TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "a"));
		TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetSize(obj));
		TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(obj, "a"));
		RyanJsonDelete(obj);
		return;
	}

	uint32_t aCount = 0;
	RyanJson_t item = RyanJsonGetObjectValue(obj);
	while (item)
	{
		if (RyanJsonIsKey(item) && 0 == strcmp(RyanJsonGetKey(item), "a")) { aCount++; }
		item = RyanJsonGetNext(item);
	}
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, aCount, "重复 key 计数应为 2");

	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(obj, "a"));
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, RyanJsonGetSize(obj), "DeleteByKey 后对象 size 应减少 1");

	aCount = 0;
	item = RyanJsonGetObjectValue(obj);
	while (item)
	{
		if (RyanJsonIsKey(item) && 0 == strcmp(RyanJsonGetKey(item), "a")) { aCount++; }
		item = RyanJsonGetNext(item);
	}
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, aCount, "DeleteByKey 后重复 key 计数应减少 1");
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(obj, "a"));
	int32_t remainVal = RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "a"));
	TEST_ASSERT_TRUE_MESSAGE(remainVal == 1 || remainVal == 2, "DeleteByKey 后剩余值不在预期集合");

	RyanJson_t expect = RyanJsonParse("{\"a\":9,\"b\":3}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(obj, expect), "DeleteByKey 后值不应被固定假设");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(obj, expect), "DeleteByKey 后结构应与期望对象一致");

	RyanJsonDelete(expect);
	RyanJsonDelete(obj);
}

void testKeyDuplicateLookupRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testKeyDuplicateCreatedByChangeKeyLookupDetach);
	RUN_TEST(testKeyDuplicateEscapedVsUtf8LookupDetach);
	RUN_TEST(testKeyDuplicateGetObjectByKeyAndDetachNext);
	RUN_TEST(testKeyDuplicateDeleteByKeyReducesTraversalCount);
	RUN_TEST(testKeyDuplicateDeleteByKeyTwiceRemovesAll);
}
