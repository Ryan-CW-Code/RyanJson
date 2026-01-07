
#include "RyanJsonBaseTest.h"

/* --------------------------------------- jsonTest ------------------------------------------- */

void printJsonDebug(RyanJson_t json)
{
	char *str = RyanJsonPrint(json, 1024, RyanJsonTrue, NULL);
	printf("aa %s\r\n", str);
	RyanJsonFree(str);
}

RyanJsonBool_e rootNodeCheckTest(RyanJson_t json)
{
	if (!RyanJsonIsInt(RyanJsonGetObjectToKey(json, "inter")) || 16 != RyanJsonGetIntValue(RyanJsonGetObjectToKey(json, "inter")))
	{
		RyanJsonCheckReturnFalse(NULL);
	}

	if (!RyanJsonIsDouble(RyanJsonGetObjectToKey(json, "double")) ||
	    !RyanJsonCompareDouble(RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(json, "double")), 16.89))
	{
		RyanJsonCheckReturnFalse(NULL);
	}

	if (!RyanJsonIsString(RyanJsonGetObjectToKey(json, "string")) ||
	    strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(json, "string")), "hello"))
	{
		RyanJsonCheckReturnFalse(NULL);
	}

	if (!RyanJsonIsBool(RyanJsonGetObjectToKey(json, "boolTrue")) ||
	    RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "boolTrue")) != RyanJsonTrue)
	{
		RyanJsonCheckReturnFalse(NULL);
	}

	if (!RyanJsonIsBool(RyanJsonGetObjectToKey(json, "boolFalse")) ||
	    RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "boolFalse")) != RyanJsonFalse)
	{
		RyanJsonCheckReturnFalse(NULL);
	}

	if (!RyanJsonIsNull(RyanJsonGetObjectToKey(json, "null"))) { RyanJsonCheckReturnFalse(NULL); }

	return RyanJsonTrue;
}

RyanJsonBool_e itemNodeCheckTest(RyanJson_t json)
{
	RyanJson_t item = RyanJsonGetObjectToKey(json, "item");
	if (RyanJsonTrue != rootNodeCheckTest(item)) { RyanJsonCheckReturnFalse(NULL); }

	return RyanJsonTrue;
}

RyanJsonBool_e arrayNodeCheckTest(RyanJson_t json)
{
	RyanJson_t item = NULL;

	// 判断是不是数组类型
	if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayInt"))) { RyanJsonCheckReturnFalse(NULL); }

	if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayDouble"))) { RyanJsonCheckReturnFalse(NULL); }

	if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayString"))) { RyanJsonCheckReturnFalse(NULL); }

	if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "array"))) { RyanJsonCheckReturnFalse(NULL); }

	/**
	 * @brief 检查弱类型数组
	 *
	 */
	//   array: [16, 16.89, "hello", true, false, null],
	if (!RyanJsonIsInt(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 0)) ||
	    16 != RyanJsonGetIntValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 0)))
	{
		RyanJsonCheckReturnFalse(NULL);
	}

	if (!RyanJsonIsDouble(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 1)) ||
	    !RyanJsonCompareDouble(RyanJsonGetDoubleValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 1)), 16.89))
	{
		printf("%s:%d 解析失败 %f\r\n", __FILE__, __LINE__,
		       RyanJsonGetDoubleValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 1)));
		RyanJsonCheckReturnFalse(NULL);
	}

	if (!RyanJsonIsString(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 2)) ||
	    0 != strcmp(RyanJsonGetStringValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 2)), "hello"))
	{
		RyanJsonCheckReturnFalse(NULL);
	}

	if (!RyanJsonIsBool(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 3)) ||
	    RyanJsonGetBoolValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 3)) != RyanJsonTrue)
	{
		RyanJsonCheckReturnFalse(NULL);
	}

	if (!RyanJsonIsBool(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 4)) ||
	    RyanJsonGetBoolValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 4)) != RyanJsonFalse)
	{
		RyanJsonCheckReturnFalse(NULL);
	}

	if (!RyanJsonIsNull(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 5))) { RyanJsonCheckReturnFalse(NULL); }

	/**
	 * @brief 检查强类型数组
	 *
	 */
	for (int32_t count = 0; count < RyanJsonGetSize(RyanJsonGetObjectToKey(json, "arrayInt")); count++)
	{
		item = RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "arrayInt"), count);
		if (!RyanJsonIsInt(item) || 16 != RyanJsonGetIntValue(item)) { RyanJsonCheckReturnFalse(NULL); }
	}

	for (int32_t count = 0; count < RyanJsonGetSize(RyanJsonGetObjectToKey(json, "arrayDouble")); count++)
	{
		item = RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "arrayDouble"), count);
		if (!RyanJsonIsDouble(item) || fabs(RyanJsonGetDoubleValue(item) - 16.8) < 0.001) { RyanJsonCheckReturnFalse(NULL); }
	}

	for (int32_t count = 0; count < RyanJsonGetSize(RyanJsonGetObjectToKey(json, "arrayString")); count++)
	{
		item = RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "arrayString"), count);
		if (!RyanJsonIsString(item) || strcmp(RyanJsonGetStringValue(item), "hello")) { RyanJsonCheckReturnFalse(NULL); }
	}

	if (6 != RyanJsonGetSize(RyanJsonGetObjectToKey(json, "array"))) { RyanJsonCheckReturnFalse(NULL); }

	return RyanJsonTrue;
}

RyanJsonBool_e arrayItemNodeCheckTest(RyanJson_t json)
{
	if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayItem"))) { RyanJsonCheckReturnFalse(NULL); }

	if (RyanJsonTrue != rootNodeCheckTest(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 0)))
	{
		RyanJsonCheckReturnFalse(NULL);
	}

	if (RyanJsonTrue != rootNodeCheckTest(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 1)))
	{
		RyanJsonCheckReturnFalse(NULL);
	}
	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonBaseTestCheckRoot(RyanJson_t pJson)
{
	RyanJsonCheckReturnFalse(RyanJsonTrue == rootNodeCheckTest(pJson));

	RyanJsonCheckReturnFalse(RyanJsonTrue == itemNodeCheckTest(pJson));

	RyanJsonCheckReturnFalse(RyanJsonTrue == arrayNodeCheckTest(pJson));

	RyanJsonCheckReturnFalse(RyanJsonTrue == arrayItemNodeCheckTest(pJson));

	return RyanJsonTrue;
}
