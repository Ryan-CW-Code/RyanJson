#include "testBase.h"

static void testLoadFailureNullInput(void)
{
	// 仅验证 API 级别的空指针输入保护（不与 RFC8259 语法样本重复）。
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse(NULL), "Parse(NULL) 应返回 NULL");

	const char *end = (const char *)0x1;
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParseOptions(NULL, 0, RyanJsonFalse, &end), "ParseOptions(NULL) 应返回 NULL");
	TEST_ASSERT_EQUAL_PTR_MESSAGE((const char *)0x1, end, "ParseOptions(NULL) 不应改写 end 指针");
}

static void testLoadFailureWhitespaceOnlySlice(void)
{
	const char *text = " \t\r\n ";

	TEST_ASSERT_NULL_MESSAGE(RyanJsonParseOptions(text, (uint32_t)strlen(text), RyanJsonFalse, NULL),
				 "ParseOptions(仅空白, 非 strict) 应失败");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParseOptions(text, (uint32_t)strlen(text), RyanJsonTrue, NULL),
				 "ParseOptions(仅空白, strict) 应失败");
}

static void testLoadFailureHugeNumberOverflow(void)
{
	// 超长 Int：应在数值累乘过程中触发 isfinite 防御并失败
	const uint32_t intLen = 1024;
	char *hugeInt = (char *)malloc((size_t)intLen + 1U);
	TEST_ASSERT_NOT_NULL(hugeInt);

	hugeInt[0] = '1';
	memset(hugeInt + 1, '9', intLen - 1U);
	hugeInt[intLen] = '\0';
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse(hugeInt), "Parse(超长 Int 溢出) 应返回 NULL");
	free(hugeInt);

	// 超长小数：同样应触发 isfinite 防御并失败
	const uint32_t fracLen = 1024;
	char *hugeFrac = (char *)malloc((size_t)fracLen + 3U);
	TEST_ASSERT_NOT_NULL(hugeFrac);

	hugeFrac[0] = '0';
	hugeFrac[1] = '.';
	memset(hugeFrac + 2, '9', fracLen);
	hugeFrac[fracLen + 2U] = '\0';
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse(hugeFrac), "Parse(超长小数溢出) 应返回 NULL");
	free(hugeFrac);
}

static void testLoadFailureExponentAccumulatorOverflow(void)
{
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("1e2147483647"), "指数过大应触发 isfinite 防御");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("1e2147483648"), "指数累积溢出应解析失败");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("1e-2147483648"), "负指数累积溢出应解析失败");
}

static void testLoadParseOptionsFailure(void)
{
	// 禁止尾部垃圾：requireNullTerminator = true
	const char *text = " {\"a\":1} trailing";
	RyanJson_t json = RyanJsonParseOptions(text, (uint32_t)strlen(text), RyanJsonTrue, NULL);
	TEST_ASSERT_NULL_MESSAGE(json, "ParseOptions(require null terminator) 应失败");
}

static void testLoadParseOptionsSequentialValidThenInvalidIsolation(void)
{
	// 复杂链路：
	// ParseOptions(文档1成功, 非强制) -> ParseOptions(文档2失败) -> 校验文档1语义未被污染。
	const char *stream = "{\"ok\":1}{\"bad\":}";
	const uint32_t len = (uint32_t)strlen(stream);
	const char *end = NULL;

	RyanJson_t first = RyanJsonParseOptions(stream, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL_MESSAGE(first, "顺序流文档1 解析应成功");
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_CHAR('{', *end);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(first, "ok")));

	uint32_t remain = (uint32_t)(len - (uint32_t)(end - stream));
	RyanJson_t second = RyanJsonParseOptions(end, remain, RyanJsonFalse, NULL);
	TEST_ASSERT_NULL_MESSAGE(second, "顺序流文档2（非法）解析应失败");

	RyanJson_t baseline = RyanJsonParse("{\"ok\":1}");
	TEST_ASSERT_NOT_NULL(baseline);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(first, baseline), "文档2 失败后，文档1 语义不应改变");

	RyanJson_t strictWhole = RyanJsonParseOptions(stream, len, RyanJsonTrue, NULL);
	TEST_ASSERT_NULL_MESSAGE(strictWhole, "整串包含非法尾文档时 strict 解析应失败");

	RyanJsonDelete(baseline);
	RyanJsonDelete(first);
}

static void testLoadParseOptionsBinaryTailFailureIsolation(void)
{
	// 复杂链路：
	// ParseOptions(二进制切片, 首文档成功) -> ParseOptions(尾片段失败) -> 校验首文档结果仍稳定。
	const uint8_t buffer[] = {'{', '"', 'a', '"', ':', '1', '}', '#', '!', '\0'};
	const uint32_t bufLen = 9U; // 不包含末尾 '\0'
	const char *end = NULL;

	RyanJson_t head = RyanJsonParseOptions((const char *)buffer, bufLen, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL_MESSAGE(head, "二进制切片首文档解析应成功");
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_CHAR('#', *end);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(head, "a")));

	uint32_t remain = (uint32_t)(bufLen - (uint32_t)(end - (const char *)buffer));
	RyanJson_t tail = RyanJsonParseOptions(end, remain, RyanJsonFalse, NULL);
	TEST_ASSERT_NULL_MESSAGE(tail, "二进制切片尾片段解析应失败");

	// 校验首文档保持稳定。
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(head, "a")));

	RyanJson_t strictWhole = RyanJsonParseOptions((const char *)buffer, bufLen, RyanJsonTrue, NULL);
	TEST_ASSERT_NULL_MESSAGE(strictWhole, "strict 模式应拒绝二进制切片中的尾部垃圾");

	RyanJsonDelete(head);
}

static void testLoadParseOptionsSequentialInvalidThenTailResync(void)
{
	// 复杂链路：
	// ParseOptions(文档1成功) -> ParseOptions(文档2失败) -> 手动重同步到文档3并解析成功。
	const char *stream = "{\"head\":1}{\"bad\":}{\"tail\":3}";
	const uint32_t len = (uint32_t)strlen(stream);
	const char *end = NULL;

	RyanJson_t head = RyanJsonParseOptions(stream, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL_MESSAGE(head, "文档1 解析应成功");
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(head, "head")));

	uint32_t remain = (uint32_t)(len - (uint32_t)(end - stream));
	RyanJson_t bad = RyanJsonParseOptions(end, remain, RyanJsonFalse, NULL);
	TEST_ASSERT_NULL_MESSAGE(bad, "文档2(非法) 解析应失败");

	const char *tailStart = strstr(end, "{\"tail\":3}");
	TEST_ASSERT_NOT_NULL_MESSAGE(tailStart, "应可在原始缓冲中定位到文档3 起点");
	RyanJson_t tail = RyanJsonParseOptions(tailStart, (uint32_t)strlen(tailStart), RyanJsonTrue, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(tail, "文档3 解析应成功");
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(RyanJsonGetObjectToKey(tail, "tail")));

	// 再次确认文档1 未受失败路径影响。
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(head, "head")));

	RyanJsonDelete(tail);
	RyanJsonDelete(head);
}

static void testLoadParseOptionsFailureKeepsEndPtrStable(void)
{
	// 复杂链路：
	// ParseOptions(文档1成功) -> ParseOptions(文档2失败, 传入 parseEndPtr) -> 复用旧偏移重同步文档3。
	const char *stream = "{\"head\":1}{\"bad\":}{\"tail\":2}";
	const uint32_t len = (uint32_t)strlen(stream);
	const char *end1 = NULL;

	RyanJson_t head = RyanJsonParseOptions(stream, len, RyanJsonFalse, &end1);
	TEST_ASSERT_NOT_NULL_MESSAGE(head, "文档1 解析应成功");
	TEST_ASSERT_NOT_NULL(end1);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(head, "head")));

	// 将 parseEndPtr 预置为“文档1结束位置”，用于验证失败调用不会修改它。
	const char *failEnd = end1;
	uint32_t remain = (uint32_t)(len - (uint32_t)(end1 - stream));
	RyanJson_t bad = RyanJsonParseOptions(end1, remain, RyanJsonFalse, &failEnd);
	TEST_ASSERT_NULL_MESSAGE(bad, "文档2(非法) 解析应失败");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(end1, failEnd, "失败解析不应改写调用方的 parseEndPtr");

	// 从保留下来的偏移位置继续重同步到文档3。
	const char *tailStart = strstr(failEnd, "{\"tail\":2}");
	TEST_ASSERT_NOT_NULL_MESSAGE(tailStart, "应可基于 failEnd 重同步定位文档3");
	RyanJson_t tail = RyanJsonParseOptions(tailStart, (uint32_t)strlen(tailStart), RyanJsonTrue, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(tail, "文档3 解析应成功");
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectToKey(tail, "tail")));

	// 再次确认文档1 未被失败路径污染。
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(head, "head")));

	RyanJsonDelete(tail);
	RyanJsonDelete(head);
}

static void testLoadFailureOomParse(void)
{
	const char *longKeyJson =
		"{\"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\":1}";

	UNITY_TEST_OOM_BEGIN(0);
	RyanJson_t json = RyanJsonParse("{\"a\":1}");
	UNITY_TEST_OOM_END();
	if (json) { RyanJsonDelete(json); }
	TEST_ASSERT_NULL_MESSAGE(json, "Parse OOM(根节点分配失败) 应返回 NULL");

	UNITY_TEST_OOM_BEGIN(1); // root 成功，key buffer 失败
	json = RyanJsonParse(longKeyJson);
	UNITY_TEST_OOM_END();
	if (json) { RyanJsonDelete(json); }
	TEST_ASSERT_NULL_MESSAGE(json, "Parse OOM 应返回 NULL");
}

static void testLoadFailureAllocatedKeyCleanupOnValueError(void)
{
	const char *bad = "{\"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\":}";

	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse(bad), "长 key 后 value 非法时应解析失败");
}

static void testLoadParseOptionsSequentialOomRecovery(void)
{
	// 复杂链路：
	// ParseOptions(文档1成功) -> 人工 OOM 让文档2 失败 -> 恢复 hooks 后文档2 再解析成功。
	const char *stream = "{\"a\":1}{\"b\":2}";
	const uint32_t len = (uint32_t)strlen(stream);
	const char *end = NULL;

	RyanJson_t first = RyanJsonParseOptions(stream, len, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL_MESSAGE(first, "文档1 解析应成功");
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(first, "a")));

	uint32_t remain = (uint32_t)(len - (uint32_t)(end - stream));

	UNITY_TEST_OOM_BEGIN(0);
	RyanJson_t secondFail = RyanJsonParseOptions(end, remain, RyanJsonTrue, NULL);
	UNITY_TEST_OOM_END();
	TEST_ASSERT_NULL_MESSAGE(secondFail, "OOM 注入下文档2 解析应失败");

	RyanJson_t second = RyanJsonParseOptions(end, remain, RyanJsonTrue, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(second, "恢复 hooks 后文档2 应可解析成功");
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectToKey(second, "b")));

	// 再确认文档1 不受 OOM 链路污染。
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(first, "a")));

	RyanJsonDelete(second);
	RyanJsonDelete(first);
}

static void testLoadFailureInvalidUtf8AndEmbeddedNull(void)
{
	// 复杂输入场景：
	// - 非法 UTF-8 字节序列（截断、错误续字节、过长编码）；
	// - String 内容中内嵌 NUL（\0）字节。
	// 目标：
	// - 记录当前实现对“非法 UTF-8 原始字节”的行为：按普通字节透传，不在 Parse 阶段拦截；
	// - 验证内嵌 NUL（控制字符）仍会严格失败，防止混入不可见截断字节。
	const uint8_t invalidUtf8Truncated2[] = {'{', '"', 's', '"', ':', '"', 0xC2, '"', '}', '\0'};
	const uint8_t invalidUtf8BadContinuation[] = {'{', '"', 's', '"', ':', '"', 0xE2, 0x28, 0xA1, '"', '}', '\0'};
	const uint8_t invalidUtf8Overlong[] = {'{', '"', 's', '"', ':', '"', 0xC0, 0x80, '"', '}', '\0'};
	const uint8_t invalidUtf8Truncated4[] = {'{', '"', 's', '"', ':', '"', 0xF0, 0x9F, 0x92, '"', '}', '\0'};

	{
		RyanJson_t doc = RyanJsonParseOptions((const char *)invalidUtf8Truncated2, (uint32_t)(sizeof(invalidUtf8Truncated2) - 1U),
						      RyanJsonTrue, NULL);
		TEST_ASSERT_NOT_NULL_MESSAGE(doc, "当前实现下，截断 2 字节 UTF-8 原始字节应被透传解析");
		char *s = RyanJsonGetStringValue(RyanJsonGetObjectToKey(doc, "s"));
		TEST_ASSERT_NOT_NULL(s);
		TEST_ASSERT_EQUAL_UINT8((uint8_t)0xC2, (uint8_t)s[0]);
		TEST_ASSERT_EQUAL_UINT8((uint8_t)0x00, (uint8_t)s[1]);
		RyanJsonDelete(doc);
	}

	{
		RyanJson_t doc = RyanJsonParseOptions((const char *)invalidUtf8BadContinuation,
						      (uint32_t)(sizeof(invalidUtf8BadContinuation) - 1U), RyanJsonTrue, NULL);
		TEST_ASSERT_NOT_NULL_MESSAGE(doc, "当前实现下，错误续字节 UTF-8 原始字节应被透传解析");
		char *s = RyanJsonGetStringValue(RyanJsonGetObjectToKey(doc, "s"));
		TEST_ASSERT_NOT_NULL(s);
		TEST_ASSERT_EQUAL_UINT8((uint8_t)0xE2, (uint8_t)s[0]);
		TEST_ASSERT_EQUAL_UINT8((uint8_t)0x28, (uint8_t)s[1]);
		TEST_ASSERT_EQUAL_UINT8((uint8_t)0xA1, (uint8_t)s[2]);
		TEST_ASSERT_EQUAL_UINT8((uint8_t)0x00, (uint8_t)s[3]);
		RyanJsonDelete(doc);
	}

	{
		RyanJson_t doc = RyanJsonParseOptions((const char *)invalidUtf8Overlong, (uint32_t)(sizeof(invalidUtf8Overlong) - 1U),
						      RyanJsonTrue, NULL);
		TEST_ASSERT_NOT_NULL_MESSAGE(doc, "当前实现下，过长编码 UTF-8 原始字节应被透传解析");
		char *s = RyanJsonGetStringValue(RyanJsonGetObjectToKey(doc, "s"));
		TEST_ASSERT_NOT_NULL(s);
		TEST_ASSERT_EQUAL_UINT8((uint8_t)0xC0, (uint8_t)s[0]);
		TEST_ASSERT_EQUAL_UINT8((uint8_t)0x80, (uint8_t)s[1]);
		TEST_ASSERT_EQUAL_UINT8((uint8_t)0x00, (uint8_t)s[2]);
		RyanJsonDelete(doc);
	}

	{
		RyanJson_t doc = RyanJsonParseOptions((const char *)invalidUtf8Truncated4, (uint32_t)(sizeof(invalidUtf8Truncated4) - 1U),
						      RyanJsonTrue, NULL);
		TEST_ASSERT_NOT_NULL_MESSAGE(doc, "当前实现下，截断 4 字节 UTF-8 原始字节应被透传解析");
		char *s = RyanJsonGetStringValue(RyanJsonGetObjectToKey(doc, "s"));
		TEST_ASSERT_NOT_NULL(s);
		TEST_ASSERT_EQUAL_UINT8((uint8_t)0xF0, (uint8_t)s[0]);
		TEST_ASSERT_EQUAL_UINT8((uint8_t)0x9F, (uint8_t)s[1]);
		TEST_ASSERT_EQUAL_UINT8((uint8_t)0x92, (uint8_t)s[2]);
		TEST_ASSERT_EQUAL_UINT8((uint8_t)0x00, (uint8_t)s[3]);
		RyanJsonDelete(doc);
	}

	const uint8_t embeddedNulInString[] = {'{', '"', 's', '"', ':', '"', 'a', 'b', '\0', 'c', 'd', '"', '}', '\0'};
	TEST_ASSERT_NULL_MESSAGE(
		RyanJsonParseOptions((const char *)embeddedNulInString, (uint32_t)(sizeof(embeddedNulInString) - 1U), RyanJsonTrue, NULL),
		"String 内嵌 NUL 且 strict 终止时应解析失败");
	TEST_ASSERT_NULL_MESSAGE(
		RyanJsonParseOptions((const char *)embeddedNulInString, (uint32_t)(sizeof(embeddedNulInString) - 1U), RyanJsonFalse, NULL),
		"String 内嵌 NUL 且流式解析时也应解析失败");
}

void testLoadFailureRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testLoadFailureNullInput);
	RUN_TEST(testLoadFailureWhitespaceOnlySlice);
	RUN_TEST(testLoadFailureHugeNumberOverflow);
	RUN_TEST(testLoadFailureExponentAccumulatorOverflow);
	RUN_TEST(testLoadParseOptionsFailure);
	RUN_TEST(testLoadParseOptionsSequentialValidThenInvalidIsolation);
	RUN_TEST(testLoadParseOptionsBinaryTailFailureIsolation);
	RUN_TEST(testLoadParseOptionsSequentialInvalidThenTailResync);
	RUN_TEST(testLoadParseOptionsFailureKeepsEndPtrStable);
	RUN_TEST(testLoadFailureOomParse);
	RUN_TEST(testLoadFailureAllocatedKeyCleanupOnValueError);
	RUN_TEST(testLoadParseOptionsSequentialOomRecovery);
	RUN_TEST(testLoadFailureInvalidUtf8AndEmbeddedNull);
}
