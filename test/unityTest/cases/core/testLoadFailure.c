#include "testBase.h"

static void testLoadFailureNullAndWhitespace(void)
{
	// NULL 和空字符串
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse(NULL), "Parse(NULL) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse(""), "Parse(\"\") 应返回 NULL");

	// 仅有空白字符
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("   "), "Parse(只含空格) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("\t\r\n"), "Parse(只含换行符) 应返回 NULL");
}

static void testLoadFailureTruncated(void)
{
	// 畸形 Json（缺少闭合）
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("[1, 2, 3"), "Parse(未闭合数组) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\": 1"), "Parse(未闭合对象) 应返回 NULL");

	// 截断的输入 (非正常结束)
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{"), "截断的对象应解析失败");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("["), "截断的数组应解析失败");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":"), "缺少值的对象应解析失败");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1"), "未闭合的对象应解析失败");

	// 未闭合字符串
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("\"just a string"), "未闭合字符串应解析失败");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":\"unterminated}"), "未闭合字符串应解析失败");
}

static void testLoadFailureInvalidTokens(void)
{
	// 非法字符开头
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("x123"), "Parse(非法开头) 应返回 NULL");

	// 无效数字
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"inter\":16poi}"), "应拒绝无效数字 16poi");

	// 无效浮点数
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"double\":16.8yu9}"), "应拒绝无效浮点数 16.8yu9");

	// boolTrue 拼写错误
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"boolTrue\":tru}"), "应拒绝错误拼写的 true (tru)");

	// boolFalse 拼写错误
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"boolFalse\":fale}"), "应拒绝错误拼写的 false (fale)");

	// null 拼写错误
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"null\":nul}"), "应拒绝错误拼写的 null (nul)");

	// null 大写错误（Json 规范要求小写）
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"null\":NULL}"), "应拒绝大写的 NULL");

	// 缺少逗号
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"inter\":16\"double\":16.89}"), "应拒绝缺少逗号的对象");

	// 数组项缺少逗号
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("[16,16.89\"hello\"]"), "应拒绝数组项缺少逗号");

	// 键值缺少引号
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"item:{\"inter\":16}}"), "应拒绝键值缺少引号");

	// 嵌套对象键值缺少引号
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"item\":{inter:16}}"), "应拒绝嵌套对象键值缺少引号");

	// 多余引号
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"item\":{\"\"double\":16.89}}"), "应拒绝多余引号");

	// 键值后多余逗号或引号
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"item\":{\"inter\":16\"\"}}"), "应拒绝非法结尾引号");

	// 数组中无效数字
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"arrayInt\":[16,16,16m,16,16]}"), "应拒绝数组中含有无效数字 16m");
}

static void testLoadFailureInvalidNumbers(void)
{
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("012"), "Parse(前导 0) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":01}"), "Parse(01) 应返回 NULL (不允许前导 0)");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":-01}"), "Parse(-01) 应返回 NULL (不允许前导 0)");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":00}"), "Parse(00) 应返回 NULL (不允许前导 0)");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":+1}"), "Parse(+1) 应返回 NULL (Json 不允许前导 +)");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":--1}"), "Parse(--1) 应返回 NULL");

	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1.2.3}"), "Parse(1.2.3) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1.}"), "Parse(1.) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":-.1}"), "Parse(-.1) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":-}"), "Parse(-) 应返回 NULL");

	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1e}"), "Parse(1e) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1e+}"), "Parse(1e+) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1e-}"), "Parse(1e-) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1e309}"), "Parse(1e309) 应返回 NULL (溢出)");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("1e2147483647"), "Parse(纯数字指数边界) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("1e2147483648"), "Parse(纯数字指数累积溢出) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("1e-2147483648"), "Parse(纯数字负指数累积溢出) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1e2147483648}"), "Parse(指数累积溢出) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1e-2147483648}"), "Parse(负指数累积溢出) 应返回 NULL");
}

static void testLoadFailureHugeNumberOverflow(void)
{
	// 超长整数：应在数值累乘过程中触发 isfinite 防御并失败
	const uint32_t intLen = 1024;
	char *hugeInt = (char *)malloc((size_t)intLen + 1U);
	TEST_ASSERT_NOT_NULL(hugeInt);

	hugeInt[0] = '1';
	memset(hugeInt + 1, '9', intLen - 1U);
	hugeInt[intLen] = '\0';
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse(hugeInt), "Parse(超长整数溢出) 应返回 NULL");
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

static void testLoadFailureInvalidEscapes(void)
{
	// 疯狂的转义符
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":\"\\\"}"), "Parse(末尾转义未完成) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":\"\\v\"}"), "Parse(非法转义 \\v) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":\"\\x12\"}"), "Parse(非法转义 \\x12) 应返回 NULL");

	// 控制字符插入
	// Json 规范不允许未转义的控制字符（0x00-0x1F）
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":\"\x01\"}"), "Parse(含控制字符 0x01) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":\"\n\"}"), "Parse(含未转义换行) 应返回 NULL");
}

static void testLoadFailureInvalidUnicode(void)
{
	// Unicode 截断
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"invalid\":\"\\u123\"}"), "应拒绝截断的 Unicode");

	// 非法十六进制字符
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"invalid\":\"\\uGGGG\"}"), "应拒绝非法 Unicode GGGG");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"invalid\":\"\\u00ZZ\"}"), "应拒绝非法 Unicode 00ZZ");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"invalid\":\"\\u00!!\"}"), "应拒绝非法 Unicode 00!!");

	// UTF-16 代理对错误
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"invalid\":\"\\uD800\"}"), "应拒绝缺少低位代理的高位代理");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"invalid\":\"\\uDC00\"}"), "应拒绝单独低位代理");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"invalid\":\"\\uD800\\u0041\"}"), "应拒绝无效代理对");
}

static void testLoadFailureInvalidStructure(void)
{
	// 结构混乱
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1, \"b\":2, }"), "Parse(尾部逗号) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1, , \"b\":2}"), "Parse(双逗号) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1 : \"b\":2}"), "Parse(冒号代替逗号) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1 \"b\":2}"), "Parse(缺少逗号) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1,}"), "Parse(对象尾逗号) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1,,\"b\":2}"), "Parse(对象双逗号) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":}"), "Parse(缺少值) 应返回 NULL");
#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"dup\":1,\"dup\":2}"), "严格模式 Parse(对象重复 key) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"obj\":{\"dup\":1,\"dup\":2}}"), "严格模式 Parse(嵌套对象重复 key) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("[{\"dup\":1,\"dup\":2}]"), "严格模式 Parse(数组内对象重复 key) 应返回 NULL");
#else
	RyanJson_t dupObj = RyanJsonParse("{\"dup\":1,\"dup\":2}");
	RyanJson_t dupNestedObj = RyanJsonParse("{\"obj\":{\"dup\":1,\"dup\":2}}");
	RyanJson_t dupInArray = RyanJsonParse("[{\"dup\":1,\"dup\":2}]");
	TEST_ASSERT_NOT_NULL_MESSAGE(dupObj, "非严格模式 Parse(对象重复 key) 应成功");
	TEST_ASSERT_NOT_NULL_MESSAGE(dupNestedObj, "非严格模式 Parse(嵌套对象重复 key) 应成功");
	TEST_ASSERT_NOT_NULL_MESSAGE(dupInArray, "非严格模式 Parse(数组内对象重复 key) 应成功");
	RyanJsonDelete(dupObj);
	RyanJsonDelete(dupNestedObj);
	RyanJsonDelete(dupInArray);
#endif

	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("[,1]"), "Parse(数组前置逗号) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("[1,,2]"), "Parse(数组双逗号) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("[1,]"), "Parse(数组尾逗号) 应返回 NULL");
}

static void testLoadFailureDuplicateKeyAfterDecode(void)
{
#if true == RyanJsonStrictObjectKeyCheck
	// 转义后 key 冲突（"\u0061" == "a"）
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"a\":1,\"\\u0061\":2}"), "严格模式 Parse(转义后重复 key) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"\\u0061\":1,\"a\":2}"), "严格模式 Parse(转义后重复 key 反序) 应返回 NULL");

	// 大小写字符同理："\u0041" == "A"
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"A\":1,\"\\u0041\":2}"), "严格模式 Parse(转义后重复大写 key) 应返回 NULL");

	// 空 key 重复
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"\":1,\"\":2}"), "严格模式 Parse(空 key 重复) 应返回 NULL");
#else
	RyanJson_t dupEscaped1 = RyanJsonParse("{\"a\":1,\"\\u0061\":2}");
	RyanJson_t dupEscaped2 = RyanJsonParse("{\"\\u0061\":1,\"a\":2}");
	RyanJson_t dupEscaped3 = RyanJsonParse("{\"A\":1,\"\\u0041\":2}");
	RyanJson_t dupEscaped4 = RyanJsonParse("{\"\":1,\"\":2}");
	TEST_ASSERT_NOT_NULL_MESSAGE(dupEscaped1, "非严格模式 Parse(转义后重复 key) 应成功");
	TEST_ASSERT_NOT_NULL_MESSAGE(dupEscaped2, "非严格模式 Parse(转义后重复 key 反序) 应成功");
	TEST_ASSERT_NOT_NULL_MESSAGE(dupEscaped3, "非严格模式 Parse(转义后重复大写 key) 应成功");
	TEST_ASSERT_NOT_NULL_MESSAGE(dupEscaped4, "非严格模式 Parse(空 key 重复) 应成功");
	RyanJsonDelete(dupEscaped1);
	RyanJsonDelete(dupEscaped2);
	RyanJsonDelete(dupEscaped3);
	RyanJsonDelete(dupEscaped4);
#endif
}

static void testLoadFailureMalformedNesting(void)
{
	// 深度嵌套但没有闭合
	char deepOpen[200];
	memset(deepOpen, '[', 199);
	deepOpen[199] = '\0';
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse(deepOpen), "Parse(199个[) 应返回 NULL");

	// 深度嵌套但不匹配
	char deepMix[200];
	memset(deepMix, '[', 100);
	memset(deepMix + 100, '}', 99); // 应该是 ]
	deepMix[199] = '\0';
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse(deepMix), "Parse(100个[ + 99个}) 应返回 NULL");
}

static void testLoadParseOptionsFailure(void)
{
	// 禁止尾部垃圾：requireNullTerminator = true
	const char *text = " {\"a\":1} trailing";
	RyanJson_t json = RyanJsonParseOptions(text, (uint32_t)strlen(text), RyanJsonTrue, NULL);
	TEST_ASSERT_NULL_MESSAGE(json, "ParseOptions(require null terminator) 应失败");
}

static int32_t gLoadFailAfter = -1;
static int32_t gLoadAllocCount = 0;

static void *loadFailMalloc(size_t size)
{
	if (gLoadFailAfter >= 0 && gLoadAllocCount++ >= gLoadFailAfter) { return NULL; }
	return unityTestMalloc(size);
}

static void *loadFailRealloc(void *block, size_t size)
{
	if (gLoadFailAfter >= 0 && gLoadAllocCount++ >= gLoadFailAfter) { return NULL; }
	return unityTestRealloc(block, size);
}

static void loadSetFailAfter(int32_t n)
{
	gLoadFailAfter = n;
	gLoadAllocCount = 0;
	RyanJsonInitHooks(loadFailMalloc, unityTestFree, loadFailRealloc);
}

static void loadRestoreHooks(void)
{
	RyanJsonInitHooks(unityTestMalloc, unityTestFree, unityTestRealloc);
	gLoadFailAfter = -1;
	gLoadAllocCount = 0;
}

static void testLoadFailureOomParse(void)
{
	const char *longKeyJson =
		"{\"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\":1}";

	loadSetFailAfter(0);
	RyanJson_t json = RyanJsonParse("{\"a\":1}");
	loadRestoreHooks();
	if (json) { RyanJsonDelete(json); }
	TEST_ASSERT_NULL_MESSAGE(json, "Parse OOM(根节点分配失败) 应返回 NULL");

	loadSetFailAfter(1); // root 成功，key buffer 失败
	json = RyanJsonParse(longKeyJson);
	loadRestoreHooks();
	if (json) { RyanJsonDelete(json); }
	TEST_ASSERT_NULL_MESSAGE(json, "Parse OOM 应返回 NULL");
}

void testLoadFailureRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testLoadFailureNullAndWhitespace);
	RUN_TEST(testLoadFailureTruncated);
	RUN_TEST(testLoadFailureInvalidTokens);
	RUN_TEST(testLoadFailureInvalidNumbers);
	RUN_TEST(testLoadFailureHugeNumberOverflow);
	RUN_TEST(testLoadFailureInvalidEscapes);
	RUN_TEST(testLoadFailureInvalidUnicode);
	RUN_TEST(testLoadFailureInvalidStructure);
	RUN_TEST(testLoadFailureDuplicateKeyAfterDecode);
	RUN_TEST(testLoadFailureMalformedNesting);
	RUN_TEST(testLoadParseOptionsFailure);
	RUN_TEST(testLoadFailureOomParse);
}
