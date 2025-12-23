
#include "RyanJsonBaseTest.h"

RyanJsonBool_e RyanJsonBaseTestForEachJson(void)
{
	char *str = NULL;
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
	RyanJson_t item = NULL;
	RyanJsonArrayForEach(RyanJsonGetObjectToKey(json, "arrayDouble"), item)
	{
		if (!RyanJsonIsDouble(item) || !compare_double(16.89, RyanJsonGetDoubleValue(item))) { goto err; }
	}

	RyanJsonArrayForEach(RyanJsonGetObjectToKey(json, "arrayInt"), item)
	{
		if (!RyanJsonIsInt(item) || 16 != RyanJsonGetIntValue(item)) { goto err; }
	}

	int32_t strLen;
	RyanJsonObjectForEach(RyanJsonGetObjectToKey(json, "item"), item)
	{
		str = RyanJsonPrint(item, 50, RyanJsonTrue, &strLen);
		printf("item { %s : %s }  %d\r\n", RyanJsonGetKey(item), str, strLen);
		RyanJsonFree(str);
	}

	RyanJsonDelete(json);
	return RyanJsonTrue;

err:
	RyanJsonDelete(json);
	return RyanJsonFalse;
}
