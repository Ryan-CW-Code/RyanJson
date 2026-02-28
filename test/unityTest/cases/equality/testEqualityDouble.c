#include "testBase.h"

#define DoubleList                                                                                                                         \
	/* 零值测试 */                                                                                                                     \
	X(0.0)                                                                                                                             \
	X(-0.0)                                                                                                                            \
	/* 正负整数边界 */                                                                                                                 \
	X(1.0)                                                                                                                             \
	X(-1.0)                                                                                                                            \
	X(2.0)                                                                                                                             \
	X(-2.0)                                                                                                                            \
	X(10.0)                                                                                                                            \
	X(-10.0)                                                                                                                           \
	X(100.0)                                                                                                                           \
	X(1000.0)                                                                                                                          \
	X(10000.0)                                                                                                                         \
	X(100000.0)                                                                                                                        \
	/* 简单小数(二进制精确表示) */                                                                                                     \
	X(0.5)                                                                                                                             \
	X(-0.5)                                                                                                                            \
	X(0.25)                                                                                                                            \
	X(-0.25)                                                                                                                           \
	X(0.125)                                                                                                                           \
	X(0.0625)                                                                                                                          \
	X(0.03125)                                                                                                                         \
	X(0.015625)                                                                                                                        \
	/* 常见小数 */                                                                                                                     \
	X(16.89)                                                                                                                           \
	X(-16.89)                                                                                                                          \
	X(123.456)                                                                                                                         \
	X(-123.456)                                                                                                                        \
	X(99.99)                                                                                                                           \
	X(-99.99)                                                                                                                          \
	X(1.5)                                                                                                                             \
	X(2.5)                                                                                                                             \
	X(3.5)                                                                                                                             \
	/* 小于1的小数 */                                                                                                                  \
	X(0.001)                                                                                                                           \
	X(-0.001)                                                                                                                          \
	X(0.0001)                                                                                                                          \
	X(0.00001)                                                                                                                         \
	X(0.000001)                                                                                                                        \
	X(0.123456789)                                                                                                                     \
	X(0.987654321)                                                                                                                     \
	X(0.111111111111111)                                                                                                               \
	/* 大数测试 */                                                                                                                     \
	X(999999.999999)                                                                                                                   \
	X(-999999.999999)                                                                                                                  \
	X(12345678.9)                                                                                                                      \
	X(99999999.0)                                                                                                                      \
	X(123456789.123456)                                                                                                                \
	X(9876543210.12345)                                                                                                                \
	/* 科学计数法 - 大数 */                                                                                                            \
	X(1.5e10)                                                                                                                          \
	X(-1.5e10)                                                                                                                         \
	X(1.23e8)                                                                                                                          \
	X(9.99e12)                                                                                                                         \
	X(1.0e15)                                                                                                                          \
	X(1.0e18)                                                                                                                          \
	X(1.0e20)                                                                                                                          \
	X(5.55e15)                                                                                                                         \
	/* 科学计数法 - 小数 */                                                                                                            \
	X(1.5e-10)                                                                                                                         \
	X(-1.5e-10)                                                                                                                        \
	X(9.87e-5)                                                                                                                         \
	X(1.0e-15)                                                                                                                         \
	X(5.5e-8)                                                                                                                          \
	X(1.0e-18)                                                                                                                         \
	X(1.0e-20)                                                                                                                         \
	X(1.23e-3)                                                                                                                         \
	X(-9.87e-7)                                                                                                                        \
	/* 数学常量 */                                                                                                                     \
	X(3.14159265358979)                                                                                                                \
	X(2.71828182845904)                                                                                                                \
	X(1.41421356237309)                                                                                                                \
	X(1.73205080756888)                                                                                                                \
	X(1.61803398874989)                                                                                                                \
	X(0.69314718055994)                                                                                                                \
	/* 浮点精度经典测试 */                                                                                                             \
	X(0.1)                                                                                                                             \
	X(0.2)                                                                                                                             \
	X(0.3)                                                                                                                             \
	X(0.6)                                                                                                                             \
	X(0.7)                                                                                                                             \
	X(0.9)                                                                                                                             \
	X(0.123456)                                                                                                                        \
	/* 整数边界值 */                                                                                                                   \
	X(2147483647.0)                                                                                                                    \
	X(-2147483648.0)                                                                                                                   \
	X(4294967295.0)                                                                                                                    \
	X(9007199254740991.0)                                                                                                              \
	X(-9007199254740991.0)                                                                                                             \
	/* 极端小值 */                                                                                                                     \
	X(1.0e-100)                                                                                                                        \
	X(-1.0e-100)                                                                                                                       \
	X(1.0e-200)                                                                                                                        \
	X(1.0e-300)                                                                                                                        \
	X(2.225073858507201e-308)                                                                                                          \
	/* 极端大值 */                                                                                                                     \
	X(1.0e100)                                                                                                                         \
	X(-1.0e100)                                                                                                                        \
	X(1.0e200)                                                                                                                         \
	X(1.0e300)                                                                                                                         \
	X(1.797693134862315e308)                                                                                                           \
	/* 特殊精度值 */                                                                                                                   \
	X(1.0000000000001)                                                                                                                 \
	X(0.9999999999999)                                                                                                                 \
	X(1.23456789012345)                                                                                                                \
	X(9.87654321098765)                                                                                                                \
	/* 重复数字模式 */                                                                                                                 \
	X(1.1111111111111)                                                                                                                 \
	X(2.2222222222222)                                                                                                                 \
	X(9.9999999999999)                                                                                                                 \
	/* 混合符号和指数 */                                                                                                               \
	X(-1.23e-45)                                                                                                                       \
	X(-9.87e67)                                                                                                                        \
	X(1.11e-11)                                                                                                                        \
	X(-2.22e22)

static const double DoubleValueTable[] = {
#define X(a) a,
	DoubleList
#undef X
};

static const char *DoubleStringTable[] = {
#define X(a) "{\"double\":" #a "}",
	DoubleList
#undef X
};

static const char *DoubleStringTable2[] = {
#define X(a) #a,
	DoubleList
#undef X
};

static const char *DoubleStringTable3[] = {
#define X(a) "[" #a "]",
	DoubleList
#undef X
};

static RyanJsonBool_e shouldSkipLargeDoubleWhenNoScientific(double value)
{
#if false == RyanJsonSnprintfSupportScientific
	/* 不支持科学计数法的平台上，跳过超大值序列化往返校验。 */
	return RyanJsonMakeBool(fabs(value) >= 1.0e100);
#else
	(void)value;
	return RyanJsonFalse;
#endif
}

/**
 * @brief 浮点类型边界与一致性测试
 */
void testEqualityDoubleEdgeCases(void)
{
	// NULL 输入
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsDouble(NULL), "RyanJsonIsDouble(NULL) 应返回 false");

	// 类型混淆测试
	RyanJson_t num = RyanJsonCreateInt("num", 123);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsNumber(num), "RyanJsonIsNumber(Int) 应返回 true");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsDouble(num), "RyanJsonIsDouble(Int) 应返回 false");
	RyanJsonDelete(num);

	RyanJson_t str = RyanJsonCreateString("str", "123.456");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsDouble(str), "RyanJsonIsDouble(String) 应返回 false");
	RyanJsonDelete(str);

	RyanJson_t obj = RyanJsonCreateObject();
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsDouble(obj), "RyanJsonIsDouble(Object) 应返回 false");
	RyanJsonDelete(obj);

	RyanJson_t boolNode = RyanJsonCreateBool("bool", RyanJsonTrue);
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsDouble(boolNode), "RyanJsonIsDouble(Bool) 应返回 false");
	RyanJsonDelete(boolNode);
}

static void testEqualityDoubleTypeIdentity(void)
{
	// 指数形式即使数值是整数，也应保持 double 类型
	RyanJson_t expNode = RyanJsonParse("1e0");
	TEST_ASSERT_NOT_NULL(expNode);
	TEST_ASSERT_TRUE(RyanJsonIsDouble(expNode));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(1.0, RyanJsonGetDoubleValue(expNode)));

	char *expPrinted = RyanJsonPrint(expNode, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(expPrinted);
	RyanJson_t expRoundtrip = RyanJsonParse(expPrinted);
	RyanJsonFree(expPrinted);
	TEST_ASSERT_NOT_NULL(expRoundtrip);
	TEST_ASSERT_TRUE(RyanJsonIsDouble(expRoundtrip));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(1.0, RyanJsonGetDoubleValue(expRoundtrip)));
	RyanJsonDelete(expRoundtrip);
	RyanJsonDelete(expNode);

	// 小数形式应保持 double
	RyanJson_t fracNode = RyanJsonParse("-2.5000");
	TEST_ASSERT_NOT_NULL(fracNode);
	TEST_ASSERT_TRUE(RyanJsonIsDouble(fracNode));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(-2.5, RyanJsonGetDoubleValue(fracNode)));
	RyanJsonDelete(fracNode);

	// 超出 int32_t 范围的纯数字，应归类为 double
	RyanJson_t overInt = RyanJsonParse("2147483648");
	TEST_ASSERT_NOT_NULL(overInt);
	TEST_ASSERT_TRUE(RyanJsonIsDouble(overInt));
	TEST_ASSERT_FALSE(RyanJsonIsInt(overInt));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(2147483648.0, RyanJsonGetDoubleValue(overInt)));
	RyanJsonDelete(overInt);

	RyanJson_t belowInt = RyanJsonParse("-2147483649");
	TEST_ASSERT_NOT_NULL(belowInt);
	TEST_ASSERT_TRUE(RyanJsonIsDouble(belowInt));
	TEST_ASSERT_FALSE(RyanJsonIsInt(belowInt));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(-2147483649.0, RyanJsonGetDoubleValue(belowInt)));
	RyanJsonDelete(belowInt);
}

static void testEqualityDoubleTableCommon(const char *const *stringTable, RyanJsonBool_e withKey)
{
	for (uint32_t i = 0; i < sizeof(DoubleValueTable) / sizeof(DoubleValueTable[0]); i++)
	{
		double expectValue = DoubleValueTable[i];
		if (shouldSkipLargeDoubleWhenNoScientific(expectValue)) { continue; }

		const char *jsondoubleStr = stringTable[i];
		RyanJson_t jsonRoot = RyanJsonParse(jsondoubleStr);
		TEST_ASSERT_NOT_NULL_MESSAGE(jsonRoot, jsondoubleStr);

		RyanJson_t valueNode = withKey ? RyanJsonGetObjectToKey(jsonRoot, "double") : jsonRoot;
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDouble(valueNode), withKey ? "字段 'double' 类型错误" : "Root node not double");

		// 验证解析后的数值是否正确
		double doubleValue = RyanJsonGetDoubleValue(valueNode);
		if (!RyanJsonCompareDouble(expectValue, doubleValue))
		{
			TEST_PRINTF("字符串: %s, 期望: %g, 实际: %g", jsondoubleStr, expectValue, doubleValue);
			TEST_FAIL();
		}

		// 往返校验：序列化后再次解析，数值应保持一致
		char *serializedStr = RyanJsonPrint(jsonRoot, 128, RyanJsonFalse, NULL);
		if (NULL == serializedStr)
		{
			TEST_PRINTF("序列化失败样本: %s", jsondoubleStr);
			RyanJsonDelete(jsonRoot);
			TEST_FAIL_MESSAGE("序列化失败");
		}
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);
		if (NULL == roundtripJson)
		{
			TEST_PRINTF("往返解析失败样本: %s", jsondoubleStr);
			TEST_FAIL_MESSAGE("往返测试：重新解析失败");
		}
		RyanJson_t roundtripValueNode = withKey ? RyanJsonGetObjectToKey(roundtripJson, "double") : roundtripJson;
		if (RyanJsonFalse == RyanJsonIsDouble(roundtripValueNode))
		{
			RyanJsonDelete(roundtripJson);
			TEST_FAIL_MESSAGE(withKey ? "往返测试：字段 'double' 类型错误" : "往返测试：Root node not double");
		}

		double roundtripValue = RyanJsonGetDoubleValue(roundtripValueNode);
		if (!RyanJsonCompareDouble(expectValue, roundtripValue))
		{
			TEST_PRINTF("往返测试失败：期望: %g, 实际: %g", expectValue, roundtripValue);
			TEST_FAIL();
		}

		RyanJsonDelete(roundtripJson);
	}
}

static void testEqualityDoubleTable(void)
{
	testEqualityDoubleTableCommon(DoubleStringTable, RyanJsonTrue);
}

static void testEqualityDoubleTable2(void)
{
	testEqualityDoubleTableCommon(DoubleStringTable2, RyanJsonFalse);
}

static void testEqualityDoubleTable3(void)
{
	for (uint32_t i = 0; i < sizeof(DoubleValueTable) / sizeof(DoubleValueTable[0]); i++)
	{
		double expectValue = DoubleValueTable[i];
		if (shouldSkipLargeDoubleWhenNoScientific(expectValue)) { continue; }

		const char *jsondoubleStr = DoubleStringTable3[i];
		RyanJson_t jsonRoot = RyanJsonParse(jsondoubleStr);
		TEST_ASSERT_NOT_NULL_MESSAGE(jsonRoot, jsondoubleStr);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsArray(jsonRoot), "Root node not array");
		TEST_ASSERT_EQUAL_INT_MESSAGE(1, RyanJsonGetSize(jsonRoot), "Array size not 1");

		RyanJson_t valueNode = RyanJsonGetObjectByIndex(jsonRoot, 0);
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDouble(valueNode), "Array[0] not double");

		double doubleValue = RyanJsonGetDoubleValue(valueNode);
		if (!RyanJsonCompareDouble(expectValue, doubleValue))
		{
			TEST_PRINTF("字符串: %s, 期望: %g, 实际: %g", jsondoubleStr, expectValue, doubleValue);
			TEST_FAIL();
		}

		// 往返校验：序列化后再次解析，数值应保持一致
		char *serializedStr = RyanJsonPrint(jsonRoot, 128, RyanJsonFalse, NULL);
		if (NULL == serializedStr)
		{
			TEST_PRINTF("序列化失败样本: %s", jsondoubleStr);
			RyanJsonDelete(jsonRoot);
			TEST_FAIL_MESSAGE("序列化失败");
		}
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);
		if (NULL == roundtripJson)
		{
			TEST_PRINTF("往返解析失败样本: %s", jsondoubleStr);
			TEST_FAIL_MESSAGE("往返测试：重新解析失败");
		}
		if (RyanJsonFalse == RyanJsonIsArray(roundtripJson))
		{
			RyanJsonDelete(roundtripJson);
			TEST_FAIL_MESSAGE("往返测试：Root node not array");
		}
		if (1 != RyanJsonGetSize(roundtripJson))
		{
			RyanJsonDelete(roundtripJson);
			TEST_FAIL_MESSAGE("往返测试：Array size not 1");
		}

		RyanJson_t roundtripValueNode = RyanJsonGetObjectByIndex(roundtripJson, 0);
		if (RyanJsonFalse == RyanJsonIsDouble(roundtripValueNode))
		{
			RyanJsonDelete(roundtripJson);
			TEST_FAIL_MESSAGE("往返测试：Array[0] not double");
		}

		double roundtripValue = RyanJsonGetDoubleValue(roundtripValueNode);
		if (!RyanJsonCompareDouble(expectValue, roundtripValue))
		{
			TEST_PRINTF("往返测试失败：期望: %g, 实际: %g", expectValue, roundtripValue);
			TEST_FAIL();
		}

		RyanJsonDelete(roundtripJson);
	}
}

void testEqualityDoubleRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testEqualityDoubleEdgeCases);
	RUN_TEST(testEqualityDoubleTypeIdentity);
	RUN_TEST(testEqualityDoubleTable);
	RUN_TEST(testEqualityDoubleTable2);
	RUN_TEST(testEqualityDoubleTable3);
}
