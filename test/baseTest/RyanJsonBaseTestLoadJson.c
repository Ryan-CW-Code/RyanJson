
#include "RyanJsonBaseTest.h"

/* --------------------------------------------------------------------- */

RyanJsonBool_e RyanJsonBaseTestLoadJson(void)
{
	char *str = NULL;
	RyanJson_t json;
	char *jsonstr = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":"
			"{\"inter\":16,\"double\":16."
			"89,\"string\":\"hello\","
			"\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,"
			"16.89,16.89,16.89],"
			"\"arrayString\":[\"hello\",\"hello\","
			"\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			"\"double\":16.89,\"string\":"
			"\"hello\",\"boolTrue\":true,"
			"\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			"\"boolFalse\":false,\"null\":null}],\"unicode\":\"😀\"}";

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
	 * @brief 测试 Unicode
	 *
	 */
	char printfBuf[1024] = {0};
	json = RyanJsonParse("{\"emoji\":\"\\uD83D\\uDE00\"}");
	RyanJsonCheckReturnFalse(NULL != json);
	str = RyanJsonPrintPreallocated(json, printfBuf, sizeof(printfBuf), RyanJsonFalse, NULL);
	RyanJsonDelete(json);

	// 测试数字 0-9 分支: \u0030 = '0', \u0039 = '9'
	json = RyanJsonParse("{\"num\":\"\\u0030\\u0039\"}");
	RyanJsonCheckReturnFalse(NULL != json);
	str = RyanJsonPrintPreallocated(json, printfBuf, sizeof(printfBuf), RyanJsonFalse, NULL);
	RyanJsonCheckCode(0 == strcmp(str, "{\"num\":\"09\"}"), {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});
	RyanJsonDelete(json);

	// 测试小写 a-f 分支: \u0061 = 'a', \u0066 = 'f'
	json = RyanJsonParse("{\"lower\":\"\\u0061\\u0062\\u0063\\u0064\\u0065\\u0066\"}");
	RyanJsonCheckReturnFalse(NULL != json);
	str = RyanJsonPrintPreallocated(json, printfBuf, sizeof(printfBuf), RyanJsonFalse, NULL);
	RyanJsonCheckCode(0 == strcmp(str, "{\"lower\":\"abcdef\"}"), {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});
	RyanJsonDelete(json);

	// 测试大写 A-F 分支: \u0041 = 'A', \u0046 = 'F'
	json = RyanJsonParse("{\"upper\":\"\\u0041\\u0042\\u0043\\u0044\\u0045\\u0046\"}");
	RyanJsonCheckReturnFalse(NULL != json);
	str = RyanJsonPrintPreallocated(json, printfBuf, sizeof(printfBuf), RyanJsonFalse, NULL);
	RyanJsonCheckCode(0 == strcmp(str, "{\"upper\":\"ABCDEF\"}"), {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});
	RyanJsonDelete(json);

	// 测试混合大小写: \uAbCd (混合大小写十六进制)
	json = RyanJsonParse("{\"mixed\":\"\\uAbCd\"}");
	RyanJsonCheckReturnFalse(NULL != json);
	RyanJsonFree(RyanJsonPrint(json, 50, RyanJsonFalse, NULL));
	RyanJsonDelete(json);

	// 测试 default 分支 (非法十六进制字符 'G')
	json = RyanJsonParse("{\"invalid\":\"\\uGGGG\"}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	// 测试 default 分支 (非法十六进制字符 'Z')
	json = RyanJsonParse("{\"invalid\":\"\\u00ZZ\"}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

	// 测试 default 分支 (非法十六进制字符 '!')
	json = RyanJsonParse("{\"invalid\":\"\\u00!!\"}");
	RyanJsonCheckCode(NULL == json, {
		RyanJsonDelete(json);
		return RyanJsonFalse;
	});

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
