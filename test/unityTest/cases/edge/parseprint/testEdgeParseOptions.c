#include "testBase.h"

static void testEdgeParsePrintRequireTerminatorRejectTail(void)
{
	// 复杂链路：
	// ParseOptions(requireNullTerminator=true, 尾部垃圾) -> 失败。
	// 目标：验证严格尾部校验会拒绝非空白尾部。
	const char *text = "{\"a\":1}x";
	const uint32_t len = (uint32_t)strlen(text);
	const char *sentinel = "sentinel";
	const char *end = sentinel;

	RyanJson_t json = RyanJsonParseOptions(text, len, RyanJsonTrue, &end);
	TEST_ASSERT_NULL(json);
	TEST_ASSERT_EQUAL_PTR(sentinel, end);
}

static void testEdgeParsePrintRequireTerminatorAcceptWhitespace(void)
{
	// 复杂链路：
	// ParseOptions(requireNullTerminator=true, 尾部空白) -> 成功。
	// 目标：验证严格模式允许尾部空白。
	const char *text = "{\"a\":1}\n\t  ";
	const uint32_t len = (uint32_t)strlen(text);
	const char *end = NULL;

	RyanJson_t json = RyanJsonParseOptions(text, len, RyanJsonTrue, &end);
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_EQUAL_PTR(text + len, end);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByKey(json, "a")));

	RyanJsonDelete(json);
}

static void testEdgeParsePrintParseEndPtrSequentialObjectArray(void)
{
	// 复杂链路：
	// ParseOptions(对象) -> parseEndPtr -> ParseOptions(数组)。
	// 目标：验证 end 指针可驱动顺序解析。
	const char *text = " {\"a\":1} [2,3]";
	const uint32_t len = (uint32_t)strlen(text);
	const char *end = NULL;

	RyanJson_t first = RyanJsonParseOptions(text, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL(first);
	TEST_ASSERT_NOT_NULL(end);

	uint32_t remain = (uint32_t)(len - (uint32_t)(end - text));
	RyanJson_t second = RyanJsonParseOptions(end, remain, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(second);
	TEST_ASSERT_TRUE(RyanJsonIsArray(second));
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(second));

	RyanJsonDelete(second);
	RyanJsonDelete(first);
}

static void testEdgeParsePrintParseEndPtrSequentialScalarObject(void)
{
	// 复杂链路：
	// ParseOptions(标量) -> parseEndPtr -> ParseOptions(对象)。
	// 目标：验证标量后续解析可继续推进。
	const char *text = "true{\"b\":2}";
	const uint32_t len = (uint32_t)strlen(text);
	const char *end = NULL;

	RyanJson_t first = RyanJsonParseOptions(text, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL(first);
	TEST_ASSERT_TRUE(RyanJsonIsBool(first));

	uint32_t remain = (uint32_t)(len - (uint32_t)(end - text));
	RyanJson_t second = RyanJsonParseOptions(end, remain, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(second);
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectByKey(second, "b")));

	RyanJsonDelete(second);
	RyanJsonDelete(first);
}

static void testEdgeParsePrintParseEndPtrStringWithBracesThenObject(void)
{
	// 复杂链路：
	// ParseOptions(字符串含 '}'/'{'/转义引号) -> parseEndPtr -> ParseOptions(对象)。
	// 目标：验证字符串内容不会干扰 end 指针定位。
	const char *text = "\"a}b{\\\"c\\\"\"{\"k\":1}";
	const uint32_t len = (uint32_t)strlen(text);
	const char *end = NULL;

	RyanJson_t first = RyanJsonParseOptions(text, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL(first);
	TEST_ASSERT_TRUE(RyanJsonIsString(first));
	TEST_ASSERT_EQUAL_STRING("a}b{\"c\"", RyanJsonGetStringValue(first));
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_CHAR('{', *end);

	uint32_t remain = (uint32_t)(len - (uint32_t)(end - text));
	RyanJson_t second = RyanJsonParseOptions(end, remain, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(second);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByKey(second, "k")));

	RyanJsonDelete(second);
	RyanJsonDelete(first);
}

static void testEdgeParsePrintParseEndPtrStringWithEscapesThenArray(void)
{
	// 复杂链路：
	// ParseOptions(含 \\\" 与 \\\\ 的字符串) -> parseEndPtr -> ParseOptions(数组)。
	// 目标：验证转义序列不会干扰 end 指针定位。
	const char *text = "\"a\\\"b\\\\c\"[1,2]";
	const uint32_t len = (uint32_t)strlen(text);
	const char *end = NULL;

	RyanJson_t first = RyanJsonParseOptions(text, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL(first);
	TEST_ASSERT_TRUE(RyanJsonIsString(first));
	TEST_ASSERT_EQUAL_STRING("a\"b\\c", RyanJsonGetStringValue(first));
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_CHAR('[', *end);

	uint32_t remain = (uint32_t)(len - (uint32_t)(end - text));
	RyanJson_t second = RyanJsonParseOptions(end, remain, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(second);
	TEST_ASSERT_TRUE(RyanJsonIsArray(second));
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(second));

	RyanJsonDelete(second);
	RyanJsonDelete(first);
}

static void testEdgeParsePrintParseEndPtrStringWithUnicodeEscapeThenObject(void)
{
	// 复杂链路：
	// ParseOptions(含 \\uXXXX 字符串) -> parseEndPtr -> ParseOptions(对象)。
	// 目标：验证 Unicode 转义解码不影响 end 指针。
	const char *text = "\"\\u4E2D\"{\"k\":1}";
	const uint32_t len = (uint32_t)strlen(text);
	const char *end = NULL;

	RyanJson_t first = RyanJsonParseOptions(text, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL(first);
	TEST_ASSERT_TRUE(RyanJsonIsString(first));
	TEST_ASSERT_EQUAL_STRING("\xE4\xB8\xAD", RyanJsonGetStringValue(first));
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_CHAR('{', *end);

	uint32_t remain = (uint32_t)(len - (uint32_t)(end - text));
	RyanJson_t second = RyanJsonParseOptions(end, remain, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(second);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByKey(second, "k")));

	RyanJsonDelete(second);
	RyanJsonDelete(first);
}

static void testEdgeParsePrintParseEndPtrStringWithSurrogateEscapeThenBool(void)
{
	// 复杂链路：
	// ParseOptions(含代理对字符串) -> parseEndPtr -> ParseOptions(布尔)。
	// 目标：验证代理对解码不影响 end 指针。
	const char *text = "\"\\uD83D\\uDE03\" true";
	const uint32_t len = (uint32_t)strlen(text);
	const char *end = NULL;

	RyanJson_t first = RyanJsonParseOptions(text, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL(first);
	TEST_ASSERT_TRUE(RyanJsonIsString(first));
	TEST_ASSERT_EQUAL_STRING("\xF0\x9F\x98\x83", RyanJsonGetStringValue(first));
	TEST_ASSERT_NOT_NULL(end);

	uint32_t remain = (uint32_t)(len - (uint32_t)(end - text));
	RyanJson_t second = RyanJsonParseOptions(end, remain, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(second);
	TEST_ASSERT_TRUE(RyanJsonIsBool(second));
	TEST_ASSERT_TRUE(RyanJsonGetBoolValue(second));

	RyanJsonDelete(second);
	RyanJsonDelete(first);
}

static void testEdgeParsePrintMinifyStripComments(void)
{
	// 复杂链路：
	// Minify(包含注释) -> 结果校验。
	// 目标：验证注释与空白被正确剔除。
	char buf[] = " { /*c*/ \"a\" : 1 , //x\n \"b\" : 2 } ";
	uint32_t len = RyanJsonMinify(buf, (int32_t)(sizeof(buf) - 1U));
	buf[len] = '\0';
	TEST_ASSERT_EQUAL_STRING("{\"a\":1,\"b\":2}", buf);
}

static void testEdgeParsePrintMinifyPreserveSlashInString(void)
{
	// 复杂链路：
	// Minify(字符串含注释样式) -> 结果校验。
	// 目标：验证字符串内的 // 与 /* */ 不被误删。
	char buf[] = "{\"pat\":\"a//b\",\"block\":\"x/*y*/z\"}";
	uint32_t len = RyanJsonMinify(buf, (int32_t)(sizeof(buf) - 1U));
	buf[len] = '\0';
	TEST_ASSERT_EQUAL_STRING("{\"pat\":\"a//b\",\"block\":\"x/*y*/z\"}", buf);
}

static void testEdgeParsePrintMinifyPreserveEscapedQuote(void)
{
	// 复杂链路：
	// Minify(含转义引号与 //) -> 结果校验。
	// 目标：验证转义序列不被误处理。
	char buf[] = "{\"msg\":\"he said: \\\"//not\\\"\"}";
	uint32_t len = RyanJsonMinify(buf, (int32_t)(sizeof(buf) - 1U));
	buf[len] = '\0';
	TEST_ASSERT_EQUAL_STRING("{\"msg\":\"he said: \\\"//not\\\"\"}", buf);
}

static void testEdgeParsePrintMinifyPreserveEscapedCommentMarkers(void)
{
	// 复杂链路：
	// Minify(字符串含 \\// 与 \\/* */) -> 结果校验。
	// 目标：验证转义反斜杠 + 注释标记组合不会被误删。
	char buf[] = "/*h*/{\"s\":\"x\\\\//y\",\"t\":\"u\\\\/*v*/w\",\"q\":\"\\\\\\\"/*\"}//t\n";
	uint32_t len = RyanJsonMinify(buf, (int32_t)(sizeof(buf) - 1U));
	buf[len] = '\0';
	TEST_ASSERT_EQUAL_STRING("{\"s\":\"x\\\\//y\",\"t\":\"u\\\\/*v*/w\",\"q\":\"\\\\\\\"/*\"}", buf);
}

static void testEdgeParsePrintParseOptionsSizeZeroFailure(void)
{
	// 复杂链路：
	// ParseOptions(size=0) -> 失败 -> end 指针不应被污染。
	const char *text = "{}";
	const char *sentinel = "sentinel";
	const char *end = sentinel;

	RyanJson_t json = RyanJsonParseOptions(text, 0, RyanJsonFalse, &end);
	TEST_ASSERT_NULL_MESSAGE(json, "ParseOptions(size=0) 应失败");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(sentinel, end, "ParseOptions 失败时不应修改 end 指针");
}

static void testEdgeParsePrintParseOptionsTruncatedScalar(void)
{
	// 复杂链路：
	// ParseOptions(被截断的 true) -> 失败 -> end 指针不应被污染。
	const char *text = "true";
	const char *sentinel = "sentinel";
	const char *end = sentinel;

	RyanJson_t json = RyanJsonParseOptions(text, 3, RyanJsonFalse, &end); // "tru"
	TEST_ASSERT_NULL_MESSAGE(json, "ParseOptions(截断标量) 应失败");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(sentinel, end, "ParseOptions 失败时不应修改 end 指针");
}

static void testEdgeParsePrintParseOptionsTruncatedStringBySize(void)
{
	// 复杂链路：
	// ParseOptions(被截断的字符串) -> 失败 -> end 指针不应被污染。
	const char *text = "\"ok\"";
	const char *sentinel = "sentinel";
	const char *end = sentinel;

	RyanJson_t json = RyanJsonParseOptions(text, 3, RyanJsonFalse, &end); // "\"ok"
	TEST_ASSERT_NULL_MESSAGE(json, "ParseOptions(截断字符串) 应失败");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(sentinel, end, "ParseOptions 失败时不应修改 end 指针");
}

static void testEdgeParsePrintParseOptionsTruncatedUnicodeBySize(void)
{
	// 复杂链路：
	// ParseOptions(截断 Unicode 转义) -> 失败 -> end 指针不应被污染。
	const char *text = "\"\\u1234\"";
	const char *sentinel = "sentinel";
	const char *end = sentinel;

	RyanJson_t json = RyanJsonParseOptions(text, 5, RyanJsonFalse, &end); // "\"\\u12"
	TEST_ASSERT_NULL_MESSAGE(json, "ParseOptions(截断 Unicode 转义) 应失败");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(sentinel, end, "ParseOptions 失败时不应修改 end 指针");
}

static void testEdgeParsePrintParseOptionsTruncatedArray(void)
{
	// 复杂链路：
	// ParseOptions(截断数组) -> 失败 -> end 指针不变。
	// 目标：验证截断数组被拒绝。
	const char *text = "[1,2";
	const char *sentinel = "sentinel";
	const char *end = sentinel;

	RyanJson_t json = RyanJsonParseOptions(text, 4, RyanJsonFalse, &end);
	TEST_ASSERT_NULL(json);
	TEST_ASSERT_EQUAL_PTR(sentinel, end);
}

static void testEdgeParsePrintMinifyThenParseRoundtrip(void)
{
	// 复杂链路：
	// Minify -> Parse -> Print -> Parse -> Compare。
	// 目标：验证 Minify 后的文档可稳定回环。
	char raw[] = "{\n\t\"a\" : 1 , /*c*/ \"b\" : [ true , false ] }";
	uint32_t len = RyanJsonMinify(raw, (int32_t)(sizeof(raw) - 1U));
	raw[len] = '\0';

	RyanJson_t root = RyanJsonParse(raw);
	TEST_ASSERT_NOT_NULL(root);
	char *printed = RyanJsonPrint(root, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testEdgeParsePrintParseOptionsLeadingWhitespaceEndPtr(void)
{
	// 复杂链路：
	// ParseOptions(含前导空白) -> parseEndPtr -> 解析第二文档。
	// 目标：验证前导空白不影响 end 推进。
	const char *text = "  {\"a\":1}{\"b\":2}";
	const uint32_t len = (uint32_t)strlen(text);
	const char *end = NULL;

	RyanJson_t first = RyanJsonParseOptions(text, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL(first);

	uint32_t remain = (uint32_t)(len - (uint32_t)(end - text));
	RyanJson_t second = RyanJsonParseOptions(end, remain, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(second);

	RyanJsonDelete(second);
	RyanJsonDelete(first);
}

static void testEdgeParsePrintMinifyEmbeddedTabs(void)
{
	// 复杂链路：
	// Minify(含 \t \r) -> 结果校验。
	// 目标：验证制表符与回车被剔除。
	char buf[] = "\t{\r\n\"a\" : 1\t}\r";
	uint32_t len = RyanJsonMinify(buf, (int32_t)(sizeof(buf) - 1U));
	buf[len] = '\0';
	TEST_ASSERT_EQUAL_STRING("{\"a\":1}", buf);
}

static void testEdgeParsePrintParseOptionsGarbageAfterArray(void)
{
	// 复杂链路：
	// ParseOptions(数组) -> end 指向垃圾 -> 续读校验。
	// 目标：验证垃圾段不被误解析。
	const char *text = "[1,2]x{\"b\":1}";
	const uint32_t len = (uint32_t)strlen(text);
	const char *end = NULL;

	RyanJson_t first = RyanJsonParseOptions(text, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL(first);
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_CHAR('x', *end);

	uint32_t remain = (uint32_t)(len - (uint32_t)(end - text));
	RyanJson_t bad = RyanJsonParseOptions(end, remain, RyanJsonFalse, NULL);
	TEST_ASSERT_NULL(bad);

	const char *secondStart = end + 1;
	uint32_t remain2 = (uint32_t)((text + len) - secondStart);
	RyanJson_t second = RyanJsonParseOptions(secondStart, remain2, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(second);

	RyanJsonDelete(second);
	RyanJsonDelete(first);
}

static void testEdgeParsePrintStrictTailCommentReject(void)
{
	// 复杂链路：
	// ParseOptions(requireNullTerminator=true, 尾部注释) -> 失败。
	// 目标：验证严格模式拒绝非空白尾部。
	const char *text = "{\"a\":1}/*x*/";
	const uint32_t len = (uint32_t)strlen(text);
	const char *sentinel = "sentinel";
	const char *end = sentinel;

	RyanJson_t json = RyanJsonParseOptions(text, len, RyanJsonTrue, &end);
	TEST_ASSERT_NULL(json);
	TEST_ASSERT_EQUAL_PTR(sentinel, end);
}

static void testEdgeParsePrintParseEndPtrTrailingWhitespace(void)
{
	// 复杂链路：
	// ParseOptions(requireNullTerminator=false, 尾部空白) -> end 指向空白。
	// 目标：验证 parseEndPtr 停留在空白起始位置。
	const char *text = "{\"a\":1}   ";
	const uint32_t len = (uint32_t)strlen(text);
	const char *end = NULL;

	RyanJson_t json = RyanJsonParseOptions(text, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_CHAR(' ', *end);

	RyanJsonDelete(json);
}

static void testEdgeParsePrintParseEndPtrAfterWhitespacePrefix(void)
{
	// 复杂链路：
	// ParseOptions(前导空白) -> parseEndPtr -> 指向后续文本。
	// 目标：验证前导空白不影响 end 指针推进。
	const char *text = "  {\"a\":1}tail";
	const uint32_t len = (uint32_t)strlen(text);
	const char *end = NULL;

	RyanJson_t json = RyanJsonParseOptions(text, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_STRING("tail", end);

	RyanJsonDelete(json);
}

static void testEdgeParsePrintParseOptionsFailDoesNotWriteEnd(void)
{
	// 复杂链路：
	// ParseOptions(失败) -> end 指针不变。
	// 目标：验证失败路径不污染 end。
	const char *text = "{";
	const char *sentinel = "sentinel";
	const char *end = sentinel;

	RyanJson_t json = RyanJsonParseOptions(text, 1, RyanJsonFalse, &end);
	TEST_ASSERT_NULL(json);
	TEST_ASSERT_EQUAL_PTR(sentinel, end);
}

static void testEdgeParsePrintParseOptionsSecondFailEndUnchanged(void)
{
	// 复杂链路：
	// ParseOptions(首段成功) -> ParseOptions(二段失败) -> end 不污染。
	// 目标：验证二段失败不会修改 end 指针。
	const char *text = "{\"a\":1}{\"b\":2}";
	const uint32_t len = (uint32_t)strlen(text);
	const char *end = NULL;

	RyanJson_t first = RyanJsonParseOptions(text, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL(first);
	TEST_ASSERT_NOT_NULL(end);

	const char *sentinel = end;
	RyanJson_t second = RyanJsonParseOptions(end, 2, RyanJsonFalse, &end);
	TEST_ASSERT_NULL(second);
	TEST_ASSERT_EQUAL_PTR(sentinel, end);

	RyanJsonDelete(first);
}

void testEdgeParseOptionsRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testEdgeParsePrintRequireTerminatorRejectTail);
	RUN_TEST(testEdgeParsePrintRequireTerminatorAcceptWhitespace);
	RUN_TEST(testEdgeParsePrintParseEndPtrSequentialObjectArray);
	RUN_TEST(testEdgeParsePrintParseEndPtrSequentialScalarObject);
	RUN_TEST(testEdgeParsePrintParseEndPtrStringWithBracesThenObject);
	RUN_TEST(testEdgeParsePrintParseEndPtrStringWithEscapesThenArray);
	RUN_TEST(testEdgeParsePrintParseEndPtrStringWithUnicodeEscapeThenObject);
	RUN_TEST(testEdgeParsePrintParseEndPtrStringWithSurrogateEscapeThenBool);
	RUN_TEST(testEdgeParsePrintMinifyStripComments);
	RUN_TEST(testEdgeParsePrintMinifyPreserveSlashInString);
	RUN_TEST(testEdgeParsePrintMinifyPreserveEscapedQuote);
	RUN_TEST(testEdgeParsePrintMinifyPreserveEscapedCommentMarkers);
	RUN_TEST(testEdgeParsePrintParseOptionsSizeZeroFailure);
	RUN_TEST(testEdgeParsePrintParseOptionsTruncatedScalar);
	RUN_TEST(testEdgeParsePrintParseOptionsTruncatedStringBySize);
	RUN_TEST(testEdgeParsePrintParseOptionsTruncatedUnicodeBySize);
	RUN_TEST(testEdgeParsePrintParseOptionsTruncatedArray);
	RUN_TEST(testEdgeParsePrintMinifyThenParseRoundtrip);
	RUN_TEST(testEdgeParsePrintParseOptionsLeadingWhitespaceEndPtr);
	RUN_TEST(testEdgeParsePrintMinifyEmbeddedTabs);
	RUN_TEST(testEdgeParsePrintParseOptionsGarbageAfterArray);
	RUN_TEST(testEdgeParsePrintStrictTailCommentReject);
	RUN_TEST(testEdgeParsePrintParseEndPtrTrailingWhitespace);
	RUN_TEST(testEdgeParsePrintParseEndPtrAfterWhitespacePrefix);
	RUN_TEST(testEdgeParsePrintParseOptionsFailDoesNotWriteEnd);
	RUN_TEST(testEdgeParsePrintParseOptionsSecondFailEndUnchanged);
}
