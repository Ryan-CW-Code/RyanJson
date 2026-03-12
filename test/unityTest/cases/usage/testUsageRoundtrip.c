#include "testBase.h"

static void assertUsageRoundtripDocument(const char *jsonText)
{
	RyanJson_t root = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL(root);

	char *printed = RyanJsonPrint(root, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);

	RyanJson_t reparsed = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(reparsed);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, reparsed));

	RyanJsonDelete(reparsed);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

// usage 层只保留真实文档结构的 roundtrip 契约；
// 字符串转义、Unicode、空容器基础值矩阵等已由 standard/equality 承接。
#define USAGE_ROUNDTRIP_STRUCTURAL_CASES                                                                                                   \
	X(testUsageRoundtripRootArrayRecords, "[{\"id\":1,\"v\":true},{\"id\":2,\"v\":false}]")                                            \
	X(testUsageRoundtripMixedLiteralArrayInObject, "{\"mix\":[1,\"2\",null,true,false,{\"x\":1}],\"end\":0}")                          \
	X(testUsageRoundtripDeepMixedContainers, "{\"arr\":[0,{\"m\":{\"n\":[1,{\"o\":\"p\"}]}}],\"flag\":false}")                         \
	X(testUsageRoundtripNestedNullAndEmptyFields, "{\"doc\":{\"title\":\"x\",\"subtitle\":null,\"tags\":[],\"meta\":{}},\"ok\":true}") \
	X(testUsageRoundtripSiblingListsBoard,                                                                                             \
	  "{\"board\":{\"todo\":[{\"id\":1,\"tags\":[\"a\"]}],\"doing\":[{\"id\":2,\"tags\":[]}],\"done\":[]}}")                           \
	X(testUsageRoundtripListItemsWithNestedTags,                                                                                       \
	  "{\"list\":[{\"id\":1,\"tags\":[\"a\",\"b\"]},{\"id\":2,\"tags\":[]}],\"meta\":{\"count\":2}}")

#define USAGE_ROUNDTRIP_BUSINESS_CASES                                                                                                     \
	X(testUsageRoundtripOrderSnapshot,                                                                                                 \
	  "{\"order\":{\"id\":\"O1001\",\"items\":[{\"sku\":\"A1\",\"qty\":2,\"price\":9.99},{\"sku\":\"B2\",\"qty\":1,\"price\":19.5}],"  \
	  "\"discount\":1.5},\"total\":37.98,\"paid\":true}")                                                                              \
	X(testUsageRoundtripFeatureFlagRules, "{\"flags\":{\"new_ui\":true,\"beta\":false},\"conditions\":[{\"key\":\"region\","           \
					      "\"in\":[\"us\",\"eu\"]},{\"key\":\"tier\",\"in\":[\"pro\"]}]}")                             \
	X(testUsageRoundtripMetricsSeries,                                                                                                 \
	  "{\"metrics\":{\"series\":[{\"t\":1,\"v\":1.1},{\"t\":2,\"v\":1.2},{\"t\":3,\"v\":1.3}],\"unit\":\"ms\"}}")                      \
	X(testUsageRoundtripI18nCatalog,                                                                                                   \
	  "{\"i18n\":{\"en\":{\"ok\":\"OK\",\"cancel\":\"Cancel\"},\"zh\":{\"ok\":\"\\u786e\\u5b9a\",\"cancel\":\"\\u53d6\\u6d88\"}}}")    \
	X(testUsageRoundtripProductCatalog,                                                                                                \
	  "{\"catalog\":{\"id\":\"C1\",\"products\":[{\"id\":\"P1\",\"name\":\"Pen\",\"attrs\":"                                           \
	  "{\"color\":\"blue\",\"size\":\"M\"}},{\"id\":\"P2\",\"name\":\"Box\",\"attrs\":{}}]},\"ver\":3}")

#define X(name, jsonText)                                                                                                                  \
	static void name(void)                                                                                                             \
	{                                                                                                                                  \
		assertUsageRoundtripDocument((jsonText));                                                                                  \
	}
USAGE_ROUNDTRIP_STRUCTURAL_CASES
USAGE_ROUNDTRIP_BUSINESS_CASES
#undef X

void testUsageRoundtripRunner(void)
{
	UnitySetTestFile(__FILE__);

#define X(name, jsonText) RUN_TEST(name);
	USAGE_ROUNDTRIP_STRUCTURAL_CASES
	USAGE_ROUNDTRIP_BUSINESS_CASES
#undef X
}
