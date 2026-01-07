#include "RyanJsonBaseTest.h"

// ========== 简单字符串（使用X-macro） ==========
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

// ========== 转义字符（需分离JSON和值） ==========
typedef struct
{
	const char *json;     // JSON字符串（带转义）
	const char *expected; // 期望的C字符串值
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

// 字符串一致性测试
RyanJsonBool_e RyanJsonBaseTestEqualityString(void)
{
	// ========== 测试简单字符串 ==========
	for (uint32_t i = 0; i < sizeof(SimpleStringValueTable) / sizeof(SimpleStringValueTable[0]); i++)
	{
		const char *jsonStrInput = SimpleStringJsonTable[i];
		RyanJson_t jsonRoot = RyanJsonParse(jsonStrInput);
		RyanJsonCheckReturnFalse(NULL != jsonRoot);
		RyanJsonCheckReturnFalse(RyanJsonIsString(RyanJsonGetObjectToKey(jsonRoot, "str")));

		const char *strValue = RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "str"));
		RyanJsonCheckCode(0 == strcmp(strValue, SimpleStringValueTable[i]), {
			jsonLog("simple str failed: expected: %s, got: %s\n", SimpleStringValueTable[i], strValue);
			RyanJsonDelete(jsonRoot);
			goto err;
		});

		// 往返测试
		char *serializedStr = RyanJsonPrint(jsonRoot, 128, RyanJsonFalse, NULL);
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);
		RyanJsonCheckReturnFalse(NULL != roundtripJson);

		const char *roundtripValue = RyanJsonGetStringValue(RyanJsonGetObjectToKey(roundtripJson, "str"));
		RyanJsonCheckCode(0 == strcmp(roundtripValue, SimpleStringValueTable[i]), {
			jsonLog("simple roundtrip failed: expected: %s, got: %s\n", SimpleStringValueTable[i], roundtripValue);
			RyanJsonDelete(roundtripJson);
			goto err;
		});

		RyanJsonDelete(roundtripJson);
	}

	// ========== 测试转义字符 ==========
	for (uint32_t i = 0; i < sizeof(EscapeTestCases) / sizeof(EscapeTestCases[0]); i++)
	{
		const EscapeTestCase *tc = &EscapeTestCases[i];
		RyanJson_t jsonRoot = RyanJsonParse(tc->json);
		RyanJsonCheckCode(NULL != jsonRoot, {
			jsonLog("escape parse failed: %s\n", tc->json);
			goto err;
		});
		RyanJsonCheckReturnFalse(RyanJsonIsString(RyanJsonGetObjectToKey(jsonRoot, "str")));

		const char *strValue = RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "str"));
		RyanJsonCheckCode(0 == strcmp(strValue, tc->expected), {
			jsonLog("escape str failed: json=%s, expected=%s, got=%s\n", tc->json, tc->expected, strValue);
			RyanJsonDelete(jsonRoot);
			goto err;
		});

		// 往返测试
		char *serializedStr = RyanJsonPrint(jsonRoot, 128, RyanJsonFalse, NULL);
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);
		RyanJsonCheckCode(NULL != roundtripJson, {
			jsonLog("escape roundtrip parse failed\n");
			goto err;
		});

		const char *roundtripValue = RyanJsonGetStringValue(RyanJsonGetObjectToKey(roundtripJson, "str"));
		RyanJsonCheckCode(0 == strcmp(roundtripValue, tc->expected), {
			jsonLog("escape roundtrip failed: expected=%s, got=%s\n", tc->expected, roundtripValue);
			RyanJsonDelete(roundtripJson);
			goto err;
		});

		RyanJsonDelete(roundtripJson);
	}

	return RyanJsonTrue;

err:
	return RyanJsonFalse;
}
