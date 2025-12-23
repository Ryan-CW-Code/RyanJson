
#include "RyanJsonBaseTest.h"

/* --------------------------------------------------------------------- */

RyanJsonBool_e RyanJsonBaseTestLoadJson()
{
	char *str = NULL;
	RyanJson_t json;
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

	json = RyanJsonParse(jsonstr);
	RyanJsonCheckReturnFalse(NULL != json);

	str = RyanJsonPrint(json, 250, RyanJsonFalse, NULL);
	RyanJsonCheckCode(0 == strcmp(str, jsonstr), {
		RyanJsonFree(str);
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	RyanJsonFree(str);

	RyanJsonCheckCode(RyanJsonTrue == RyanJsonBaseTestCheckRoot(json), {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	RyanJsonDelete(json);

	/**
	 * @brief 测试序列化错误json结构
	 *
	 */
	// \"inter\":16poi,  无效数字
	json = RyanJsonParse("{\"inter\":16poi,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
			     "\"item\":{\"inter\":"
			     "16,\"double\":16.89,\"string\":\"hello\","
			     "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16."
			     "89,16.89,16.89,16."
			     "89],\"arrayString\":[\"hello\",\"hello\","
			     "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			     "\"double\":16.89,"
			     "\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,"
			     "\"null\":null}]}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	// \"double\":16.8yu9,,  无效浮点数
	json = RyanJsonParse("{\"inter\":16,\"double\":16.8yu9,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
			     "\"item\":{\"inter\":16,"
			     "\"double\":16.89,\"string\":\"hello\","
			     "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16."
			     "89,16.89,16.89,16."
			     "89],\"arrayString\":[\"hello\",\"hello\","
			     "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			     "\"double\":16.89,"
			     "\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,"
			     "\"null\":null}]}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	// boolTrue 设置为 tru
	json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":tru,\"boolFalse\":false,\"null\":null,"
			     "\"item\":{\"inter\":16,"
			     "\"double\":16.89,\"string\":\"hello\","
			     "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16."
			     "89,16.89,16.89,16."
			     "89],\"arrayString\":[\"hello\",\"hello\","
			     "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			     "\"double\":16.89,"
			     "\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,"
			     "\"null\":null}]}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	// boolFalse 设置为 fale
	json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":fale,\"null\":null,"
			     "\"item\":{\"inter\":16,"
			     "\"double\":16.89,\"string\":\"hello\","
			     "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16."
			     "89,16.89,16.89,16."
			     "89],\"arrayString\":[\"hello\",\"hello\","
			     "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			     "\"double\":16.89,"
			     "\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,"
			     "\"null\":null}]}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	// null 设置为 nul
	json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":nul,"
			     "\"item\":{\"inter\":16,"
			     "\"double\":16.89,\"string\":\"hello\","
			     "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16."
			     "89,16.89,16.89,16."
			     "89],\"arrayString\":[\"hello\",\"hello\","
			     "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			     "\"double\":16.89,"
			     "\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,"
			     "\"null\":null}]}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	// null 设置为 NULL
	json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":NULL,"
			     "\"item\":{\"inter\":16,"
			     "\"double\":16.89,\"string\":\"hello\","
			     "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16."
			     "89,16.89,16.89,16."
			     "89],\"arrayString\":[\"hello\",\"hello\","
			     "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			     "\"double\":16.89,"
			     "\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,"
			     "\"null\":null}]}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	// \"inter\":16后面少个,
	json = RyanJsonParse("{\"inter\":16\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
			     "\"item\":{\"inter\":16,"
			     "\"double\":16.89,\"string\":\"hello\","
			     "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16."
			     "89,16.89,16.89,16."
			     "89],\"arrayString\":[\"hello\",\"hello\","
			     "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			     "\"double\":16.89,"
			     "\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,"
			     "\"null\":null}]}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});
	// array数组项少一个,
	json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
			     "\"item\":{\"inter\":16,"
			     "\"double\":16.89,\"string\":\"hello\","
			     "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16."
			     "89,16.89,16.89,16."
			     "89],\"arrayString\":[\"hello\",\"hello\","
			     "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			     "\"double\":16.89,"
			     "\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,"
			     "\"null\":null}]}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	// \"item:{\"inter\":16,\"  少一个"
	json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
			     "\"item:{\"inter\":16,"
			     "\"double\":16.89,\"string\":\"hello\","
			     "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16."
			     "89,16.89,16.89,16."
			     "89],\"arrayString\":[\"hello\",\"hello\","
			     "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			     "\"double\":16.89,"
			     "\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,"
			     "\"null\":null}]}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	// \"item\":{\"inter\":16,double\"  少一个"
	json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
			     "\"item\":{\"inter\":16,"
			     "double\":16.89,\"string\":\"hello\","
			     "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16."
			     "89,16.89,16.89,16."
			     "89],\"arrayString\":[\"hello\",\"hello\","
			     "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			     "\"double\":16.89,"
			     "\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,"
			     "\"null\":null}]}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	// \"item\":{\"inter\":16,\"\"double\"  多一个"
	json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
			     "\"item\":{\"inter\":16,"
			     "\"\"double\":16.89,\"string\":\"hello\","
			     "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16."
			     "89,16.89,16.89,16."
			     "89],\"arrayString\":[\"hello\",\"hello\","
			     "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			     "\"double\":16.89,"
			     "\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,"
			     "\"null\":null}]}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	// \"item\":{\"inter\":16\",\"double\"  多一个"
	json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
			     "\"item\":{\"inter\":16\","
			     "\"double\":16.89,\"string\":\"hello\","
			     "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16."
			     "89,16.89,16.89,16."
			     "89],\"arrayString\":[\"hello\",\"hello\","
			     "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			     "\"double\":16.89,"
			     "\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,"
			     "\"null\":null}]}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	// \"arrayInt\":[16,16,16m,16,16]  无效数组数字
	json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
			     "\"item\":{\"inter\":16,"
			     "\"double\":16.89,\"string\":\"hello\","
			     "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16m,16,16],\"arrayDouble\":[16.89,"
			     "16.89,16.89,16.89,16."
			     "89],\"arrayString\":[\"hello\",\"hello\","
			     "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			     "\"double\":16.89,"
			     "\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			     "\"boolFalse\":false,"
			     "\"null\":null}]}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});
	return RyanJsonTrue;
}
