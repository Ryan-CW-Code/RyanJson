#include "testBase.h"

static void testCompareMutationIgnoreValueChanges(void)
{
	// 复杂链路：
	// Parse -> Duplicate -> ChangeInt/ChangeString -> Compare/CompareOnlyKey。
	// 目标：验证值变更不影响 CompareOnlyKey，但 Compare 必须失败。
	RyanJson_t root = RyanJsonParse("{\"obj\":{\"a\":1,\"b\":\"x\"},\"arr\":[1,\"x\"]}");
	TEST_ASSERT_NOT_NULL(root);
	RyanJson_t copy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(copy);

	TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(copy, "obj", "a"), 9));
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(RyanJsonGetObjectToKey(copy, "obj", "b"), "y"));

	RyanJson_t arr = RyanJsonGetObjectToKey(copy, "arr");
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectByIndex(arr, 0), 2));
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(RyanJsonGetObjectByIndex(arr, 1), "z"));

	TEST_ASSERT_FALSE(RyanJsonCompare(root, copy));
	TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(root, copy));

	RyanJsonDelete(copy);
	RyanJsonDelete(root);
}

static void testCompareMutationFailOnTypeChange(void)
{
	// 复杂链路：
	// Parse -> Duplicate -> ReplaceByKey(类型变化) -> CompareOnlyKey。
	// 目标：验证类型变化会导致 CompareOnlyKey 失败。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"b\":true}");
	TEST_ASSERT_NOT_NULL(root);
	RyanJson_t copy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(copy);

	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(copy, "a", RyanJsonCreateString("a", "1")));
	TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(root, copy));

	RyanJsonDelete(copy);
	RyanJsonDelete(root);
}

static void testCompareMutationFailOnSizeChange(void)
{
	// 复杂链路：
	// Parse -> Duplicate -> DetachByIndex(Array) -> CompareOnlyKey。
	// 目标：验证容器尺寸变化会导致 CompareOnlyKey 失败。
	RyanJson_t root = RyanJsonParse("{\"arr\":[1,2,3]}");
	TEST_ASSERT_NOT_NULL(root);
	RyanJson_t copy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(copy);

	RyanJson_t arr = RyanJsonGetObjectToKey(copy, "arr");
	TEST_ASSERT_NOT_NULL(arr);
	RyanJson_t detached = RyanJsonDetachByIndex(arr, 1);
	TEST_ASSERT_NOT_NULL(detached);
	RyanJsonDelete(detached);

	TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(root, copy));

	RyanJsonDelete(copy);
	RyanJsonDelete(root);
}

static void testCompareMutationDuplicateCountMismatch(void)
{
	// 复杂链路：
	// Parse(重复 key) -> Duplicate -> DeleteByKey -> CompareOnlyKey。
	// 目标：验证重复 key 数量变化会导致 CompareOnlyKey 失败。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"a\":2,\"b\":3}");
	if (NULL == root)
	{
		// strict 控制组：无重复 key 时也应识别结构差异。
		RyanJson_t strictRoot = RyanJsonParse("{\"a\":1,\"b\":3}");
		TEST_ASSERT_NOT_NULL(strictRoot);
		RyanJson_t strictCopy = RyanJsonDuplicate(strictRoot);
		TEST_ASSERT_NOT_NULL(strictCopy);
		TEST_ASSERT_TRUE(RyanJsonDeleteByKey(strictCopy, "a"));
		TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(strictRoot, strictCopy));
		RyanJsonDelete(strictCopy);
		RyanJsonDelete(strictRoot);
		return;
	}

	RyanJson_t copy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(copy);
	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(copy, "a"));
	TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(root, copy));

	RyanJsonDelete(copy);
	RyanJsonDelete(root);
}

static void testCompareMutationFailOnKeyRename(void)
{
	// 复杂链路：
	// Parse -> Duplicate -> ChangeKey -> CompareOnlyKey。
	// 目标：验证 key 变化会导致 CompareOnlyKey 失败。
	RyanJson_t root = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(root);
	RyanJson_t copy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(copy);

	TEST_ASSERT_TRUE(RyanJsonChangeKey(RyanJsonGetObjectByKey(copy, "a"), "b"));
	TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(root, copy));

	RyanJsonDelete(copy);
	RyanJsonDelete(root);
}

static void testCompareMutationDuplicateReorderWithValueChange(void)
{
	// 复杂链路：
	// Parse(重复 key) -> Duplicate -> DetachByKey -> ChangeIntValue -> Insert(尾部) -> Compare/CompareOnlyKey。
	// 目标：验证重复 key 重新排列且值变化时，CompareOnlyKey 仍为 True。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"a\":2,\"b\":3}");
	if (NULL == root)
	{
		// strict 控制组：无重复 key，值变化时 CompareOnlyKey 仍应为 True。
		RyanJson_t strictRoot = RyanJsonParse("{\"a\":1,\"b\":3}");
		TEST_ASSERT_NOT_NULL(strictRoot);
		RyanJson_t strictCopy = RyanJsonDuplicate(strictRoot);
		TEST_ASSERT_NOT_NULL(strictCopy);

		TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectByKey(strictCopy, "a"), 9));
		TEST_ASSERT_FALSE(RyanJsonCompare(strictRoot, strictCopy));
		TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(strictRoot, strictCopy));

		RyanJsonDelete(strictCopy);
		RyanJsonDelete(strictRoot);
		return;
	}

	RyanJson_t copy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(copy);

	RyanJson_t moved = RyanJsonDetachByKey(copy, "a");
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonChangeIntValue(moved, 9));
	TEST_ASSERT_TRUE(RyanJsonInsert(copy, 99, moved));

	TEST_ASSERT_FALSE(RyanJsonCompare(root, copy));
	TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(root, copy));

	RyanJsonDelete(copy);
	RyanJsonDelete(root);
}

static void testCompareMutationDuplicateCountMismatchAfterInsert(void)
{
	// 复杂链路：
	// Parse(重复 key) -> Duplicate -> Insert(增加重复 key) -> CompareOnlyKey。
	// 目标：验证重复 key 数量变化会导致 CompareOnlyKey 失败。
	RyanJson_t root = RyanJsonParse("{\"a\":1,\"a\":2,\"b\":3}");
	if (NULL == root)
	{
		// strict 控制组：新增 key 导致结构变化。
		RyanJson_t strictRoot = RyanJsonParse("{\"a\":1,\"b\":3}");
		TEST_ASSERT_NOT_NULL(strictRoot);
		RyanJson_t strictCopy = RyanJsonDuplicate(strictRoot);
		TEST_ASSERT_NOT_NULL(strictCopy);
		TEST_ASSERT_TRUE(RyanJsonInsert(strictCopy, 99, RyanJsonCreateInt("c", 5)));
		TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(strictRoot, strictCopy));
		RyanJsonDelete(strictCopy);
		RyanJsonDelete(strictRoot);
		return;
	}

	RyanJson_t copy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(copy);
	TEST_ASSERT_TRUE(RyanJsonInsert(copy, 0, RyanJsonCreateInt("a", 9)));
	TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(root, copy));

	RyanJsonDelete(copy);
	RyanJsonDelete(root);
}

static void testCompareMutationAfterAddItemWrap(void)
{
	// 复杂链路：
	// Parse -> Duplicate -> AddItemToObject(Array) -> Compare/CompareOnlyKey。
	// 目标：验证包装新增结构后，CompareOnlyKey 只关注结构与 key。
	// 说明：期望文档刻意使用不同值，以确保 Compare=false、CompareOnlyKey=true。
	RyanJson_t root = RyanJsonParse("{\"a\":{}}");
	TEST_ASSERT_NOT_NULL(root);
	RyanJson_t copy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(copy);

	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, RyanJsonCreateInt(NULL, 1)));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(copy, "b", arr));

	RyanJson_t expect = RyanJsonParse("{\"a\":{},\"b\":[0]}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_FALSE(RyanJsonCompare(copy, expect));
	TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(copy, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(copy);
	RyanJsonDelete(root);
}

static void testCompareMutationWrappedArrayValueChurn(void)
{
	// 复杂链路：
	// Parse -> AddItemToObject(Array) -> Duplicate -> ChangeIntValue -> CompareOnlyKey。
	// 目标：验证包装 Array 值变化不影响 CompareOnlyKey。
	// 说明：期望文档刻意使用不同值，以确保 Compare=false、CompareOnlyKey=true。
	RyanJson_t root = RyanJsonParse("{\"a\":{}}");
	TEST_ASSERT_NOT_NULL(root);
	RyanJson_t work = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(work);

	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 0, RyanJsonCreateInt(NULL, 1)));
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 1, RyanJsonCreateInt(NULL, 2)));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(work, "b", arr));

	RyanJson_t arrNode = RyanJsonGetObjectByKey(work, "b");
	TEST_ASSERT_NOT_NULL(arrNode);
	TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectByIndex(arrNode, 0), 7));

	RyanJson_t expect = RyanJsonParse("{\"a\":{},\"b\":[0,0]}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_FALSE(RyanJsonCompare(work, expect));
	TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(work, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(work);
	RyanJsonDelete(root);
}

static void testCompareMutationReplaceScalarKeepsOnlyKey(void)
{
	// 复杂链路：
	// Parse -> Duplicate -> ReplaceByKey(标量) -> CompareOnlyKey。
	// 目标：验证标量替换仅改变值，不影响 CompareOnlyKey。
	RyanJson_t root = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(root);
	RyanJson_t copy = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(copy);

	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(copy, "a", RyanJsonCreateInt("a", 9)));
	TEST_ASSERT_FALSE(RyanJsonCompare(root, copy));
	TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(root, copy));

	RyanJsonDelete(copy);
	RyanJsonDelete(root);
}

static void testCompareMutationNestedArraySizeMismatch(void)
{
	// 复杂链路：
	// Parse -> CompareOnlyKey(失败) -> DeleteByIndex(修复长度) -> CompareOnlyKey(成功)。
	// 目标：验证 CompareOnlyKey 会检查嵌套 Array 长度，不因值变化而误判。
	RyanJson_t left = RyanJsonParse("{\"arr\":[[1,2],{\"k\":1}]}");
	RyanJson_t right = RyanJsonParse("{\"arr\":[[1,2,3],{\"k\":9}]}");
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(right);

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, right), "嵌套 Array 长度不一致时 CompareOnlyKey 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, right), "嵌套 Array 长度不一致时 Compare 应返回 False");

	RyanJson_t inner = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(right, "arr"), 0);
	TEST_ASSERT_NOT_NULL(inner);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonDeleteByIndex(inner, 0), "修复嵌套 Array 长度失败");
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(inner));

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, right), "修复长度后 CompareOnlyKey 应返回 True");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, right), "值不同但结构一致时 Compare 应返回 False");

	RyanJsonDelete(right);
	RyanJsonDelete(left);
}

void testCompareMutationRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testCompareMutationIgnoreValueChanges);
	RUN_TEST(testCompareMutationFailOnTypeChange);
	RUN_TEST(testCompareMutationFailOnSizeChange);
	RUN_TEST(testCompareMutationNestedArraySizeMismatch);
	RUN_TEST(testCompareMutationDuplicateCountMismatch);
	RUN_TEST(testCompareMutationFailOnKeyRename);
	RUN_TEST(testCompareMutationDuplicateReorderWithValueChange);
	RUN_TEST(testCompareMutationDuplicateCountMismatchAfterInsert);
	RUN_TEST(testCompareMutationAfterAddItemWrap);
	RUN_TEST(testCompareMutationWrappedArrayValueChurn);
	RUN_TEST(testCompareMutationReplaceScalarKeepsOnlyKey);
}
