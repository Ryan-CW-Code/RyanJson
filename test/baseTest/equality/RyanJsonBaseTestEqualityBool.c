#include "RyanJsonBaseTest.h"

// 布尔值一致性测试
RyanJsonBool_e RyanJsonBaseTestEqualityBool(void)
{
	// 测试 true
	{
		const char *jsonBoolStr = "{\"bool\":true}";
		RyanJson_t jsonRoot = RyanJsonParse(jsonBoolStr);
		RyanJsonCheckReturnFalse(NULL != jsonRoot);
		RyanJsonCheckReturnFalse(RyanJsonIsBool(RyanJsonGetObjectToKey(jsonRoot, "bool")));
		RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonGetBoolValue(RyanJsonGetObjectToKey(jsonRoot, "bool")));

		// 往返测试
		char *serializedStr = RyanJsonPrint(jsonRoot, 64, RyanJsonFalse, NULL);
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);
		RyanJsonCheckReturnFalse(NULL != roundtripJson);
		RyanJsonCheckReturnFalse(RyanJsonIsBool(RyanJsonGetObjectToKey(roundtripJson, "bool")));
		RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonGetBoolValue(RyanJsonGetObjectToKey(roundtripJson, "bool")));

		RyanJsonDelete(roundtripJson);
	}

	// 测试 false
	{
		const char *jsonBoolStr = "{\"bool\":false}";
		RyanJson_t jsonRoot = RyanJsonParse(jsonBoolStr);
		RyanJsonCheckReturnFalse(NULL != jsonRoot);
		RyanJsonCheckReturnFalse(RyanJsonIsBool(RyanJsonGetObjectToKey(jsonRoot, "bool")));
		RyanJsonCheckReturnFalse(RyanJsonFalse == RyanJsonGetBoolValue(RyanJsonGetObjectToKey(jsonRoot, "bool")));

		// 往返测试
		char *serializedStr = RyanJsonPrint(jsonRoot, 64, RyanJsonFalse, NULL);
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);
		RyanJsonCheckReturnFalse(NULL != roundtripJson);
		RyanJsonCheckReturnFalse(RyanJsonIsBool(RyanJsonGetObjectToKey(roundtripJson, "bool")));
		RyanJsonCheckReturnFalse(RyanJsonFalse == RyanJsonGetBoolValue(RyanJsonGetObjectToKey(roundtripJson, "bool")));

		RyanJsonDelete(roundtripJson);
	}

	// 测试数组中的布尔值
	{
		const char *jsonArrayStr = "[true, false, true, false]";
		RyanJson_t jsonRoot = RyanJsonParse(jsonArrayStr);
		RyanJsonCheckReturnFalse(NULL != jsonRoot);
		RyanJsonCheckReturnFalse(4 == RyanJsonGetArraySize(jsonRoot));

		RyanJsonBool_e expected[] = {RyanJsonTrue, RyanJsonFalse, RyanJsonTrue, RyanJsonFalse};
		int idx = 0;
		RyanJson_t item = NULL;
		RyanJsonArrayForEach(jsonRoot, item)
		{
			RyanJsonCheckReturnFalse(RyanJsonIsBool(item));
			RyanJsonCheckReturnFalse(expected[idx] == RyanJsonGetBoolValue(item));
			idx++;
		}

		// 往返测试
		char *serializedStr = RyanJsonPrint(jsonRoot, 64, RyanJsonFalse, NULL);
		RyanJsonDelete(jsonRoot);

		RyanJson_t roundtripJson = RyanJsonParse(serializedStr);
		RyanJsonFree(serializedStr);
		RyanJsonCheckReturnFalse(NULL != roundtripJson);
		RyanJsonCheckReturnFalse(4 == RyanJsonGetArraySize(roundtripJson));

		idx = 0;
		RyanJsonArrayForEach(roundtripJson, item)
		{
			RyanJsonCheckReturnFalse(RyanJsonIsBool(item));
			RyanJsonCheckReturnFalse(expected[idx] == RyanJsonGetBoolValue(item));
			idx++;
		}

		RyanJsonDelete(roundtripJson);
	}

	return RyanJsonTrue;
}
