#include "RyanJsonBaseTest.h"

#define IntList                                                                                                                            \
	/* ========== 零值测试 ========== */                                                                                               \
	X(0)                                                                                                                               \
	/* ========== 正负边界 ========== */                                                                                               \
	X(1)                                                                                                                               \
	X(-1)                                                                                                                              \
	X(2)                                                                                                                               \
	X(-2)                                                                                                                              \
	/* ========== 常见小整数 ========== */                                                                                             \
	X(10)                                                                                                                              \
	X(-10)                                                                                                                             \
	X(100)                                                                                                                             \
	X(-100)                                                                                                                            \
	X(255)                                                                                                                             \
	X(-255)                                                                                                                            \
	X(256)                                                                                                                             \
	X(-256)                                                                                                                            \
	/* ========== 常见数值 ========== */                                                                                               \
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
	/* ========== 大整数 ========== */                                                                                                 \
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
	/* ========== 8位边界 ========== */                                                                                                \
	X(127)                                                                                                                             \
	X(-128)                                                                                                                            \
	/* ========== 16位边界 ========== */                                                                                               \
	X(32767)                                                                                                                           \
	X(-32768)                                                                                                                          \
	/* ========== 32位边界 ========== */                                                                                               \
	X(2147483647)                                                                                                                      \
	X(-2147483648)                                                                                                                     \
	/* ========== 特殊模式 ========== */                                                                                               \
	X(1234567890)                                                                                                                      \
	X(-1234567890)                                                                                                                     \
	X(123456789)                                                                                                                       \
	X(-123456789)                                                                                                                      \
	/* ========== 2的幂次 ========== */                                                                                                \
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
#define X(a) "{\"int\":" #a "}",
	IntList
#undef X
};

// 整数一致性测试
RyanJsonBool_e RyanJsonBaseTestEqualityInt(void)
{

	for (uint32_t i = 0; i < sizeof(IntValueTable) / sizeof(IntValueTable[0]); i++)
	{
		const char *jsonIntStr = IntStringTable[i];
		RyanJson_t jsonRoot = RyanJsonParse(jsonIntStr);
		RyanJsonCheckCode(NULL != jsonRoot, {
			jsonLog("str: %s", jsonIntStr);
			goto err;
		});
		RyanJsonCheckReturnFalse(RyanJsonIsInt(RyanJsonGetObjectToKey(jsonRoot, "int")));

		// 验证解析后的数值是否正确
		int32_t intValue = RyanJsonGetIntValue(RyanJsonGetObjectToKey(jsonRoot, "int"));
		RyanJsonCheckCode(intValue == IntValueTable[i], {
			jsonLog("str: %s, expected: %" PRId32 ", got: %" PRId32, jsonIntStr, IntValueTable[i], intValue);
			RyanJsonDelete(jsonRoot);
			goto err;
		});

		// 验证序列化后再解析，然后判断int是否一致（往返测试）
		char *serializedStr = RyanJsonPrint(jsonRoot, 128, RyanJsonFalse, NULL);
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);
		RyanJsonCheckReturnFalse(NULL != roundtripJson);
		RyanJsonCheckReturnFalse(RyanJsonIsInt(RyanJsonGetObjectToKey(roundtripJson, "int")));

		int32_t roundtripValue = RyanJsonGetIntValue(RyanJsonGetObjectToKey(roundtripJson, "int"));
		RyanJsonCheckCode(roundtripValue == IntValueTable[i], {
			jsonLog("roundtrip failed: expected: %" PRId32 ", got: %" PRId32, IntValueTable[i], roundtripValue);
			RyanJsonDelete(roundtripJson);
			goto err;
		});

		RyanJsonDelete(roundtripJson);
	}

	return RyanJsonTrue;

err:
	return RyanJsonFalse;
}
