#include "testBase.h"

static void testNumericLikeKeyLookupKeepsLiteralSpellings(void)
{
	// 覆盖“看起来像 Number”的 key 仍按字面文本存取，不做归一化。
	struct
	{
		const char *key;
		int32_t value;
	} cases[] = {{"0", 1},        {"00", 2},  {"01", 3},   {"1e2", 4}, {"1E2", 5}, {"00123", 6}, {"9007199254740993", 7},
		     {"Infinity", 8}, {"NaN", 9}, {"0x1", 10}, {"2-3", 11}};
	const char *jsonText = "{\"0\":1,\"00\":2,\"01\":3,\"1e2\":4,\"1E2\":5,\"00123\":6,\"9007199254740993\":7,\"Infinity\":8,\"NaN\":9,"
			       "\"0x1\":10,\"2-3\":11}";
	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "numeric-like key 样本解析失败");
	TEST_ASSERT_EQUAL_UINT32((uint32_t)(sizeof(cases) / sizeof(cases[0])), RyanJsonGetSize(root));

	for (uint32_t i = 0; i < (uint32_t)(sizeof(cases) / sizeof(cases[0])); i++)
	{
		RyanJson_t byKey = RyanJsonGetObjectByKey(root, cases[i].key);
		RyanJson_t byIndex = RyanJsonGetObjectByIndex(root, i);
		TEST_ASSERT_NOT_NULL_MESSAGE(byKey, "字面 key 查询失败");
		TEST_ASSERT_NOT_NULL_MESSAGE(byIndex, "按索引读取 numeric-like key 失败");
		TEST_ASSERT_EQUAL_INT(cases[i].value, RyanJsonGetIntValue(byKey));
		TEST_ASSERT_EQUAL_STRING(cases[i].key, RyanJsonGetKey(byIndex));
	}

	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(root, "123"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(root, "100"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(root, "-Infinity"));

	RyanJsonDelete(root);
}

static void testNumericLikeKeyNestedPathUsesExactSegments(void)
{
	// 覆盖多段 numeric-like key 路径：每一段都必须按原始拼写命中。
	const char *jsonText =
		"{\"00123\":{\"123\":{\"1e2\":3},\"9007199254740993\":{\"1E2\":4}},\"Infinity\":{\"NaN\":5,\"-Infinity\":6}}";
	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "nested numeric-like key 样本解析失败");

	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(RyanJsonGetObjectByKeys(root, "00123", "123", "1e2", NULL)));
	TEST_ASSERT_EQUAL_INT(4, RyanJsonGetIntValue(RyanJsonGetObjectByKeys(root, "00123", "9007199254740993", "1E2", NULL)));
	TEST_ASSERT_EQUAL_INT(5, RyanJsonGetIntValue(RyanJsonGetObjectByKeys(root, "Infinity", "NaN", NULL)));
	TEST_ASSERT_EQUAL_INT(6, RyanJsonGetIntValue(RyanJsonGetObjectByKeys(root, "Infinity", "-Infinity", NULL)));

	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "00123", "123", "1E2", NULL), "大小写不同的指数 key 不应混淆");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "00123", "00123", "1e2", NULL), "带前导零的 key 不应被归一化到无前导零版本");

	RyanJsonDelete(root);
}

static void testNumericLikeKeyHasApisTreatSimilarSpellingsAsDifferentKeys(void)
{
	// 覆盖 HasObjectByKey/HasObjectToKey 对相似 numeric-like key 的区分语义。
	RyanJson_t root = RyanJsonParse("{\"00123\":{\"000\":1},\"123\":{\"000\":2},\"1e2\":{\"1E2\":3},\"1E2\":{\"1e2\":4}}");
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "Has API numeric-like key 样本解析失败");

	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, "00123"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, "123"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, "1e2"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, "1E2"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(root, "000123"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(root, "100"));

	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "00123", "000"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "123", "000"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "1e2", "1E2"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "1E2", "1e2"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(root, "00123", "001"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(root, "1e2", "1e2"));

	RyanJsonDelete(root);
}

static void testNumericLikeKeyChangeKeyRetainsTextualIdentityAfterRoundtrip(void)
{
	// 复杂链路：ChangeKey(leading-zero -> big-integer-like) -> Print -> Parse。
	// 目标：验证 key 改名后仍按文本保真，不会被数值化。
	RyanJson_t root = RyanJsonParse("{\"00123\":7,\"keep\":17}");
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "ChangeKey numeric-like 样本解析失败");

	RyanJson_t node = RyanJsonGetObjectByKey(root, "00123");
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(node, "9007199254740993"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(root, "00123"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, "9007199254740993"));
	TEST_ASSERT_EQUAL_INT(7, RyanJsonGetIntValue(RyanJsonGetObjectByKey(root, "9007199254740993")));
	TEST_ASSERT_EQUAL_INT(17, RyanJsonGetIntValue(RyanJsonGetObjectByKey(root, "keep")));

	char *printed = RyanJsonPrint(root, 160, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(roundtrip, "9007199254740993"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(roundtrip, "00123"));
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testNumericLikeKeyDeleteDetachReplaceMatchExactLiteralOnly(void)
{
	// 覆盖 Delete/Detach/Replace 对 numeric-like key 的精确匹配语义。
	RyanJson_t root = RyanJsonParse("{\"1e2\":1,\"1E2\":2,\"0x1\":3,\"Infinity\":4,\"NaN\":5}");
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "numeric-like ops 样本解析失败");

	TEST_ASSERT_TRUE(RyanJsonDeleteByKey(root, "1e2"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(root, "1e2"));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByKey(root, "1E2")));

	RyanJson_t detached = RyanJsonDetachByKey(root, "0x1");
	TEST_ASSERT_NOT_NULL(detached);
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(detached));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(root, "0x1"));
	TEST_ASSERT_EQUAL_INT(4, RyanJsonGetIntValue(RyanJsonGetObjectByKey(root, "Infinity")));

	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "Infinity", RyanJsonCreateInt("Infinity", 40)));
	TEST_ASSERT_EQUAL_INT(40, RyanJsonGetIntValue(RyanJsonGetObjectByKey(root, "Infinity")));
	TEST_ASSERT_EQUAL_INT(5, RyanJsonGetIntValue(RyanJsonGetObjectByKey(root, "NaN")));
	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetSize(root));

	RyanJsonDelete(detached);
	RyanJsonDelete(root);
}

void testKeyNumericLikeRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testNumericLikeKeyLookupKeepsLiteralSpellings);
	RUN_TEST(testNumericLikeKeyNestedPathUsesExactSegments);
	RUN_TEST(testNumericLikeKeyHasApisTreatSimilarSpellingsAsDifferentKeys);
	RUN_TEST(testNumericLikeKeyChangeKeyRetainsTextualIdentityAfterRoundtrip);
	RUN_TEST(testNumericLikeKeyDeleteDetachReplaceMatchExactLiteralOnly);
}
