#include "testBase.h"

static void testKeyMutationChangeToEmptyThenLookup(void)
{
	// 复杂链路：
	// Parse -> ChangeKey(空字符串) -> PathLookup -> Compare(期望文档)。
	// 目标：验证空 key 可被正确定位与序列化。
	RyanJson_t root = RyanJsonParse("{\"a\":{\"b\":1}}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t target = RyanJsonGetObjectToKey(root, "a", "b");
	TEST_ASSERT_NOT_NULL(target);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(target, ""));

	TEST_ASSERT_NULL(RyanJsonGetObjectToKey(root, "a", "b"));
	TEST_ASSERT_NOT_NULL(RyanJsonGetObjectToKey(root, "a", ""));

	RyanJson_t expect = RyanJsonParse("{\"a\":{\"\":1}}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

static void testKeyMutationChangeToEscapedThenLookup(void)
{
	// 复杂链路：
	// Parse -> ChangeKey(含引号/反斜杠) -> Lookup -> Compare(期望文档)。
	// 目标：验证转义 key 需使用“解码后字符串”进行查找。
	const char *newKey = "a\"b\\c";
	RyanJson_t root = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t target = RyanJsonGetObjectByKey(root, "a");
	TEST_ASSERT_NOT_NULL(target);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(target, newKey));

	TEST_ASSERT_NULL(RyanJsonGetObjectByKey(root, "a"));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, newKey));

	RyanJson_t expect = RyanJsonParse("{\"a\\\"b\\\\c\":1}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

static void testKeyMutationControlKeyRoundtrip(void)
{
	// 复杂链路：
	// Parse -> ChangeKey(含\t/\n) -> Print -> Parse -> Compare。
	// 目标：验证控制字符 key 的转义输出与回环解析。
	const char *key = "a\tb\nc";
	RyanJson_t root = RyanJsonParse("{\"k\":1}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t node = RyanJsonGetObjectByKey(root, "k");
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(node, key));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, key));

	char *printed = RyanJsonPrint(root, 0, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testKeyMutationUtf8KeyRoundtrip(void)
{
	// 复杂链路：
	// Parse -> ChangeKey(中文) -> Print -> Parse -> Compare。
	// 目标：验证 UTF-8 key 的序列化与解析一致性。
	const char *key = "中文";
	RyanJson_t root = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t node = RyanJsonGetObjectByKey(root, "a");
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(node, key));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, key));

	char *printed = RyanJsonPrint(root, 0, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testKeyMutationBackspaceKeyLookupRoundtrip(void)
{
	// 复杂链路：
	// Parse -> ChangeKey(含\b) -> Lookup -> Print/Parse -> Compare。
	// 目标：验证反斜杠控制字符 key 的查找与回环。
	const char *key = "a\b";
	RyanJson_t root = RyanJsonParse("{\"a\":1}");
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t node = RyanJsonGetObjectByKey(root, "a");
	TEST_ASSERT_NOT_NULL(node);
	TEST_ASSERT_TRUE(RyanJsonChangeKey(node, key));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, key));

	char *printed = RyanJsonPrint(root, 0, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

void testKeyMutationLookupRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testKeyMutationChangeToEmptyThenLookup);
	RUN_TEST(testKeyMutationChangeToEscapedThenLookup);
	RUN_TEST(testKeyMutationControlKeyRoundtrip);
	RUN_TEST(testKeyMutationUtf8KeyRoundtrip);
	RUN_TEST(testKeyMutationBackspaceKeyLookupRoundtrip);
}
