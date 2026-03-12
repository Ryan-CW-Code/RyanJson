#include "testBase.h"

static void testCompareDuplicateKeyInterleavedWithUnique(void)
{
	// 该用例覆盖“重复 key 与唯一 key 交错”场景：
	// 1) 非严格模式：验证重复 key 的出现序号匹配不会被交错 key 干扰；
	// 2) 严格模式：重复 key 解析应失败，并使用无重复 key 控制组验证普通语义。
	RyanJson_t left = RyanJsonParse("{\"a\":1,\"b\":2,\"a\":3,\"c\":4}");
	RyanJson_t rightSame = RyanJsonParse("{\"c\":4,\"a\":1,\"b\":2,\"a\":3}");
	RyanJson_t rightDupSwapped = RyanJsonParse("{\"c\":4,\"a\":3,\"b\":2,\"a\":1}");
	RyanJson_t rightMissingUnique = RyanJsonParse("{\"c\":4,\"a\":1,\"a\":3,\"d\":2}");
	RyanJson_t rightTypeMismatch = RyanJsonParse("{\"c\":4,\"a\":1,\"b\":2,\"a\":\"3\"}");

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL_MESSAGE(left, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightSame, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightDupSwapped, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightMissingUnique, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightTypeMismatch, "strict 模式下重复 key 应解析失败");

	// 控制组：无重复 key 时，对象乱序应相等，key 不同应不等。
	{
		RyanJson_t normalLeft = RyanJsonParse("{\"a\":1,\"b\":2,\"c\":3,\"d\":4}");
		RyanJson_t normalRight = RyanJsonParse("{\"d\":4,\"c\":3,\"b\":2,\"a\":1}");
		RyanJson_t normalMismatch = RyanJsonParse("{\"d\":4,\"x\":3,\"b\":2,\"a\":1}");
		TEST_ASSERT_NOT_NULL(normalLeft);
		TEST_ASSERT_NOT_NULL(normalRight);
		TEST_ASSERT_NOT_NULL(normalMismatch);

		TEST_ASSERT_TRUE(RyanJsonCompare(normalLeft, normalRight));
		TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(normalLeft, normalRight));
		TEST_ASSERT_FALSE(RyanJsonCompare(normalLeft, normalMismatch));
		TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(normalLeft, normalMismatch));

		RyanJsonDelete(normalLeft);
		RyanJsonDelete(normalRight);
		RyanJsonDelete(normalMismatch);
	}
#else
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(rightSame);
	TEST_ASSERT_NOT_NULL(rightDupSwapped);
	TEST_ASSERT_NOT_NULL(rightMissingUnique);
	TEST_ASSERT_NOT_NULL(rightTypeMismatch);

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(left, rightSame), "交错场景下重复 key 同序号一致时 Compare 应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightSame), "交错场景下重复 key 同序号一致时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightDupSwapped), "交错场景下重复 key 同序号值错位时 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightDupSwapped),
				 "交错场景下重复 key 同序号类型一致时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightMissingUnique), "交错场景下唯一 key 不一致时 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightMissingUnique),
				  "交错场景下唯一 key 不一致时 CompareOnlyKey 应返回 False");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightTypeMismatch), "交错场景下重复 key 同序号类型变化时 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightTypeMismatch),
				  "交错场景下重复 key 同序号类型变化时 CompareOnlyKey 应返回 False");
#endif

	RyanJsonDelete(left);
	RyanJsonDelete(rightSame);
	RyanJsonDelete(rightDupSwapped);
	RyanJsonDelete(rightMissingUnique);
	RyanJsonDelete(rightTypeMismatch);
}

static void testCompareDuplicateKeyMutationCountMismatchAndRecover(void)
{
	// 该用例覆盖“重复 key 在变更后计数不一致”的检测与恢复路径。
	RyanJson_t left = RyanJsonParse("{\"a\":1,\"a\":2,\"b\":3}");
	RyanJson_t right = RyanJsonParse("{\"a\":1,\"a\":2,\"b\":3}");

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL_MESSAGE(left, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(right, "strict 模式下重复 key 应解析失败");

	// 控制组：无重复 key 的变更链路应保持 CompareOnlyKey 语义正确。
	{
		RyanJson_t normalLeft = RyanJsonParse("{\"a\":1,\"b\":2}");
		RyanJson_t normalRight = RyanJsonDuplicate(normalLeft);
		TEST_ASSERT_NOT_NULL(normalLeft);
		TEST_ASSERT_NOT_NULL(normalRight);

		RyanJson_t removed = RyanJsonDetachByKey(normalRight, "a");
		TEST_ASSERT_NOT_NULL(removed);
		RyanJsonDelete(removed);
		TEST_ASSERT_TRUE(RyanJsonAddIntToObject(normalRight, "c", 3));

		TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(normalLeft, normalRight));
		TEST_ASSERT_FALSE(RyanJsonCompare(normalLeft, normalRight));

		RyanJsonDelete(normalLeft);
		RyanJsonDelete(normalRight);
	}
#else
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(right);

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, right), "初始重复 key 结构一致时 CompareOnlyKey 应返回 True");

	// 移除一个 a，再补一个 b -> key 计数变化，应判不等。
	RyanJson_t removedA = RyanJsonDetachByKey(right, "a");
	TEST_ASSERT_NOT_NULL(removedA);
	RyanJsonDelete(removedA);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(right, "b", 4));

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, right), "重复 key 计数不一致时 CompareOnlyKey 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, right), "重复 key 计数不一致时 Compare 应返回 False");

	// 恢复：移除一个 b，再补一个 a -> key 计数对齐，应恢复 CompareOnlyKey。
	RyanJson_t removedB = RyanJsonDetachByKey(right, "b");
	TEST_ASSERT_NOT_NULL(removedB);
	RyanJsonDelete(removedB);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(right, "a", 9));

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, right), "重复 key 计数恢复后 CompareOnlyKey 应返回 True");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, right), "值变化后 Compare 应返回 False");
#endif

	RyanJsonDelete(left);
	RyanJsonDelete(right);
}

static void testCompareDuplicateKeyChangeKeyCreatesMismatch(void)
{
	// 该用例验证 ChangeKey 在非严格模式下制造重复 key 后 CompareOnlyKey 的行为。
	RyanJson_t left = RyanJsonParse("{\"a\":1,\"a\":2,\"b\":3}");
	RyanJson_t right = RyanJsonParse("{\"a\":1,\"a\":2,\"b\":3}");

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL_MESSAGE(left, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(right, "strict 模式下重复 key 应解析失败");

	// 控制组：strict 模式下 ChangeKey 不允许制造重复 key。
	{
		RyanJson_t normal = RyanJsonParse("{\"a\":1,\"b\":2}");
		TEST_ASSERT_NOT_NULL(normal);
		TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeKey(RyanJsonGetObjectToKey(normal, "a"), "b"),
					  "strict 模式下 ChangeKey 制造重复 key 应失败");
		RyanJson_t expect = RyanJsonParse("{\"a\":1,\"b\":2}");
		TEST_ASSERT_NOT_NULL(expect);
		TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(normal, expect));
		RyanJsonDelete(expect);
		RyanJsonDelete(normal);
	}
#else
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(right);

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeKey(RyanJsonGetObjectToKey(right, "a"), "b"), "非严格模式下 ChangeKey 制造重复 key 应成功");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, right), "重复 key 计数变化后 CompareOnlyKey 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, right), "重复 key 计数变化后 Compare 应返回 False");
#endif

	RyanJsonDelete(left);
	RyanJsonDelete(right);
}

static void testCompareDuplicateKeyNestedMutationIsolation(void)
{
	// 该用例覆盖“嵌套重复 key 变更”对 CompareOnlyKey 的影响。
	RyanJson_t left = RyanJsonParse("{\"outer\":{\"a\":1,\"a\":2},\"keep\":1}");
	RyanJson_t right = RyanJsonParse("{\"outer\":{\"a\":1,\"a\":2},\"keep\":1}");

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL_MESSAGE(left, "strict 模式下嵌套重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(right, "strict 模式下嵌套重复 key 应解析失败");

	// 控制组：无重复 key 的嵌套对象变更应被 CompareOnlyKey 捕获。
	{
		RyanJson_t normalLeft = RyanJsonParse("{\"outer\":{\"a\":1,\"b\":2},\"keep\":1}");
		RyanJson_t normalRight = RyanJsonDuplicate(normalLeft);
		TEST_ASSERT_NOT_NULL(normalLeft);
		TEST_ASSERT_NOT_NULL(normalRight);

		RyanJson_t outer = RyanJsonGetObjectToKey(normalRight, "outer");
		RyanJson_t removed = RyanJsonDetachByKey(outer, "a");
		TEST_ASSERT_NOT_NULL(removed);
		RyanJsonDelete(removed);
		TEST_ASSERT_TRUE(RyanJsonAddIntToObject(outer, "c", 3));

		TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(normalLeft, normalRight));
		TEST_ASSERT_FALSE(RyanJsonCompare(normalLeft, normalRight));

		RyanJsonDelete(normalLeft);
		RyanJsonDelete(normalRight);
	}
#else
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(right);

	RyanJson_t outerRight = RyanJsonGetObjectToKey(right, "outer");
	TEST_ASSERT_NOT_NULL(outerRight);

	// 移除一个 a，再补 b -> 嵌套 key 计数变化，应判不等。
	RyanJson_t removedA = RyanJsonDetachByKey(outerRight, "a");
	TEST_ASSERT_NOT_NULL(removedA);
	RyanJsonDelete(removedA);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(outerRight, "b", 3));

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, right), "嵌套重复 key 计数不一致时 CompareOnlyKey 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, right), "嵌套重复 key 计数不一致时 Compare 应返回 False");

	// 恢复：移除一个 b，再补一个 a -> key 计数对齐，应恢复 CompareOnlyKey。
	RyanJson_t removedB = RyanJsonDetachByKey(outerRight, "b");
	TEST_ASSERT_NOT_NULL(removedB);
	RyanJsonDelete(removedB);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(outerRight, "a", 9));

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, right), "嵌套重复 key 计数恢复后 CompareOnlyKey 应返回 True");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, right), "值变化后 Compare 应返回 False");
#endif

	RyanJsonDelete(left);
	RyanJsonDelete(right);
}

static void testCompareDuplicateKeyCreatedByApi(void)
{
	// 该用例验证通过 API 构造重复 key 后的 Compare/CompareOnlyKey 行为。
#if true == RyanJsonStrictObjectKeyCheck
	{
		RyanJson_t strictObj = RyanJsonCreateObject();
		TEST_ASSERT_NOT_NULL(strictObj);
		TEST_ASSERT_TRUE(RyanJsonAddIntToObject(strictObj, "a", 1));
		TEST_ASSERT_FALSE(RyanJsonAddIntToObject(strictObj, "a", 2));
		RyanJsonDelete(strictObj);
	}
#else
	RyanJson_t left = RyanJsonCreateObject();
	RyanJson_t right = RyanJsonCreateObject();
	RyanJson_t rightTypeMismatch = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(right);
	TEST_ASSERT_NOT_NULL(rightTypeMismatch);

	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(left, "a", 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(left, "a", 2));

	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(right, "a", 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(right, "a", 2));

	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(rightTypeMismatch, "a", 1));
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(rightTypeMismatch, "a", "2"));

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(left, right), "API 构造的重复 key 完全一致时 Compare 应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, right), "API 构造的重复 key 完全一致时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightTypeMismatch), "API 构造的重复 key 同序号类型变化，Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightTypeMismatch),
				  "API 构造的重复 key 同序号类型变化，CompareOnlyKey 应返回 False");

	RyanJsonDelete(left);
	RyanJsonDelete(right);
	RyanJsonDelete(rightTypeMismatch);
#endif
}

static void testCompareDuplicateKeyReplaceByKeyFirstOccurrence(void)
{
	// 该用例验证 ReplaceByKey 在重复 key 下仅替换首个命中。
#if true == RyanJsonStrictObjectKeyCheck
	RyanJson_t strict = RyanJsonParse("{\"a\":1,\"a\":2}");
	TEST_ASSERT_NULL_MESSAGE(strict, "strict 模式下重复 key 应解析失败");
#else
	RyanJson_t base = RyanJsonParse("{\"a\":1,\"a\":2}");
	RyanJson_t changed = RyanJsonDuplicate(base);
	TEST_ASSERT_NOT_NULL(base);
	TEST_ASSERT_NOT_NULL(changed);

	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(changed, "a", RyanJsonCreateInt("a", 9)));

	RyanJson_t expect = RyanJsonParse("{\"a\":9,\"a\":2}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(changed, expect), "ReplaceByKey 应仅替换首个命中节点");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(changed, expect), "ReplaceByKey 不应改变 key/type 结构");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(base, changed), "值变化后 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(base, changed), "仅值变化时 CompareOnlyKey 应返回 True");

	RyanJsonDelete(expect);
	RyanJsonDelete(changed);
	RyanJsonDelete(base);
#endif
}

static void testCompareDuplicateKeyContainerIsolation(void)
{
	// 该用例验证“不同对象容器”的重复 key 计数不应互相抵消。
#if true == RyanJsonStrictObjectKeyCheck
	RyanJson_t strict = RyanJsonParse("{\"obj\":{\"a\":1,\"a\":2}}");
	TEST_ASSERT_NULL_MESSAGE(strict, "strict 模式下重复 key 应解析失败");
#else
	RyanJson_t left = RyanJsonParse("{\"obj\":{\"a\":1,\"a\":2},\"other\":{\"a\":1}}");
	RyanJson_t right = RyanJsonParse("{\"obj\":{\"a\":1,\"a\":2},\"other\":{\"a\":1,\"a\":2}}");
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(right);

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, right), "不同容器的重复 key 计数不一致时 CompareOnlyKey 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, right), "不同容器的重复 key 计数不一致时 Compare 应返回 False");

	RyanJsonDelete(left);
	RyanJsonDelete(right);
#endif
}

static void testCompareDuplicateKeyEscapedAndEmptyKey(void)
{
	// 该用例验证两类容易遗漏的重复 key：
	// 1) 空 key（""）重复；
	// 2) 转义后等价 key（"a" 与 "\u0061"）重复。
	RyanJson_t left = RyanJsonParse("{\"\":1,\"\":2,\"a\":3,\"\\u0061\":4}");
	RyanJson_t rightSame = RyanJsonParse("{\"a\":3,\"\\u0061\":4,\"\":1,\"\":2}");
	RyanJson_t rightValueSwap = RyanJsonParse("{\"\\u0061\":4,\"a\":3,\"\":1,\"\":2}");
	RyanJson_t rightTypeMismatch = RyanJsonParse("{\"a\":3,\"\\u0061\":\"4\",\"\":1,\"\":2}");
	RyanJson_t rightCountMismatch = RyanJsonParse("{\"a\":3,\"\":1,\"\":2}");

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL_MESSAGE(left, "strict 模式下空 key 重复应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightSame, "strict 模式下空 key/转义重复应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightValueSwap, "strict 模式下空 key/转义重复应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightTypeMismatch, "strict 模式下空 key/转义重复应解析失败");
	TEST_ASSERT_NULL_MESSAGE(rightCountMismatch, "strict 模式下空 key 重复应解析失败");

	// 控制组：转义 key 与普通 key 但不重复时，乱序比较应成立。
	{
		RyanJson_t normalLeft = RyanJsonParse("{\"\":1,\"a\":3,\"\\u0062\":4}");
		RyanJson_t normalRight = RyanJsonParse("{\"\\u0062\":4,\"a\":3,\"\":1}");
		RyanJson_t normalMismatch = RyanJsonParse("{\"\\u0063\":4,\"a\":3,\"\":1}");
		TEST_ASSERT_NOT_NULL(normalLeft);
		TEST_ASSERT_NOT_NULL(normalRight);
		TEST_ASSERT_NOT_NULL(normalMismatch);

		TEST_ASSERT_TRUE(RyanJsonCompare(normalLeft, normalRight));
		TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(normalLeft, normalRight));
		TEST_ASSERT_FALSE(RyanJsonCompare(normalLeft, normalMismatch));
		TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(normalLeft, normalMismatch));

		RyanJsonDelete(normalLeft);
		RyanJsonDelete(normalRight);
		RyanJsonDelete(normalMismatch);
	}
#else
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(rightSame);
	TEST_ASSERT_NOT_NULL(rightValueSwap);
	TEST_ASSERT_NOT_NULL(rightTypeMismatch);
	TEST_ASSERT_NOT_NULL(rightCountMismatch);

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(left, rightSame), "空 key 与转义重复 key 同序号一致时 Compare 应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightSame), "空 key 与转义重复 key 同序号一致时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightValueSwap), "转义重复 key 同序号值错位时 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightValueSwap), "转义重复 key 同序号类型一致时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightTypeMismatch), "转义重复 key 同序号类型变化时 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightTypeMismatch),
				  "转义重复 key 同序号类型变化时 CompareOnlyKey 应返回 False");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightCountMismatch), "空 key/转义重复 key 个数不一致时 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightCountMismatch),
				  "空 key/转义重复 key 个数不一致时 CompareOnlyKey 应返回 False");
#endif

	RyanJsonDelete(left);
	RyanJsonDelete(rightSame);
	RyanJsonDelete(rightValueSwap);
	RyanJsonDelete(rightTypeMismatch);
	RyanJsonDelete(rightCountMismatch);
}

static void testCompareDuplicateKeySymmetry(void)
{
	// 该用例验证比较运算的对称性：
	// 对任意 A/B，Compare(A,B) 与 Compare(B,A) 结果应一致，
	// CompareOnlyKey 同理。
	RyanJson_t trueLeft = RyanJsonParse("{\"k\":1,\"k\":2,\"x\":3}");
	RyanJson_t trueRight = RyanJsonParse("{\"x\":3,\"k\":1,\"k\":2}");
	RyanJson_t falseTypeLeft = RyanJsonParse("{\"k\":1,\"k\":2,\"x\":3}");
	RyanJson_t falseTypeRight = RyanJsonParse("{\"x\":3,\"k\":1,\"k\":\"2\"}");
	RyanJson_t falseKeyLeft = RyanJsonParse("{\"k\":1,\"k\":2}");
	RyanJson_t falseKeyRight = RyanJsonParse("{\"k\":1,\"x\":2}");

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL_MESSAGE(trueLeft, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(trueRight, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(falseTypeLeft, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(falseTypeRight, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NULL_MESSAGE(falseKeyLeft, "strict 模式下重复 key 应解析失败");
	TEST_ASSERT_NOT_NULL(falseKeyRight);

	// 控制组：无重复 key 的对称性。
	{
		RyanJson_t normalLeft = RyanJsonParse("{\"a\":1,\"b\":2}");
		RyanJson_t normalRight = RyanJsonParse("{\"b\":2,\"a\":1}");
		RyanJson_t normalMismatch = RyanJsonParse("{\"a\":1,\"c\":2}");
		TEST_ASSERT_NOT_NULL(normalLeft);
		TEST_ASSERT_NOT_NULL(normalRight);
		TEST_ASSERT_NOT_NULL(normalMismatch);

		TEST_ASSERT_TRUE(RyanJsonCompare(normalLeft, normalRight));
		TEST_ASSERT_TRUE(RyanJsonCompare(normalRight, normalLeft));
		TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(normalLeft, normalRight));
		TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(normalRight, normalLeft));

		TEST_ASSERT_FALSE(RyanJsonCompare(normalLeft, normalMismatch));
		TEST_ASSERT_FALSE(RyanJsonCompare(normalMismatch, normalLeft));
		TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(normalLeft, normalMismatch));
		TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(normalMismatch, normalLeft));

		RyanJsonDelete(normalLeft);
		RyanJsonDelete(normalRight);
		RyanJsonDelete(normalMismatch);
	}
#else
	TEST_ASSERT_NOT_NULL(trueLeft);
	TEST_ASSERT_NOT_NULL(trueRight);
	TEST_ASSERT_NOT_NULL(falseTypeLeft);
	TEST_ASSERT_NOT_NULL(falseTypeRight);
	TEST_ASSERT_NOT_NULL(falseKeyLeft);
	TEST_ASSERT_NOT_NULL(falseKeyRight);

	TEST_ASSERT_TRUE(RyanJsonCompare(trueLeft, trueRight));
	TEST_ASSERT_TRUE(RyanJsonCompare(trueRight, trueLeft));
	TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(trueLeft, trueRight));
	TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(trueRight, trueLeft));

	TEST_ASSERT_FALSE(RyanJsonCompare(falseTypeLeft, falseTypeRight));
	TEST_ASSERT_FALSE(RyanJsonCompare(falseTypeRight, falseTypeLeft));
	TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(falseTypeLeft, falseTypeRight));
	TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(falseTypeRight, falseTypeLeft));

	TEST_ASSERT_FALSE(RyanJsonCompare(falseKeyLeft, falseKeyRight));
	TEST_ASSERT_FALSE(RyanJsonCompare(falseKeyRight, falseKeyLeft));
	TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(falseKeyLeft, falseKeyRight));
	TEST_ASSERT_FALSE(RyanJsonCompareOnlyKey(falseKeyRight, falseKeyLeft));
#endif

	RyanJsonDelete(trueLeft);
	RyanJsonDelete(trueRight);
	RyanJsonDelete(falseTypeLeft);
	RyanJsonDelete(falseTypeRight);
	RyanJsonDelete(falseKeyLeft);
	RyanJsonDelete(falseKeyRight);
}

static void testCompareDuplicateKeyHighCardinality(void)
{
	// 该用例覆盖大量元素时的重复 key 对齐路径。
	// 为控制内存占用，只使用 256 个节点，避免测试本身成为内存压力源。
	const uint32_t repeatCount = 256U;

#if true == RyanJsonStrictObjectKeyCheck
	// strict 模式控制组：重复 key 从第二次起必须拒绝。
	{
		RyanJson_t strictObj = RyanJsonCreateObject();
		TEST_ASSERT_NOT_NULL(strictObj);
		TEST_ASSERT_TRUE(RyanJsonAddIntToObject(strictObj, "dup", 1));
		TEST_ASSERT_FALSE(RyanJsonAddIntToObject(strictObj, "dup", 2));
		RyanJsonDelete(strictObj);
	}

	// strict 模式下用大规模“唯一 key”对象验证普通比较路径稳定性。
	{
		RyanJson_t left = RyanJsonCreateObject();
		RyanJson_t rightSame = RyanJsonCreateObject();
		RyanJson_t rightValueMismatch = RyanJsonCreateObject();
		TEST_ASSERT_NOT_NULL(left);
		TEST_ASSERT_NOT_NULL(rightSame);
		TEST_ASSERT_NOT_NULL(rightValueMismatch);

		for (uint32_t i = 0; i < repeatCount; i++)
		{
			char key[32];
			RyanJsonSnprintf(key, sizeof(key), "k%" PRIu32, i);
			TEST_ASSERT_TRUE(RyanJsonAddIntToObject(left, key, (int32_t)i));
			TEST_ASSERT_TRUE(RyanJsonAddIntToObject(rightSame, key, (int32_t)i));
			TEST_ASSERT_TRUE(RyanJsonAddIntToObject(rightValueMismatch, key, (int32_t)(i == 177U ? (i + 1U) : i)));
		}

		TEST_ASSERT_TRUE(RyanJsonCompare(left, rightSame));
		TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(left, rightSame));
		TEST_ASSERT_FALSE(RyanJsonCompare(left, rightValueMismatch));
		TEST_ASSERT_TRUE(RyanJsonCompareOnlyKey(left, rightValueMismatch));

		RyanJsonDelete(left);
		RyanJsonDelete(rightSame);
		RyanJsonDelete(rightValueMismatch);
	}
#else
	RyanJson_t left = RyanJsonCreateObject();
	RyanJson_t rightSame = RyanJsonCreateObject();
	RyanJson_t rightValueMismatch = RyanJsonCreateObject();
	RyanJson_t rightTypeMismatch = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(left);
	TEST_ASSERT_NOT_NULL(rightSame);
	TEST_ASSERT_NOT_NULL(rightValueMismatch);
	TEST_ASSERT_NOT_NULL(rightTypeMismatch);

	for (uint32_t i = 0; i < repeatCount; i++)
	{
		TEST_ASSERT_TRUE(RyanJsonAddIntToObject(left, "dup", (int32_t)i));
		TEST_ASSERT_TRUE(RyanJsonAddIntToObject(rightSame, "dup", (int32_t)i));
		TEST_ASSERT_TRUE(RyanJsonAddIntToObject(rightValueMismatch, "dup", (int32_t)(i == 177U ? (i + 1U) : i)));

		// 在固定序号注入 number/string 类型差异，验证 CompareOnlyKey 的类型判定不被大数据量掩盖。
		if (i == 177U) { TEST_ASSERT_TRUE(RyanJsonAddStringToObject(rightTypeMismatch, "dup", "177")); }
		else
		{
			TEST_ASSERT_TRUE(RyanJsonAddIntToObject(rightTypeMismatch, "dup", (int32_t)i));
		}
	}

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(left, rightSame), "大量重复 key 同序号完全一致时 Compare 应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightSame), "大量重复 key 同序号完全一致时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightValueMismatch), "大量重复 key 存在同序号值差异时 Compare 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(left, rightValueMismatch),
				 "大量重复 key 仅值不同且类型一致时 CompareOnlyKey 应返回 True");

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(left, rightTypeMismatch), "大量重复 key 存在同序号类型差异时 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(left, rightTypeMismatch),
				  "大量重复 key 存在同序号类型差异时 CompareOnlyKey 应返回 False");

	RyanJsonDelete(left);
	RyanJsonDelete(rightSame);
	RyanJsonDelete(rightValueMismatch);
	RyanJsonDelete(rightTypeMismatch);
#endif
}

static void testCompareDuplicateKeyStreamParseMutationChain(void)
{
	// 复杂链路：
	// ParseOptions(文档1) -> ParseOptions(文档2) -> Compare/CompareOnlyKey
	// -> ReplaceByKey(失败) -> CompareOnlyKey 再校验 -> Detach/Add 修复结构 -> Compare。
	// 目标：
	// 1) 覆盖“流式多文档 + 重复 key 比较”的真实调用链；
	// 2) 验证失败 API 不会污染 CompareOnlyKey 的结构判断；
	// 3) 验证结构修复后 Compare/CompareOnlyKey 可恢复一致。
	const char *dupStream = "{\"a\":1,\"a\":2,\"c\":3}{\"a\":1,\"b\":2,\"c\":3}";
	const uint32_t dupLen = (uint32_t)strlen(dupStream);
	const char *dupEnd = NULL;

	RyanJson_t first = RyanJsonParseOptions(dupStream, dupLen, RyanJsonFalse, &dupEnd);

	// strict 模式下，文档1 含重复 key，会在解析阶段失败。
	// 这里走一条无重复 key 的控制链路，保证该用例在两种模式都可执行。
	if (NULL == first)
	{
		const char *strictStream = "{\"a\":1,\"c\":3}{\"a\":1,\"b\":2,\"c\":3}";
		const uint32_t strictLen = (uint32_t)strlen(strictStream);
		const char *strictEnd = NULL;
		RyanJson_t strictFirst = RyanJsonParseOptions(strictStream, strictLen, RyanJsonFalse, &strictEnd);
		TEST_ASSERT_NOT_NULL_MESSAGE(strictFirst, "strict 控制链路文档1 解析应成功");
		TEST_ASSERT_NOT_NULL(strictEnd);

		uint32_t strictRemain = (uint32_t)(strictLen - (uint32_t)(strictEnd - strictStream));
		RyanJson_t strictSecond = RyanJsonParseOptions(strictEnd, strictRemain, RyanJsonTrue, NULL);
		TEST_ASSERT_NOT_NULL_MESSAGE(strictSecond, "strict 控制链路文档2 解析应成功");

		TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(strictFirst, strictSecond),
					  "strict 控制链路初始 key 集不一致，应返回 False");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddIntToObject(strictFirst, "b", 2), "strict 控制链路补齐 key=b 失败");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(strictFirst, strictSecond), "strict 控制链路补齐后 Compare 应返回 True");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(strictFirst, strictSecond),
					 "strict 控制链路补齐后 CompareOnlyKey 应返回 True");

		RyanJsonDelete(strictSecond);
		RyanJsonDelete(strictFirst);
		return;
	}

	TEST_ASSERT_NOT_NULL(dupEnd);
	TEST_ASSERT_EQUAL_CHAR('{', *dupEnd);

	uint32_t dupRemain = (uint32_t)(dupLen - (uint32_t)(dupEnd - dupStream));
	RyanJson_t second = RyanJsonParseOptions(dupEnd, dupRemain, RyanJsonTrue, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(second, "文档2 解析应成功");

	// 初始时：文档1 为 a,a,c；文档2 为 a,b,c。应明显不等。
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(first, second), "重复 key 文档与唯一 key 文档初始 Compare 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(first, second), "重复 key 文档与唯一 key 文档初始 CompareOnlyKey 应返回 False");

	RyanJson_t sameShape = RyanJsonParse("{\"c\":30,\"a\":10,\"a\":20}");
	RyanJson_t keyMismatch = RyanJsonParse("{\"c\":30,\"a\":10,\"b\":20}");
	TEST_ASSERT_NOT_NULL(sameShape);
	TEST_ASSERT_NOT_NULL(keyMismatch);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(first, sameShape), "同 key 同序号类型一致时 CompareOnlyKey 应返回 True");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompareOnlyKey(first, keyMismatch), "重复 key 与不同 key 混合时 CompareOnlyKey 应返回 False");

	// 失败路径：不存在 key 的替换失败，结构语义不应被破坏。
	RyanJson_t failCandidate = RyanJsonCreateInt("missing", 7);
	TEST_ASSERT_NOT_NULL(failCandidate);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByKey(first, "missing", failCandidate), "ReplaceByKey(不存在 key) 应失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDetachedItem(failCandidate), "失败后 failCandidate 应保持 detached");
	RyanJsonDelete(failCandidate);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(first, sameShape), "失败路径后 CompareOnlyKey 语义不应变化");

	// 修复结构：删除第二个 a，再补 b，使其与文档2 结构和值都对齐。
	RyanJson_t removedDup = RyanJsonDetachByIndex(first, 1);
	TEST_ASSERT_NOT_NULL_MESSAGE(removedDup, "分离第二个重复 key 节点失败");
	RyanJsonDelete(removedDup);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddIntToObject(first, "b", 2), "补齐 key=b 失败");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareOnlyKey(first, second), "结构修复后 CompareOnlyKey 应返回 True");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(first, second), "结构和值修复后 Compare 应返回 True");

	char *printed = RyanJsonPrint(first, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(first, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(keyMismatch);
	RyanJsonDelete(sameShape);
	RyanJsonDelete(second);
	RyanJsonDelete(first);
}

void testCompareDuplicateKeyAdvancedRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testCompareDuplicateKeyInterleavedWithUnique);
	RUN_TEST(testCompareDuplicateKeyMutationCountMismatchAndRecover);
	RUN_TEST(testCompareDuplicateKeyChangeKeyCreatesMismatch);
	RUN_TEST(testCompareDuplicateKeyNestedMutationIsolation);
	RUN_TEST(testCompareDuplicateKeyCreatedByApi);
	RUN_TEST(testCompareDuplicateKeyReplaceByKeyFirstOccurrence);
	RUN_TEST(testCompareDuplicateKeyContainerIsolation);
	RUN_TEST(testCompareDuplicateKeyEscapedAndEmptyKey);
	RUN_TEST(testCompareDuplicateKeySymmetry);
	RUN_TEST(testCompareDuplicateKeyHighCardinality);
	RUN_TEST(testCompareDuplicateKeyStreamParseMutationChain);
}
