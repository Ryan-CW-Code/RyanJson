#include "testBase.h"

static void testStandardParseOptionsMixedTopLevelSequence(void)
{
	// 覆盖 object -> string -> null -> array 的混合顶层流式推进。
	const char *stream = " {\"a\":1}\n \"txt\" \n null \n [1,{\"k\":2}] ";
	const uint32_t streamLen = (uint32_t)strlen(stream);

	const char *end1 = NULL;
	RyanJson_t doc1 = RyanJsonParseOptions(stream, streamLen, RyanJsonFalse, &end1);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc1, "混合序列文档#1(object) 解析失败");
	TEST_ASSERT_NOT_NULL(end1);
	TEST_ASSERT_TRUE(RyanJsonIsObject(doc1));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(doc1, "a")));

	uint32_t remain1 = (uint32_t)(streamLen - (uint32_t)(end1 - stream));
	const char *end2 = NULL;
	RyanJson_t doc2 = RyanJsonParseOptions(end1, remain1, RyanJsonFalse, &end2);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc2, "混合序列文档#2(string) 解析失败");
	TEST_ASSERT_NOT_NULL(end2);
	TEST_ASSERT_TRUE(RyanJsonIsString(doc2));
	TEST_ASSERT_EQUAL_STRING("txt", RyanJsonGetStringValue(doc2));

	uint32_t remain2 = (uint32_t)(streamLen - (uint32_t)(end2 - stream));
	const char *end3 = NULL;
	RyanJson_t doc3 = RyanJsonParseOptions(end2, remain2, RyanJsonFalse, &end3);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc3, "混合序列文档#3(null) 解析失败");
	TEST_ASSERT_NOT_NULL(end3);
	TEST_ASSERT_TRUE(RyanJsonIsNull(doc3));

	uint32_t remain3 = (uint32_t)(streamLen - (uint32_t)(end3 - stream));
	const char *end4 = NULL;
	RyanJson_t doc4 = RyanJsonParseOptions(end3, remain3, RyanJsonTrue, &end4);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc4, "混合序列文档#4(array) 解析失败");
	TEST_ASSERT_NOT_NULL(end4);
	TEST_ASSERT_TRUE(RyanJsonIsArray(doc4));
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(doc4));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(doc4, 0)));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectToKey(RyanJsonGetObjectByIndex(doc4, 1), "k")));
	TEST_ASSERT_EQUAL_CHAR('\0', *end4);

	RyanJsonDelete(doc1);
	RyanJsonDelete(doc2);
	RyanJsonDelete(doc3);
	RyanJsonDelete(doc4);
}

static void testStandardParseOptionsTruncatedSliceBehavior(void)
{
	// 覆盖 ParseOptions 的 size 截断语义与完整切片语义。
	const char *doc = "{\"a\":1}";
	const uint32_t fullLen = (uint32_t)strlen(doc);
	const char *end = NULL;

	RyanJson_t truncated = RyanJsonParseOptions(doc, fullLen - 1U, RyanJsonFalse, &end);
	TEST_ASSERT_NULL_MESSAGE(truncated, "截断切片不应解析成功");

	RyanJson_t exact = RyanJsonParseOptions(doc, fullLen, RyanJsonTrue, &end);
	TEST_ASSERT_NOT_NULL_MESSAGE(exact, "完整切片应解析成功");
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_CHAR('\0', *end);
	TEST_ASSERT_TRUE(RyanJsonIsObject(exact));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(exact, "a")));

	RyanJsonDelete(exact);
}

static void testStandardParseOptionsWithoutEndPointer(void)
{
	// 覆盖 parseEndPtr == NULL 时 strict / non-strict 的分流。
	const char *text = " {\"a\":1} trailing";
	const uint32_t textLen = (uint32_t)strlen(text);

	RyanJson_t doc = RyanJsonParseOptions(text, textLen, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc, "parseEndPtr=NULL + 非强制模式应解析成功");
	TEST_ASSERT_TRUE(RyanJsonIsObject(doc));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(doc, "a")));
	RyanJsonDelete(doc);

	doc = RyanJsonParseOptions(text, textLen, RyanJsonTrue, NULL);
	TEST_ASSERT_NULL_MESSAGE(doc, "parseEndPtr=NULL + 强制模式应拒绝尾部垃圾");
}

static void testStandardParseOptionsStreamDocsStayIndependentAfterMutation(void)
{
	// 覆盖顺序取出多文档后，首文档修改不应污染后续文档。
	const char *stream = "{\"a\":[1,2]}{\"b\":true}";
	const uint32_t len = (uint32_t)strlen(stream);
	const char *end = NULL;

	RyanJson_t doc1 = RyanJsonParseOptions(stream, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc1, "流文档#1 解析失败");
	TEST_ASSERT_NOT_NULL(end);

	RyanJson_t doc2 = RyanJsonParseOptions(end, (uint32_t)(len - (uint32_t)(end - stream)), RyanJsonTrue, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc2, "流文档#2 解析失败");

	RyanJson_t arr = RyanJsonGetObjectToKey(doc1, "a");
	TEST_ASSERT_NOT_NULL(arr);
	RyanJson_t detached = RyanJsonDetachByIndex(arr, 0);
	TEST_ASSERT_NOT_NULL(detached);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeIntValue(detached, 9), "修改分离数组元素失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(arr, 1, detached), "回插修改后的数组元素失败");

	RyanJson_t expect = RyanJsonParse("{\"a\":[2,9]}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(doc1, expect), "首文档变更结果与期望文档不一致");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonGetBoolValue(RyanJsonGetObjectToKey(doc2, "b")), "第二文档应保持原布尔值");

	RyanJsonDelete(expect);
	RyanJsonDelete(doc2);
	RyanJsonDelete(doc1);
}

void testStandardStreamRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testStandardParseOptionsMixedTopLevelSequence);
	RUN_TEST(testStandardParseOptionsTruncatedSliceBehavior);
	RUN_TEST(testStandardParseOptionsWithoutEndPointer);
	RUN_TEST(testStandardParseOptionsStreamDocsStayIndependentAfterMutation);
}
