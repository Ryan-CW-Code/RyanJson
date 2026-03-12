#include "testBase.h"

typedef struct
{
	const char *name;
	const char *input;
	const char *expect;
} minifyTextSliceCase_t;

typedef struct
{
	const char *name;
	const uint8_t *input;
	uint32_t len;
	const char *expect;
} minifyBinarySliceCase_t;

static void assertMinifyTextSliceCase(const minifyTextSliceCase_t *testCase)
{
	const size_t len = strlen(testCase->input);
	const size_t expectLen = strlen(testCase->expect);
	char *buf = (char *)malloc(len + 2U);
	TEST_ASSERT_NOT_NULL(buf);

	memcpy(buf, testCase->input, len);
	buf[len] = '#';
	buf[len + 1U] = '\0';

	uint32_t outLen = RyanJsonMinify(buf, (int32_t)len);
	if (outLen < (uint32_t)len)
	{
		buf[outLen] = '\0';
		TEST_ASSERT_EQUAL_STRING_MESSAGE(testCase->expect, buf, testCase->name);
	}
	else
	{
		TEST_ASSERT_EQUAL_UINT32_MESSAGE((uint32_t)expectLen, outLen, testCase->name);
		TEST_ASSERT_EQUAL_MEMORY_MESSAGE(testCase->expect, buf, outLen, testCase->name);
	}
	TEST_ASSERT_EQUAL_UINT8_MESSAGE((uint8_t)'#', (uint8_t)buf[len], testCase->name);
	free(buf);
}

static void assertMinifyBinarySliceCase(const minifyBinarySliceCase_t *testCase)
{
	char *buf = (char *)malloc((size_t)testCase->len + 2U);
	TEST_ASSERT_NOT_NULL(buf);

	memcpy(buf, testCase->input, testCase->len);
	buf[testCase->len] = '#';
	buf[testCase->len + 1U] = '\0';

	uint32_t outLen = RyanJsonMinify(buf, (int32_t)testCase->len);
	if (outLen < testCase->len) { buf[outLen] = '\0'; }
	TEST_ASSERT_EQUAL_STRING_MESSAGE(testCase->expect, buf, testCase->name);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE((uint8_t)'#', (uint8_t)buf[testCase->len], testCase->name);
	free(buf);
}

#define MINIFY_TEXT_SLICE_CASE(name, label, input, expect)                                                                                 \
	static void name(void)                                                                                                             \
	{                                                                                                                                  \
		const minifyTextSliceCase_t testCase = {label, input, expect};                                                             \
		assertMinifyTextSliceCase(&testCase);                                                                                      \
	}

#define MINIFY_BINARY_SLICE_CASE(name, label, expect, ...)                                                                                 \
	static void name(void)                                                                                                             \
	{                                                                                                                                  \
		static const uint8_t input[] = {__VA_ARGS__};                                                                              \
		const minifyBinarySliceCase_t testCase = {label, input, (uint32_t)sizeof(input), expect};                                  \
		assertMinifyBinarySliceCase(&testCase);                                                                                    \
	}

// 块注释: 对象 token 边界
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockObjectLeadingComment, "leading block comment before object", "/*c*/{\"a\":1,\"b\":2}",
		       "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockObjectAfterOpenBrace, "block comment after object start", "{/*c*/\"a\":1,\"b\":2}",
		       "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockObjectBetweenKeyAndColon, "block comment between key and colon", "{\"a\"/*c*/:1,\"b\":2}",
		       "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockObjectBetweenColonAndValue, "block comment between colon and value",
		       "{\"a\":/*c*/1,\"b\":2}", "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockObjectAfterFirstValue, "block comment after first value", "{\"a\":1/*c*/,\"b\":2}",
		       "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockObjectAfterMemberComma, "block comment after member comma", "{\"a\":1,/*c*/\"b\":2}",
		       "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockObjectBeforeSecondMemberColon, "block comment before second member colon",
		       "{\"a\":1,\"b\"/*c*/:2}", "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockObjectBeforeSecondMemberValue, "block comment before second member value",
		       "{\"a\":1,\"b\":/*c*/2}", "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockObjectBeforeCloseBrace, "block comment before object close", "{\"a\":1,\"b\":2/*c*/}",
		       "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockObjectTrailingComment, "trailing block comment after object", "{\"a\":1,\"b\":2}/*c*/",
		       "{\"a\":1,\"b\":2}")

// 行注释: 对象 token 边界
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineObjectLeadingComment, "leading line comment before object", "//c\n{\"a\":1,\"b\":2}",
		       "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineObjectAfterOpenBrace, "line comment after object start", "{//c\n\"a\":1,\"b\":2}",
		       "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineObjectBetweenKeyAndColon, "line comment between key and colon", "{\"a\"//c\n:1,\"b\":2}",
		       "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineObjectBetweenColonAndValue, "line comment between colon and value", "{\"a\"://c\n1,\"b\":2}",
		       "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineObjectAfterFirstValue, "line comment after first value", "{\"a\":1//c\n,\"b\":2}",
		       "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineObjectAfterMemberComma, "line comment after member comma", "{\"a\":1,//c\n\"b\":2}",
		       "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineObjectBeforeSecondMemberColon, "line comment before second member colon",
		       "{\"a\":1,\"b\"//c\n:2}", "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineObjectBeforeSecondMemberValue, "line comment before second member value",
		       "{\"a\":1,\"b\"://c\n2}", "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineObjectBeforeCloseBrace, "line comment before object close", "{\"a\":1,\"b\":2//c\n}",
		       "{\"a\":1,\"b\":2}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineObjectTrailingComment, "trailing line comment after object", "{\"a\":1,\"b\":2}//c\n",
		       "{\"a\":1,\"b\":2}")

// 注释: 数组 token 边界
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockArrayLeadingComment, "leading block comment before array", "/*c*/[1,2,3]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockArrayAfterOpenBracket, "block comment after array start", "[/*c*/1,2,3]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockArrayAfterFirstItem, "block comment after first item", "[1/*c*/,2,3]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockArrayBeforeSecondItem, "block comment before second item", "[1,/*c*/2,3]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockArrayAfterSecondItem, "block comment after second item", "[1,2/*c*/,3]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockArrayBeforeThirdItem, "block comment before third item", "[1,2,/*c*/3]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockArrayBeforeCloseBracket, "block comment before array close", "[1,2,3/*c*/]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockArrayTrailingComment, "trailing block comment after array", "[1,2,3]/*c*/", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockArrayMultipleInteriorComments, "multiple block comments inside array",
		       "[1/*c*/,/*d*/2,/*e*/3]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockArrayTailComments, "block comments near array tail", "[1,2/*c*/,3/*d*/]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineArrayLeadingComment, "leading line comment before array", "//c\n[1,2,3]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineArrayAfterOpenBracket, "line comment after array start", "[//c\n1,2,3]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineArrayAfterFirstItem, "line comment after first item", "[1//c\n,2,3]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineArrayBeforeSecondItem, "line comment before second item", "[1,//c\n2,3]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineArrayAfterSecondItem, "line comment after second item", "[1,2//c\n,3]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineArrayBeforeThirdItem, "line comment before third item", "[1,2,//c\n3]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineArrayBeforeCloseBracket, "line comment before array close", "[1,2,3//c\n]", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineArrayTrailingComment, "trailing line comment after array", "[1,2,3]//c\n", "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineArrayMultipleInteriorComments, "multiple line comments inside array", "[1//c\n,2//d\n,3]",
		       "[1,2,3]")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineArrayTailComments, "line comments near array tail", "[1,2//c\n,3//d\n]", "[1,2,3]")

// 注释: 嵌套容器
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockNestedBeforeArrayValue, "block comment before nested array value",
		       "{\"a\":/*c*/[1,2],\"b\":{\"c\":3}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockNestedInsideArray, "block comment inside nested array", "{\"a\":[1,/*c*/2],\"b\":{\"c\":3}}",
		       "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockNestedBetweenTopLevelMembers, "block comment between top-level members",
		       "{\"a\":[1,2],/*c*/\"b\":{\"c\":3}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockNestedBeforeObjectKey, "block comment before nested object key",
		       "{\"a\":[1,2],\"b\":{/*c*/\"c\":3}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockNestedBeforeObjectValue, "block comment before nested object value",
		       "{\"a\":[1,2],\"b\":{\"c\":/*c*/3}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockNestedLeadingAndTrailingDocComments, "leading and trailing block comments around document",
		       "/*c*/{\"a\":[1,2],\"b\":{\"c\":3}}/*d*/", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockNestedBeforeRootClose, "block comment before root close",
		       "{\"a\":[1,2],\"b\":{\"c\":3}/*c*/}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockNestedAcrossMultipleTokens, "multiple block comments across nested tokens",
		       "{\"a\":/*c*/[1/*d*/,2/*e*/],\"b\":{/*f*/\"c\":3}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockNestedBeforeFirstArrayItem, "block comment before first nested array item",
		       "{\"a\":[/*c*/1,2],\"b\":{\"c\":3}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceBlockNestedAroundObjectKeyToken, "block comments around nested object key token",
		       "{\"a\":[1,2],\"b\":{/*c*/\"c\"/*d*/:3}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineNestedBeforeArrayValue, "line comment before nested array value",
		       "{\"a\"://c\n[1,2],\"b\":{\"c\":3}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineNestedInsideArray, "line comment inside nested array", "{\"a\":[1,//c\n2],\"b\":{\"c\":3}}",
		       "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineNestedBetweenTopLevelMembers, "line comment between top-level members",
		       "{\"a\":[1,2],//c\n\"b\":{\"c\":3}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineNestedBeforeObjectValue, "line comment before nested object value",
		       "{\"a\":[1,2],\"b\"://c\n{\"c\":3}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineNestedAfterObjectValue, "line comment after nested object value",
		       "{\"a\":[1,2],\"b\":{\"c\":3//c\n}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineNestedTrailingDocComment, "trailing line comment after document",
		       "{\"a\":[1,2],\"b\":{\"c\":3}}//c\n", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineNestedLeadingDocComment, "leading line comment before document",
		       "//c\n{\"a\":[1,2],\"b\":{\"c\":3}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineNestedBeforeObjectKey, "line comment before nested object key",
		       "{\"a\":[1,2],\"b\":{\n//c\n\"c\":3}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineNestedAfterObjectValueBeforeClose, "line comment after nested object value before close",
		       "{\"a\":[1,2],\"b\":{\n\"c\":3//c\n}}", "{\"a\":[1,2],\"b\":{\"c\":3}}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceLineNestedBetweenInnerObjectAndRootClose, "line comment between nested object and root close",
		       "{\"a\":[1,2],\"b\":{\n\"c\":3}\n//c\n}", "{\"a\":[1,2],\"b\":{\"c\":3}}")

// 字符串中的注释标记必须保留
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceStringMarkerLiteralSlashesAndBlock, "double slash and block marker strings",
		       "{\"s\":\"//\",\"t\":\"/* */\"}", "{\"s\":\"//\",\"t\":\"/* */\"}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceStringMarkerLiteralSplitBlock, "block marker split across strings",
		       "{\"s\":\"a/*b\",\"t\":\"c*/d\"}", "{\"s\":\"a/*b\",\"t\":\"c*/d\"}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceStringMarkerLiteralLineAndEscapedQuote, "line marker and escaped quote",
		       "{\"s\":\"a//b\",\"t\":\"c\\\"//d\"}", "{\"s\":\"a//b\",\"t\":\"c\\\"//d\"}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceStringMarkerLiteralEscapedSlashBeforeMarkers, "escaped slash before comment markers",
		       "{\"s\":\"a\\\\//b\",\"t\":\"a\\\\/*b*/\"}", "{\"s\":\"a\\\\//b\",\"t\":\"a\\\\/*b*/\"}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceStringMarkerLiteralUrlAndEscapedBlock, "url slashes and escaped block markers",
		       "{\"s\":\"http:\\/\\/x\",\"t\":\"x\\/*y*\\/z\"}", "{\"s\":\"http:\\/\\/x\",\"t\":\"x\\/*y*\\/z\"}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceStringMarkerLiteralUnicodeSlashes, "unicode slash escapes",
		       "{\"s\":\"\\u002F\\u002F\",\"t\":\"\\u002F*\"}", "{\"s\":\"\\u002F\\u002F\",\"t\":\"\\u002F*\"}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceStringMarkerLiteralUnicodeSlashAndStar, "unicode slash and star escapes",
		       "{\"s\":\"\\u002F\\/\",\"t\":\"\\u002A\\/\"}", "{\"s\":\"\\u002F\\/\",\"t\":\"\\u002A\\/\"}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceStringMarkerLiteralEscapedQuoteBeforeLine, "escaped quote before line marker",
		       "{\"s\":\"\\\"//\",\"t\":\"\\\\/*\"}", "{\"s\":\"\\\"//\",\"t\":\"\\\\/*\"}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceStringMarkerLiteralMixedBlockAndLine, "mixed block and line markers in string",
		       "{\"s\":\"x/*y*/z//w\",\"t\":\"x\\/\\/y\"}", "{\"s\":\"x/*y*/z//w\",\"t\":\"x\\/\\/y\"}")
MINIFY_TEXT_SLICE_CASE(testEdgeMinifySliceStringMarkerLiteralTripleEscapedSlash, "triple escaped slash before markers",
		       "{\"s\":\"x\\\\\\//y\",\"t\":\"x\\\\\\/*y*/\"}", "{\"s\":\"x\\\\\\//y\",\"t\":\"x\\\\\\/*y*/\"}")

// 内嵌 NUL: 显式长度 slice
MINIFY_BINARY_SLICE_CASE(testEdgeMinifySliceEmbeddedNulCompleteObjectTail, "embedded NUL after complete object", "{\"a\":1}", '{', '"', 'a',
			 '"', ':', '1', '}', '\0', 'X')
MINIFY_BINARY_SLICE_CASE(testEdgeMinifySliceEmbeddedNulCompleteArrayTail, "embedded NUL after complete array", "[1,2]", '[', '1', ',', '2',
			 ']', '\0', '!')
MINIFY_BINARY_SLICE_CASE(testEdgeMinifySliceEmbeddedNulAfterLineCommentObjectStart, "embedded NUL after line comment opens object", "{",
			 '/', '/', 'c', '\n', '{', '\0')
MINIFY_BINARY_SLICE_CASE(testEdgeMinifySliceEmbeddedNulAfterEscapedQuoteInString, "embedded NUL after escaped quote inside string",
			 "{\"s\":\"a\\\"b\"}", '{', '"', 's', '"', ':', '"', 'a', '\\', '"', 'b', '"', '}', '\0')
MINIFY_BINARY_SLICE_CASE(testEdgeMinifySliceEmbeddedNulAfterBlockCommentBeforeObject, "embedded NUL after block comment then object", "{",
			 '/', '*', 'c', '*', '/', '{', '\0', 'x')
MINIFY_BINARY_SLICE_CASE(testEdgeMinifySliceEmbeddedNulAfterObjectComma, "embedded NUL after comma in object", "{\"a\":1,", '{', '"', 'a',
			 '"', ':', '1', ',', '\0', '2')
MINIFY_BINARY_SLICE_CASE(testEdgeMinifySliceEmbeddedNulAfterArrayComma, "embedded NUL after comma in array", "[1,", '[', '1', ',', '\0',
			 '2', '3')
MINIFY_BINARY_SLICE_CASE(testEdgeMinifySliceEmbeddedNulInsideStringLiteral, "embedded NUL inside string literal", "{\"k\":\"x", '{', '"',
			 'k', '"', ':', '"', 'x', '\0', 'y', '"')
MINIFY_BINARY_SLICE_CASE(testEdgeMinifySliceEmbeddedNulInsideBareString, "embedded NUL in bare string slice", "\"a", '"', 'a', '\0', 'b')
MINIFY_BINARY_SLICE_CASE(testEdgeMinifySliceEmbeddedNulInsideBlockCommentOpener, "embedded NUL inside block comment opener", "/{", '/', '*',
			 'c', '\0', '*', '/', '{')

void testEdgeMinifySlicesRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testEdgeMinifySliceBlockObjectLeadingComment);
	RUN_TEST(testEdgeMinifySliceBlockObjectAfterOpenBrace);
	RUN_TEST(testEdgeMinifySliceBlockObjectBetweenKeyAndColon);
	RUN_TEST(testEdgeMinifySliceBlockObjectBetweenColonAndValue);
	RUN_TEST(testEdgeMinifySliceBlockObjectAfterFirstValue);
	RUN_TEST(testEdgeMinifySliceBlockObjectAfterMemberComma);
	RUN_TEST(testEdgeMinifySliceBlockObjectBeforeSecondMemberColon);
	RUN_TEST(testEdgeMinifySliceBlockObjectBeforeSecondMemberValue);
	RUN_TEST(testEdgeMinifySliceBlockObjectBeforeCloseBrace);
	RUN_TEST(testEdgeMinifySliceBlockObjectTrailingComment);
	RUN_TEST(testEdgeMinifySliceLineObjectLeadingComment);
	RUN_TEST(testEdgeMinifySliceLineObjectAfterOpenBrace);
	RUN_TEST(testEdgeMinifySliceLineObjectBetweenKeyAndColon);
	RUN_TEST(testEdgeMinifySliceLineObjectBetweenColonAndValue);
	RUN_TEST(testEdgeMinifySliceLineObjectAfterFirstValue);
	RUN_TEST(testEdgeMinifySliceLineObjectAfterMemberComma);
	RUN_TEST(testEdgeMinifySliceLineObjectBeforeSecondMemberColon);
	RUN_TEST(testEdgeMinifySliceLineObjectBeforeSecondMemberValue);
	RUN_TEST(testEdgeMinifySliceLineObjectBeforeCloseBrace);
	RUN_TEST(testEdgeMinifySliceLineObjectTrailingComment);
	RUN_TEST(testEdgeMinifySliceBlockArrayLeadingComment);
	RUN_TEST(testEdgeMinifySliceBlockArrayAfterOpenBracket);
	RUN_TEST(testEdgeMinifySliceBlockArrayAfterFirstItem);
	RUN_TEST(testEdgeMinifySliceBlockArrayBeforeSecondItem);
	RUN_TEST(testEdgeMinifySliceBlockArrayAfterSecondItem);
	RUN_TEST(testEdgeMinifySliceBlockArrayBeforeThirdItem);
	RUN_TEST(testEdgeMinifySliceBlockArrayBeforeCloseBracket);
	RUN_TEST(testEdgeMinifySliceBlockArrayTrailingComment);
	RUN_TEST(testEdgeMinifySliceBlockArrayMultipleInteriorComments);
	RUN_TEST(testEdgeMinifySliceBlockArrayTailComments);
	RUN_TEST(testEdgeMinifySliceLineArrayLeadingComment);
	RUN_TEST(testEdgeMinifySliceLineArrayAfterOpenBracket);
	RUN_TEST(testEdgeMinifySliceLineArrayAfterFirstItem);
	RUN_TEST(testEdgeMinifySliceLineArrayBeforeSecondItem);
	RUN_TEST(testEdgeMinifySliceLineArrayAfterSecondItem);
	RUN_TEST(testEdgeMinifySliceLineArrayBeforeThirdItem);
	RUN_TEST(testEdgeMinifySliceLineArrayBeforeCloseBracket);
	RUN_TEST(testEdgeMinifySliceLineArrayTrailingComment);
	RUN_TEST(testEdgeMinifySliceLineArrayMultipleInteriorComments);
	RUN_TEST(testEdgeMinifySliceLineArrayTailComments);
	RUN_TEST(testEdgeMinifySliceBlockNestedBeforeArrayValue);
	RUN_TEST(testEdgeMinifySliceBlockNestedInsideArray);
	RUN_TEST(testEdgeMinifySliceBlockNestedBetweenTopLevelMembers);
	RUN_TEST(testEdgeMinifySliceBlockNestedBeforeObjectKey);
	RUN_TEST(testEdgeMinifySliceBlockNestedBeforeObjectValue);
	RUN_TEST(testEdgeMinifySliceBlockNestedLeadingAndTrailingDocComments);
	RUN_TEST(testEdgeMinifySliceBlockNestedBeforeRootClose);
	RUN_TEST(testEdgeMinifySliceBlockNestedAcrossMultipleTokens);
	RUN_TEST(testEdgeMinifySliceBlockNestedBeforeFirstArrayItem);
	RUN_TEST(testEdgeMinifySliceBlockNestedAroundObjectKeyToken);
	RUN_TEST(testEdgeMinifySliceLineNestedBeforeArrayValue);
	RUN_TEST(testEdgeMinifySliceLineNestedInsideArray);
	RUN_TEST(testEdgeMinifySliceLineNestedBetweenTopLevelMembers);
	RUN_TEST(testEdgeMinifySliceLineNestedBeforeObjectValue);
	RUN_TEST(testEdgeMinifySliceLineNestedAfterObjectValue);
	RUN_TEST(testEdgeMinifySliceLineNestedTrailingDocComment);
	RUN_TEST(testEdgeMinifySliceLineNestedLeadingDocComment);
	RUN_TEST(testEdgeMinifySliceLineNestedBeforeObjectKey);
	RUN_TEST(testEdgeMinifySliceLineNestedAfterObjectValueBeforeClose);
	RUN_TEST(testEdgeMinifySliceLineNestedBetweenInnerObjectAndRootClose);
	RUN_TEST(testEdgeMinifySliceStringMarkerLiteralSlashesAndBlock);
	RUN_TEST(testEdgeMinifySliceStringMarkerLiteralSplitBlock);
	RUN_TEST(testEdgeMinifySliceStringMarkerLiteralLineAndEscapedQuote);
	RUN_TEST(testEdgeMinifySliceStringMarkerLiteralEscapedSlashBeforeMarkers);
	RUN_TEST(testEdgeMinifySliceStringMarkerLiteralUrlAndEscapedBlock);
	RUN_TEST(testEdgeMinifySliceStringMarkerLiteralUnicodeSlashes);
	RUN_TEST(testEdgeMinifySliceStringMarkerLiteralUnicodeSlashAndStar);
	RUN_TEST(testEdgeMinifySliceStringMarkerLiteralEscapedQuoteBeforeLine);
	RUN_TEST(testEdgeMinifySliceStringMarkerLiteralMixedBlockAndLine);
	RUN_TEST(testEdgeMinifySliceStringMarkerLiteralTripleEscapedSlash);
	RUN_TEST(testEdgeMinifySliceEmbeddedNulCompleteObjectTail);
	RUN_TEST(testEdgeMinifySliceEmbeddedNulCompleteArrayTail);
	RUN_TEST(testEdgeMinifySliceEmbeddedNulAfterLineCommentObjectStart);
	RUN_TEST(testEdgeMinifySliceEmbeddedNulAfterEscapedQuoteInString);
	RUN_TEST(testEdgeMinifySliceEmbeddedNulAfterBlockCommentBeforeObject);
	RUN_TEST(testEdgeMinifySliceEmbeddedNulAfterObjectComma);
	RUN_TEST(testEdgeMinifySliceEmbeddedNulAfterArrayComma);
	RUN_TEST(testEdgeMinifySliceEmbeddedNulInsideStringLiteral);
	RUN_TEST(testEdgeMinifySliceEmbeddedNulInsideBareString);
	RUN_TEST(testEdgeMinifySliceEmbeddedNulInsideBlockCommentOpener);
}
