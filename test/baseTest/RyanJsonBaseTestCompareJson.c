#include "RyanJsonBaseTest.h"
/* --------------------------------------------------------------------- */

RyanJsonBool_e RyanJsonBaseTestCompareJson()
{
	char jsonstr[] = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":"
			 "{\"inter\":16,\"double\":16."
			 "89,\"string\":\"hello\","
			 "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,"
			 "16.89,16.89,16.89],"
			 "\"arrayString\":[\"hello\",\"hello\","
			 "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			 "\"double\":16.89,\"string\":"
			 "\"hello\",\"boolTrue\":true,"
			 "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			 "\"boolFalse\":false,\"null\":null}]}";

	RyanJson_t json = RyanJsonParse(jsonstr);
	RyanJson_t json2 = RyanJsonParse(jsonstr);

	// 比较函数
	RyanJsonCheckCode(RyanJsonTrue == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddStringToObject(json2, "test", "hello");
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddIntToObject(json2, "test", 1);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddDoubleToObject(json2, "test", 2.0);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddBoolToObject(json2, "test", RyanJsonTrue);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddNullToObject(json2, "test");
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddIntToArray(RyanJsonGetObjectToKey(json2, "arrayInt"), 2);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddDoubleToArray(RyanJsonGetObjectToKey(json2, "arrayDouble"), 2.0);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddStringToArray(RyanJsonGetObjectToKey(json2, "arrayString"), "hello");
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonAddItemToArray(RyanJsonGetObjectToKey(json2, "arrayItem"), RyanJsonCreateString("test", "hello"));
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeKey(RyanJsonGetObjectToKey(json2, "inter"), "int2");
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeIntValue(RyanJsonGetObjectToKey(json2, "inter"), 17);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeDoubleValue(RyanJsonGetObjectToKey(json2, "double"), 20.89);
	if (RyanJsonFalse != RyanJsonCompare(json, json2))
	{
		printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
		goto err;
	}

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonDelete(RyanJsonDetachByKey(json2, "double"));
	RyanJsonAddIntToObject(json2, "double", 20); // 改为int
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeStringValue(RyanJsonGetObjectToKey(json2, "string"), "49");
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(json2, "boolTrue"), RyanJsonFalse);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(json2, "item", "boolTrue"), RyanJsonFalse);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeIntValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "arrayInt"), 0), 17);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeDoubleValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "arrayDouble"), 0), 20.89);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "arrayString"), 0), "20.89");
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeIntValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "array"), 0), 17);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonChangeIntValue(RyanJsonGetObjectToKey(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "arrayItem"), 0), "inter"),
			       17);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonDeleteByKey(json2, "arrayItem");
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonDeleteByIndex(RyanJsonGetObjectToKey(json2, "arrayInt"), 2);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json2);
	json2 = RyanJsonParse(jsonstr);
	RyanJsonDeleteByIndex(RyanJsonGetObjectToKey(json2, "arrayItem"), 0);
	RyanJsonCheckCode(RyanJsonFalse == RyanJsonCompare(json, json2), { goto err; });

	RyanJsonDelete(json);
	RyanJsonDelete(json2);
	return RyanJsonTrue;

err:
	RyanJsonDelete(json);
	RyanJsonDelete(json2);
	return RyanJsonFalse;
}
