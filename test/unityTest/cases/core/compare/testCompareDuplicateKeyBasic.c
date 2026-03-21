#include "testBase.h"

static void testCompareDuplicateKeyPolicyBasic(void)
{
	// 该用例在 strict/non-strict 两种编译模式都执行：
	// - strict 模式：重复 key 应在解析阶段被拒绝；
	// - non-strict 模式：重复 key 允许存在，Compare/CompareOnlyKey 需按“同 key 出现序号”一一对齐。
	RyanJson_t dupLeft = RyanJsonParse("{\"a\":1,\"a\":1}");
	RyanJson_t diffKeyRight = RyanJsonParse("{\"a\":1,\"b\":1}");

	TEST_ASSERT_NOT_NULL(diffKeyRight);

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL_MESSAGE(dupLeft, "strict 模式下重复 key 应解析失败");

	// 控制组：普通 Object 乱序比较在 strict 模式下仍应正常。
	{
		RyanJson_t ordered = RyanJsonParse("{\"a\":1,\"b\":1}");
		RyanJson_t unordered = RyanJsonParse("{\"b\":1,\"a\":1}");
		TEST_ASSERT_NOT_NULL(ordered);
		TEST_ASSERT_NOT_NULL(unordered);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(ordered, unordered), "strict 模式下普通 Object 乱序 Compare 应返回 True");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(ordered, unordered),
					 "strict 模式下普通 Object 乱序 CompareOnlyKey 应返回 True");
		RyanJsonDelete(ordered);
		RyanJsonDelete(unordered);
	}
#else
	// non-strict 模式允许重复 key，本分支验证 Compare/CompareOnlyKey 不会把右侧同一节点重复匹配。
	RyanJson_t sameLeft = RyanJsonParse("{\"a\":1,\"a\":2}");
	RyanJson_t sameRight = RyanJsonParse("{\"a\":1,\"a\":2}");

	// 第二次出现位置的类型不一致（String vs Number）时，CompareOnlyKey 也必须失败。
	RyanJson_t typeLeft = RyanJsonParse("{\"a\":1,\"a\":\"2\"}");
	RyanJson_t typeRight = RyanJsonParse("{\"a\":1,\"a\":2}");

	TEST_ASSERT_NOT_NULL(dupLeft);
	TEST_ASSERT_NOT_NULL(sameLeft);
	TEST_ASSERT_NOT_NULL(sameRight);
	TEST_ASSERT_NOT_NULL(typeLeft);
	TEST_ASSERT_NOT_NULL(typeRight);

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(dupLeft, diffKeyRight), "重复 key 与不同 key 混合时 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(dupLeft, diffKeyRight), "重复 key 与不同 key 混合时 CompareOnlyKey 应返回 False");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(sameLeft, sameRight), "重复 key 的同序号节点完全一致时 Compare 应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(sameLeft, sameRight), "重复 key 的同序号节点完全一致时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(typeLeft, typeRight), "重复 key 的同序号节点类型不同，Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(typeLeft, typeRight),
				  "重复 key 的同序号节点类型不同，CompareOnlyKey 应返回 False");

	RyanJsonDelete(sameLeft);
	RyanJsonDelete(sameRight);
	RyanJsonDelete(typeLeft);
	RyanJsonDelete(typeRight);
#endif

	RyanJsonDelete(dupLeft);
	RyanJsonDelete(diffKeyRight);
}

static void testCompareDuplicateKeyCountAndOrder(void)
{
	RyanJson_t left = RyanJsonParse("{\"a\":1,\"a\":2,\"a\":3}");
	RyanJson_t rightCountMismatch = RyanJsonParse("{\"a\":1,\"a\":2,\"b\":3}");
	RyanJson_t rightOrderSwapped = RyanJsonParse("{\"a\":3,\"a\":2,\"a\":1}");
	RyanJson_t rightSame = RyanJsonParse("{\"a\":1,\"a\":2,\"a\":3}");
	RyanJson_t rightTypeChanged = RyanJsonParse("{\"a\":1,\"a\":\"2\",\"a\":3}");

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL_MESSAGE(left, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightCountMismatch, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightOrderSwapped, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightSame, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightTypeChanged, "strict 模式下重复 key 应解析失败");

	// 控制组：无重复 key 时，key 不一致应判 False。
	{
		RyanJson_t normalLeft = RyanJsonParse("{\"a\":1,\"b\":2,\"c\":3}");
		RyanJson_t normalRight = RyanJsonParse("{\"a\":1,\"b\":2,\"x\":3}");
		TEST_ASSERT_NOT_NULL(normalLeft);
		TEST_ASSERT_NOT_NULL(normalRight);
		TEST_ASSERT_FALSE(RyanJsonCompare(normalLeft, normalRight));
		TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(normalLeft, normalRight));
		RyanJsonDelete(normalLeft);
		RyanJsonDelete(normalRight);
	}
#else
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(rightCountMismatch);
	TEST_ASSERT_NOT_NULL(rightOrderSwapped);
	TEST_ASSERT_NOT_NULL(rightSame);
	TEST_ASSERT_NOT_NULL(rightTypeChanged);

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightCountMismatch), "重复 key 个数不一致时 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightCountMismatch), "重复 key 个数不一致时 CompareOnlyKey 应返回 False");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightOrderSwapped), "重复 key 同序号值不同，Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightOrderSwapped), "重复 key 同序号类型一致，CompareOnlyKey 应返回 True");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(left, rightSame), "重复 key 完全一致时 Compare 应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightSame), "重复 key 完全一致时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightTypeChanged), "重复 key 同序号类型变化，Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightTypeChanged), "重复 key 同序号类型变化，CompareOnlyKey 应返回 False");
#endif

	RyanJsonDelete(left);
	RyanJsonDelete(rightCountMismatch);
	RyanJsonDelete(rightOrderSwapped);
	RyanJsonDelete(rightSame);
	RyanJsonDelete(rightTypeChanged);
}

static void testCompareDuplicateKeyNestedObject(void)
{
	RyanJson_t left = RyanJsonParse("{\"outer\":{\"k\":1,\"k\":2},\"stable\":1}");
	RyanJson_t rightSame = RyanJsonParse("{\"stable\":1,\"outer\":{\"k\":1,\"k\":2}}");
	RyanJson_t rightKeyMismatch = RyanJsonParse("{\"stable\":1,\"outer\":{\"k\":1,\"x\":2}}");
	RyanJson_t rightTypeMismatch = RyanJsonParse("{\"stable\":1,\"outer\":{\"k\":1,\"k\":\"2\"}}");
	RyanJson_t rightValueMismatch = RyanJsonParse("{\"stable\":1,\"outer\":{\"k\":1,\"k\":9}}");

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL_MESSAGE(left, "strict 模式下嵌套重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightSame, "strict 模式下嵌套重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightTypeMismatch, "strict 模式下嵌套重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightValueMismatch, "strict 模式下嵌套重复 key 应解析失败");

	TEST_ASSERT_NOT_NULL(rightKeyMismatch);
	// 控制组：无重复 key 的嵌套 Object 可以正常比较。
	{
		RyanJson_t normalLeft = RyanJsonParse("{\"outer\":{\"k\":1,\"m\":2},\"stable\":1}");
		TEST_ASSERT_NOT_NULL(normalLeft);
		TEST_ASSERT_FALSE(RyanJsonCompare(normalLeft, rightKeyMismatch));
		TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(normalLeft, rightKeyMismatch));
		RyanJsonDelete(normalLeft);
	}
#else
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(rightSame);
	TEST_ASSERT_NOT_NULL(rightKeyMismatch);
	TEST_ASSERT_NOT_NULL(rightTypeMismatch);
	TEST_ASSERT_NOT_NULL(rightValueMismatch);

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(left, rightSame), "嵌套重复 key 完全一致时 Compare 应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightSame), "嵌套重复 key 完全一致时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightKeyMismatch), "嵌套重复 key 与普通 key 混合时 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightKeyMismatch),
				  "嵌套重复 key 与普通 key 混合时 CompareOnlyKey 应返回 False");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightTypeMismatch), "嵌套重复 key 同序号类型变化，Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightTypeMismatch),
				  "嵌套重复 key 同序号类型变化，CompareOnlyKey 应返回 False");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightValueMismatch), "嵌套重复 key 同序号值变化，Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightValueMismatch),
				 "嵌套重复 key 同序号类型一致时 CompareOnlyKey 应返回 True");
#endif

	RyanJsonDelete(left);
	RyanJsonDelete(rightSame);
	RyanJsonDelete(rightKeyMismatch);
	RyanJsonDelete(rightTypeMismatch);
	RyanJsonDelete(rightValueMismatch);
}

static void testCompareDuplicateKeyArrayObject(void)
{
	RyanJson_t left = RyanJsonParse("[{\"a\":1,\"a\":2},{\"b\":1}]");
	RyanJson_t rightSame = RyanJsonParse("[{\"a\":1,\"a\":2},{\"b\":1}]");
	RyanJson_t rightArraySwap = RyanJsonParse("[{\"b\":1},{\"a\":1,\"a\":2}]");
	RyanJson_t rightInnerMismatch = RyanJsonParse("[{\"a\":1,\"b\":2},{\"b\":1}]");
	RyanJson_t rightInnerValueDiff = RyanJsonParse("[{\"a\":9,\"a\":8},{\"b\":1}]");

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL_MESSAGE(left, "strict 模式下 Array 元素中的重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightSame, "strict 模式下 Array 元素中的重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightArraySwap, "strict 模式下 Array 元素中的重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightInnerValueDiff, "strict 模式下 Array 元素中的重复 key 应解析失败");

	TEST_ASSERT_NOT_NULL(rightInnerMismatch);
	// 控制组：无重复 key 的 Array 中 Object 语义正常。
	{
		RyanJson_t normalLeft = RyanJsonParse("[{\"a\":1,\"b\":2},{\"b\":1}]");
		TEST_ASSERT_NOT_NULL(normalLeft);
		TEST_ASSERT_TRUE(RyanJsonCompare(normalLeft, rightInnerMismatch));
		TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(normalLeft, rightInnerMismatch));
		RyanJsonDelete(normalLeft);
	}
#else
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(rightSame);
	TEST_ASSERT_NOT_NULL(rightArraySwap);
	TEST_ASSERT_NOT_NULL(rightInnerMismatch);
	TEST_ASSERT_NOT_NULL(rightInnerValueDiff);

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(left, rightSame), "Array 内重复 key Object 完全一致时 Compare 应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightSame), "Array 内重复 key Object 完全一致时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightArraySwap), "Array 顺序改变后 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightArraySwap), "Array 顺序改变后 CompareOnlyKey 应返回 False");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightInnerMismatch), "Array 内重复 key 与普通 key 混合时 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightInnerMismatch),
				  "Array 内重复 key 与普通 key 混合时 CompareOnlyKey 应返回 False");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightInnerValueDiff), "Array 内重复 key 同序号值变化，Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightInnerValueDiff),
				 "Array 内重复 key 同序号类型一致时 CompareOnlyKey 应返回 True");
#endif

	RyanJsonDelete(left);
	RyanJsonDelete(rightSame);
	RyanJsonDelete(rightArraySwap);
	RyanJsonDelete(rightInnerMismatch);
	RyanJsonDelete(rightInnerValueDiff);
}

static void testCompareDuplicateKeyDeepOccurrenceAlign(void)
{
	RyanJson_t left = RyanJsonParse("{\"a\":{\"v\":1},\"a\":{\"v\":2}}");
	RyanJson_t rightSame = RyanJsonParse("{\"a\":{\"v\":1},\"a\":{\"v\":2}}");
	RyanJson_t rightSwap = RyanJsonParse("{\"a\":{\"v\":2},\"a\":{\"v\":1}}");
	RyanJson_t rightTypeChanged = RyanJsonParse("{\"a\":{\"v\":1},\"a\":{\"v\":\"2\"}}");

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL_MESSAGE(left, "strict 模式下深层重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightSame, "strict 模式下深层重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightSwap, "strict 模式下深层重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightTypeChanged, "strict 模式下深层重复 key 应解析失败");

	// 控制组：无重复 key 的深层 Object 比较应正常。
	{
		RyanJson_t normalLeft = RyanJsonParse("{\"a1\":{\"v\":1},\"a2\":{\"v\":2}}");
		RyanJson_t normalRight = RyanJsonParse("{\"a2\":{\"v\":2},\"a1\":{\"v\":1}}");
		TEST_ASSERT_NOT_NULL(normalLeft);
		TEST_ASSERT_NOT_NULL(normalRight);
		TEST_ASSERT_TRUE(RyanJsonCompare(normalLeft, normalRight));
		TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(normalLeft, normalRight));
		RyanJsonDelete(normalLeft);
		RyanJsonDelete(normalRight);
	}
#else
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(rightSame);
	TEST_ASSERT_NOT_NULL(rightSwap);
	TEST_ASSERT_NOT_NULL(rightTypeChanged);

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(left, rightSame), "深层重复 key 同序号完全一致时 Compare 应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightSame), "深层重复 key 同序号完全一致时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightSwap), "深层重复 key 同序号值错位时 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightSwap), "深层重复 key 同序号类型一致时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightTypeChanged), "深层重复 key 同序号类型变化时 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightTypeChanged),
				  "深层重复 key 同序号类型变化时 CompareOnlyKey 应返回 False");
#endif

	RyanJsonDelete(left);
	RyanJsonDelete(rightSame);
	RyanJsonDelete(rightSwap);
	RyanJsonDelete(rightTypeChanged);
}

void testCompareDuplicateKeyRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testCompareDuplicateKeyPolicyBasic);
	RUN_TEST(testCompareDuplicateKeyCountAndOrder);
	RUN_TEST(testCompareDuplicateKeyNestedObject);
	RUN_TEST(testCompareDuplicateKeyArrayObject);
	RUN_TEST(testCompareDuplicateKeyDeepOccurrenceAlign);
}
