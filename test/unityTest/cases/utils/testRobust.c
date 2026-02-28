#include "testBase.h"

static int32_t gFailAfter = -1;
static int32_t gAllocCount = 0;

static void *failingMalloc(size_t size)
{
	if (gFailAfter >= 0 && gAllocCount++ >= gFailAfter) { return NULL; }
	return unityTestMalloc(size);
}

static void *failingRealloc(void *block, size_t size)
{
	if (gFailAfter >= 0 && gAllocCount++ >= gFailAfter) { return NULL; }
	return unityTestRealloc(block, size);
}

static void setFailAfter(int32_t n)
{
	gFailAfter = n;
	gAllocCount = 0;
	RyanJsonInitHooks(failingMalloc, unityTestFree, failingRealloc);
}

static void setFailAfterNoRealloc(int32_t n)
{
	gFailAfter = n;
	gAllocCount = 0;
	RyanJsonInitHooks(failingMalloc, unityTestFree, NULL);
}

static void restoreHooks(void)
{
	RyanJsonInitHooks(unityTestMalloc, unityTestFree, unityTestRealloc);
	gFailAfter = -1;
	gAllocCount = 0;
}

static void assertMinifyEq(char *buffer, int32_t textLen, const char *expected)
{
	uint32_t len = RyanJsonMinify(buffer, textLen);
	TEST_ASSERT_EQUAL_STRING(expected, buffer);
	TEST_ASSERT_EQUAL_UINT32((uint32_t)strlen(expected), len);
}

#define expectCreateNullUnderOom(failAfter, expr, msg)                                                                                     \
	do                                                                                                                                 \
	{                                                                                                                                  \
		setFailAfter((failAfter));                                                                                                 \
		RyanJson_t _node = (expr);                                                                                                 \
		restoreHooks();                                                                                                            \
		if (_node) { RyanJsonDelete(_node); }                                                                                      \
		TEST_ASSERT_NULL_MESSAGE(_node, (msg));                                                                                    \
	} while (0)

static void testParseOptionsTerminator(void)
{
	const char *end = NULL;

	// 允许尾部内容：requireNullTerminator = false
	const char *text = " {\"a\":1} trailing";
	RyanJson_t json = RyanJsonParseOptions(text, (uint32_t)strlen(text), RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "ParseOptions(允许尾部) 失败");
	TEST_ASSERT_NOT_NULL_MESSAGE(end, "parseEndPtr 不应为 NULL");
	TEST_ASSERT_EQUAL_STRING_MESSAGE(" trailing", end, "parseEndPtr 位置错误");
	RyanJsonDelete(json);

	// 禁止尾部内容：requireNullTerminator = true
	json = RyanJsonParseOptions(text, (uint32_t)strlen(text), RyanJsonTrue, NULL);
	TEST_ASSERT_NULL_MESSAGE(json, "ParseOptions(强制结尾) 应失败");

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
	TEST_ASSERT_EQUAL_STRING_MESSAGE("{\"b\":2}", end, "size-limited parseEndPtr 错误");
	RyanJsonDelete(json);
}

static void testMinifyComplexEscapesAndComments(void)
{
	// 注释应被剔除，但字符串里的注释片段必须保留
	char buf[] = " { \"url\" : \"http://x//y\" , /* block */ \"path\" : \"C:\\\\tmp\\\\/*file*/\" , // line\n \"ok\" : true } ";
	assertMinifyEq(buf, (int32_t)strlen(buf), "{\"url\":\"http://x//y\",\"path\":\"C:\\\\tmp\\\\/*file*/\",\"ok\":true}");

	// 转义引号与字符串内 // 也应保持原样
	char buf2[] = "{\"msg\":\"he said: \\\"/*no*/\\\" //keep\" , \"v\" : 1}   ";
	assertMinifyEq(buf2, (int32_t)strlen(buf2), "{\"msg\":\"he said: \\\"/*no*/\\\" //keep\",\"v\":1}");
}

static void testMinifyNoTerminatorOverflow(void)
{
	// textLen 不包含额外 '\0' 空间时，不应越界写终止符
	uint8_t rawBuf[8] = {'{', '\"', 'a', '\"', ':', '1', '}', '#'};
	uint32_t len = RyanJsonMinify((char *)rawBuf, 7);
	TEST_ASSERT_EQUAL_UINT32(7, len);
	TEST_ASSERT_EQUAL_UINT8('#', rawBuf[7]);
	TEST_ASSERT_EQUAL_UINT8('{', rawBuf[0]);
	TEST_ASSERT_EQUAL_UINT8('\"', rawBuf[1]);
	TEST_ASSERT_EQUAL_UINT8('a', rawBuf[2]);
	TEST_ASSERT_EQUAL_UINT8('\"', rawBuf[3]);
	TEST_ASSERT_EQUAL_UINT8(':', rawBuf[4]);
	TEST_ASSERT_EQUAL_UINT8('1', rawBuf[5]);
	TEST_ASSERT_EQUAL_UINT8('}', rawBuf[6]);
}

static void testMinifyCommentBoundaryCases(void)
{
	// 行注释直到输入结束（无换行）
	char lineTail[] = "{\"a\":1}//tail";
	assertMinifyEq(lineTail, (int32_t)strlen(lineTail), "{\"a\":1}");

	// 块注释未闭合：应安全走到 end，不越界
	char blockTail[] = "{\"a\":1}/*tail";
	assertMinifyEq(blockTail, (int32_t)strlen(blockTail), "{\"a\":1}");

	// 末尾孤立 '/' 不是注释起始，应保留
	char loneSlash[] = "{\"a\":1}/";
	assertMinifyEq(loneSlash, (int32_t)strlen(loneSlash), "{\"a\":1}/");

	// 字符串内末尾反斜杠且 textLen 截断，覆盖 text + 1 < end 为 false 的路径
	char rawTruncated[3] = {'\"', 'a', '\\'};
	uint32_t len = RyanJsonMinify(rawTruncated, 3);
	TEST_ASSERT_EQUAL_UINT32(3U, len);
	TEST_ASSERT_EQUAL_UINT8('\"', (uint8_t)rawTruncated[0]);
	TEST_ASSERT_EQUAL_UINT8('a', (uint8_t)rawTruncated[1]);
	TEST_ASSERT_EQUAL_UINT8('\\', (uint8_t)rawTruncated[2]);
}

static void testMinifyWhitespaceAndNonCommentSlashPaths(void)
{
	char tabLeading[] = "\t{\"a\":1}";
	assertMinifyEq(tabLeading, (int32_t)strlen(tabLeading), "{\"a\":1}");

	char crLeading[] = "\r{\"b\":2}";
	assertMinifyEq(crLeading, (int32_t)strlen(crLeading), "{\"b\":2}");

	// "/x" 不是注释，应完整保留
	char slashNonComment[] = "/x";
	assertMinifyEq(slashNonComment, (int32_t)strlen(slashNonComment), "/x");

	// 块注释内出现 '*' 但后续不是 '/'，应继续扫描直到真正闭合
	char blockWithStar[] = "/*a*b*/{\"k\":1}";
	assertMinifyEq(blockWithStar, (int32_t)strlen(blockWithStar), "{\"k\":1}");

	// 未闭合块注释且最后一个字符是 '*'
	char blockEndWithStar[] = "/*abc*";
	assertMinifyEq(blockEndWithStar, (int32_t)strlen(blockEndWithStar), "");
}

static void testParseAllocatedKeyCleanupOnValueError(void)
{
	const char *bad = "{\"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\":}";
	RyanJson_t json = RyanJsonParse(bad);
	TEST_ASSERT_NULL_MESSAGE(json, "长 key 后 value 非法时应解析失败");
}

static void testPrintPreallocatedArgGuards(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	RyanJsonAddStringToObject(obj, "k", "v");

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

static void testInitHooksAndCreateApiGuards(void)
{
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonInitHooks(NULL, unityTestFree, unityTestRealloc), "InitHooks(NULL malloc) 应失败");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonInitHooks(unityTestMalloc, NULL, unityTestRealloc), "InitHooks(NULL free) 应失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInitHooks(unityTestMalloc, unityTestFree, unityTestRealloc), "恢复默认 hooks 应成功");

	TEST_ASSERT_NULL_MESSAGE(RyanJsonCreateString("k", NULL), "CreateString(NULL value) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonCreateIntArray(NULL, 1), "CreateIntArray(NULL,1) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonCreateDoubleArray(NULL, 1), "CreateDoubleArray(NULL,1) 应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonCreateStringArray(NULL, 1), "CreateStringArray(NULL,1) 应返回 NULL");
}

static void testNumberBoundaries(void)
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
}

static void testVarargsPathTypeMismatchAndNullInput(void)
{
	RyanJson_t root = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 7));
	RyanJsonAddItemToObject(root, "arr", arr);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(root, "n", 42));

	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(NULL, "a"), "NULL 根节点应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToIndex(NULL, 0), "NULL 根节点应返回 NULL");

	// 根节点是 object，也支持按索引获取直接子节点
	TEST_ASSERT_NOT_NULL_MESSAGE(RyanJsonGetObjectToIndex(root, 0), "object 根节点按 index=0 获取应成功");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToIndex(root, 100), "object 根节点 index 越界应返回 NULL");

	// 标量节点继续向下取 key/index 都应失败
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(root, "n", "x"), "标量节点不应继续按 key 深入");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToIndex(arr, 0, 0), "标量节点不应继续按 index 深入");

	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(root, "n", "x"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToIndex(arr, 0, 0));

	// 首层查找失败路径（不进入可变参数迭代）
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByKeys(root, "missing", NULL), "首层 key 缺失应返回 NULL");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectByIndexs(arr, 1, UINT32_MAX), "首层 index 越界应返回 NULL");

	RyanJsonDelete(root);
}

static void testDuplicateKeyDetach(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "dup", 1));
#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonAddIntToObject(obj, "dup", 2), "严格模式下对象不应允许重复 key");
#else
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddIntToObject(obj, "dup", 2), "非严格模式下对象应允许重复 key");
#endif

	RyanJson_t only = RyanJsonGetObjectByKey(obj, "dup");
	TEST_ASSERT_NOT_NULL(only);
#if true == RyanJsonDefaultAddAtHead && false == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(only));
#else
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(only));
#endif

	RyanJson_t detached = RyanJsonDetachByKey(obj, "dup");
	TEST_ASSERT_NOT_NULL(detached);
#if true == RyanJsonDefaultAddAtHead && false == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(detached));
#else
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(detached));
#endif
	RyanJsonDelete(detached);

#if true == RyanJsonStrictObjectKeyCheck
	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(obj, "dup"));
#else
	RyanJson_t second = RyanJsonGetObjectByKey(obj, "dup");
	TEST_ASSERT_NOT_NULL(second);
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(second));
#else
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(second));
#endif
	RyanJson_t detached2 = RyanJsonDetachByKey(obj, "dup");
	TEST_ASSERT_NOT_NULL(detached2);
#if true == RyanJsonDefaultAddAtHead
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(detached2));
#else
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(detached2));
#endif
	RyanJsonDelete(detached2);
	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(obj, "dup"));
#endif

	RyanJsonDelete(obj);
}

static void testInsertOutOfRangeAndKeyValidation(void)
{
	// Array：index 超出范围应追加到尾部
	RyanJson_t arr = RyanJsonCreateArray();
	RyanJsonAddIntToArray(arr, 1);
	RyanJsonAddIntToArray(arr, 2);
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 100, RyanJsonCreateInt(NULL, 3)));
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetSize(arr));
	TEST_ASSERT_EQUAL_INT(3, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 2)));
	RyanJsonDelete(arr);

	// Object：item 无 key 应失败
	RyanJson_t obj = RyanJsonCreateObject();
	RyanJsonAddIntToObject(obj, "a", 1);
	RyanJson_t noKey = RyanJsonCreateInt(NULL, 2);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonInsert(obj, 0, noKey), "Object 插入无 key item 应失败");
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetSize(obj));
	RyanJsonDelete(obj);
}

static void testGetSizeNullAndContainer(void)
{
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, RyanJsonGetSize(NULL), "NULL GetSize 应返回 0");

	RyanJson_t num = RyanJsonParse("1");
	TEST_ASSERT_NOT_NULL(num);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, RyanJsonGetSize(num), "标量 GetSize 应返回 1");
	RyanJsonDelete(num);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "a", 1));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(obj, "b", RyanJsonCreateObject()));
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, RyanJsonGetSize(obj), "对象 GetSize 应返回直接子节点数量");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, RyanJsonGetSize(RyanJsonGetObjectToKey(obj, "b")), "空对象 GetSize 应为 0");
	RyanJsonDelete(obj);

	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 1));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 2));
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, RyanJsonGetSize(arr), "数组 GetSize 应返回元素数量");
	RyanJsonDelete(arr);
}

static void testInternalUtilsBranchCoverage(void)
{
	const char *same = "same";
	TEST_ASSERT_TRUE(RyanJsonInternalStrEq(same, same));
	TEST_ASSERT_FALSE(RyanJsonInternalStrEq("abc", "abd"));

	TEST_ASSERT_EQUAL_UINT8(4, RyanJsonInternalDecodeKeyLenField(3));
	TEST_ASSERT_EQUAL_UINT8(2, RyanJsonInternalDecodeKeyLenField(2));
	TEST_ASSERT_EQUAL_UINT8(1, RyanJsonInternalCalcLenBytes(UINT8_MAX));
	TEST_ASSERT_EQUAL_UINT8(2, RyanJsonInternalCalcLenBytes((uint32_t)UINT8_MAX + 1U));
	TEST_ASSERT_EQUAL_UINT8(3, RyanJsonInternalCalcLenBytes((uint32_t)UINT16_MAX + 1U));

	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 7));
	RyanJson_t v = RyanJsonGetObjectByIndexs(arr, 0, UINT32_MAX);
	TEST_ASSERT_NOT_NULL(v);
	TEST_ASSERT_EQUAL_INT(7, RyanJsonGetIntValue(v));
	RyanJsonDelete(arr);
}

static void testTypedArrayCreationZeroCount(void)
{
	int32_t intOne[1] = {1};
	double doubleOne[1] = {1.5};
	const char *strOne[1] = {"x"};

	RyanJson_t iArr = RyanJsonCreateIntArray(intOne, 0);
	RyanJson_t dArr = RyanJsonCreateDoubleArray(doubleOne, 0);
	RyanJson_t sArr = RyanJsonCreateStringArray(strOne, 0);

	TEST_ASSERT_NOT_NULL(iArr);
	TEST_ASSERT_NOT_NULL(dArr);
	TEST_ASSERT_NOT_NULL(sArr);
	TEST_ASSERT_EQUAL_UINT32(0, RyanJsonGetSize(iArr));
	TEST_ASSERT_EQUAL_UINT32(0, RyanJsonGetSize(dArr));
	TEST_ASSERT_EQUAL_UINT32(0, RyanJsonGetSize(sArr));

	RyanJsonDelete(iArr);
	RyanJsonDelete(dArr);
	RyanJsonDelete(sArr);
}

static void testTypedArrayCreationOomPaths(void)
{
	int32_t ints[2] = {1, 2};
	double doubles[2] = {1.1, 2.2};
	const char *strs[2] = {"a", "b"};

	expectCreateNullUnderOom(1, RyanJsonCreateIntArray(ints, 2), "CreateIntArray OOM 路径应返回 NULL");
	expectCreateNullUnderOom(1, RyanJsonCreateDoubleArray(doubles, 2), "CreateDoubleArray OOM 路径应返回 NULL");
	expectCreateNullUnderOom(1, RyanJsonCreateStringArray(strs, 2), "CreateStringArray OOM 路径应返回 NULL");
}

static void testCreateScalarOomAndTypeGuards(void)
{
	// CreateInt/CreateDouble 失败分支
	expectCreateNullUnderOom(0, RyanJsonCreateInt(NULL, 1), "CreateInt OOM 应返回 NULL");
	expectCreateNullUnderOom(0, RyanJsonCreateDouble(NULL, 1.5), "CreateDouble OOM 应返回 NULL");

	// 类型判定与 GetObjectByKey 空参分支
	RyanJson_t nullNode = RyanJsonCreateNull(NULL);
	RyanJson_t intNode = RyanJsonCreateInt(NULL, 7);
	TEST_ASSERT_NOT_NULL(nullNode);
	TEST_ASSERT_NOT_NULL(intNode);
	TEST_ASSERT_TRUE(RyanJsonIsNull(nullNode));
	TEST_ASSERT_FALSE(RyanJsonIsNull(intNode));
	TEST_ASSERT_FALSE(RyanJsonIsNull(NULL));

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "a", 1));
	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(obj, NULL));
	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(NULL, "a"));

	RyanJsonDelete(obj);
	RyanJsonDelete(nullNode);
	RyanJsonDelete(intNode);
}

static void testPrintExpandFallbackWithoutRealloc(void)
{
	char longValue[600] = {0};
	memset(longValue, 'x', sizeof(longValue) - 1U);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(obj, "k", longValue));

	// PrintWithStyle 至少会分配 RyanJsonPrintfPreAlloSize；长字符串可稳定触发扩容
	// 先成功分配初始缓冲，再在扩容时失败，覆盖 jsonRealloc=NULL 的 fallback 路径
	setFailAfterNoRealloc(1);
	char *printed = RyanJsonPrint(obj, RyanJsonPrintfPreAlloSize, RyanJsonFalse, NULL);
	restoreHooks();

	if (printed) { RyanJsonFree(printed); }
	TEST_ASSERT_NULL_MESSAGE(printed, "Print 扩容 fallback(malloc) 失败应返回 NULL");
	RyanJsonDelete(obj);
}

static void testDuplicateEmptyContainerAndOomPaths(void)
{
	RyanJson_t root = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(root);
	RyanJson_t emptyObj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(emptyObj);
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, "e", emptyObj));

	RyanJson_t dup = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL_MESSAGE(dup, "Duplicate(含空容器) 应成功");
	RyanJson_t dupE = RyanJsonGetObjectToKey(dup, "e");
	TEST_ASSERT_NOT_NULL(dupE);
	TEST_ASSERT_TRUE(RyanJsonIsObject(dupE));
	TEST_ASSERT_EQUAL_UINT32(0U, RyanJsonGetSize(dupE));
	RyanJsonDelete(dup);
	RyanJsonDelete(root);

	root = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(root);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(root, "a", 1));
	// 根节点复制成功，首个子节点复制失败，覆盖 Duplicate error__ 路径
	expectCreateNullUnderOom(1, RyanJsonDuplicate(root), "Duplicate OOM 路径应返回 NULL");
	RyanJsonDelete(root);
}

static void testOomCreateParsePrint(void)
{
	// 创建对象：首次分配失败
	setFailAfter(0);
	RyanJson_t obj = RyanJsonCreateObject();
	restoreHooks();
	if (obj) { RyanJsonDelete(obj); }
	TEST_ASSERT_NULL_MESSAGE(obj, "CreateObject OOM 应返回 NULL");

	// 解析流程：中途分配失败（长 key 触发 key buffer 分配）
	const char *longKeyJson =
		"{\"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\":1}";
	setFailAfter(1); // root 成功，key buffer 失败
	RyanJson_t json = RyanJsonParse(longKeyJson);
	restoreHooks();
	if (json) { RyanJsonDelete(json); }
	TEST_ASSERT_NULL_MESSAGE(json, "Parse OOM 应返回 NULL");

	// 打印流程：分配打印缓冲失败
	obj = RyanJsonCreateObject();
	RyanJsonAddIntToObject(obj, "a", 1);
	setFailAfter(0);
	char *printed = RyanJsonPrint(obj, 32, RyanJsonFalse, NULL);
	restoreHooks();
	TEST_ASSERT_NULL_MESSAGE(printed, "Print OOM 应返回 NULL");
	RyanJsonDelete(obj);
}

void testRobustRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testParseOptionsTerminator);
	RUN_TEST(testMinifyComplexEscapesAndComments);
	RUN_TEST(testMinifyNoTerminatorOverflow);
	RUN_TEST(testMinifyCommentBoundaryCases);
	RUN_TEST(testMinifyWhitespaceAndNonCommentSlashPaths);
	RUN_TEST(testParseAllocatedKeyCleanupOnValueError);
	RUN_TEST(testPrintPreallocatedArgGuards);
	RUN_TEST(testInitHooksAndCreateApiGuards);
	RUN_TEST(testNumberBoundaries);
	RUN_TEST(testVarargsPathTypeMismatchAndNullInput);
	RUN_TEST(testDuplicateKeyDetach);
	RUN_TEST(testInsertOutOfRangeAndKeyValidation);
	RUN_TEST(testGetSizeNullAndContainer);
	RUN_TEST(testInternalUtilsBranchCoverage);
	RUN_TEST(testTypedArrayCreationZeroCount);
	RUN_TEST(testTypedArrayCreationOomPaths);
	RUN_TEST(testCreateScalarOomAndTypeGuards);
	RUN_TEST(testPrintExpandFallbackWithoutRealloc);
	RUN_TEST(testDuplicateEmptyContainerAndOomPaths);
	RUN_TEST(testOomCreateParsePrint);
}
