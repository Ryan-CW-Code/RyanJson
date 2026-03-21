#include "testBase.h"

#define IntList                                                                                                                            \
	/* 零值测试 */                                                                                                                     \
	X(0)                                                                                                                               \
	X(-0)                                                                                                                              \
	/* 正负边界 */                                                                                                                     \
	X(1)                                                                                                                               \
	X(-1)                                                                                                                              \
	X(2)                                                                                                                               \
	X(-2)                                                                                                                              \
	/* 常见小 Int */                                                                                                                   \
	X(10)                                                                                                                              \
	X(-10)                                                                                                                             \
	X(100)                                                                                                                             \
	X(-100)                                                                                                                            \
	X(255)                                                                                                                             \
	X(-255)                                                                                                                            \
	X(256)                                                                                                                             \
	X(-256)                                                                                                                            \
	/* 常见数值 */                                                                                                                     \
	X(1000)                                                                                                                            \
	X(-1000)                                                                                                                           \
	X(9999)                                                                                                                            \
	X(-9999)                                                                                                                           \
	X(12345)                                                                                                                           \
	X(-12345)                                                                                                                          \
	X(65535)                                                                                                                           \
	X(-65535)                                                                                                                          \
	X(65536)                                                                                                                           \
	X(-65536)                                                                                                                          \
	/* 大 Int */                                                                                                                       \
	X(100000)                                                                                                                          \
	X(-100000)                                                                                                                         \
	X(1000000)                                                                                                                         \
	X(-1000000)                                                                                                                        \
	X(10000000)                                                                                                                        \
	X(-10000000)                                                                                                                       \
	X(100000000)                                                                                                                       \
	X(-100000000)                                                                                                                      \
	X(1000000000)                                                                                                                      \
	X(-1000000000)                                                                                                                     \
	/* 8位边界 */                                                                                                                      \
	X(127)                                                                                                                             \
	X(-128)                                                                                                                            \
	/* 16位边界 */                                                                                                                     \
	X(32767)                                                                                                                           \
	X(-32768)                                                                                                                          \
	/* 32位边界 */                                                                                                                     \
	X(2147483647)                                                                                                                      \
	X(-2147483648)                                                                                                                     \
	/* 特殊模式 */                                                                                                                     \
	X(1234567890)                                                                                                                      \
	X(-1234567890)                                                                                                                     \
	X(123456789)                                                                                                                       \
	X(-123456789)                                                                                                                      \
	/* 2的幂次 */                                                                                                                      \
	X(2)                                                                                                                               \
	X(4)                                                                                                                               \
	X(8)                                                                                                                               \
	X(16)                                                                                                                              \
	X(32)                                                                                                                              \
	X(64)                                                                                                                              \
	X(128)                                                                                                                             \
	X(512)                                                                                                                             \
	X(1024)                                                                                                                            \
	X(2048)                                                                                                                            \
	X(4096)                                                                                                                            \
	X(8192)                                                                                                                            \
	X(16384)                                                                                                                           \
	X(32768)                                                                                                                           \
	X(65536)                                                                                                                           \
	X(131072)                                                                                                                          \
	X(262144)                                                                                                                          \
	X(524288)                                                                                                                          \
	X(1048576)                                                                                                                         \
	X(2097152)                                                                                                                         \
	X(4194304)                                                                                                                         \
	X(8388608)                                                                                                                         \
	X(16777216)                                                                                                                        \
	X(33554432)                                                                                                                        \
	X(67108864)                                                                                                                        \
	X(134217728)                                                                                                                       \
	X(268435456)                                                                                                                       \
	X(536870912)                                                                                                                       \
	X(1073741824)

static const int32_t IntValueTable[] = {
#define X(a) a,
	IntList
#undef X
};

static const char *IntStringTable[] = {
#define X(a) "{\"int32_t\":" #a "}",
	IntList
#undef X
};

static const char *IntStringTable2[] = {
#define X(a) #a,
	IntList
#undef X
};

static const char *IntStringTable3[] = {
#define X(a) "[" #a "]",
	IntList
#undef X
};

/**
 * @brief Int 类型边界与一致性测试
 */
void testEqualityIntEdgeCases(void)
{
	// NULL 输入
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsInt(NULL), "RyanJsonIsInt(NULL) 应返回 false");

	// 类型混淆测试
	RyanJson_t dbl = RyanJsonCreateDouble("dbl", 123.456);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsNumber(dbl), "RyanJsonIsNumber(Double) 应返回 true");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsInt(dbl), "RyanJsonIsInt(Double) 应返回 false");
	RyanJsonDelete(dbl);

	RyanJson_t str = RyanJsonCreateString("str", "123");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsInt(str), "RyanJsonIsInt(String) 应返回 false");
	RyanJsonDelete(str);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsInt(obj), "RyanJsonIsInt(Object) 应返回 false");
	RyanJsonDelete(obj);

	RyanJson_t boolNode = RyanJsonCreateBool("bool", RyanJsonTrue);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsInt(boolNode), "RyanJsonIsInt(Bool) 应返回 false");
	RyanJsonDelete(boolNode);
}

static void testEqualityIntTypeBoundaries(void)
{
	// int32_t 边界内：应为 Int
	RyanJson_t intMax = RyanJsonParse("2147483647");
	TEST_ASSERT_NOT_NULL(intMax);
	TEST_ASSERT_TRUE(RyanJsonIsInt(intMax));
	TEST_ASSERT_EQUAL_INT32(2147483647, RyanJsonGetIntValue(intMax));
	RyanJsonDelete(intMax);

	RyanJson_t intMin = RyanJsonParse("-2147483648");
	TEST_ASSERT_NOT_NULL(intMin);
	TEST_ASSERT_TRUE(RyanJsonIsInt(intMin));
	TEST_ASSERT_EQUAL_INT32(INT32_MIN, RyanJsonGetIntValue(intMin));
	RyanJsonDelete(intMin);

	// 超出 int32_t 范围：应退化为 Double
	RyanJson_t overMax = RyanJsonParse("2147483648");
	TEST_ASSERT_NOT_NULL(overMax);
	TEST_ASSERT_TRUE(RyanJsonIsDouble(overMax));
	TEST_ASSERT_FALSE(RyanJsonIsInt(overMax));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(2147483648.0, RyanJsonGetDoubleValue(overMax)));
	RyanJsonDelete(overMax);

	RyanJson_t belowMin = RyanJsonParse("-2147483649");
	TEST_ASSERT_NOT_NULL(belowMin);
	TEST_ASSERT_TRUE(RyanJsonIsDouble(belowMin));
	TEST_ASSERT_FALSE(RyanJsonIsInt(belowMin));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(-2147483649.0, RyanJsonGetDoubleValue(belowMin)));
	RyanJsonDelete(belowMin);

	// 指数/小数语义：数值等于 Int 也应按 Double 处理
	RyanJson_t expInt = RyanJsonParse("1e0");
	TEST_ASSERT_NOT_NULL(expInt);
	TEST_ASSERT_TRUE(RyanJsonIsDouble(expInt));
	TEST_ASSERT_FALSE(RyanJsonIsInt(expInt));
	RyanJsonDelete(expInt);

	RyanJson_t fracInt = RyanJsonParse("1.0");
	TEST_ASSERT_NOT_NULL(fracInt);
	TEST_ASSERT_TRUE(RyanJsonIsDouble(fracInt));
	TEST_ASSERT_FALSE(RyanJsonIsInt(fracInt));
	RyanJsonDelete(fracInt);

	// -0 保持 Int 路径
	RyanJson_t negZero = RyanJsonParse("-0");
	TEST_ASSERT_NOT_NULL(negZero);
	TEST_ASSERT_TRUE(RyanJsonIsInt(negZero));
	TEST_ASSERT_EQUAL_INT32(0, RyanJsonGetIntValue(negZero));
	RyanJsonDelete(negZero);
}

static void testEqualityIntTableCommon(const char *const *stringTable, RyanJsonBool_e withKey)
{
	for (uint32_t i = 0; i < sizeof(IntValueTable) / sizeof(IntValueTable[0]); i++)
	{
		const char *jsonIntStr = stringTable[i];
		RyanJson_t jsonRoot = RyanJsonParse(jsonIntStr);
		TEST_ASSERT_NOT_NULL_MESSAGE(jsonRoot, jsonIntStr);

		RyanJson_t valueNode = withKey ? RyanJsonGetObjectToKey(jsonRoot, "int32_t") : jsonRoot;
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(valueNode),
					 withKey ? "Key 'int32_t' not found or not int32_t" : "Root node not int32_t");

		// 验证解析后的数值是否正确
		int32_t intValue = RyanJsonGetIntValue(valueNode);
		TEST_ASSERT_EQUAL_INT32_MESSAGE(IntValueTable[i], intValue, jsonIntStr);

		// 往返校验：序列化后再次解析，Int 值应保持一致
		char *serializedStr = RyanJsonPrint(jsonRoot, 128, RyanJsonFalse, NULL);
		TEST_ASSERT_NOT_NULL_MESSAGE(serializedStr, "序列化失败");
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);

		TEST_ASSERT_NOT_NULL(roundtripJson);
		RyanJson_t roundtripValueNode = withKey ? RyanJsonGetObjectToKey(roundtripJson, "int32_t") : roundtripJson;
		TEST_ASSERT_TRUE(RyanJsonIsInt(roundtripValueNode));

		int32_t roundtripValue = RyanJsonGetIntValue(roundtripValueNode);
		TEST_ASSERT_EQUAL_INT32_MESSAGE(IntValueTable[i], roundtripValue, "往返测试数值不匹配");

		RyanJsonDelete(roundtripJson);
	}
}

static void testEqualityIntTable(void)
{
	testEqualityIntTableCommon(IntStringTable, RyanJsonTrue);
}

static void testEqualityIntTable2(void)
{
	testEqualityIntTableCommon(IntStringTable2, RyanJsonFalse);
}

static void testEqualityIntArrayTable(const char *const *stringTable)
{
	for (uint32_t i = 0; i < sizeof(IntValueTable) / sizeof(IntValueTable[0]); i++)
	{
		const char *jsonIntStr = stringTable[i];
		RyanJson_t jsonRoot = RyanJsonParse(jsonIntStr);
		TEST_ASSERT_NOT_NULL_MESSAGE(jsonRoot, jsonIntStr);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsArray(jsonRoot), "Root node not array");
		TEST_ASSERT_EQUAL_INT_MESSAGE(1, RyanJsonGetSize(jsonRoot), "Array size not 1");

		RyanJson_t valueNode = RyanJsonGetObjectByIndex(jsonRoot, 0);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(valueNode), "Array[0] not int32_t");

		int32_t intValue = RyanJsonGetIntValue(valueNode);
		TEST_ASSERT_EQUAL_INT32_MESSAGE(IntValueTable[i], intValue, jsonIntStr);

		// 往返校验：序列化后再次解析，Int 值应保持一致
		char *serializedStr = RyanJsonPrint(jsonRoot, 128, RyanJsonFalse, NULL);
		TEST_ASSERT_NOT_NULL_MESSAGE(serializedStr, "序列化失败");
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);

		TEST_ASSERT_NOT_NULL(roundtripJson);
		TEST_ASSERT_TRUE(RyanJsonIsArray(roundtripJson));
		TEST_ASSERT_EQUAL_INT_MESSAGE(1, RyanJsonGetSize(roundtripJson), "Roundtrip array size not 1");

		RyanJson_t roundtripValueNode = RyanJsonGetObjectByIndex(roundtripJson, 0);
		TEST_ASSERT_TRUE(RyanJsonIsInt(roundtripValueNode));

		int32_t roundtripValue = RyanJsonGetIntValue(roundtripValueNode);
		TEST_ASSERT_EQUAL_INT32_MESSAGE(IntValueTable[i], roundtripValue, "往返测试数值不匹配");

		RyanJsonDelete(roundtripJson);
	}
}

static void testEqualityIntTable3(void)
{
	testEqualityIntArrayTable(IntStringTable3);
}

void testEqualityIntRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testEqualityIntEdgeCases);
	RUN_TEST(testEqualityIntTypeBoundaries);
	RUN_TEST(testEqualityIntTable);
	RUN_TEST(testEqualityIntTable2);
	RUN_TEST(testEqualityIntTable3);
}
