#include "RyanJsonBaseTest.h"

#define DoubleList                                                                                                                         \
	/* ========== 零值测试 ========== */                                                                                               \
	X(0.0)                                                                                                                             \
	/* ========== 正负整数边界 ========== */                                                                                           \
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
	/* ========== 简单小数(二进制精确表示) ========== */                                                                               \
	X(0.5)                                                                                                                             \
	X(-0.5)                                                                                                                            \
	X(0.25)                                                                                                                            \
	X(-0.25)                                                                                                                           \
	X(0.125)                                                                                                                           \
	X(0.0625)                                                                                                                          \
	X(0.03125)                                                                                                                         \
	X(0.015625)                                                                                                                        \
	/* ========== 常见小数 ========== */                                                                                               \
	X(16.89)                                                                                                                           \
	X(-16.89)                                                                                                                          \
	X(123.456)                                                                                                                         \
	X(-123.456)                                                                                                                        \
	X(99.99)                                                                                                                           \
	X(-99.99)                                                                                                                          \
	X(1.5)                                                                                                                             \
	X(2.5)                                                                                                                             \
	X(3.5)                                                                                                                             \
	/* ========== 小于1的小数 ========== */                                                                                            \
	X(0.001)                                                                                                                           \
	X(-0.001)                                                                                                                          \
	X(0.0001)                                                                                                                          \
	X(0.00001)                                                                                                                         \
	X(0.000001)                                                                                                                        \
	X(0.123456789)                                                                                                                     \
	X(0.987654321)                                                                                                                     \
	X(0.111111111111111)                                                                                                               \
	/* ========== 大数测试 ========== */                                                                                               \
	X(999999.999999)                                                                                                                   \
	X(-999999.999999)                                                                                                                  \
	X(12345678.9)                                                                                                                      \
	X(99999999.0)                                                                                                                      \
	X(123456789.123456)                                                                                                                \
	X(9876543210.12345)                                                                                                                \
	/* ========== 科学计数法 - 大数 ========== */                                                                                      \
	X(1.5e10)                                                                                                                          \
	X(-1.5e10)                                                                                                                         \
	X(1.23e8)                                                                                                                          \
	X(9.99e12)                                                                                                                         \
	X(1.0e15)                                                                                                                          \
	X(1.0e18)                                                                                                                          \
	X(1.0e20)                                                                                                                          \
	X(5.55e15)                                                                                                                         \
	/* ========== 科学计数法 - 小数 ========== */                                                                                      \
	X(1.5e-10)                                                                                                                         \
	X(-1.5e-10)                                                                                                                        \
	X(9.87e-5)                                                                                                                         \
	X(1.0e-15)                                                                                                                         \
	X(5.5e-8)                                                                                                                          \
	X(1.0e-18)                                                                                                                         \
	X(1.0e-20)                                                                                                                         \
	X(1.23e-3)                                                                                                                         \
	X(-9.87e-7)                                                                                                                        \
	/* ========== 数学常量 ========== */                                                                                               \
	X(3.14159265358979)                                                                                                                \
	X(2.71828182845904)                                                                                                                \
	X(1.41421356237309)                                                                                                                \
	X(1.73205080756888)                                                                                                                \
	X(1.61803398874989)                                                                                                                \
	X(0.69314718055994)                                                                                                                \
	/* ========== 浮点精度经典测试 ========== */                                                                                       \
	X(0.1)                                                                                                                             \
	X(0.2)                                                                                                                             \
	X(0.3)                                                                                                                             \
	X(0.6)                                                                                                                             \
	X(0.7)                                                                                                                             \
	X(0.9)                                                                                                                             \
	X(0.123456)                                                                                                                        \
	/* ========== 整数边界值 ========== */                                                                                             \
	X(2147483647.0)                                                                                                                    \
	X(-2147483648.0)                                                                                                                   \
	X(4294967295.0)                                                                                                                    \
	X(9007199254740991.0)                                                                                                              \
	X(-9007199254740991.0)                                                                                                             \
	/* ========== 极端小值 ========== */                                                                                               \
	X(1.0e-100)                                                                                                                        \
	X(-1.0e-100)                                                                                                                       \
	X(1.0e-200)                                                                                                                        \
	X(1.0e-300)                                                                                                                        \
	X(2.225073858507201e-308)                                                                                                          \
	/* ========== 极端大值 ========== */                                                                                               \
	X(1.0e100)                                                                                                                         \
	X(-1.0e100)                                                                                                                        \
	X(1.0e200)                                                                                                                         \
	X(1.0e300)                                                                                                                         \
	X(1.797693134862315e308)                                                                                                           \
	/* ========== 特殊精度值 ========== */                                                                                             \
	X(1.0000000000001)                                                                                                                 \
	X(0.9999999999999)                                                                                                                 \
	X(1.23456789012345)                                                                                                                \
	X(9.87654321098765)                                                                                                                \
	/* ========== 重复数字模式 ========== */                                                                                           \
	X(1.1111111111111)                                                                                                                 \
	X(2.2222222222222)                                                                                                                 \
	X(9.9999999999999)                                                                                                                 \
	/* ========== 混合符号和指数 ========== */                                                                                         \
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

// 浮点数一致性测试
RyanJsonBool_e RyanJsonBaseTestEqualityDouble(void)
{

	for (uint32_t i = 0; i < sizeof(DoubleValueTable) / sizeof(DoubleValueTable[0]); i++)
	{
		const char *jsondoubleStr = DoubleStringTable[i];
		RyanJson_t jsonRoot = RyanJsonParse(jsondoubleStr);
		RyanJsonCheckCode(NULL != jsonRoot, {
			jsonLog("str: %s", jsondoubleStr);
			goto err;
		});

		RyanJsonCheckReturnFalse(NULL != jsonRoot);
		RyanJsonCheckReturnFalse(RyanJsonIsDouble(RyanJsonGetObjectToKey(jsonRoot, "double")));

		// 验证解析后的数值是否正确
		double doubleValue = RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(jsonRoot, "double"));
		RyanJsonCheckCode(RyanJsonCompareDouble(doubleValue, DoubleValueTable[i]), {
			jsonLog("str: %s, expected: %g, got: %g", jsondoubleStr, DoubleValueTable[i], doubleValue);
			RyanJsonDelete(jsonRoot);
			goto err;
		});

		// 验证序列化后再解析，然后判断double是否一致（往返测试）
		char *serializedStr = RyanJsonPrint(jsonRoot, 128, RyanJsonFalse, NULL);
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);
		RyanJsonCheckReturnFalse(NULL != roundtripJson);
		RyanJsonCheckReturnFalse(RyanJsonIsDouble(RyanJsonGetObjectToKey(roundtripJson, "double")));

		double roundtripValue = RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(roundtripJson, "double"));
		RyanJsonCheckCode(RyanJsonCompareDouble(roundtripValue, DoubleValueTable[i]), {
			jsonLog("roundtrip failed: expected: %g, got: %g ", DoubleValueTable[i], roundtripValue);
			RyanJsonDelete(roundtripJson);
			goto err;
		});

		RyanJsonDelete(roundtripJson);
	}

	return RyanJsonTrue;

err:
	return RyanJsonFalse;
}
