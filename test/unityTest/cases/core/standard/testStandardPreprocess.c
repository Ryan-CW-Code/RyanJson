#include "testBase.h"

static void testStandardWhitespaceCharsetAndBomPolicy(void)
{
	// 仅保留标准空白集合的正向语义与 BOM 拒绝策略。
	const char *wrapped = " \n\t\r{\"a\":1}\r\t\n ";
	const char *end = NULL;

	RyanJson_t doc = RyanJsonParseOptions(wrapped, (uint32_t)strlen(wrapped), RyanJsonTrue, &end);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc, "标准空白包裹的 Json 在 strict 终止下应解析成功");
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_CHAR('\0', *end);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(doc, "a")));
	RyanJsonDelete(doc);

	const uint8_t bomText[] = {0xEF, 0xBB, 0xBF, '{', '"', 'a', '"', ':', '1', '}', '\0'};
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse((const char *)bomText), "带 UTF-8 BOM 前缀的文本应解析失败");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParseOptions((const char *)bomText, (uint32_t)(sizeof(bomText) - 1U), RyanJsonTrue, NULL),
				 "带 UTF-8 BOM 前缀的切片在 strict 终止下应解析失败");
}

static void testStandardRejectCommentWithoutMinifyButAcceptAfterMinify(void)
{
	// 标准 Json 不接受注释；Minify 预处理后才可进入正常解析路径。
	char withComment[] = " {\"a\":1,/*comment*/\"b\":\"//keep\"} ";

	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse(withComment), "标准 Json 解析不应接受注释语法");

	uint32_t minLen = RyanJsonMinify(withComment, (int32_t)strlen(withComment));
	TEST_ASSERT_EQUAL_STRING("{\"a\":1,\"b\":\"//keep\"}", withComment);
	TEST_ASSERT_EQUAL_UINT32((uint32_t)strlen(withComment), minLen);

	RyanJson_t doc = RyanJsonParse(withComment);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc, "Minify 预处理后的文本应可解析");
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(doc, "a")));
	TEST_ASSERT_EQUAL_STRING("//keep", RyanJsonGetStringValue(RyanJsonGetObjectToKey(doc, "b")));
	RyanJsonDelete(doc);
}

void testStandardPreprocessRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testStandardWhitespaceCharsetAndBomPolicy);
	RUN_TEST(testStandardRejectCommentWithoutMinifyButAcceptAfterMinify);
}
