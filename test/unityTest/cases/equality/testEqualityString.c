#include "testBase.h"

/**
 * @brief 简单字符串样例（X-macro）
 */
#define SimpleStringList                                                                                                                   \
	X("")                                                                                                                              \
	X("hello")                                                                                                                         \
	X("world")                                                                                                                         \
	X("test")                                                                                                                          \
	X("RyanJson")                                                                                                                      \
	X("123")                                                                                                                           \
	X("0")                                                                                                                             \
	X("-1")                                                                                                                            \
	X("3.14")                                                                                                                          \
	X("1e10")                                                                                                                          \
	X("hello world")                                                                                                                   \
	X("path/to/file")                                                                                                                  \
	X("abcdefghijklmnopqrstuvwxyz")                                                                                                    \
	X("ABCDEFGHIJKLMNOPQRSTUVWXYZ")                                                                                                    \
	X("0123456789")                                                                                                                    \
	X("The quick brown fox jumps over the lazy dog")                                                                                   \
	X(" ")                                                                                                                             \
	X("  ")                                                                                                                            \
	X("   leading")                                                                                                                    \
	X("trailing   ")                                                                                                                   \
	X("  both  ")                                                                                                                      \
	X("true")                                                                                                                          \
	X("false")                                                                                                                         \
	X("null")                                                                                                                          \
	X("@#$%^&*()")                                                                                                                     \
	X("!@#$%")                                                                                                                         \
	X("a=b&c=d")                                                                                                                       \
	X("user@example.com")                                                                                                              \
	X("中文测试")                                                                                                                      \
	X("日本語テスト")                                                                                                                  \
	X("한국어테스트")                                                                                                                  \
	X("混合Mixed混合")                                                                                                                 \
	X("Привет мир")                                                                                                                    \
	X("مرحبا بالعالم")                                                                                                                 \
	X("שלום עולם")

static const char *SimpleStringValueTable[] = {
#define X(a) a,
	SimpleStringList
#undef X
};

static const char *SimpleStringJsonTable[] = {
#define X(a) "{\"str\":\"" a "\"}",
	SimpleStringList
#undef X
};

/**
 * @brief 转义字符串样例（Json 文本与期望值分离）
 */
typedef struct
{
	const char *json;     // Json 字符串（带转义）
	const char *expected; // 期望的 C strValue
} EscapeTestCase;

static const EscapeTestCase EscapeTestCases[] = {
	// 制表符
	{"{\"str\":\"hello\\tworld\"}", "hello\tworld"},
	{"{\"str\":\"tab\\there\"}", "tab\there"},
	// 换行符
	{"{\"str\":\"hello\\nworld\"}", "hello\nworld"},
	{"{\"str\":\"line1\\nline2\\nline3\"}", "line1\nline2\nline3"},
	// 回车符
	{"{\"str\":\"hello\\rworld\"}", "hello\rworld"},
	// 引号转义
	{"{\"str\":\"quote\\\"inside\"}", "quote\"inside"},
	{"{\"str\":\"say \\\"hello\\\"\"}", "say \"hello\""},
	// 反斜杠转义
	{"{\"str\":\"backslash\\\\here\"}", "backslash\\here"},
	{"{\"str\":\"C:\\\\Windows\\\\System32\"}", "C:\\Windows\\System32"},
	{"{\"str\":\"path\\\\to\\\\file\"}", "path\\to\\file"},
	// 斜杠（可选转义）
	{"{\"str\":\"a\\/b\"}", "a/b"},
	// 退格符
	{"{\"str\":\"back\\bspace\"}", "back\bspace"},
	// 换页符
	{"{\"str\":\"form\\ffeed\"}", "form\ffeed"},
	// 组合转义
	{"{\"str\":\"line1\\nline2\\ttab\"}", "line1\nline2\ttab"},
	{"{\"str\":\"\\\"quoted\\\" and \\\\escaped\\\\\"}", "\"quoted\" and \\escaped\\"},
	// Unicode转义
	{"{\"str\":\"\\u0048\\u0065\\u006C\\u006C\\u006F\"}", "Hello"},
	{"{\"str\":\"\\u4E2D\\u6587\"}", "中文"},
	{"{\"str\":\"euro: \\u20AC\"}", "euro: €"},
	{"{\"str\":\"smile: \\u263A\"}", "smile: ☺"},
};

/**
 * @brief 字符串类型边界与类型一致性测试
 */
void testEqualityStringEdgeCases(void)
{
	// NULL 输入
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsString(NULL), "RyanJsonIsString(NULL) 应返回 false");

	// 类型混淆测试
	RyanJson_t num = RyanJsonCreateInt("num", 123);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsString(num), "RyanJsonIsString(Int) 应返回 false");
	RyanJsonDelete(num);

	RyanJson_t dbl = RyanJsonCreateDouble("dbl", 1.23);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsString(dbl), "RyanJsonIsString(Double) 应返回 false");
	RyanJsonDelete(dbl);

	RyanJson_t boolNode = RyanJsonCreateBool("bool", RyanJsonTrue);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsString(boolNode), "RyanJsonIsString(Bool) 应返回 false");
	RyanJsonDelete(boolNode);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsString(obj), "RyanJsonIsString(Object) 应返回 false");
	RyanJsonDelete(obj);

	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsString(arr), "RyanJsonIsString(Array) 应返回 false");
	RyanJsonDelete(arr);

	RyanJson_t nullNode = RyanJsonCreateNull("null");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsString(nullNode), "RyanJsonIsString(Null) 应返回 false");
	RyanJsonDelete(nullNode);
}

/**
 * @brief 简单字符串解析与往返测试
 */
static void testEqualityStringSimple(void)
{
	for (uint32_t i = 0; i < sizeof(SimpleStringValueTable) / sizeof(SimpleStringValueTable[0]); i++)
	{
		const char *jsonStrInput = SimpleStringJsonTable[i];
		RyanJson_t jsonRoot = RyanJsonParse(jsonStrInput);
		TEST_ASSERT_NOT_NULL_MESSAGE(jsonRoot, "简单字符串解析失败");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(RyanJsonGetObjectToKey(jsonRoot, "str")), "字段 'str' 不是字符串类型");

		const char *strValue = RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "str"));
		TEST_ASSERT_EQUAL_STRING_MESSAGE(SimpleStringValueTable[i], strValue, jsonStrInput);

		// 往返测试
		char *serializedStr = RyanJsonPrint(jsonRoot, 128, RyanJsonFalse, NULL);
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);
		TEST_ASSERT_NOT_NULL_MESSAGE(roundtripJson, "简单字符串往返测试：重新解析失败");

		const char *roundtripValue = RyanJsonGetStringValue(RyanJsonGetObjectToKey(roundtripJson, "str"));
		TEST_ASSERT_EQUAL_STRING_MESSAGE(SimpleStringValueTable[i], roundtripValue, "简单字符串往返测试不匹配");

		RyanJsonDelete(roundtripJson);
	}
}

/**
 * @brief 转义字符串解析与往返测试
 */
static void testEqualityStringEscape(void)
{
	for (uint32_t i = 0; i < sizeof(EscapeTestCases) / sizeof(EscapeTestCases[0]); i++)
	{
		const EscapeTestCase *tc = &EscapeTestCases[i];
		RyanJson_t jsonRoot = RyanJsonParse(tc->json);
		TEST_ASSERT_NOT_NULL_MESSAGE(jsonRoot, tc->json);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(RyanJsonGetObjectToKey(jsonRoot, "str")), "转义字段 'str' 类型错误");

		const char *strValue = RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "str"));
		TEST_ASSERT_EQUAL_STRING_MESSAGE(tc->expected, strValue, tc->json);

		// 往返测试
		char *serializedStr = RyanJsonPrint(jsonRoot, 128, RyanJsonFalse, NULL);
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);
		TEST_ASSERT_NOT_NULL_MESSAGE(roundtripJson, "转义字符串往返测试：重新解析失败");

		const char *roundtripValue = RyanJsonGetStringValue(RyanJsonGetObjectToKey(roundtripJson, "str"));
		TEST_ASSERT_EQUAL_STRING_MESSAGE(tc->expected, roundtripValue, "转义字符串往返测试不匹配");

		RyanJsonDelete(roundtripJson);
	}
}

void testEqualityStringRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testEqualityStringEdgeCases);
	RUN_TEST(testEqualityStringSimple);
	RUN_TEST(testEqualityStringEscape);
}
