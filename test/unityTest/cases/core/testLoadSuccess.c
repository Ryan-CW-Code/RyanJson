#include "testBase.h"

static void testLoadRootZero(void)
{
	RyanJson_t json = RyanJsonParse("0");
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "Parse 数字 \"0\" 作为根节点 应正常");
	if (json)
	{
		TEST_ASSERT_TRUE(RyanJsonIsInt(json));
		TEST_ASSERT_EQUAL_INT(0, RyanJsonGetIntValue(json));
		RyanJsonDelete(json);
	}

	json = RyanJsonParse("-0");
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "Parse 数字 \"-0\" 作为根节点 应正常");
	if (json)
	{
		TEST_ASSERT_TRUE(RyanJsonIsInt(json));
		TEST_ASSERT_EQUAL_INT(0, RyanJsonGetIntValue(json));
		RyanJsonDelete(json);
	}
}

static void testLoadUtf8(void)
{
	// UTF-8 边界测试
	// 双字节字符（© -> \xC2\xA9）
	RyanJson_t json = RyanJsonParse("{\"c\":\"\xC2\xA9\"}");
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_EQUAL_STRING("\xC2\xA9", RyanJsonGetStringValue(RyanJsonGetObjectToKey(json, "c")));
	RyanJsonDelete(json);

	// 三字节字符（中 -> \xE4\xB8\xAD）
	json = RyanJsonParse("{\"z\":\"\xE4\xB8\xAD\"}");
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_EQUAL_STRING("\xE4\xB8\xAD", RyanJsonGetStringValue(RyanJsonGetObjectToKey(json, "z")));
	RyanJsonDelete(json);

	// 四字节字符（Emoji 🐂 -> \xF0\x9F\x90\x82）
	json = RyanJsonParse("{\"e\":\"\xF0\x9F\x90\x82\"}");
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_EQUAL_STRING("\xF0\x9F\x90\x82", RyanJsonGetStringValue(RyanJsonGetObjectToKey(json, "e")));
	RyanJsonDelete(json);
}

static void testLoadStandardObject(void)
{
	char *str = NULL;
	RyanJson_t json;
	char *jsonstr = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":"
			"{\"inter\":16,\"double\":16."
			"89,\"string\":\"hello\","
			"\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,"
			"16.89,16.89,16.89],"
			"\"arrayString\":[\"hello\",\"hello\","
			"\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			"\"double\":16.89,\"string\":"
			"\"hello\",\"boolTrue\":true,"
			"\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			"\"boolFalse\":false,\"null\":null}],\"unicode\":\"😀\"}";

	// 标准对象加载测试
	json = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "解析基础 Json 失败");

	str = RyanJsonPrint(json, 250, RyanJsonFalse, NULL);
	TEST_ASSERT_EQUAL_STRING_MESSAGE(jsonstr, str, "打印生成的字符串与原始字符串不匹配");
	RyanJsonFree(str);

	// 使用公共验证函数进一步检查
	testCheckRoot(json);
	RyanJsonDelete(json);
}

static void testLoadUnicodeValid(void)
{
	char printfBuf[256] = {0};
	char *str = NULL;
	RyanJson_t json;

	// Emoji 测试
	json = RyanJsonParse("{\"emoji\":\"\\uD83D\\uDE00\"}");
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "解析 Unicode Emoji 失败");
	str = RyanJsonPrintPreallocated(json, printfBuf, sizeof(printfBuf), RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(str, "打印 Unicode Emoji 失败");
	RyanJsonDelete(json);

	// 测试数字 0-9 分支: \u0030 = '0', \u0039 = '9'
	json = RyanJsonParse("{\"num\":\"\\u0030\\u0039\"}");
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "解析 Unicode 数字失败");
	str = RyanJsonPrintPreallocated(json, printfBuf, sizeof(printfBuf), RyanJsonFalse, NULL);
	TEST_ASSERT_EQUAL_STRING_MESSAGE("{\"num\":\"09\"}", str, "Unicode 数字解析/打印错误");
	RyanJsonDelete(json);

	// 测试小写 a-f 分支: \u0061 = 'a', \u0066 = 'f'
	json = RyanJsonParse("{\"lower\":\"\\u0061\\u0062\\u0063\\u0064\\u0065\\u0066\"}");
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "解析 Unicode 小写字母失败");
	str = RyanJsonPrintPreallocated(json, printfBuf, sizeof(printfBuf), RyanJsonFalse, NULL);
	TEST_ASSERT_EQUAL_STRING_MESSAGE("{\"lower\":\"abcdef\"}", str, "Unicode 小写字母解析/打印错误");
	RyanJsonDelete(json);

	// 测试大写 A-F 分支: \u0041 = 'A', \u0046 = 'F'
	json = RyanJsonParse("{\"upper\":\"\\u0041\\u0042\\u0043\\u0044\\u0045\\u0046\"}");
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "解析 Unicode 大写字母失败");
	str = RyanJsonPrintPreallocated(json, printfBuf, sizeof(printfBuf), RyanJsonFalse, NULL);
	TEST_ASSERT_EQUAL_STRING_MESSAGE("{\"upper\":\"ABCDEF\"}", str, "Unicode 大写字母解析/打印错误");
	RyanJsonDelete(json);

	// 测试混合大小写: \uAbCd
	json = RyanJsonParse("{\"mixed\":\"\\uAbCd\"}");
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "解析 Unicode 混合大小写失败");
	RyanJsonFree(RyanJsonPrint(json, 50, RyanJsonFalse, NULL));
	RyanJsonDelete(json);
}

static void testLoadBoundaryConditionsSuccess(void)
{
	RyanJson_t json;

	// 空结构
	json = RyanJsonParse("{}");
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_TRUE(RyanJsonIsObject(json));
	TEST_ASSERT_NULL(RyanJsonGetObjectValue(json));
	RyanJsonDelete(json);

	json = RyanJsonParse("[]");
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_TRUE(RyanJsonIsArray(json));
	TEST_ASSERT_NULL(RyanJsonGetArrayValue(json));
	RyanJsonDelete(json);

	// 极端空白字符
	const char *wsJson = "  \n\t  {  \r\n  \"key\"  :  [  \t  ]  \n  }  \r  ";
	json = RyanJsonParse(wsJson);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "极端空白字符解析失败");
	TEST_ASSERT_NOT_NULL(RyanJsonGetObjectToKey(json, "key"));
	RyanJsonDelete(json);

	// 空 key 和空 strValue
	json = RyanJsonParse("{\"\": \"\"}");
	TEST_ASSERT_NOT_NULL(json);
	RyanJson_t emptyNode = RyanJsonGetObjectValue(json);
	TEST_ASSERT_NOT_NULL(emptyNode);
	TEST_ASSERT_EQUAL_STRING("", RyanJsonGetKey(emptyNode));
	TEST_ASSERT_EQUAL_STRING("", RyanJsonGetStringValue(emptyNode));
	RyanJsonDelete(json);

	// 极长 key
	char longKeyJson[1024];
	snprintf(longKeyJson, sizeof(longKeyJson), "{\"%s\":1}",
		 "a_very_long_key_padding_........................................................................");
	json = RyanJsonParse(longKeyJson);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "Parse(长 Key) 失败");
	RyanJsonDelete(json);

	// 纯标量测试
	json = RyanJsonParse("\"just a string\"");
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_TRUE(RyanJsonIsString(json));
	TEST_ASSERT_EQUAL_STRING("just a string", RyanJsonGetStringValue(json));
	RyanJsonDelete(json);

	json = RyanJsonParse("123.456");
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_TRUE(RyanJsonIsDouble(json));
	RyanJsonDelete(json);

	// 包含 \0 的输入 (应在 \0 处停止或报错，取决于解析逻辑)
	// RyanJsonParse 使用 strlen 确定长度，所以会自动在第一个 \0 处截断
	json = RyanJsonParse("{\"a\":1}\0{\"b\":2}");
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_NULL(RyanJsonGetObjectToKey(json, "b"));
	RyanJsonDelete(json);
}

static void testLoadParseOptionsSuccess(void)
{
	const char *end = NULL;

	// 允许尾部内容：requireNullTerminator = false
	const char *text = " {\"a\":1} trailing";
	RyanJson_t json = RyanJsonParseOptions(text, (uint32_t)strlen(text), RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "ParseOptions(允许尾部) 失败");
	TEST_ASSERT_NOT_NULL_MESSAGE(end, "parseEndPtr 不应为 NULL");
	TEST_ASSERT_EQUAL_STRING_MESSAGE(" trailing", end, "parseEndPtr 位置错误");
	RyanJsonDelete(json);

	// 仅包含空白尾部：应成功，parseEndPtr 应指向末尾
	text = "{\"a\":1}   \t\r\n";
	json = RyanJsonParseOptions(text, (uint32_t)strlen(text), RyanJsonTrue, &end);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "ParseOptions(空白尾部) 失败");
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_CHAR('\0', *end);
	RyanJsonDelete(json);

	// 限长解析：仅解析前半段
	const char *concat = "{\"a\":1}{\"b\":2}";
	uint32_t firstLen = (uint32_t)strlen("{\"a\":1}");
	end = NULL;
	json = RyanJsonParseOptions(concat, firstLen, RyanJsonTrue, &end);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "ParseOptions(限长解析) 失败");
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_STRING_MESSAGE("{\"b\":2}", end, "限长解析 parseEndPtr 错误");
	RyanJsonDelete(json);
}

static void testLoadParseOptionsBinaryTail(void)
{
	// 包含内嵌 '\0' 与后续数据，验证 size 驱动的解析行为
	const char rawText[] = {'{', '\"', 'a', '\"', ':', '1', '}', '\0', '{', '\"', 'b', '\"', ':', '2', '}', '\0'};
	const char *end = NULL;

	RyanJson_t json = RyanJsonParseOptions(rawText, (uint32_t)(sizeof(rawText) - 1U), RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "ParseOptions(含二进制尾部, 允许尾部) 应成功");
	TEST_ASSERT_NOT_NULL(end);
	TEST_ASSERT_EQUAL_PTR(rawText + 7, end);
	TEST_ASSERT_EQUAL_CHAR('\0', *end);
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(json, "a")));
	RyanJsonDelete(json);

	// requireNullTerminator=true 时，内嵌 '\0' 后仍有剩余数据，应失败
	json = RyanJsonParseOptions(rawText, (uint32_t)(sizeof(rawText) - 1U), RyanJsonTrue, &end);
	TEST_ASSERT_NULL_MESSAGE(json, "ParseOptions(含二进制尾部, 强制结尾) 应失败");
}

static void testLoadNumberBoundaries(void)
{
	RyanJson_t json = RyanJsonParse("{\"i\":2147483647,\"i2\":-2147483648,\"i3\":2147483648,\"n\":-0}");
	TEST_ASSERT_NOT_NULL(json);

	RyanJson_t i = RyanJsonGetObjectToKey(json, "i");
	RyanJson_t i2 = RyanJsonGetObjectToKey(json, "i2");
	RyanJson_t i3 = RyanJsonGetObjectToKey(json, "i3");
	RyanJson_t n = RyanJsonGetObjectToKey(json, "n");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(i), "2147483647 应解析为 int32_t");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(i2), "-2147483648 应解析为 int32_t");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDouble(i3), "2147483648 应解析为 double");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(n), "-0 应解析为 int32_t");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, RyanJsonGetIntValue(n), "-0 值错误");

	RyanJsonDelete(json);

	// 极大负指数会下溢到 0（有限数），应作为合法数字解析成功
	json = RyanJsonParse("1e-2147483647");
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "1e-2147483647 应解析成功");
	if (json)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDouble(json), "1e-2147483647 应解析为 double");
		RyanJsonDelete(json);
	}
}

static void testLoadScientificNumberRoundtrip(void)
{
	const char *jsonText = "{\"a\":1e0,\"b\":1E+2,\"c\":-2.5e-3,\"d\":0e+1}";
	RyanJson_t json = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "科学计数法解析失败");

	RyanJson_t a = RyanJsonGetObjectToKey(json, "a");
	RyanJson_t b = RyanJsonGetObjectToKey(json, "b");
	RyanJson_t c = RyanJsonGetObjectToKey(json, "c");
	RyanJson_t d = RyanJsonGetObjectToKey(json, "d");

	TEST_ASSERT_TRUE(RyanJsonIsDouble(a));
	TEST_ASSERT_TRUE(RyanJsonIsDouble(b));
	TEST_ASSERT_TRUE(RyanJsonIsDouble(c));
	TEST_ASSERT_TRUE(RyanJsonIsDouble(d));

	TEST_ASSERT_TRUE(RyanJsonCompareDouble(1.0, RyanJsonGetDoubleValue(a)));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(100.0, RyanJsonGetDoubleValue(b)));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(-0.0025, RyanJsonGetDoubleValue(c)));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(0.0, RyanJsonGetDoubleValue(d)));

	char *printed = RyanJsonPrint(json, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);

	RyanJson_t roundtrip = RyanJsonParse(printed);
	RyanJsonFree(printed);
	TEST_ASSERT_NOT_NULL_MESSAGE(roundtrip, "科学计数法往返解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(json, roundtrip), "科学计数法往返后 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonDelete(json);
}

static void testLoadDuplicateKeyScopeIsolation(void)
{
	// 允许不同作用域使用同名 key（仅同一 Object 作用域内禁止重复）
	const char *jsonText = "{\"a\":1,\"obj\":{\"a\":2},\"arr\":[{\"a\":3},{\"a\":4}]}";
	RyanJson_t json = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "不同作用域同名 key 应解析成功");

	if (json)
	{
		RyanJson_t arr = RyanJsonGetObjectToKey(json, "arr");
		RyanJson_t arrObj0 = RyanJsonGetObjectByIndex(arr, 0);
		RyanJson_t arrObj1 = RyanJsonGetObjectByIndex(arr, 1);

		TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectToKey(json, "a")));
		TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectToKey(json, "obj", "a")));
		TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(RyanJsonGetObjectToKey(arrObj0, "a")));
		TEST_ASSERT_EQUAL_INT(4, RyanJsonGetIntValue(RyanJsonGetObjectToKey(arrObj1, "a")));
		RyanJsonDelete(json);
	}
}

void testLoadSuccessRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testLoadRootZero);
	RUN_TEST(testLoadUtf8);
	RUN_TEST(testLoadStandardObject);
	RUN_TEST(testLoadUnicodeValid);
	RUN_TEST(testLoadBoundaryConditionsSuccess);
	RUN_TEST(testLoadParseOptionsSuccess);
	RUN_TEST(testLoadParseOptionsBinaryTail);
	RUN_TEST(testLoadNumberBoundaries);
	RUN_TEST(testLoadScientificNumberRoundtrip);
	RUN_TEST(testLoadDuplicateKeyScopeIsolation);
}
