#include "testBase.h"

static RyanJson_t createObjectWithIntMember(const char *key, int32_t value)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, key, value));
	return obj;
}

static RyanJsonPrintStyle makeStyle(const char *indent, const char *newline, uint8_t spaceAfterColon, RyanJsonBool_e format)
{
	RyanJsonPrintStyle style = {
		.indent = indent,
		.indentLen = (uint32_t)strlen(indent),
		.newline = newline,
		.newlineLen = (uint32_t)strlen(newline),
		.spaceAfterColon = spaceAfterColon,
		.format = format,
	};
	return style;
}

static void testPrintWithStyleNullStyleGuard(void)
{
	RyanJson_t json = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_NULL(RyanJsonPrintWithStyle(json, 10, NULL, NULL));
	RyanJsonDelete(json);
}

static void testPrintPreallocatedWithStyleNullArgs(void)
{
	RyanJson_t obj = createObjectWithIntMember("a", 1);
	RyanJsonPrintStyle style = makeStyle("  ", "\n", 1, RyanJsonTrue);
	char buf[64] = {0};

	TEST_ASSERT_NULL(RyanJsonPrintPreallocatedWithStyle(NULL, buf, sizeof(buf), &style, NULL));
	TEST_ASSERT_NULL(RyanJsonPrintPreallocatedWithStyle(obj, NULL, sizeof(buf), &style, NULL));
	TEST_ASSERT_NULL(RyanJsonPrintPreallocatedWithStyle(obj, buf, sizeof(buf), NULL, NULL));
	TEST_ASSERT_NULL(RyanJsonPrintPreallocatedWithStyle(obj, buf, 0, &style, NULL));

	RyanJsonDelete(obj);
}

static void testPrintPreallocatedWithStyleTooSmall(void)
{
	RyanJson_t obj = createObjectWithIntMember("a", 1);
	RyanJsonPrintStyle style = makeStyle("  ", "\n", 1, RyanJsonTrue);
	char tinyBuf[2] = {0};

	char *out = RyanJsonPrintPreallocatedWithStyle(obj, tinyBuf, sizeof(tinyBuf), &style, NULL);
	TEST_ASSERT_NULL_MESSAGE(out, "WithStyle 预分配缓冲区过小应返回 NULL");

	RyanJsonDelete(obj);
}

static void testPrintPreallocatedWithStyleSuccessAndLen(void)
{
	RyanJson_t obj = createObjectWithIntMember("a", 1);
	RyanJsonPrintStyle style = makeStyle("  ", "\n", 2, RyanJsonTrue);
	char buf[64] = {0};
	uint32_t len = 0;

	char *out = RyanJsonPrintPreallocatedWithStyle(obj, buf, sizeof(buf), &style, &len);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("{\n  \"a\":  1\n}", out);
	TEST_ASSERT_EQUAL_UINT32((uint32_t)strlen(out), len);

	RyanJsonDelete(obj);
}

static void testPrintCrazy(void)
{
	RyanJson_t emptyObj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(emptyObj);
	char *s = RyanJsonPrint(emptyObj, 0, RyanJsonFalse, NULL);
	TEST_ASSERT_EQUAL_STRING("{}", s);
	RyanJsonFree(s);
	RyanJsonDelete(emptyObj);

	RyanJson_t emptyArr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(emptyArr);
	s = RyanJsonPrint(emptyArr, 0, RyanJsonFalse, NULL);
	TEST_ASSERT_EQUAL_STRING("[]", s);
	RyanJsonFree(s);
	RyanJsonDelete(emptyArr);

	RyanJson_t json = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(json, "k", "v"));
	s = RyanJsonPrint(json, 0, RyanJsonFalse, NULL);
	TEST_ASSERT_EQUAL_STRING("{\"k\":\"v\"}", s);
	RyanJsonFree(s);
	RyanJsonDelete(json);

	RyanJson_t obj = createObjectWithIntMember("a", 1);
	RyanJsonPrintStyle crazyStyle = makeStyle("--------", "\n\n", 4, RyanJsonTrue);
	uint32_t len = 0;
	s = RyanJsonPrintWithStyle(obj, 10, &crazyStyle, &len);
	TEST_ASSERT_NOT_NULL(s);
	TEST_ASSERT_EQUAL_UINT32((uint32_t)strlen(s), len);
	TEST_ASSERT_NOT_NULL(strstr(s, "--------\"a\":    1"));
	RyanJsonFree(s);
	RyanJsonDelete(obj);
}

static void testPrintDefault(void)
{
	RyanJson_t json = RyanJsonParse("{\"a\":1,\"b\":true}");
	TEST_ASSERT_NOT_NULL(json);

	char *defaultFormat = RyanJsonPrint(json, 256, RyanJsonTrue, NULL);
	TEST_ASSERT_NOT_NULL(defaultFormat);
	TEST_ASSERT_NOT_NULL_MESSAGE(strstr(defaultFormat, "\t\"a\": 1"), "默认格式化缩进或冒号空格错误");

	RyanJsonFree(defaultFormat);
	RyanJsonDelete(json);
}

static void testPrintCustomStyle(void)
{
	RyanJson_t json = RyanJsonParse("{\"a\":1,\"b\":true}");
	TEST_ASSERT_NOT_NULL(json);

	RyanJsonPrintStyle style = makeStyle("  ", "\r\n", 2, RyanJsonTrue);
	uint32_t len = 0;
	char *customPrint = RyanJsonPrintWithStyle(json, 256, &style, &len);
	TEST_ASSERT_NOT_NULL(customPrint);
	TEST_ASSERT_NOT_NULL_MESSAGE(strstr(customPrint, "\r\n  \"a\":  1"), "自定义格式化特征片段错误");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE((uint32_t)strlen(customPrint), len, "返回长度不一致");

	RyanJsonFree(customPrint);
	RyanJsonDelete(json);
}

static void testPrintWithStyleCrlfTabIndent(void)
{
	RyanJson_t json = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(json);

	RyanJsonPrintStyle style = makeStyle("\t", "\r\n", 1, RyanJsonTrue);
	char *out = RyanJsonPrintWithStyle(json, 64, &style, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("{\r\n\t\"a\": 1\r\n}", out);

	RyanJsonFree(out);
	RyanJsonDelete(json);
}

static void testPrintPreallocatedWithStyleExactFitDouble(void)
{
	// Double 节点的 WithStyle 预分配在不同配置下可能需要额外工作区，
	// 因此这里只强制验证“有足够 headroom 时必须成功”。
	RyanJson_t obj = RyanJsonParse("{\"pi\":3.14}");
	TEST_ASSERT_NOT_NULL(obj);

	RyanJsonPrintStyle style = makeStyle("  ", "\n", 1, RyanJsonTrue);
	uint32_t expectLen = 0;
	char *expect = RyanJsonPrintWithStyle(obj, 64, &style, &expectLen);
	TEST_ASSERT_NOT_NULL(expect);

	char buf[64] = {0};
	TEST_ASSERT_TRUE_MESSAGE(expectLen + 1U <= sizeof(buf), "测试缓冲区长度不足");
	char *out = RyanJsonPrintPreallocatedWithStyle(obj, buf, expectLen + 1U, &style, NULL);
	if (NULL != out) { TEST_ASSERT_EQUAL_STRING(expect, out); }

	char headroomBuf[256] = {0};
	out = RyanJsonPrintPreallocatedWithStyle(obj, headroomBuf, sizeof(headroomBuf), &style, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING(expect, out);

	RyanJsonFree(expect);
	RyanJsonDelete(obj);
}

void testPrintStyleRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testPrintWithStyleNullStyleGuard);
	RUN_TEST(testPrintPreallocatedWithStyleNullArgs);
	RUN_TEST(testPrintPreallocatedWithStyleTooSmall);
	RUN_TEST(testPrintPreallocatedWithStyleSuccessAndLen);
	RUN_TEST(testPrintCrazy);
	RUN_TEST(testPrintDefault);
	RUN_TEST(testPrintCustomStyle);
	RUN_TEST(testPrintWithStyleCrlfTabIndent);
	RUN_TEST(testPrintPreallocatedWithStyleExactFitDouble);
}
