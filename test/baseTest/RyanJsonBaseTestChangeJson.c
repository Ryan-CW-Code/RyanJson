#include "RyanJsonBaseTest.h"

/* --------------------------------------------------------------------- */

RyanJsonBool_e RyanJsonBaseTestChangeJson(void)
{
	char jsonstr[] =
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
		"\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],"
		"\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],"
		"\"array\":[16,16.89,\"hello\",true,false,null],"
		"\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}],"
		"\"string2222\":\"hello\",\"0\":\"1\",\"nameaaaaaaaaaaaaaaaaaaaaaaaaaaaa\":\"Mash\",\"2\":\"3\",\"name\":"
		"\"Mashaaaaaaaaaaaaaaaaaaaaaaaa\"}";

	RyanJson_t jsonRoot = RyanJsonParse(jsonstr);
	RyanJsonCheckReturnFalse(NULL != jsonRoot);

	/**
	 * @brief 修改基本类型
	 */
	RyanJsonChangeIntValue(RyanJsonGetObjectToKey(jsonRoot, "inter"), 20);
	RyanJsonCheckCode(RyanJsonIsInt(RyanJsonGetObjectToKey(jsonRoot, "inter")) &&
				  20 == RyanJsonGetIntValue(RyanJsonGetObjectToKey(jsonRoot, "inter")),
			  { goto err; });

	RyanJsonChangeDoubleValue(RyanJsonGetObjectToKey(jsonRoot, "double"), 20.89);
	RyanJsonCheckCode(RyanJsonIsDouble(RyanJsonGetObjectToKey(jsonRoot, "double")) &&
				  RyanJsonCompareDouble(RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(jsonRoot, "double")), 20.89),
			  { goto err; });

	// inline模式只修改key，并且不超过inline长度
	RyanJsonChangeKey(RyanJsonGetObjectByKey(jsonRoot, "0"), "type");
	RyanJsonCheckCode(strcmp(RyanJsonGetKey(RyanJsonGetObjectToKey(jsonRoot, "type")), "type") == 0 &&
				  strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "type")), "1") == 0,
			  { goto err; });

	// inline模式修改key，并且超过inline长度,进入ptr模式
	RyanJsonChangeKey(RyanJsonGetObjectByKey(jsonRoot, "type"), "type000000000000000");
	RyanJsonCheckCode(strcmp(RyanJsonGetKey(RyanJsonGetObjectToKey(jsonRoot, "type000000000000000")), "type000000000000000") == 0 &&
				  strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "type000000000000000")), "1") == 0,
			  { goto err; });

	// ptr模式只修改key，不超过inline长度,进入inline模式
	RyanJsonChangeKey(RyanJsonGetObjectByKey(jsonRoot, "nameaaaaaaaaaaaaaaaaaaaaaaaaaaaa"), "na");
	RyanJsonCheckCode(strcmp(RyanJsonGetKey(RyanJsonGetObjectToKey(jsonRoot, "na")), "na") == 0 &&
				  strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "na")), "Mash") == 0,
			  { goto err; });

	// inline模式只修改Value，并且不超过inline长度
	RyanJsonChangeStringValue(RyanJsonGetObjectByKey(jsonRoot, "2"), "type");
	RyanJsonCheckCode(strcmp(RyanJsonGetKey(RyanJsonGetObjectToKey(jsonRoot, "2")), "2") == 0 &&
				  strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "2")), "type") == 0,
			  { goto err; });

	// ptr模式只修改Value，不超过inline长度,进入inline模式
	RyanJsonChangeStringValue(RyanJsonGetObjectByKey(jsonRoot, "name"), "Ma");
	RyanJsonCheckCode(strcmp(RyanJsonGetKey(RyanJsonGetObjectToKey(jsonRoot, "name")), "name") == 0 &&
				  strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "name")), "Ma") == 0,
			  { goto err; });

	// ptr模式只修改Value，超过inline长度,进入ptr模式
	RyanJsonChangeStringValue(RyanJsonGetObjectByKey(jsonRoot, "name"), "Mashaaaaaaaaaaaaaaaaaaaaaaaa");
	RyanJsonCheckCode(
		strcmp(RyanJsonGetKey(RyanJsonGetObjectToKey(jsonRoot, "name")), "name") == 0 &&
			strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "name")), "Mashaaaaaaaaaaaaaaaaaaaaaaaa") == 0,
		{ goto err; });

	RyanJsonChangeStringValue(RyanJsonGetObjectToKey(jsonRoot, "string"), "world");
	RyanJsonCheckCode(RyanJsonIsString(RyanJsonGetObjectToKey(jsonRoot, "string")) &&
				  strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "string")), "world") == 0,
			  { goto err; });

	RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(jsonRoot, "boolTrue"), RyanJsonFalse);
	RyanJsonCheckCode(RyanJsonIsBool(RyanJsonGetObjectToKey(jsonRoot, "boolTrue")) &&
				  RyanJsonGetBoolValue(RyanJsonGetObjectToKey(jsonRoot, "boolTrue")) == RyanJsonFalse,
			  { goto err; });

	RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(jsonRoot, "boolFalse"), RyanJsonTrue);
	RyanJsonCheckCode(RyanJsonIsBool(RyanJsonGetObjectToKey(jsonRoot, "boolFalse")) &&
				  RyanJsonGetBoolValue(RyanJsonGetObjectToKey(jsonRoot, "boolFalse")) == RyanJsonTrue,
			  { goto err; });

	/**
	 * @brief 修改数组元素 (arrayInt)
	 */
	RyanJsonChangeIntValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayInt"), 0), 99);
	RyanJsonCheckCode(RyanJsonIsInt(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayInt"), 0)) &&
				  RyanJsonGetIntValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayInt"), 0)) == 99,
			  { goto err; });

	/**
	 * @brief 修改数组元素 (arrayDouble)
	 */
	RyanJsonChangeDoubleValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayDouble"), 1), 99.99);
	RyanJsonCheckCode(RyanJsonIsDouble(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayDouble"), 1)) &&
				  RyanJsonCompareDouble(RyanJsonGetDoubleValue(RyanJsonGetObjectToIndex(
								RyanJsonGetObjectToKey(jsonRoot, "arrayDouble"), 1)),
							99.99),
			  { goto err; });

	/**
	 * @brief 修改数组元素 (arrayString)
	 */
	RyanJsonChangeStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayString"), 2), "changedString");
	RyanJsonCheckCode(
		RyanJsonIsString(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayString"), 2)) &&
			strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayString"), 2)),
			       "changedString") == 0,
		{ goto err; });

	/**
	 * @brief 修改嵌套对象
	 */
	RyanJsonChangeStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectToKey(jsonRoot, "item"), "string"), "nestedWorld");
	RyanJsonCheckCode(RyanJsonIsString(RyanJsonGetObjectToKey(RyanJsonGetObjectToKey(jsonRoot, "item"), "string")) &&
				  strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectToKey(jsonRoot, "item"), "string")),
					 "nestedWorld") == 0,
			  { goto err; });

	/**
	 * @brief 修改数组对象中的字段 (arrayItem[0].inter -> 123)
	 */
	RyanJsonChangeIntValue(RyanJsonGetObjectToKey(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayItem"), 0), "inter"),
			       123);
	RyanJsonCheckCode(RyanJsonIsInt(RyanJsonGetObjectToKey(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayItem"), 0),
							       "inter")) &&
				  RyanJsonGetIntValue(RyanJsonGetObjectToKey(
					  RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayItem"), 0), "inter")) == 123,
			  { goto err; });

	char *str = RyanJsonPrint(jsonRoot, 1024, RyanJsonTrue, NULL);
	RyanJsonFree(str);
	RyanJsonDelete(jsonRoot);
	return RyanJsonTrue;

err:
	RyanJsonDelete(jsonRoot);
	return RyanJsonFalse;
}
