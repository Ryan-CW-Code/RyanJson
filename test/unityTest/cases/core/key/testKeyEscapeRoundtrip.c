#include "testBase.h"

static void testKeyEscapePrintRoundtripQuote(void)
{
	// 复杂链路：
	// Parse(含引号 key) -> Print -> Parse -> Compare。
	// 目标：验证 key 中的 \" 被正确转义与往返保持一致。
	RyanJson_t root = RyanJsonParse("{\"\\\"\":1}");
	TEST_ASSERT_NOT_NULL(root);

	char *out = RyanJsonPrint(root, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("{\"\\\"\":1}", out);

	RyanJson_t roundtrip = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(out);
	RyanJsonDelete(root);
}

static void testKeyEscapePrintRoundtripBackslash(void)
{
	// 复杂链路：
	// Parse(含反斜杠 key) -> Print -> Parse -> Compare。
	// 目标：验证 key 中的 \\ 被正确转义与往返保持一致。
	RyanJson_t root = RyanJsonParse("{\"\\\\\":1}");
	TEST_ASSERT_NOT_NULL(root);

	char *out = RyanJsonPrint(root, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("{\"\\\\\":1}", out);

	RyanJson_t roundtrip = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(out);
	RyanJsonDelete(root);
}

static void testKeyEscapePrintRoundtripSolidus(void)
{
	// 复杂链路：
	// Parse(含斜杠 key) -> Print -> Parse -> Compare。
	// 目标：验证 key 中的 \\/ 会被稳定输出并往返一致。
	RyanJson_t root = RyanJsonParse("{\"\\/\":1}");
	TEST_ASSERT_NOT_NULL(root);

	char *out = RyanJsonPrint(root, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(out);
	// 当前实现不会强制转义 '/', 输出应为 "/"
	TEST_ASSERT_EQUAL_STRING("{\"/\":1}", out);

	RyanJson_t roundtrip = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(out);
	RyanJsonDelete(root);
}

static void testKeyEscapePrintRoundtripTab(void)
{
	// 复杂链路：
	// Parse(含制表符 key) -> Print -> Parse -> Compare。
	// 目标：验证 key 中的 \t 输出为转义序列并往返一致。
	RyanJson_t root = RyanJsonParse("{\"\\t\":1}");
	TEST_ASSERT_NOT_NULL(root);

	char *out = RyanJsonPrint(root, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("{\"\\t\":1}", out);

	RyanJson_t roundtrip = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(out);
	RyanJsonDelete(root);
}

static void testKeyEscapePrintRoundtripNewline(void)
{
	// 复杂链路：
	// Parse(含换行 key) -> Print -> Parse -> Compare。
	// 目标：验证 key 中的 \n 输出为转义序列并往返一致。
	RyanJson_t root = RyanJsonParse("{\"\\n\":1}");
	TEST_ASSERT_NOT_NULL(root);

	char *out = RyanJsonPrint(root, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("{\"\\n\":1}", out);

	RyanJson_t roundtrip = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(out);
	RyanJsonDelete(root);
}

static void testKeyEscapePrintRoundtripCarriageReturn(void)
{
	// 复杂链路：
	// Parse(含回车 key) -> Print -> Parse -> Compare。
	// 目标：验证 key 中的 \r 输出为转义序列并往返一致。
	RyanJson_t root = RyanJsonParse("{\"\\r\":1}");
	TEST_ASSERT_NOT_NULL(root);

	char *out = RyanJsonPrint(root, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("{\"\\r\":1}", out);

	RyanJson_t roundtrip = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(out);
	RyanJsonDelete(root);
}

static void testKeyEscapePrintRoundtripBackspace(void)
{
	// 复杂链路：
	// Parse(含退格 key) -> Print -> Parse -> Compare。
	// 目标：验证 key 中的 \b 输出为转义序列并往返一致。
	RyanJson_t root = RyanJsonParse("{\"\\b\":1}");
	TEST_ASSERT_NOT_NULL(root);

	char *out = RyanJsonPrint(root, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("{\"\\b\":1}", out);

	RyanJson_t roundtrip = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(out);
	RyanJsonDelete(root);
}

static void testKeyEscapePrintRoundtripUnicodeControl(void)
{
	// 复杂链路：
	// Parse(含 \\u0001 key) -> Print -> Parse -> Compare。
	// 目标：验证低位控制字符会被输出为 \\uXXXX。
	RyanJson_t root = RyanJsonParse("{\"\\u0001\":1}");
	TEST_ASSERT_NOT_NULL(root);

	char *out = RyanJsonPrint(root, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("{\"\\u0001\":1}", out);

	RyanJson_t roundtrip = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(out);
	RyanJsonDelete(root);
}

static void testStringValueEscapePrintRoundtrip(void)
{
	// 复杂链路：
	// Parse(含 \\u0001 的字符串值) -> Print -> Parse -> Compare。
	// 目标：验证低位控制字符在 value 中的转义与往返一致性（避免与基础转义矩阵重复）。
	RyanJson_t root = RyanJsonParse("{\"v\":\"\\u0001\"}");
	TEST_ASSERT_NOT_NULL(root);

	char *out = RyanJsonPrint(root, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("{\"v\":\"\\u0001\"}", out);

	RyanJson_t roundtrip = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(out);
	RyanJsonDelete(root);
}

static void testKeyEscapePrintRoundtripUtf8(void)
{
	// 复杂链路：
	// Parse(\\u4E2D) -> Print -> Parse -> Compare。
	// 目标：验证 \\uXXXX 会被解码为 UTF-8 并按原字节打印输出。
	RyanJson_t root = RyanJsonParse("{\"\\u4E2D\":1}");
	TEST_ASSERT_NOT_NULL(root);

	char *out = RyanJsonPrint(root, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(out);

	// 注意：C 字符串中 \\x 会吞掉后续十六进制字符，需拆分字面量。
	char expect[] = "{\"\xE4"
			"\xB8"
			"\xAD\":1}";
	TEST_ASSERT_EQUAL_STRING(expect, out);

	RyanJson_t roundtrip = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(out);
	RyanJsonDelete(root);
}

void testKeyEscapeRoundtripRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testKeyEscapePrintRoundtripQuote);
	RUN_TEST(testKeyEscapePrintRoundtripBackslash);
	RUN_TEST(testKeyEscapePrintRoundtripSolidus);
	RUN_TEST(testKeyEscapePrintRoundtripTab);
	RUN_TEST(testKeyEscapePrintRoundtripNewline);
	RUN_TEST(testKeyEscapePrintRoundtripCarriageReturn);
	RUN_TEST(testKeyEscapePrintRoundtripBackspace);
	RUN_TEST(testKeyEscapePrintRoundtripUnicodeControl);
	RUN_TEST(testStringValueEscapePrintRoundtrip);
	RUN_TEST(testKeyEscapePrintRoundtripUtf8);
}
