
#include "RyanJsonBaseTest.h"

/* --------------------------------------- jsonTest ------------------------------------------- */
// !(fabs(RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(json, "double")) - 16.89) < 1e-6)
RyanJsonBool_e compare_double(double a, double b)
{
	double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
	return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

void printfJsonaaaa(RyanJson_t json)
{
	char *str = RyanJsonPrint(json, 1024, RyanJsonTrue, NULL);
	printf("aa %s\r\n", str);
	RyanJsonFree(str);
}

RyanJsonBool_e rootNodeCheckTest(RyanJson_t json)
{
	if (!RyanJsonIsInt(RyanJsonGetObjectToKey(json, "inter")) || 16 != RyanJsonGetIntValue(RyanJsonGetObjectToKey(json, "inter")))
	{
		RyanJsonCheckReturnFlase(NULL);
	}

	if (!RyanJsonIsDouble(RyanJsonGetObjectToKey(json, "double")) ||
	    !compare_double(RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(json, "double")), 16.89))
	{
		RyanJsonCheckReturnFlase(NULL);
	}

	if (!RyanJsonIsString(RyanJsonGetObjectToKey(json, "string")) ||
	    strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(json, "string")), "hello"))
	{
		RyanJsonCheckReturnFlase(NULL);
	}

	if (!RyanJsonIsBool(RyanJsonGetObjectToKey(json, "boolTrue")) ||
	    RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "boolTrue")) != RyanJsonTrue)
	{
		RyanJsonCheckReturnFlase(NULL);
	}

	if (!RyanJsonIsBool(RyanJsonGetObjectToKey(json, "boolFalse")) ||
	    RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "boolFalse")) != RyanJsonFalse)
	{
		RyanJsonCheckReturnFlase(NULL);
	}

	if (!RyanJsonIsNull(RyanJsonGetObjectToKey(json, "null"))) { RyanJsonCheckReturnFlase(NULL); }

	return RyanJsonTrue;
}

RyanJsonBool_e itemNodeCheckTest(RyanJson_t json)
{
	RyanJson_t item = RyanJsonGetObjectToKey(json, "item");
	if (RyanJsonTrue != rootNodeCheckTest(item)) { RyanJsonCheckReturnFlase(NULL); }

	return RyanJsonTrue;
}

RyanJsonBool_e arrayNodeCheckTest(RyanJson_t json)
{
	RyanJson_t item = NULL;

	// 判断是不是数组类型
	if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayInt"))) { RyanJsonCheckReturnFlase(NULL); }

	if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayDouble"))) { RyanJsonCheckReturnFlase(NULL); }

	if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayString"))) { RyanJsonCheckReturnFlase(NULL); }

	if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "array"))) { RyanJsonCheckReturnFlase(NULL); }

	/**
	 * @brief 检查弱类型数组
	 *
	 */
	//   array: [16, 16.89, "hello", true, false, null],
	if (!RyanJsonIsInt(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 0)) ||
	    16 != RyanJsonGetIntValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 0)))
	{
		RyanJsonCheckReturnFlase(NULL);
	}

	if (!RyanJsonIsDouble(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 1)) ||
	    !compare_double(RyanJsonGetDoubleValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 1)), 16.89))
	{
		printf("%s:%d 解析失败 %f\r\n", __FILE__, __LINE__,
		       RyanJsonGetDoubleValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 1)));
		RyanJsonCheckReturnFlase(NULL);
	}

	if (!RyanJsonIsString(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 2)) ||
	    0 != strcmp(RyanJsonGetStringValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 2)), "hello"))
	{
		RyanJsonCheckReturnFlase(NULL);
	}

	if (!RyanJsonIsBool(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 3)) ||
	    RyanJsonGetBoolValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 3)) != RyanJsonTrue)
	{
		RyanJsonCheckReturnFlase(NULL);
	}

	if (!RyanJsonIsBool(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 4)) ||
	    RyanJsonGetBoolValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 4)) != RyanJsonFalse)
	{
		RyanJsonCheckReturnFlase(NULL);
	}

	if (!RyanJsonIsNull(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 5))) { RyanJsonCheckReturnFlase(NULL); }

	/**
	 * @brief 检查强类型数组
	 *
	 */
	for (int32_t count = 0; count < RyanJsonGetSize(RyanJsonGetObjectToKey(json, "arrayInt")); count++)
	{
		item = RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "arrayInt"), count);
		if (!RyanJsonIsInt(item) || 16 != RyanJsonGetIntValue(item)) { RyanJsonCheckReturnFlase(NULL); }
	}

	for (int32_t count = 0; count < RyanJsonGetSize(RyanJsonGetObjectToKey(json, "arrayDouble")); count++)
	{
		item = RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "arrayDouble"), count);
		if (!RyanJsonIsDouble(item) || fabs(RyanJsonGetDoubleValue(item) - 16.8) < 0.001) { RyanJsonCheckReturnFlase(NULL); }
	}

	for (int32_t count = 0; count < RyanJsonGetSize(RyanJsonGetObjectToKey(json, "arrayString")); count++)
	{
		item = RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "arrayString"), count);
		if (!RyanJsonIsString(item) || strcmp(RyanJsonGetStringValue(item), "hello")) { RyanJsonCheckReturnFlase(NULL); }
	}

	if (6 != RyanJsonGetSize(RyanJsonGetObjectToKey(json, "array"))) { RyanJsonCheckReturnFlase(NULL); }

	return RyanJsonTrue;
}

RyanJsonBool_e arrayItemNodeCheckTest(RyanJson_t json)
{
	if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayItem"))) { RyanJsonCheckReturnFlase(NULL); }

	if (RyanJsonTrue != rootNodeCheckTest(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 0)))
	{
		RyanJsonCheckReturnFlase(NULL);
	}

	if (RyanJsonTrue != rootNodeCheckTest(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 1)))
	{
		RyanJsonCheckReturnFlase(NULL);
	}
	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonBaseTestCheckRoot(RyanJson_t pJson)
{
	RyanJsonCheckReturnFlase(RyanJsonTrue != rootNodeCheckTest(pJson));

	RyanJsonCheckReturnFlase(RyanJsonTrue != itemNodeCheckTest(pJson));

	RyanJsonCheckReturnFlase(RyanJsonTrue != arrayNodeCheckTest(pJson));

	RyanJsonCheckReturnFlase(RyanJsonTrue != arrayItemNodeCheckTest(pJson));

	return RyanJsonTrue;
}
