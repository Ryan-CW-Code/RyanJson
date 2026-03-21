#include "testBase.h"

static void testUsageRecipeMinifyParseAddAndRoundtrip(void)
{
	// 复杂链路：
	// Minify -> Parse -> AddIntToObject -> Print -> Parse -> Compare。
	// 目标：验证用户侧常见“压缩输入后再增量构造”的完整文档管线。
	char raw[] = "{ /*c*/ \"a\" : 1 }";
	uint32_t len = RyanJsonMinify(raw, (int32_t)(sizeof(raw) - 1U));
	raw[len] = '\0';

	RyanJson_t root = RyanJsonParse(raw);
	TEST_ASSERT_NOT_NULL(root);
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(root, "b", 2));

	char *printed = RyanJsonPrint(root, 32, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);

	RyanJson_t expect = RyanJsonParse("{\"a\":1,\"b\":2}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE(RyanJsonCompare(reparsed, expect));

	RyanJsonDelete(expect);
	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testUsageRecipeMinifyParseReplaceAndRoundtrip(void)
{
	// 复杂链路：
	// Minify -> Parse -> ReplaceByKey/ReplaceByIndex -> Print -> Parse -> Compare。
	// 目标：验证用户侧“压缩输入后再局部替换”的文档管线。
	char raw[] = "/*h*/{\"cfg\":{\"x\":1},\"arr\":[1,2]}/*t*/";
	uint32_t minLen = RyanJsonMinify(raw, (int32_t)strlen(raw));
	TEST_ASSERT_EQUAL_STRING("{\"cfg\":{\"x\":1},\"arr\":[1,2]}", raw);
	TEST_ASSERT_EQUAL_UINT32((uint32_t)strlen(raw), minLen);

	RyanJson_t root = RyanJsonParse(raw);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "Minify 后文档解析失败");

	RyanJson_t cfgNew = RyanJsonParse("{\"x\":2,\"y\":3}");
	TEST_ASSERT_NOT_NULL(cfgNew);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(root, "cfg", cfgNew), "ReplaceByKey(cfg) 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(root, "arr"), 1, RyanJsonCreateInt(NULL, 5)),
				 "ReplaceByIndex(arr[1]) 失败");

	RyanJson_t expect = RyanJsonParse("{\"cfg\":{\"x\":2,\"y\":3},\"arr\":[1,5]}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, expect), "Minify + Replace 后结果与期望文档不一致");

	char *printed = RyanJsonPrint(root, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, roundtrip), "Minify + Replace 链路往返 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

static void testUsageRecipeMinifyStreamParseOptionsMergeDocs(void)
{
	// 复杂链路：
	// Minify(多文档流) -> ParseOptions(doc1) -> ParseOptions(doc2) -> AddItemToObject(root, parsedRoot)
	// -> Print -> Parse -> Compare。
	// 目标：验证注释流经 Minify 后，可顺序解析多个文档，
	// 且“第二个解析出来的根 Array 文档”可直接作为容器挂到第一个根 Object 上。
	char raw[] = "/*lead*/ {\"meta\":{\"id\":1}} /*mid*/ [1,{\"k\":2},[]] /*tail*/";
	uint32_t minLen = RyanJsonMinify(raw, (int32_t)strlen(raw));
	TEST_ASSERT_EQUAL_STRING("{\"meta\":{\"id\":1}}[1,{\"k\":2},[]]", raw);
	TEST_ASSERT_EQUAL_UINT32((uint32_t)strlen(raw), minLen);

	const char *end = NULL;
	RyanJson_t doc1 = RyanJsonParseOptions(raw, minLen, RyanJsonFalse, &end);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc1, "流文档 #1 解析失败");
	TEST_ASSERT_NOT_NULL(end);

	uint32_t remain = (uint32_t)(minLen - (uint32_t)(end - raw));
	RyanJson_t doc2 = RyanJsonParseOptions(end, remain, RyanJsonTrue, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc2, "流文档 #2 解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToObject(doc1, "payload", doc2), "将解析出的根 Array 挂到 doc1.payload 失败");

	RyanJson_t expect = RyanJsonParse("{\"meta\":{\"id\":1},\"payload\":[1,{\"k\":2},[]]}");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(doc1, expect), "多文档合并后的结果与期望文档不一致");

	char *printed = RyanJsonPrint(doc1, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(doc1, roundtrip), "多文档合并链路往返 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(expect);
	RyanJsonDelete(doc1);
}

static void testUsageRecipeCreateCollectorFromMinifiedStreamDocs(void)
{
	// 复杂链路：
	// Minify(多文档流) -> ParseOptions(doc1/doc2/doc3) -> Create(ArrayRoot)
	// -> Insert(顺序收集 Object/Array/标量 根文档) -> Print -> Parse -> Compare。
	// 目标：验证用户可先 Create 收集容器，再顺序吸收 ParseOptions 解析出的多份顶层文档；
	// 该链路与“parsed root 直接合并 parsed root”不同，专门覆盖 create-root + parsed-docs 的 recipe。
	char raw[] = "/*lead*/ {\"meta\":{\"id\":1}} /*mid*/ [3,{\"k\":4}] /*tail*/ true";
	uint32_t minLen = RyanJsonMinify(raw, (int32_t)strlen(raw));
	TEST_ASSERT_EQUAL_STRING("{\"meta\":{\"id\":1}}[3,{\"k\":4}]true", raw);
	TEST_ASSERT_EQUAL_UINT32((uint32_t)strlen(raw), minLen);

	RyanJson_t collector = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL_MESSAGE(collector, "创建收集根 Array 失败");

	const char *end1 = NULL;
	RyanJson_t doc1 = RyanJsonParseOptions(raw, minLen, RyanJsonFalse, &end1);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc1, "流文档 #1(object) 解析失败");
	TEST_ASSERT_NOT_NULL(end1);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(collector, RyanJsonGetArraySize(collector), doc1), "将流文档 #1 插入收集 Array 失败");

	uint32_t remain1 = (uint32_t)(minLen - (uint32_t)(end1 - raw));
	const char *end2 = NULL;
	RyanJson_t doc2 = RyanJsonParseOptions(end1, remain1, RyanJsonFalse, &end2);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc2, "流文档 #2(array) 解析失败");
	TEST_ASSERT_NOT_NULL(end2);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(collector, RyanJsonGetArraySize(collector), doc2), "将流文档 #2 插入收集 Array 失败");

	uint32_t remain2 = (uint32_t)(minLen - (uint32_t)(end2 - raw));
	const char *end3 = NULL;
	RyanJson_t doc3 = RyanJsonParseOptions(end2, remain2, RyanJsonTrue, &end3);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc3, "流文档 #3(bool) 解析失败");
	TEST_ASSERT_NOT_NULL(end3);
	TEST_ASSERT_EQUAL_CHAR('\0', *end3);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(collector, RyanJsonGetArraySize(collector), doc3), "将流文档 #3 插入收集 Array 失败");

	RyanJson_t expect = RyanJsonParse("[{\"meta\":{\"id\":1}},[3,{\"k\":4}],true]");
	TEST_ASSERT_NOT_NULL(expect);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(collector, expect), "create-root 收集多文档流后的结构不符合预期");

	char *printed = RyanJsonPrint(collector, 160, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(collector, roundtrip), "create-root 收集多文档流后的往返 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(expect);
	RyanJsonDelete(collector);
}

void testUsageRecipesRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testUsageRecipeMinifyParseAddAndRoundtrip);
	RUN_TEST(testUsageRecipeMinifyParseReplaceAndRoundtrip);
	RUN_TEST(testUsageRecipeMinifyStreamParseOptionsMergeDocs);
	RUN_TEST(testUsageRecipeCreateCollectorFromMinifiedStreamDocs);
}
