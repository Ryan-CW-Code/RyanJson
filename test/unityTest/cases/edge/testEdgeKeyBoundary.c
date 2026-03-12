#include "testBase.h"

static char *allocPatternString(uint32_t len, char ch)
{
	char *buf = (char *)malloc(len + 1U);
	if (NULL == buf) { return NULL; }
	for (uint32_t i = 0; i < len; i++)
	{
		buf[i] = ch;
	}
	buf[len] = '\0';
	return buf;
}

static void testEdgeKeyBoundaryChangeKeyShortToLong(void)
{
	// 复杂链路：
	// Create(Object) -> AddIntToObject(短 key) -> ChangeKey(长 key) -> Compare。
	// 目标：验证短 key 切换到长 key 后结构正确。
	uint32_t shortLen = (RyanJsonInlineStringSize > 8U) ? (RyanJsonInlineStringSize - 2U) : 8U;
	uint32_t longLen = RyanJsonInlineStringSize + 8U;
	char *shortKey = allocPatternString(shortLen, 's');
	char *longKey = allocPatternString(longLen, 'l');
	TEST_ASSERT_NOT_NULL(shortKey);
	TEST_ASSERT_NOT_NULL(longKey);

	RyanJson_t root = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(root);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(root, shortKey, 1));

	RyanJson_t node = RyanJsonGetObjectByKey(root, shortKey);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(node, longKey));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, longKey));

	RyanJson_t expect = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(expect, longKey, 1));
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
	free(shortKey);
	free(longKey);
}

static void testEdgeKeyBoundaryChangeKeyLongToShort(void)
{
	// 复杂链路：
	// Create(Object) -> AddIntToObject(长 key) -> ChangeKey(短 key) -> Compare。
	// 目标：验证长 key 切换到短 key 后结构正确。
	uint32_t shortLen = (RyanJsonInlineStringSize > 8U) ? (RyanJsonInlineStringSize - 2U) : 8U;
	uint32_t longLen = RyanJsonInlineStringSize + 8U;
	char *shortKey = allocPatternString(shortLen, 's');
	char *longKey = allocPatternString(longLen, 'l');
	TEST_ASSERT_NOT_NULL(shortKey);
	TEST_ASSERT_NOT_NULL(longKey);

	RyanJson_t root = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(root);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(root, longKey, 2));

	RyanJson_t node = RyanJsonGetObjectByKey(root, longKey);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(node, shortKey));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, shortKey));

	RyanJson_t expect = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(expect, shortKey, 2));
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
	free(shortKey);
	free(longKey);
}

static void testEdgeKeyBoundaryChangeKeyLongToLongDifferent(void)
{
	// 复杂链路：
	// Create(Object) -> AddIntToObject(长 key) -> ChangeKey(另一个长 key)。
	// 目标：验证长 key 切换到另一长 key 后结构正确。
	uint32_t longLen = RyanJsonInlineStringSize + 8U;
	char *longKeyA = allocPatternString(longLen, 'a');
	char *longKeyB = allocPatternString(longLen, 'b');
	TEST_ASSERT_NOT_NULL(longKeyA);
	TEST_ASSERT_NOT_NULL(longKeyB);

	RyanJson_t root = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(root);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(root, longKeyA, 3));

	RyanJson_t node = RyanJsonGetObjectByKey(root, longKeyA);
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(node, longKeyB));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, longKeyB));

	RyanJson_t expect = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(expect, longKeyB, 3));
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
	free(longKeyA);
	free(longKeyB);
}

static void testEdgeKeyBoundaryChangeKeyKeepsValue(void)
{
	// 复杂链路：
	// Create(Object) -> AddIntToObject -> ChangeKey -> GetIntValue。
	// 目标：验证改 key 不影响原值。
	uint32_t longLen = RyanJsonInlineStringSize + 8U;
	char *longKey = allocPatternString(longLen, 'k');
	TEST_ASSERT_NOT_NULL(longKey);

	RyanJson_t root = RyanJsonParse("{\"a\":7}");
	TEST_ASSERT_NOT_NULL(root);
	RyanJson_t node = RyanJsonGetObjectByKey(root, "a");
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(node, longKey));
	TEST_ASSERT_EQUAL_INT(7, RyanJsonGetIntValue(RyanJsonGetObjectByKey(root, longKey)));

	RyanJsonDelete(root);
	free(longKey);
}

static void testEdgeKeyRoundtripLongKey(void)
{
	// 复杂链路：
	// Create(Object长 key) -> Print -> Parse -> Compare。
	// 目标：验证长 key 往返一致。
	uint32_t longLen = RyanJsonInlineStringSize + 8U;
	char *longKey = allocPatternString(longLen, 'k');
	TEST_ASSERT_NOT_NULL(longKey);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, longKey, 1));

	char *printed = RyanJsonPrint(obj, 32, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(obj);
	free(longKey);
}

static void testEdgeKeyRoundtripNestedLongKey(void)
{
	// 复杂链路：
	// Create(嵌套对象长 key) -> Print -> Parse -> Compare。
	// 目标：验证长 key 在嵌套对象中的往返一致性。
	uint32_t longLen = RyanJsonInlineStringSize + 8U;
	char *longKey = allocPatternString(longLen, 'k');
	TEST_ASSERT_NOT_NULL(longKey);

	RyanJson_t root = RyanJsonCreateObject();
	RyanJson_t child = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(root);
	TEST_ASSERT_NOT_NULL(child);

	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(child, longKey, "v"));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, "child", child));

	char *printed = RyanJsonPrint(root, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
	free(longKey);
}

static void testEdgeKeyBoundaryExactInlineLenKey(void)
{
	// 复杂链路：
	// Create(Object) -> AddIntToObject(key 长度=InlineSize) -> Print/Parse -> Compare。
	// 目标：验证边界长度 key 往返一致。
	uint32_t keyLen = RyanJsonInlineStringSize;
	char *key = allocPatternString(keyLen, 'k');
	TEST_ASSERT_NOT_NULL(key);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, key, 1));

	char *printed = RyanJsonPrint(obj, 32, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(obj);
	free(key);
}

static void testEdgeKeyBoundaryInlinePlusOneKey(void)
{
	// 复杂链路：
	// Create(Object) -> AddIntToObject(key 长度=InlineSize+1) -> Print/Parse -> Compare。
	// 目标：验证边界+1 长度 key 往返一致。
	uint32_t keyLen = RyanJsonInlineStringSize + 1U;
	char *key = allocPatternString(keyLen, 'k');
	TEST_ASSERT_NOT_NULL(key);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(obj);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(obj, key, 1));

	char *printed = RyanJsonPrint(obj, 32, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(obj);
	free(key);
}

void testEdgeKeyBoundaryRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testEdgeKeyBoundaryChangeKeyShortToLong);
	RUN_TEST(testEdgeKeyBoundaryChangeKeyLongToShort);
	RUN_TEST(testEdgeKeyBoundaryChangeKeyLongToLongDifferent);
	RUN_TEST(testEdgeKeyBoundaryChangeKeyKeepsValue);
	RUN_TEST(testEdgeKeyRoundtripLongKey);
	RUN_TEST(testEdgeKeyRoundtripNestedLongKey);
	RUN_TEST(testEdgeKeyBoundaryExactInlineLenKey);
	RUN_TEST(testEdgeKeyBoundaryInlinePlusOneKey);
}
