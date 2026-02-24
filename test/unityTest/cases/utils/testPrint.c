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

void testPrintStyleEdgeCases(void)
{
	// NULL 输入
	TEST_ASSERT_NULL(RyanJsonPrint(NULL, 10, RyanJsonTrue, NULL));

	RyanJson_t json = RyanJsonCreateObject();
	TEST_ASSERT_NULL(RyanJsonPrintWithStyle(json, 10, NULL, NULL));

	// format=false 但提供打印风格
	RyanJsonPrintStyle style = makeStyle("  ", "\n", 1, RyanJsonFalse);
	char *minified = RyanJsonPrintWithStyle(json, 10, &style, NULL);
	TEST_ASSERT_NOT_NULL(minified);
	TEST_ASSERT_EQUAL_STRING("{}", minified); // 应为压缩输出，不包含空格与换行
	RyanJsonFree(minified);

	// 极端小缓冲区（验证内部自动扩容）
	// RyanJsonPrint 内部会强制最小缓冲区尺寸，这里只需验证行为正确且不崩溃
	RyanJsonAddIntToObject(json, "a", 1);
	char *smallBufPrint = RyanJsonPrint(json, 1, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(smallBufPrint);
	TEST_ASSERT_EQUAL_STRING("{\"a\":1}", smallBufPrint);
	RyanJsonFree(smallBufPrint);

	RyanJsonDelete(json);
}

static void testPrintPreallocatedTooSmall(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	RyanJsonAddStringToObject(obj, "k", "v");

	char smallBuf[4] = {0};
	char *out = RyanJsonPrintPreallocated(obj, smallBuf, sizeof(smallBuf), RyanJsonFalse, NULL);
	TEST_ASSERT_NULL_MESSAGE(out, "Preallocated buffer 过小应返回 NULL");

	RyanJsonDelete(obj);
}

static void testPrintPreallocatedExactFit(void)
{
	RyanJson_t nullJson = RyanJsonCreateNull(NULL);
	char buf[5];
	memset(buf, 'X', sizeof(buf));

	char *out = RyanJsonPrintPreallocated(nullJson, buf, sizeof(buf), RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(out, "Preallocated buffer 刚好够用应成功");
	TEST_ASSERT_EQUAL_STRING("null", out);

	RyanJsonDelete(nullJson);
}

static void testPrintPreallocatedExactFitObjectString(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(obj, "k", "v"));

	uint32_t expectedLen = 0;
	char *expected = RyanJsonPrint(obj, 0, RyanJsonFalse, &expectedLen);
	TEST_ASSERT_NOT_NULL(expected);

	char exactBuf[16] = {0};
	TEST_ASSERT_TRUE_MESSAGE(expectedLen + 1U <= sizeof(exactBuf), "测试缓冲区长度不足");

	char *out = RyanJsonPrintPreallocated(obj, exactBuf, expectedLen + 1U, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(out, "对象(string)预分配刚好够用应成功");
	TEST_ASSERT_EQUAL_STRING(expected, out);

	RyanJsonFree(expected);
	RyanJsonDelete(obj);
}

// 该用例覆盖预分配缓冲行为，后续可按场景继续拆分
static void testPrintPreallocatedObjectIntHeadroom(void)
{
	RyanJson_t obj = createObjectWithIntMember("a", 1);

	// 先拿到基准输出长度
	uint32_t expectLen = 0;
	char *expect = RyanJsonPrint(obj, 0, RyanJsonFalse, &expectLen);
	if (NULL == expect)
	{
		RyanJsonDelete(obj);
		TEST_FAIL_MESSAGE("基准打印失败，无法构造刚好够用缓冲区");
		return;
	}

	// 对象中包含 int 时，PrintNumber 内部会做 INT32_MIN 级别的预留校验，
	// 因此“仅够最终输出长度”的缓冲区会失败，这是当前实现的既定行为。
	char *exactBuf = (char *)malloc((size_t)expectLen + 1U);
	if (NULL == exactBuf)
	{
		RyanJsonFree(expect);
		RyanJsonDelete(obj);
		TEST_FAIL_MESSAGE("malloc 失败");
		return;
	}

	char *out = RyanJsonPrintPreallocated(obj, exactBuf, expectLen + 1U, RyanJsonFalse, NULL);
	TEST_ASSERT_NULL_MESSAGE(out, "对象(int)预分配仅按最终长度应失败（需要额外预留空间）");

	// 给足预留空间后应成功
	char headroomBuf[32] = {0};
	out = RyanJsonPrintPreallocated(obj, headroomBuf, sizeof(headroomBuf), RyanJsonFalse, NULL);
	if (NULL == out)
	{
		RyanJsonFree(expect);
		free(exactBuf);
		RyanJsonDelete(obj);
		TEST_FAIL_MESSAGE("对象(int)预分配带预留空间应成功");
		return;
	}

	TEST_ASSERT_EQUAL_STRING(expect, out);
	TEST_ASSERT_EQUAL_UINT32(expectLen, (uint32_t)strlen(out));

	RyanJsonFree(expect);
	free(exactBuf);
	RyanJsonDelete(obj);
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

static void testPrintPreallocatedWithStyleFormatFalseMinified(void)
{
	RyanJson_t obj = createObjectWithIntMember("a", 1);

	RyanJsonPrintStyle style = makeStyle("--------", "\r\n", 4, RyanJsonFalse);
	char buf[64] = {0};

	char *out = RyanJsonPrintPreallocatedWithStyle(obj, buf, sizeof(buf), &style, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("{\"a\":1}", out);

	RyanJsonDelete(obj);
}

static void testPrintIntBoundaryPreallocated(void)
{
	RyanJson_t intJson = RyanJsonCreateInt(NULL, INT32_MIN);
	TEST_ASSERT_NOT_NULL(intJson);

	char tooSmall[11] = {0};
	char *out = RyanJsonPrintPreallocated(intJson, tooSmall, sizeof(tooSmall), RyanJsonFalse, NULL);
	TEST_ASSERT_NULL_MESSAGE(out, "INT32_MIN 预分配 11 字节应失败");

	char exactFit[12] = {0};
	out = RyanJsonPrintPreallocated(intJson, exactFit, sizeof(exactFit), RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(out, "INT32_MIN 预分配 12 字节应成功");
	TEST_ASSERT_EQUAL_STRING("-2147483648", out);

	RyanJsonDelete(intJson);
}

static void testPrintDoubleBoundaryPreallocated(void)
{
	RyanJson_t doubleJson = RyanJsonCreateDouble(NULL, 1.5);
	TEST_ASSERT_NOT_NULL(doubleJson);

	uint32_t expectLen = 0;
	char *expect = RyanJsonPrint(doubleJson, 0, RyanJsonFalse, &expectLen);
	if (NULL == expect)
	{
		RyanJsonDelete(doubleJson);
		TEST_FAIL_MESSAGE("double 基准打印失败");
		return;
	}

	// 与 int 类似，double 路径会先申请固定工作区（RyanJsonDoubleBufferSize），
	// 所以仅按最终输出长度预分配会失败。
	char *exactBuf = (char *)malloc((size_t)expectLen + 1U);
	if (NULL == exactBuf)
	{
		RyanJsonFree(expect);
		RyanJsonDelete(doubleJson);
		TEST_FAIL_MESSAGE("malloc 失败");
		return;
	}

	char *out = RyanJsonPrintPreallocated(doubleJson, exactBuf, expectLen + 1U, RyanJsonFalse, NULL);
	TEST_ASSERT_NULL_MESSAGE(out, "double 预分配仅按最终长度应失败（需要额外预留空间）");

	char headroomBuf[128] = {0};
	out = RyanJsonPrintPreallocated(doubleJson, headroomBuf, sizeof(headroomBuf), RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(out, "double 预分配带预留空间应成功");
	TEST_ASSERT_EQUAL_STRING(expect, out);

	RyanJsonFree(expect);
	free(exactBuf);
	RyanJsonDelete(doubleJson);

	// 特殊值：NaN / Infinity 应打印为 null
	RyanJson_t nanJson = RyanJsonCreateDouble(NULL, NAN);
	RyanJson_t infJson = RyanJsonCreateDouble(NULL, INFINITY);
	TEST_ASSERT_NOT_NULL(nanJson);
	TEST_ASSERT_NOT_NULL(infJson);

	char specialBuf[128] = {0};
	out = RyanJsonPrintPreallocated(nanJson, specialBuf, sizeof(specialBuf), RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("null", out);

	memset(specialBuf, 0, sizeof(specialBuf));
	out = RyanJsonPrintPreallocated(infJson, specialBuf, sizeof(specialBuf), RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("null", out);

	RyanJsonDelete(nanJson);
	RyanJsonDelete(infJson);
}

static void testPrintDoubleScientificAndRoundtrip(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddDoubleToObject(obj, "large", 1.0e20));
	TEST_ASSERT_TRUE(RyanJsonAddDoubleToObject(obj, "tiny", 1.0e-5));
	TEST_ASSERT_TRUE(RyanJsonAddDoubleToObject(obj, "frac", 0.1));

	char *printed = RyanJsonPrint(obj, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);

	// large 在当前测试配置下应走科学计数法
	char *largePos = strstr(printed, "\"large\":");
	char *tinyPos = strstr(printed, "\"tiny\":");
	TEST_ASSERT_NOT_NULL(largePos);
	TEST_ASSERT_NOT_NULL(tinyPos);

	char *largeValueStart = strchr(largePos, ':');
	TEST_ASSERT_NOT_NULL(largeValueStart);
	largeValueStart++;

	char *largeComma = strpbrk(largePos, ",}");
	char *tinyComma = strpbrk(tinyPos, ",}");
	TEST_ASSERT_NOT_NULL(largeComma);
	TEST_ASSERT_NOT_NULL(tinyComma);

	int32_t largeHasScientific = (NULL != memchr((const void *)largeValueStart, 'e', (size_t)(largeComma - largeValueStart))) ||
				     (NULL != memchr((const void *)largeValueStart, 'E', (size_t)(largeComma - largeValueStart)));

#ifdef RyanJsonLinuxTestEnv
	// Linux 测试分支会在实现内覆盖格式选择，large 路径固定走科学计数法
	TEST_ASSERT_TRUE_MESSAGE(largeHasScientific, "RyanJsonLinuxTestEnv 下 large 应包含科学计数法输出");
#elif true == RyanJsonSnprintfSupportScientific
	TEST_ASSERT_TRUE_MESSAGE(largeHasScientific, "开启科学计数法时 large 应包含科学计数法输出");
#else
	TEST_ASSERT_FALSE_MESSAGE(largeHasScientific, "关闭科学计数法时 large 不应包含科学计数法输出");
#endif

	RyanJson_t roundtrip = RyanJsonParse(printed);
	RyanJsonFree(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);

	TEST_ASSERT_TRUE(RyanJsonCompareDouble(1.0e20, RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(roundtrip, "large"))));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(1.0e-5, RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(roundtrip, "tiny"))));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(0.1, RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(roundtrip, "frac"))));

	RyanJsonDelete(roundtrip);
	RyanJsonDelete(obj);
}

static int32_t gPrintFailAfter = -1;
static int32_t gPrintAllocCount = 0;

static void *printFailMalloc(size_t size)
{
	if (gPrintFailAfter >= 0 && gPrintAllocCount++ >= gPrintFailAfter) { return NULL; }
	return unityTestMalloc(size);
}

static void *printFailRealloc(void *block, size_t size)
{
	if (gPrintFailAfter >= 0 && gPrintAllocCount++ >= gPrintFailAfter) { return NULL; }
	return unityTestRealloc(block, size);
}

static void setPrintFailAfter(int32_t failAfter)
{
	gPrintFailAfter = failAfter;
	gPrintAllocCount = 0;
	RyanJsonInitHooks(printFailMalloc, unityTestFree, printFailRealloc);
}

static void restorePrintHooks(void)
{
	RyanJsonInitHooks(unityTestMalloc, unityTestFree, unityTestRealloc);
	gPrintFailAfter = -1;
	gPrintAllocCount = 0;
}

static void testPrintOom(void)
{
	RyanJson_t obj = createObjectWithIntMember("a", 1);

	setPrintFailAfter(0);

	char *printed = RyanJsonPrint(obj, 32, RyanJsonFalse, NULL);

	// 恢复 hooks
	restorePrintHooks();
	if (printed) { RyanJsonFree(printed); }
	TEST_ASSERT_NULL_MESSAGE(printed, "Print OOM 应返回 NULL");

	RyanJsonDelete(obj);
}

static void testPrintFinalAppendOom(void)
{
	char longStr[58];
	memset(longStr, 'a', sizeof(longStr) - 1);
	longStr[sizeof(longStr) - 1] = '\0';

	RyanJson_t obj = RyanJsonCreateObject();
	RyanJsonAddStringToObject(obj, "k", longStr);

	setPrintFailAfter(1);

	char *printed = RyanJsonPrint(obj, RyanJsonPrintfPreAlloSize, RyanJsonFalse, NULL);

	// 恢复 hooks
	restorePrintHooks();

	if (printed) { RyanJsonFree(printed); }
	TEST_ASSERT_NULL_MESSAGE(printed, "Print 末尾扩容失败应返回 NULL");

	RyanJsonDelete(obj);
}

/**
 * @brief 定制化打印风格测试
 * 验证 RyanJsonPrintWithStyle 接口以及各种格式化选项
 */
static void testPrintCrazy(void)
{
	// 打印空对象/空数组
	RyanJson_t emptyObj = RyanJsonCreateObject();
	char *s = RyanJsonPrint(emptyObj, 0, RyanJsonFalse, NULL);
	TEST_ASSERT_EQUAL_STRING("{}", s);
	RyanJsonFree(s);
	RyanJsonDelete(emptyObj);

	RyanJson_t emptyArr = RyanJsonCreateArray();
	s = RyanJsonPrint(emptyArr, 0, RyanJsonFalse, NULL);
	TEST_ASSERT_EQUAL_STRING("[]", s);
	RyanJsonFree(s);
	RyanJsonDelete(emptyArr);

	// 极限缓冲区测试
	RyanJson_t json = RyanJsonCreateObject();
	RyanJsonAddStringToObject(json, "k", "v");
	// size=0：自动选择初始缓冲
	s = RyanJsonPrint(json, 0, RyanJsonFalse, NULL);
	TEST_ASSERT_EQUAL_STRING("{\"k\":\"v\"}", s);
	RyanJsonFree(s);

	// size=1：初始空间过小，预期自动扩容
	s = RyanJsonPrint(json, 1, RyanJsonFalse, NULL);
	TEST_ASSERT_EQUAL_STRING("{\"k\":\"v\"}", s);
	RyanJsonFree(s);
	RyanJsonDelete(json);

	// 极端打印风格
	RyanJson_t obj = createObjectWithIntMember("a", 1);

	RyanJsonPrintStyle crazyStyle = makeStyle("--------", "\n\n", 4, RyanJsonTrue);
	uint32_t len;
	s = RyanJsonPrintWithStyle(obj, 10, &crazyStyle, &len);
	// 预期片段：{\n\n--------"a":    1\n\n}
	// 校验长度与关键片段
	TEST_ASSERT_EQUAL_INT(strlen(s), len);
	TEST_ASSERT_NOT_NULL(strstr(s, "--------\"a\":    1"));
	RyanJsonFree(s);
	RyanJsonDelete(obj);
}

static void testPrintDefault(void)
{
	const char *jsonstr = "{\"a\":1,\"b\":true}";
	RyanJson_t json = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL(json);

	// 测试默认格式化打印（使用 RyanJsonPrint 简化校验）
	char *defaultFormat = RyanJsonPrint(json, 256, RyanJsonTrue, NULL);
	// 期望形如:
	// {
	// \t"a": 1,
	// \t"b": true
	// }
	TEST_ASSERT_NOT_NULL(defaultFormat);
	TEST_ASSERT_TRUE_MESSAGE(strstr(defaultFormat, "\t\"a\": 1") != NULL, "默认格式化：缩进或冒号后空格错误");
	RyanJsonFree(defaultFormat);
	RyanJsonDelete(json);
}

static void testPrintCustomStyle(void)
{
	const char *jsonstr = "{\"a\":1,\"b\":true}";
	RyanJson_t json = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL(json);

	// 测试自定义风格：2 个空格缩进、Windows 换行、冒号后 2 个空格
	RyanJsonPrintStyle style = makeStyle("  ", "\r\n", 2, RyanJsonTrue);

	uint32_t len = 0;
	char *customPrint = RyanJsonPrintWithStyle(json, 256, &style, &len);
	TEST_ASSERT_NOT_NULL(customPrint);

	// 校验特征点
	TEST_ASSERT_TRUE_MESSAGE(strstr(customPrint, "\r\n  \"a\":  1") != NULL, "自定义格式化：缩进、换行或冒号后空格匹配失败");
	TEST_ASSERT_EQUAL_INT_MESSAGE(strlen(customPrint), len, "返回长度不一致");

	RyanJsonFree(customPrint);
	RyanJsonDelete(json);
}

void testPrintRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testPrintStyleEdgeCases);
	RUN_TEST(testPrintPreallocatedTooSmall);
	RUN_TEST(testPrintPreallocatedExactFit);
	RUN_TEST(testPrintPreallocatedExactFitObjectString);
	RUN_TEST(testPrintPreallocatedObjectIntHeadroom);
	RUN_TEST(testPrintPreallocatedWithStyleNullArgs);
	RUN_TEST(testPrintPreallocatedWithStyleTooSmall);
	RUN_TEST(testPrintPreallocatedWithStyleSuccessAndLen);
	RUN_TEST(testPrintPreallocatedWithStyleFormatFalseMinified);
	RUN_TEST(testPrintIntBoundaryPreallocated);
	RUN_TEST(testPrintDoubleBoundaryPreallocated);
	RUN_TEST(testPrintDoubleScientificAndRoundtrip);
	RUN_TEST(testPrintOom);
	RUN_TEST(testPrintFinalAppendOom);
	RUN_TEST(testPrintCrazy);
	RUN_TEST(testPrintDefault);
	RUN_TEST(testPrintCustomStyle);
}
