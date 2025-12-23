#include "RyanJsonBaseTest.h"

/* --------------------------------------------------------------------- */

RyanJsonBool_e RyanJsonBaseTestChangeJson()
{
	char jsonstr[] =
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
		"\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],"
		"\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],"
		"\"array\":[16,16.89,\"hello\",true,false,null],"
		"\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}],"
		"\"string2222\":\"hello\"}";

	RyanJson_t json = RyanJsonParse(jsonstr);
	RyanJsonCheckReturnFalse(NULL != json);

	/**
	 * @brief 修改基本类型
	 */
	RyanJsonChangeIntValue(RyanJsonGetObjectToKey(json, "inter"), 20);
	RyanJsonCheckCode(RyanJsonIsInt(RyanJsonGetObjectToKey(json, "inter")) &&
				  20 == RyanJsonGetIntValue(RyanJsonGetObjectToKey(json, "inter")),
			  { goto err; });

	RyanJsonChangeDoubleValue(RyanJsonGetObjectToKey(json, "double"), 20.89);
	RyanJsonCheckCode(RyanJsonIsDouble(RyanJsonGetObjectToKey(json, "double")) &&
				  compare_double(RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(json, "double")), 20.89),
			  { goto err; });

	RyanJsonChangeStringValue(RyanJsonGetObjectToKey(json, "string"), "world");
	RyanJsonCheckCode(RyanJsonIsString(RyanJsonGetObjectToKey(json, "string")) &&
				  strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(json, "string")), "world") == 0,
			  { goto err; });

	RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(json, "boolTrue"), RyanJsonFalse);
	RyanJsonCheckCode(RyanJsonIsBool(RyanJsonGetObjectToKey(json, "boolTrue")) &&
				  RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "boolTrue")) == RyanJsonFalse,
			  { goto err; });

	RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(json, "boolFalse"), RyanJsonTrue);
	RyanJsonCheckCode(RyanJsonIsBool(RyanJsonGetObjectToKey(json, "boolFalse")) &&
				  RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "boolFalse")) == RyanJsonTrue,
			  { goto err; });

	/**
	 * @brief 修改数组元素 (arrayInt)
	 */
	RyanJsonChangeIntValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 0), 99);
	RyanJsonCheckCode(RyanJsonIsInt(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 0)) &&
				  RyanJsonGetIntValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 0)) == 99,
			  { goto err; });

	/**
	 * @brief 修改数组元素 (arrayDouble)
	 */
	RyanJsonChangeDoubleValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayDouble"), 1), 99.99);
	RyanJsonCheckCode(
		RyanJsonIsDouble(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayDouble"), 1)) &&
			compare_double(RyanJsonGetDoubleValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayDouble"), 1)),
				       99.99),
		{ goto err; });

	/**
	 * @brief 修改数组元素 (arrayString)
	 */
	RyanJsonChangeStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayString"), 2), "changedString");
	RyanJsonCheckCode(RyanJsonIsString(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayString"), 2)) &&
				  strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayString"), 2)),
					 "changedString") == 0,
			  { goto err; });

	/**
	 * @brief 修改嵌套对象
	 */
	RyanJsonChangeStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectToKey(json, "item"), "string"), "nestedWorld");
	RyanJsonCheckCode(RyanJsonIsString(RyanJsonGetObjectToKey(RyanJsonGetObjectToKey(json, "item"), "string")) &&
				  strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectToKey(json, "item"), "string")),
					 "nestedWorld") == 0,
			  { goto err; });

	/**
	 * @brief 修改数组对象中的字段 (arrayItem[0].inter -> 123)
	 */
	RyanJsonChangeIntValue(RyanJsonGetObjectToKey(RyanJsonGetObjectToIndex(RyanJsonGetObjectByKey(json, "arrayItem"), 0), "inter"),
			       123);
	RyanJsonCheckCode(
		RyanJsonIsInt(RyanJsonGetObjectToKey(RyanJsonGetObjectToIndex(RyanJsonGetObjectByKey(json, "arrayItem"), 0), "inter")) &&
			RyanJsonGetIntValue(RyanJsonGetObjectToKey(RyanJsonGetObjectToIndex(RyanJsonGetObjectByKey(json, "arrayItem"), 0),
								   "inter")) == 123,
		{ goto err; });

	char *str = RyanJsonPrint(json, 1024, RyanJsonTrue, NULL);
	RyanJsonFree(str);
	RyanJsonDelete(json);
	return RyanJsonTrue;

err:
	RyanJsonDelete(json);
	return RyanJsonFalse;
}
