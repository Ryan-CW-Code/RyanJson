#include "testBase.h"

static void testKeyLookupEmptyString(void)
{
	// 覆盖空 key 的解析与查询。
	RyanJson_t root = RyanJsonParse("{\"\":1,\"a\":2}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t node = RyanJsonGetObjectByKey(root, "");
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(node));

	RyanJsonDelete(root);
}

static void testKeyLookupEscapedQuote(void)
{
	// 覆盖 key 中包含引号 \" 的解析与查询。
	RyanJson_t root = RyanJsonParse("{\"\\\"\":1}");
	TEST_ASSERT_NOT_NULL(root);

	const char *key = "\"";
	RyanJson_t node = RyanJsonGetObjectByKey(root, key);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(node));

	RyanJsonDelete(root);
}

static void testKeyLookupEscapedBackslash(void)
{
	// 覆盖 key 中包含反斜杠 \\ 的解析与查询。
	RyanJson_t root = RyanJsonParse("{\"\\\\\":1}");
	TEST_ASSERT_NOT_NULL(root);

	const char *key = "\\";
	RyanJson_t node = RyanJsonGetObjectByKey(root, key);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(node));

	RyanJsonDelete(root);
}

static void testKeyLookupEscapedSolidus(void)
{
	// 覆盖 key 中包含转义斜杠 \\/ 的解析与查询。
	RyanJson_t root = RyanJsonParse("{\"\\/\":1}");
	TEST_ASSERT_NOT_NULL(root);

	const char *key = "/";
	RyanJson_t node = RyanJsonGetObjectByKey(root, key);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(node));

	RyanJsonDelete(root);
}

static void testKeyLookupEscapedTab(void)
{
	// 覆盖 key 中包含 \t 的解析与查询。
	RyanJson_t root = RyanJsonParse("{\"\\t\":1}");
	TEST_ASSERT_NOT_NULL(root);

	const char *key = "\t";
	RyanJson_t node = RyanJsonGetObjectByKey(root, key);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(node));

	RyanJsonDelete(root);
}

static void testKeyLookupEscapedNewline(void)
{
	// 覆盖 key 中包含 \n 的解析与查询。
	RyanJson_t root = RyanJsonParse("{\"\\n\":1}");
	TEST_ASSERT_NOT_NULL(root);

	const char *key = "\n";
	RyanJson_t node = RyanJsonGetObjectByKey(root, key);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(node));

	RyanJsonDelete(root);
}

static void testKeyLookupEscapedCarriageReturn(void)
{
	// 覆盖 key 中包含 \r 的解析与查询。
	RyanJson_t root = RyanJsonParse("{\"\\r\":1}");
	TEST_ASSERT_NOT_NULL(root);

	const char *key = "\r";
	RyanJson_t node = RyanJsonGetObjectByKey(root, key);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(node));

	RyanJsonDelete(root);
}

static void testKeyLookupEscapedBackspace(void)
{
	// 覆盖 key 中包含 \b 的解析与查询。
	RyanJson_t root = RyanJsonParse("{\"\\b\":1}");
	TEST_ASSERT_NOT_NULL(root);

	const char *key = "\b";
	RyanJson_t node = RyanJsonGetObjectByKey(root, key);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(node));

	RyanJsonDelete(root);
}

static void testKeyLookupEscapedUnicodeControl(void)
{
	// 覆盖 key 中包含 \u0001 的解析与查询。
	RyanJson_t root = RyanJsonParse("{\"\\u0001\":1}");
	TEST_ASSERT_NOT_NULL(root);

	char key[2] = {1, 0};
	RyanJson_t node = RyanJsonGetObjectByKey(root, key);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(node));

	RyanJsonDelete(root);
}

static void testKeyChangeFromEscapeToPlain(void)
{
	// 覆盖 key 从转义字符切换到普通字符串的路径可达性。
	RyanJson_t root = RyanJsonParse("{\"\\t\":1}");
	TEST_ASSERT_NOT_NULL(root);

	const char *tabKey = "\t";
	RyanJson_t node = RyanJsonGetObjectByKey(root, tabKey);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(node, "tab"));

	TEST_ASSERT_NOT_NULL(RyanJsonGetObjectByKey(root, "tab"));
	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(root, tabKey));

	RyanJsonDelete(root);
}

static void testKeyChangeFromEmptyToEscapedNewline(void)
{
	// 覆盖 key 从空字符串切换到 \n 的路径可达性。
	RyanJson_t root = RyanJsonParse("{\"\":1}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t node = RyanJsonGetObjectByKey(root, "");
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(node, "\n"));

	TEST_ASSERT_NOT_NULL(RyanJsonGetObjectByKey(root, "\n"));
	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(root, ""));

	RyanJsonDelete(root);
}

static void testUtf8KeyCreateLookupAndRoundtrip(void)
{
	// 覆盖 UTF-8 key 的创建、查询与往返：
	// 1) Create/Add*ToObject 直接写入 UTF-8 key；
	// 2) GetObjectByKey 可定位多字节 key；
	// 3) Print/Parse 往返后语义保持一致。
	const char *keyCopyright = "\xC2\xA9";
	const char *keyCn = "\xE4\xB8\xAD";
	const char *keyOx = "\xF0\x9F\x90\x82";

	RyanJson_t root = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(root);

	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(root, keyCopyright, "copy"));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(root, keyCn, 7));
	TEST_ASSERT_TRUE(RyanJsonAddBoolToObject(root, keyOx, RyanJsonTrue));

	TEST_ASSERT_EQUAL_STRING("copy", RyanJsonGetStringValue(RyanJsonGetObjectByKey(root, keyCopyright)));
	TEST_ASSERT_EQUAL_INT(7, RyanJsonGetIntValue(RyanJsonGetObjectByKey(root, keyCn)));
	TEST_ASSERT_TRUE(RyanJsonGetBoolValue(RyanJsonGetObjectByKey(root, keyOx)));

	char *printed = RyanJsonPrint(root, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);

	TEST_ASSERT_EQUAL_STRING("copy", RyanJsonGetStringValue(RyanJsonGetObjectByKey(roundtrip, keyCopyright)));
	TEST_ASSERT_EQUAL_INT(7, RyanJsonGetIntValue(RyanJsonGetObjectByKey(roundtrip, keyCn)));
	TEST_ASSERT_TRUE(RyanJsonGetBoolValue(RyanJsonGetObjectByKey(roundtrip, keyOx)));
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testAccessorPathLookupWithEscapedAndEmptyKeys(void)
{
	// 覆盖“空 key + 特殊字符 key”的路径查询可达性：
	// 1) 空 key 作为对象字段名；
	// 2) 包含换行/斜杠的 key；
	// 3) ByKeys 与 HasObjectToKey 在该类路径上的一致性。
	const char *jsonText = "{\"\":{\"line\\nkey\":{\"slash/key\":123,\"quote\\\"k\":456}}}";
	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "特殊 key 路径样本解析失败");

	RyanJson_t v1 = RyanJsonGetObjectByKeys(root, "", "line\nkey", "slash/key", NULL);
	RyanJson_t v2 = RyanJsonGetObjectByKeys(root, "", "line\nkey", "quote\"k", NULL);
	TEST_ASSERT_NOT_NULL(v1);
	TEST_ASSERT_NOT_NULL(v2);
	TEST_ASSERT_TRUE(RyanJsonIsInt(v1));
	TEST_ASSERT_TRUE(RyanJsonIsInt(v2));
	TEST_ASSERT_EQUAL_INT(123, RyanJsonGetIntValue(v1));
	TEST_ASSERT_EQUAL_INT(456, RyanJsonGetIntValue(v2));

	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "", "line\nkey", "slash/key"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "", "line\nkey", "quote\"k"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(root, "", "line\nkey", "missing"));

	RyanJsonDelete(root);
}
static void testAccessorUtf8PathLookupAndChangeKey(void)
{
	// 覆盖 UTF-8 key 在路径 API 中的可达性与改 key 行为：
	// 1) GetObjectByKeys/HasObjectToKey 对多字节 key 可达；
	// 2) ChangeKey(UTF-8 -> ASCII) 后路径更新应一致。
	const char *keyCn = "\xE4\xB8\xAD";
	const char *keyOx = "\xF0\x9F\x90\x82";
	const char *keyCopy = "\xC2\xA9";

	RyanJson_t root = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t child = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(child);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(child, keyOx, 9));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, keyCn, child));

	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonAddStringToArray(arr, "ok"));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, keyCopy, arr));

	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, keyCn, keyOx));
	TEST_ASSERT_EQUAL_INT(9, RyanJsonGetIntValue(RyanJsonGetObjectByKeys(root, keyCn, keyOx, NULL)));

	RyanJson_t cnNode = RyanJsonGetObjectByKey(root, keyCn);
	TEST_ASSERT_NOT_NULL(cnNode);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(cnNode, "cn"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectByKey(root, keyCn));
	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "cn", keyOx));
	TEST_ASSERT_EQUAL_INT(9, RyanJsonGetIntValue(RyanJsonGetObjectByKeys(root, "cn", keyOx, NULL)));

	TEST_ASSERT_NOT_NULL(RyanJsonGetObjectByKey(root, keyCopy));
	TEST_ASSERT_TRUE(RyanJsonIsArray(RyanJsonGetObjectByKey(root, keyCopy)));

	RyanJsonDelete(root);
}
static void testAccessorPathLookupUnicodeEscapedAndEmptyKey(void)
{
	// 覆盖“空 key + Unicode 转义 key + 普通 key”的混合路径可达性。
	// 目标：验证转义 key 会被解码，且 ByKeys/HasObjectToKey 使用 UTF-8 key 可命中。
	const char *keyCn = "\xE4\xB8\xAD";
	RyanJson_t root = RyanJsonParse("{\"\":{\"\\u4E2D\":{\"\\u0061/b\":3}}}");
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "Unicode 混合路径样本解析失败");

	RyanJson_t hit = RyanJsonGetObjectByKeys(root, "", keyCn, "a/b", NULL);
	TEST_ASSERT_NOT_NULL(hit);
	TEST_ASSERT_TRUE(RyanJsonIsInt(hit));
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(hit));

	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "", keyCn, "a/b"));
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "", "\\u4E2D", "a/b", NULL), "未解码 key 字面量不应命中");

	RyanJsonDelete(root);
}

static void testKeyLookupAfterDuplicatePreservesEscapedKeys(void)
{
	// 覆盖 Duplicate 后转义 key 的查询仍使用解码语义。
	RyanJson_t root = RyanJsonParse("{\"\\t\":1,\"\\n\":2}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t dup = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(dup);

	TEST_ASSERT_NOT_NULL(RyanJsonGetObjectByKey(dup, "\t"));
	TEST_ASSERT_NOT_NULL(RyanJsonGetObjectByKey(dup, "\n"));
	TEST_ASSERT_TRUE(RyanJsonCompare(root, dup));

	RyanJsonDelete(dup);
	RyanJsonDelete(root);
}

void testKeyEscapeLookupRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testKeyLookupEmptyString);
	RUN_TEST(testKeyLookupEscapedQuote);
	RUN_TEST(testKeyLookupEscapedBackslash);
	RUN_TEST(testKeyLookupEscapedSolidus);
	RUN_TEST(testKeyLookupEscapedTab);
	RUN_TEST(testKeyLookupEscapedNewline);
	RUN_TEST(testKeyLookupEscapedCarriageReturn);
	RUN_TEST(testKeyLookupEscapedBackspace);
	RUN_TEST(testKeyLookupEscapedUnicodeControl);
	RUN_TEST(testKeyChangeFromEscapeToPlain);
	RUN_TEST(testKeyChangeFromEmptyToEscapedNewline);
	RUN_TEST(testKeyLookupAfterDuplicatePreservesEscapedKeys);
	RUN_TEST(testUtf8KeyCreateLookupAndRoundtrip);
	RUN_TEST(testAccessorPathLookupWithEscapedAndEmptyKeys);
	RUN_TEST(testAccessorUtf8PathLookupAndChangeKey);
	RUN_TEST(testAccessorPathLookupUnicodeEscapedAndEmptyKey);
}
