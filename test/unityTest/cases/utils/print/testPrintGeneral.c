#include "testBase.h"

static void testPrintNullRootGuard(void)
{
	TEST_ASSERT_NULL(RyanJsonPrint(NULL, 10, RyanJsonTrue, NULL));
}

static void testPrintPreallocatedExactFitNull(void)
{
	RyanJson_t nullJson = RyanJsonCreateNull(NULL);
	char buf[5];
	memset(buf, 'X', sizeof(buf));

	char *out = RyanJsonPrintPreallocated(nullJson, buf, sizeof(buf), RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(out, "Preallocated buffer 刚好容纳 null 应成功");
	TEST_ASSERT_EQUAL_STRING("null", out);

	RyanJsonDelete(nullJson);
}

static void testPrintPreallocatedExactFitUtf8String(void)
{
	// 覆盖 UTF-8 key/value 的预分配精确长度与输出一致性。
	const char *key = "\xE4\xB8\xAD";
	const char *val = "\xF0\x9F\x90\x82";
	char expectLiteral[] = "{\"\xE4\xB8\xAD\":\"\xF0\x9F\x90\x82\"}";

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(obj, key, val));

	uint32_t expectedLen = 0;
	char *expected = RyanJsonPrint(obj, 0, RyanJsonFalse, &expectedLen);
	TEST_ASSERT_NOT_NULL(expected);
	TEST_ASSERT_EQUAL_UINT32((uint32_t)strlen(expected), expectedLen);
	TEST_ASSERT_EQUAL_STRING(expectLiteral, expected);

	char *buf = (char *)malloc((size_t)expectedLen + 1U);
	TEST_ASSERT_NOT_NULL(buf);
	char *out = RyanJsonPrintPreallocated(obj, buf, expectedLen + 1U, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(out, "UTF-8 预分配刚好够用应成功");
	TEST_ASSERT_EQUAL_STRING(expectLiteral, out);

	RyanJson_t roundtrip = RyanJsonParse(out);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, roundtrip));

	RyanJsonDelete(roundtrip);
	free(buf);
	RyanJsonFree(expected);
	RyanJsonDelete(obj);
}

static void testPrintPreallocatedObjectIntHeadroom(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "a", 1));

	uint32_t expectLen = 0;
	char *expect = RyanJsonPrint(obj, 0, RyanJsonFalse, &expectLen);
	TEST_ASSERT_NOT_NULL(expect);

	// 对象中包含 int 时，内部数字路径会预留固定工作区，
	// 因而“仅够最终输出长度”的缓冲区不一定足够。
	char *exactBuf = (char *)malloc((size_t)expectLen + 1U);
	TEST_ASSERT_NOT_NULL(exactBuf);
	char *out = RyanJsonPrintPreallocated(obj, exactBuf, expectLen + 1U, RyanJsonFalse, NULL);
	TEST_ASSERT_NULL_MESSAGE(out, "对象(int) 预分配仅按最终长度应失败");

	char headroomBuf[32] = {0};
	out = RyanJsonPrintPreallocated(obj, headroomBuf, sizeof(headroomBuf), RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(out, "对象(int) 预分配带预留空间应成功");
	TEST_ASSERT_EQUAL_STRING(expect, out);
	TEST_ASSERT_EQUAL_UINT32(expectLen, (uint32_t)strlen(out));

	RyanJsonFree(expect);
	free(exactBuf);
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
	TEST_ASSERT_NOT_NULL(expect);

	// double 路径同样会先申请内部工作区，仅按最终输出长度预分配不一定足够。
	char *exactBuf = (char *)malloc((size_t)expectLen + 1U);
	TEST_ASSERT_NOT_NULL(exactBuf);
	char *out = RyanJsonPrintPreallocated(doubleJson, exactBuf, expectLen + 1U, RyanJsonFalse, NULL);
	TEST_ASSERT_NULL_MESSAGE(out, "double 预分配仅按最终长度应失败");

	char headroomBuf[128] = {0};
	out = RyanJsonPrintPreallocated(doubleJson, headroomBuf, sizeof(headroomBuf), RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(out, "double 预分配带预留空间应成功");
	TEST_ASSERT_EQUAL_STRING(expect, out);

	RyanJsonFree(expect);
	free(exactBuf);
	RyanJsonDelete(doubleJson);

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

static void testPrintDoubleFixedPointBoundary(void)
{
	// 固定点边界分支：
	// 1) 真实 0 必须打印成 0.0；
	// 2) 小于 1e15 的整数样式 double 仍应保留一位小数；
	// 3) 到达 1e15 后应切到“大数”分支，避免继续沿用固定点规则。
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddDoubleToObject(obj, "zero", 0.0));
	TEST_ASSERT_TRUE(RyanJsonAddDoubleToObject(obj, "belowLimit", 999999999999999.0));
	TEST_ASSERT_TRUE(RyanJsonAddDoubleToObject(obj, "atLimit", 1.0e15));

	char *printed = RyanJsonPrint(obj, 160, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	TEST_ASSERT_NOT_NULL_MESSAGE(strstr(printed, "\"zero\":0.0"), "0.0 应固定打印为 0.0");
	TEST_ASSERT_NOT_NULL_MESSAGE(strstr(printed, "\"belowLimit\":999999999999999.0"), "<1e15 的整数样式 double 应保留一位小数");

	char *atLimitPos = strstr(printed, "\"atLimit\":");
	TEST_ASSERT_NOT_NULL(atLimitPos);
	char *atLimitValue = strchr(atLimitPos, ':');
	TEST_ASSERT_NOT_NULL(atLimitValue);
	atLimitValue++;

	char *atLimitEnd = strpbrk(atLimitValue, ",}");
	TEST_ASSERT_NOT_NULL(atLimitEnd);
	int32_t atLimitHasScientific = (NULL != memchr((const void *)atLimitValue, 'e', (size_t)(atLimitEnd - atLimitValue))) ||
				       (NULL != memchr((const void *)atLimitValue, 'E', (size_t)(atLimitEnd - atLimitValue)));

#ifdef RyanJsonLinuxTestEnv
	TEST_ASSERT_TRUE_MESSAGE(atLimitHasScientific, "RyanJsonLinuxTestEnv 下 1e15 应走科学计数法路径");
#elif true == RyanJsonSnprintfSupportScientific
	TEST_ASSERT_TRUE_MESSAGE(atLimitHasScientific, "开启科学计数法时 1e15 应走科学计数法路径");
#else
	TEST_ASSERT_FALSE_MESSAGE(atLimitHasScientific, "关闭科学计数法时 1e15 不应走科学计数法路径");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE((uint32_t)strlen("1000000000000000.0"), (uint32_t)(atLimitEnd - atLimitValue),
					 "关闭科学计数法时 1e15 应保留固定一位小数");
	TEST_ASSERT_EQUAL_MEMORY("1000000000000000.0", atLimitValue, strlen("1000000000000000.0"));
#endif

	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(obj);
}

static void testPrintTinyDoubleNotZeroed(void)
{
	// 极小非零 double 不能只靠 roundtrip 证明；
	// 这里直接检查原始输出 token，防止打印阶段先被抹成 0.0 / -0.0。
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);

	TEST_ASSERT_TRUE(RyanJsonAddDoubleToObject(obj, "tinyPos", 1.0e-20));
	TEST_ASSERT_TRUE(RyanJsonAddDoubleToObject(obj, "tinyNeg", -1.0e-20));

	char *printed = RyanJsonPrint(obj, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);

	char *tinyPosField = strstr(printed, "\"tinyPos\":");
	char *tinyNegField = strstr(printed, "\"tinyNeg\":");
	TEST_ASSERT_NOT_NULL(tinyPosField);
	TEST_ASSERT_NOT_NULL(tinyNegField);

	char *tinyPosValue = strchr(tinyPosField, ':');
	char *tinyNegValue = strchr(tinyNegField, ':');
	TEST_ASSERT_NOT_NULL(tinyPosValue);
	TEST_ASSERT_NOT_NULL(tinyNegValue);
	tinyPosValue++;
	tinyNegValue++;

	char *tinyPosEnd = strpbrk(tinyPosValue, ",}");
	char *tinyNegEnd = strpbrk(tinyNegValue, ",}");
	TEST_ASSERT_NOT_NULL(tinyPosEnd);
	TEST_ASSERT_NOT_NULL(tinyNegEnd);

	TEST_ASSERT_FALSE_MESSAGE(((uint32_t)(tinyPosEnd - tinyPosValue) == (uint32_t)strlen("0.0")) &&
					  (0 == strncmp(tinyPosValue, "0.0", strlen("0.0"))),
				  "tiny 正数原始输出不应退化成 0.0");
	TEST_ASSERT_FALSE_MESSAGE(((uint32_t)(tinyNegEnd - tinyNegValue) == (uint32_t)strlen("-0.0")) &&
					  (0 == strncmp(tinyNegValue, "-0.0", strlen("-0.0"))),
				  "tiny 负数原始输出不应退化成 -0.0");

	RyanJson_t roundtrip = RyanJsonParse(printed);
	RyanJsonFree(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);

	double tinyPos = RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(roundtrip, "tinyPos"));
	double tinyNeg = RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(roundtrip, "tinyNeg"));

	TEST_ASSERT_TRUE_MESSAGE(tinyPos > 0.0, "tiny 正数不应在打印后被抹为 0");
	TEST_ASSERT_TRUE_MESSAGE(tinyNeg < 0.0, "tiny 负数不应在打印后被抹为 0");

	RyanJsonDelete(roundtrip);
	RyanJsonDelete(obj);
}

static void testPrintPreallocatedArgGuards(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(obj, "k", "v"));

	char buf[16] = {0};
	TEST_ASSERT_NULL(RyanJsonPrintPreallocated(NULL, buf, sizeof(buf), RyanJsonFalse, NULL));
	TEST_ASSERT_NULL(RyanJsonPrintPreallocated(obj, NULL, sizeof(buf), RyanJsonFalse, NULL));
	TEST_ASSERT_NULL(RyanJsonPrintPreallocated(obj, buf, 0, RyanJsonFalse, NULL));

	uint32_t len = 0;
	char *out = RyanJsonPrintPreallocated(obj, buf, sizeof(buf), RyanJsonFalse, &len);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_STRING("{\"k\":\"v\"}", out);
	TEST_ASSERT_EQUAL_UINT32((uint32_t)strlen(out), len);

	RyanJsonDelete(obj);
}

static void testPrintOom(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "a", 1));

	UNITY_TEST_OOM_BEGIN(0);
	char *printed = RyanJsonPrint(obj, 32, RyanJsonFalse, NULL);
	UNITY_TEST_OOM_END();

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
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(obj, "k", longStr));

	UNITY_TEST_OOM_BEGIN(1);
	char *printed = RyanJsonPrint(obj, RyanJsonPrintfPreAlloSize, RyanJsonFalse, NULL);
	UNITY_TEST_OOM_END();

	if (printed) { RyanJsonFree(printed); }
	TEST_ASSERT_NULL_MESSAGE(printed, "Print 末尾扩容失败应返回 NULL");

	RyanJsonDelete(obj);
}

static void testPrintExpandFallbackWithoutRealloc(void)
{
	char longValue[600] = {0};
	memset(longValue, 'x', sizeof(longValue) - 1U);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(obj, "k", longValue));

	// 先让初始缓冲成功，再让扩容阶段走“无 realloc 时改用 malloc”分支并失败。
	UNITY_TEST_OOM_BEGIN_NO_REALLOC(1);
	char *printed = RyanJsonPrint(obj, RyanJsonPrintfPreAlloSize, RyanJsonFalse, NULL);
	UNITY_TEST_OOM_END();

	if (printed) { RyanJsonFree(printed); }
	TEST_ASSERT_NULL_MESSAGE(printed, "Print 扩容 fallback(malloc) 失败应返回 NULL");

	RyanJsonDelete(obj);
}

void testPrintGeneralRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testPrintNullRootGuard);
	RUN_TEST(testPrintPreallocatedExactFitNull);
	RUN_TEST(testPrintPreallocatedExactFitUtf8String);
	RUN_TEST(testPrintPreallocatedObjectIntHeadroom);
	RUN_TEST(testPrintIntBoundaryPreallocated);
	RUN_TEST(testPrintDoubleBoundaryPreallocated);
	RUN_TEST(testPrintDoubleScientificAndRoundtrip);
	RUN_TEST(testPrintDoubleFixedPointBoundary);
	RUN_TEST(testPrintTinyDoubleNotZeroed);
	RUN_TEST(testPrintPreallocatedArgGuards);
	RUN_TEST(testPrintOom);
	RUN_TEST(testPrintFinalAppendOom);
	RUN_TEST(testPrintExpandFallbackWithoutRealloc);
}
