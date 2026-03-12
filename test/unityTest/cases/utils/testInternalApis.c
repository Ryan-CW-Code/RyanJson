#include "testBase.h"

static char *allocRepeatChar(char ch, uint32_t count)
{
	char *buf = (char *)malloc((size_t)count + 1U);
	TEST_ASSERT_NOT_NULL(buf);
	memset(buf, (int)ch, count);
	buf[count] = '\0';
	return buf;
}

static int gInternalReallocCalls = 0;

static void *internalReallocWrap(void *block, size_t size)
{
	gInternalReallocCalls++;
	return unityTestRealloc(block, size);
}

static void testInternalStrPtrModeBufSwap(void)
{
	const char *key = "k";
	uint32_t valueLen = RyanJsonInlineStringSize + 8U;
	char *value = allocRepeatChar('x', valueLen);

	RyanJson_t node = RyanJsonCreateString(key, value);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonIsString(node));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonGetPayloadStrIsPtrByFlag(node), "expected ptr mode");

	uint8_t *oldBuf = RyanJsonInternalGetStrPtrModeBuf(node);
	TEST_ASSERT_NOT_NULL(oldBuf);

	size_t keyLen = strlen(key);
	size_t total = keyLen + 1U + valueLen + 1U;
	uint8_t *newBuf = (uint8_t *)jsonMalloc(total);
	TEST_ASSERT_NOT_NULL(newBuf);

	memcpy(newBuf, key, keyLen);
	newBuf[keyLen] = '\0';
	memset(newBuf + keyLen + 1U, 'y', valueLen);
	newBuf[keyLen + 1U + valueLen] = '\0';

	RyanJsonInternalSetStrPtrModeBuf(node, newBuf);

	TEST_ASSERT_EQUAL_PTR(newBuf, RyanJsonInternalGetStrPtrModeBuf(node));
	TEST_ASSERT_EQUAL_PTR(newBuf, RyanJsonInternalGetStrPtrModeBufAt(node, 0));
	TEST_ASSERT_EQUAL_PTR(newBuf + keyLen + 1U, RyanJsonInternalGetStrPtrModeBufAt(node, (uint32_t)(keyLen + 1U)));
	TEST_ASSERT_EQUAL_STRING(key, RyanJsonGetKey(node));

	char *expect = allocRepeatChar('y', valueLen);
	TEST_ASSERT_EQUAL_STRING(expect, RyanJsonGetStringValue(node));

	free(expect);
	jsonFree(oldBuf);
	RyanJsonDelete(node);
	free(value);
}

static void testInternalExpandReallocUsesReallocHook(void)
{
	unityTestLeakScope_t scope = unityTestLeakScopeBegin();

	gInternalReallocCalls = 0;
	RyanJsonInitHooks(unityTestMalloc, unityTestFree, internalReallocWrap);

	uint8_t *buf = (uint8_t *)unityTestMalloc(8U);
	TEST_ASSERT_NOT_NULL(buf);
	for (uint32_t i = 0; i < 8U; i++)
	{
		buf[i] = (uint8_t)(0xA0U + i);
	}

	uint8_t *out = (uint8_t *)RyanJsonInternalExpandRealloc(buf, 8U, 32U);
	TEST_ASSERT_NOT_NULL(out);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, gInternalReallocCalls, "realloc hook should be used when provided");
	for (uint32_t i = 0; i < 8U; i++)
	{
		TEST_ASSERT_EQUAL_UINT8((uint8_t)(0xA0U + i), out[i]);
	}

	RyanJsonFree(out);
	RyanJsonInitHooks(unityTestMalloc, unityTestFree, unityTestRealloc);
	unityTestLeakScopeEnd(scope, "ExpandRealloc with realloc should not leak");
}

static void testInternalExpandReallocFallbackCopy(void)
{
	unityTestLeakScope_t scope = unityTestLeakScopeBegin();

	RyanJsonInitHooks(unityTestMalloc, unityTestFree, NULL);

	uint8_t *buf = (uint8_t *)unityTestMalloc(12U);
	TEST_ASSERT_NOT_NULL(buf);
	for (uint32_t i = 0; i < 12U; i++)
	{
		buf[i] = (uint8_t)(0x5AU ^ i);
	}

	uint8_t *out = (uint8_t *)RyanJsonInternalExpandRealloc(buf, 12U, 48U);
	TEST_ASSERT_NOT_NULL(out);
	for (uint32_t i = 0; i < 12U; i++)
	{
		TEST_ASSERT_EQUAL_UINT8((uint8_t)(0x5AU ^ i), out[i]);
	}

	RyanJsonFree(out);
	RyanJsonInitHooks(unityTestMalloc, unityTestFree, unityTestRealloc);
	unityTestLeakScopeEnd(scope, "ExpandRealloc fallback should not leak");
}

static void testInternalNewNodeBoolFlag(void)
{
	RyanJsonNodeInfo_t info;
	memset(&info, 0, sizeof(info));
	info.type = RyanJsonTypeBool;
	info.boolIsTrueFlag = RyanJsonTrue;

	RyanJson_t node = RyanJsonInternalNewNode(&info);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonIsBool(node));
	TEST_ASSERT_TRUE(RyanJsonGetBoolValue(node));
	RyanJsonDelete(node);

	info.boolIsTrueFlag = RyanJsonFalse;
	node = RyanJsonInternalNewNode(&info);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonIsBool(node));
	TEST_ASSERT_FALSE(RyanJsonGetBoolValue(node));
	RyanJsonDelete(node);
}

static void testInternalNewNodeNumberPayloadAccess(void)
{
	RyanJsonNodeInfo_t info;
	memset(&info, 0, sizeof(info));
	info.type = RyanJsonTypeNumber;
	info.numberIsDoubleFlag = RyanJsonFalse;

	RyanJson_t intNode = RyanJsonInternalNewNode(&info);
	TEST_ASSERT_NOT_NULL(intNode);
	TEST_ASSERT_TRUE(RyanJsonIsInt(intNode));
	int32_t intValue = -42;
	RyanJsonMemcpy(RyanJsonInternalGetValue(intNode), &intValue, sizeof(intValue));
	TEST_ASSERT_EQUAL_INT(intValue, RyanJsonGetIntValue(intNode));
	RyanJsonDelete(intNode);

	info.numberIsDoubleFlag = RyanJsonTrue;
	RyanJson_t dblNode = RyanJsonInternalNewNode(&info);
	TEST_ASSERT_NOT_NULL(dblNode);
	TEST_ASSERT_TRUE(RyanJsonIsDouble(dblNode));
	double dblValue = 3.25;
	RyanJsonMemcpy(RyanJsonInternalGetValue(dblNode), &dblValue, sizeof(dblValue));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(dblValue, RyanJsonGetDoubleValue(dblNode)));
	RyanJsonDelete(dblNode);
}

static void testInternalCreateContainerWithKeyRoot(void)
{
	RyanJson_t obj = RyanJsonInternalCreateObjectAndKey("obj");
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonIsObject(obj));
	TEST_ASSERT_TRUE(RyanJsonIsKey(obj));
	TEST_ASSERT_EQUAL_STRING("obj", RyanJsonGetKey(obj));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, "a", 1));
	TEST_ASSERT_EQUAL_INT(1, RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "a")));

	RyanJson_t arr = RyanJsonInternalCreateArrayAndKey("arr");
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonIsArray(arr));
	TEST_ASSERT_TRUE(RyanJsonIsKey(arr));
	TEST_ASSERT_EQUAL_STRING("arr", RyanJsonGetKey(arr));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(arr, 7));
	TEST_ASSERT_EQUAL_INT(7, RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr, 0)));

	RyanJsonDelete(obj);
	RyanJsonDelete(arr);
}

static void testInternalChangeObjectValueWithPreparedChild(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	RyanJson_t child = RyanJsonCreateInt("a", 1);
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_NOT_NULL(child);

	child->next = obj;
	RyanJsonSetPayloadIsLastByFlag(child, 1);

	TEST_ASSERT_TRUE(RyanJsonInternalChangeObjectValue(obj, child));
	TEST_ASSERT_EQUAL_PTR(child, RyanJsonGetObjectValue(obj));
	TEST_ASSERT_EQUAL_PTR(child, RyanJsonGetObjectByIndex(obj, 0));

	RyanJsonDelete(obj);
}

static void testInternalParseDoubleRawNonNullTerminated(void)
{
	double number = 0.0;

	const uint8_t bufInt[] = {'1', '2', '3', '4'};
	TEST_ASSERT_TRUE(RyanJsonInternalParseDoubleRaw(bufInt, (uint32_t)sizeof(bufInt), &number));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(1234.0, number));

	const uint8_t bufFrac[] = {'-', '1', '2', '.', '5'};
	TEST_ASSERT_TRUE(RyanJsonInternalParseDoubleRaw(bufFrac, (uint32_t)sizeof(bufFrac), &number));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(-12.5, number));

	const uint8_t bufExp[] = {'1', 'e', '2'};
	TEST_ASSERT_TRUE(RyanJsonInternalParseDoubleRaw(bufExp, (uint32_t)sizeof(bufExp), &number));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(100.0, number));
}

static void testInternalStrEqFastPaths(void)
{
	const char *s = "abc";
	TEST_ASSERT_TRUE(RyanJsonInternalStrEq(s, s));
	TEST_ASSERT_TRUE(RyanJsonInternalStrEq("abc", "abc"));
	TEST_ASSERT_FALSE(RyanJsonInternalStrEq("abc", "abD"));
}

static void testInternalListInsertAfterHeadAndTail(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t first = RyanJsonCreateInt("a", 1);
	TEST_ASSERT_NOT_NULL(first);
	RyanJsonInternalListInsertAfter(obj, NULL, first);

	TEST_ASSERT_EQUAL_PTR(obj, RyanJsonInternalGetParent(first));
	TEST_ASSERT_EQUAL_PTR(first, RyanJsonGetObjectValue(obj));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonGetPayloadIsLastByFlag(first), "单节点插入后应为 last");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(first), "单节点 next 应返回 NULL");
	TEST_ASSERT_EQUAL_PTR(obj, first->next);

	RyanJson_t second = RyanJsonCreateInt("b", 2);
	TEST_ASSERT_NOT_NULL(second);
	RyanJsonInternalListInsertAfter(obj, first, second);

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonGetPayloadIsLastByFlag(first), "插入尾部后 prev 不应为 last");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonGetPayloadIsLastByFlag(second), "尾节点应标记为 last");
	TEST_ASSERT_EQUAL_PTR(second, RyanJsonGetNext(first));
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetNext(second), "尾节点 next 应返回 NULL");
	TEST_ASSERT_EQUAL_PTR(obj, second->next);
	TEST_ASSERT_EQUAL_PTR(obj, RyanJsonInternalGetParent(second));

	RyanJsonDelete(obj);
}

static void testInternalListInsertAfterMiddlePreservesTail(void)
{
	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);

	RyanJson_t first = RyanJsonCreateInt("a", 1);
	RyanJson_t tail = RyanJsonCreateInt("b", 2);
	TEST_ASSERT_NOT_NULL(first);
	TEST_ASSERT_NOT_NULL(tail);
	RyanJsonInternalListInsertAfter(obj, NULL, first);
	RyanJsonInternalListInsertAfter(obj, first, tail);

	RyanJson_t middle = RyanJsonCreateInt("c", 3);
	TEST_ASSERT_NOT_NULL(middle);
	RyanJsonInternalListInsertAfter(obj, first, middle);

	TEST_ASSERT_EQUAL_PTR(middle, RyanJsonGetNext(first));
	TEST_ASSERT_EQUAL_PTR(tail, RyanJsonGetNext(middle));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonGetPayloadIsLastByFlag(middle), "中间节点不应为 last");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonGetPayloadIsLastByFlag(tail), "原尾节点应保持 last");
	TEST_ASSERT_EQUAL_PTR(obj, RyanJsonInternalGetParent(middle));
	TEST_ASSERT_EQUAL_PTR(obj, RyanJsonInternalGetParent(tail));

	RyanJsonDelete(obj);
}

static void testInternalInitHooksArgumentGuards(void)
{
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonInitHooks(NULL, unityTestFree, unityTestRealloc), "InitHooks(NULL malloc) 应失败");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonInitHooks(unityTestMalloc, NULL, unityTestRealloc), "InitHooks(NULL free) 应失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInitHooks(unityTestMalloc, unityTestFree, unityTestRealloc), "恢复默认 hooks 应成功");
}

static void testInternalGetValueForPublicKeyedNumbers(void)
{
	RyanJson_t intNode = RyanJsonCreateInt("k", -7);
	TEST_ASSERT_NOT_NULL(intNode);
	int32_t intValue = 0;
	RyanJsonMemcpy(&intValue, RyanJsonInternalGetValue(intNode), sizeof(intValue));
	TEST_ASSERT_EQUAL_INT(-7, intValue);
	RyanJsonDelete(intNode);

	RyanJson_t doubleNode = RyanJsonCreateDouble("d", -8.75);
	TEST_ASSERT_NOT_NULL(doubleNode);
	double doubleValue = 0.0;
	RyanJsonMemcpy(&doubleValue, RyanJsonInternalGetValue(doubleNode), sizeof(doubleValue));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(-8.75, doubleValue));
	RyanJsonDelete(doubleNode);
}

static void testInternalKeyLenEncodingHelpers(void)
{
	// RyanJsonInternalCalcLenBytes：keyLen -> 字节数映射
	TEST_ASSERT_EQUAL_UINT8(1, RyanJsonInternalCalcLenBytes(0));
	TEST_ASSERT_EQUAL_UINT8(1, RyanJsonInternalCalcLenBytes(UINT8_MAX));
	TEST_ASSERT_EQUAL_UINT8(2, RyanJsonInternalCalcLenBytes((uint32_t)UINT8_MAX + 1U));
	TEST_ASSERT_EQUAL_UINT8(2, RyanJsonInternalCalcLenBytes(UINT16_MAX));
	TEST_ASSERT_EQUAL_UINT8(3, RyanJsonInternalCalcLenBytes((uint32_t)UINT16_MAX + 1U));

	// RyanJsonInternalDecodeKeyLenField：编码值 3 表示 4 字节，其余保持不变
	TEST_ASSERT_EQUAL_UINT8(0, RyanJsonInternalDecodeKeyLenField(0));
	TEST_ASSERT_EQUAL_UINT8(1, RyanJsonInternalDecodeKeyLenField(1));
	TEST_ASSERT_EQUAL_UINT8(2, RyanJsonInternalDecodeKeyLenField(2));
	TEST_ASSERT_EQUAL_UINT8(4, RyanJsonInternalDecodeKeyLenField(3));
}

static void testInternalGetKeyLenForDifferentSizes(void)
{
	RyanJson_t root = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(root);

	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(root, "", 1));
	RyanJson_t empty = RyanJsonGetObjectByKey(root, "");
	TEST_ASSERT_NOT_NULL(empty);
	TEST_ASSERT_EQUAL_UINT32(0U, RyanJsonInternalGetKeyLen(empty));

	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(root, "abc", 2));
	RyanJson_t shortNode = RyanJsonGetObjectByKey(root, "abc");
	TEST_ASSERT_NOT_NULL(shortNode);
	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonInternalGetKeyLen(shortNode));

	uint32_t longLen = (uint32_t)UINT8_MAX + 5U;
	char *longKey = (char *)malloc(longLen + 1U);
	TEST_ASSERT_NOT_NULL(longKey);
	memset(longKey, 'k', longLen);
	longKey[longLen] = '\0';

	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(root, longKey, 3));
	RyanJson_t longNode = RyanJsonGetObjectByKey(root, longKey);
	TEST_ASSERT_NOT_NULL(longNode);
	TEST_ASSERT_EQUAL_UINT32(longLen, RyanJsonInternalGetKeyLen(longNode));

	free(longKey);
	RyanJsonDelete(root);
}

static void testInternalInlineStringSizeFormula(void)
{
	// 验证 InlineStringSize 与当前平台布局公式保持一致，避免静态配置回归。
	uint32_t base = (uint32_t)(2U * sizeof(void *) + (uint32_t)(RyanJsonMallocHeaderSize / 2U));
	uint32_t expected = (uint32_t)RyanJsonAlign(base, RyanJsonMallocAlign) - (uint32_t)RyanJsonFlagSize;
	uint32_t actual = (uint32_t)RyanJsonInlineStringSize;

	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, actual, "InlineStringSize 公式不一致");
	TEST_ASSERT_TRUE_MESSAGE(((actual + (uint32_t)RyanJsonFlagSize) % (uint32_t)RyanJsonMallocAlign) == 0U,
				 "InlineStringSize 与对齐粒度不匹配");
	TEST_ASSERT_TRUE_MESSAGE(actual > 0U, "InlineStringSize 应为正数");
}

void testInternalApisRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testInternalStrPtrModeBufSwap);
	RUN_TEST(testInternalExpandReallocUsesReallocHook);
	RUN_TEST(testInternalExpandReallocFallbackCopy);
	RUN_TEST(testInternalInitHooksArgumentGuards);
	RUN_TEST(testInternalNewNodeBoolFlag);
	RUN_TEST(testInternalNewNodeNumberPayloadAccess);
	RUN_TEST(testInternalGetValueForPublicKeyedNumbers);
	RUN_TEST(testInternalKeyLenEncodingHelpers);
	RUN_TEST(testInternalGetKeyLenForDifferentSizes);
	RUN_TEST(testInternalInlineStringSizeFormula);
	RUN_TEST(testInternalCreateContainerWithKeyRoot);
	RUN_TEST(testInternalChangeObjectValueWithPreparedChild);
	RUN_TEST(testInternalParseDoubleRawNonNullTerminated);
	RUN_TEST(testInternalStrEqFastPaths);
	RUN_TEST(testInternalListInsertAfterHeadAndTail);
	RUN_TEST(testInternalListInsertAfterMiddlePreservesTail);
}
