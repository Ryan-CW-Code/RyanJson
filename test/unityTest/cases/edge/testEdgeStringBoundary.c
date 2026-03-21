#include "testBase.h"

static char *allocPatternString(uint32_t len, char ch)
{
	char *buf = (char *)malloc(len + 1U);
	if (NULL == buf) { return NULL; }
	for (uint32_t i = 0; i < len; i++)
	{
		buf[i] = ch;
	}
	buf[len] = '\0';
	return buf;
}

static char *allocJsonStringWithContent(const char *content)
{
	size_t len = strlen(content);
	char *json = (char *)malloc(len + 3U);
	TEST_ASSERT_NOT_NULL(json);
	json[0] = '"';
	if (len) { memcpy(json + 1, content, len); }
	json[len + 1] = '"';
	json[len + 2] = '\0';
	return json;
}

static char *allocEscapedUnicodeRepeat(uint32_t count)
{
	const char *unit = "\\u0061";
	const size_t unitLen = 6U;
	size_t total = unitLen * (size_t)count;
	char *buf = (char *)malloc(total + 1U);
	TEST_ASSERT_NOT_NULL(buf);

	for (uint32_t i = 0; i < count; i++)
	{
		memcpy(buf + ((size_t)i * unitLen), unit, unitLen);
	}
	buf[total] = '\0';
	return buf;
}

static void assertStringNodeMode(RyanJson_t node, const char *expect, uint32_t expectLen, RyanJsonBool_e expectPtrMode)
{
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonIsString(node));
	TEST_ASSERT_EQUAL_UINT32(expectLen, (uint32_t)strlen(RyanJsonGetStringValue(node)));
	TEST_ASSERT_EQUAL_MEMORY(expect, RyanJsonGetStringValue(node), expectLen);
	TEST_ASSERT_EQUAL_UINT8((uint8_t)expectPtrMode, (uint8_t)RyanJsonGetPayloadStrIsPtrByFlag(node));
}

static void testEdgeStringBoundaryChangeValueShortToLong(void)
{
	// 复杂链路：
	// Create(String) -> ChangeStringValue(长) -> Compare。
	// 目标：验证短 value 切换到长 value 后结构正确。
	uint32_t longLen = RyanJsonInlineStringSize + 8U;
	char *longVal = allocPatternString(longLen, 'v');
	TEST_ASSERT_NOT_NULL(longVal);

	RyanJson_t obj = RyanJsonParse("{\"s\":\"a\"}");
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(RyanJsonGetObjectByKey(obj, "s"), longVal));
	TEST_ASSERT_EQUAL_STRING(longVal, RyanJsonGetStringValue(RyanJsonGetObjectByKey(obj, "s")));

	RyanJson_t expect = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(expect, "s", longVal));
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(obj);
	free(longVal);
}

static void testEdgeStringBoundaryChangeValueLongToShort(void)
{
	// 复杂链路：
	// Create(String) -> ChangeStringValue(短) -> Compare。
	// 目标：验证长 value 切换到短 value 后结构正确。
	uint32_t longLen = RyanJsonInlineStringSize + 8U;
	char *longVal = allocPatternString(longLen, 'v');
	TEST_ASSERT_NOT_NULL(longVal);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(obj, "s", longVal));
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(RyanJsonGetObjectByKey(obj, "s"), "bb"));
	TEST_ASSERT_EQUAL_STRING("bb", RyanJsonGetStringValue(RyanJsonGetObjectByKey(obj, "s")));

	RyanJson_t expect = RyanJsonParse("{\"s\":\"bb\"}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(obj);
	free(longVal);
}

static void testEdgeStringBoundaryChangeValueSameLength(void)
{
	// 复杂链路：
	// Create(String) -> ChangeStringValue(同长度) -> Compare。
	// 目标：验证同长度替换不会破坏结构。
	RyanJson_t obj = RyanJsonParse("{\"s\":\"abc\"}");
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(RyanJsonGetObjectByKey(obj, "s"), "xyz"));
	TEST_ASSERT_EQUAL_STRING("xyz", RyanJsonGetStringValue(RyanJsonGetObjectByKey(obj, "s")));

	RyanJson_t expect = RyanJsonParse("{\"s\":\"xyz\"}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(obj);
}

static void testEdgeStringRoundtripLongValue(void)
{
	// 复杂链路：
	// Create(Object 长 value) -> Print -> Parse -> Compare。
	// 目标：验证长 String value 往返一致。
	uint32_t longLen = RyanJsonInlineStringSize + 8U;
	char *longVal = allocPatternString(longLen, 'v');
	TEST_ASSERT_NOT_NULL(longVal);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(obj, "s", longVal));

	char *printed = RyanJsonPrint(obj, 32, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(obj);
	free(longVal);
}

static void testEdgeStringBoundaryExactInlineLenValue(void)
{
	// 复杂链路：
	// Create(Object) -> AddStringToObject(value 长度=InlineSize) -> Print/Parse -> Compare。
	// 目标：验证边界长度 value 往返一致。
	uint32_t valLen = RyanJsonInlineStringSize;
	char *val = allocPatternString(valLen, 'v');
	TEST_ASSERT_NOT_NULL(val);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(obj, "s", val));

	char *printed = RyanJsonPrint(obj, 32, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(obj);
	free(val);
}

static void testEdgeStringBoundaryInlinePlusOneValue(void)
{
	// 复杂链路：
	// Create(Object) -> AddStringToObject(value 长度=InlineSize+1) -> Print/Parse -> Compare。
	// 目标：验证边界+1 长度 value 往返一致。
	uint32_t valLen = RyanJsonInlineStringSize + 1U;
	char *val = allocPatternString(valLen, 'v');
	TEST_ASSERT_NOT_NULL(val);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(obj, "s", val));

	char *printed = RyanJsonPrint(obj, 32, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(obj);
	free(val);
}

static void testEdgeStringParseInlineBoundaryPlain(void)
{
	// 目标：解析路径下，恰好命中 inline 临界长度应保持 inline。
	const uint32_t maxInlineLen = RyanJsonInlineStringSize - 1U;
	char *value = allocPatternString(maxInlineLen, 'a');
	char *jsonText = allocJsonStringWithContent(value);

	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL(root);
	assertStringNodeMode(root, value, maxInlineLen, RyanJsonFalse);

	char *printed = RyanJsonPrint(root, 0, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	assertStringNodeMode(roundtrip, value, maxInlineLen, RyanJsonFalse);

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
	free(jsonText);
	free(value);
}

static void testEdgeStringParsePtrBoundaryPlain(void)
{
	// 目标：解析路径下，超过 inline 临界长度应切到 ptr。
	const uint32_t overInlineLen = RyanJsonInlineStringSize;
	char *value = allocPatternString(overInlineLen, 'b');
	char *jsonText = allocJsonStringWithContent(value);

	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL(root);
	assertStringNodeMode(root, value, overInlineLen, RyanJsonTrue);

	char *printed = RyanJsonPrint(root, 0, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	assertStringNodeMode(roundtrip, value, overInlineLen, RyanJsonTrue);

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
	free(jsonText);
	free(value);
}

static void testEdgeStringParseInlineBoundaryEscaped(void)
{
	// 目标：转义 String 的“解码长度”决定 inline/ptr 模式。
	const uint32_t maxInlineLen = RyanJsonInlineStringSize - 1U;
	char *escaped = allocEscapedUnicodeRepeat(maxInlineLen);
	char *jsonText = allocJsonStringWithContent(escaped);
	char *expect = allocPatternString(maxInlineLen, 'a');

	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL(root);
	assertStringNodeMode(root, expect, maxInlineLen, RyanJsonFalse);

	RyanJsonDelete(root);
	free(expect);
	free(jsonText);
	free(escaped);
}

static void testEdgeStringParsePtrBoundaryEscaped(void)
{
	// 目标：转义 String 解码长度超过 inline 临界后，应切到 ptr。
	const uint32_t overInlineLen = RyanJsonInlineStringSize;
	char *escaped = allocEscapedUnicodeRepeat(overInlineLen);
	char *jsonText = allocJsonStringWithContent(escaped);
	char *expect = allocPatternString(overInlineLen, 'a');

	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL(root);
	assertStringNodeMode(root, expect, overInlineLen, RyanJsonTrue);

	RyanJsonDelete(root);
	free(expect);
	free(jsonText);
	free(escaped);
}

void testEdgeStringBoundaryRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testEdgeStringBoundaryChangeValueShortToLong);
	RUN_TEST(testEdgeStringBoundaryChangeValueLongToShort);
	RUN_TEST(testEdgeStringBoundaryChangeValueSameLength);
	RUN_TEST(testEdgeStringRoundtripLongValue);
	RUN_TEST(testEdgeStringBoundaryExactInlineLenValue);
	RUN_TEST(testEdgeStringBoundaryInlinePlusOneValue);
	RUN_TEST(testEdgeStringParseInlineBoundaryPlain);
	RUN_TEST(testEdgeStringParsePtrBoundaryPlain);
	RUN_TEST(testEdgeStringParseInlineBoundaryEscaped);
	RUN_TEST(testEdgeStringParsePtrBoundaryEscaped);
}
