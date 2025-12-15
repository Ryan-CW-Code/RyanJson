#include "RyanJsonTest.h"

static void printfTitle(char *title)
{
	printf("\r\n");
	printf("\r\n");
	printf("*****************************************************************************\r\n");
	printf("*************************** %s **************************\r\n", title);
	printf("*****************************************************************************\r\n");
}

#ifndef isEnableFuzzer
int main(void)
{
	RyanJsonBool_e result = 0;
	printfTitle("RyanJson 示例程序");
	RyanJsonInitHooks(v_malloc, v_free, v_realloc);

	char *str = NULL;
	RyanJson_t jsonRoot, item;

	for (uint32_t i = 0; i < 1; i++)
	{
		// const char *jsonstr =
		// 	"{\"emoji\":\"\\uD83D\\uDE00\"} ";
		// const char *jsonstr = "{\"name\":\"Mash\",\"star\":4,\"hits\":[2,2,1,3]}";
		// const char *jsonstr = "\012{\"nnnnnnnnnnnnnnnnnnn\012nnlnnnnnn\":0}";
		// const char *jsonstr =
		//
		//
		// "\012{\"\377\377\377\377\377\377\377\377\375\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\":"
		// "0}";
		// char jsonstr[] = "[\"\\u200B\"]";
		// char jsonstr[] = "[\"new\\u000Aline\"]";
		// char jsonstr[] =
		// "\012{\"wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww\":0}"; char
		// jsonstr[] = "\"\334\334\334\""; char jsonstr[] =
		// 	"{\"\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367"
		// 	"\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367"
		// 	"\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\367\":0}"
		// 	"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\351\000\000\000\000\000\000\000\000\000\000\000"
		// 	"\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000";
		// const char *jsonstr = "{\"string\":\"\\u6211\\u662F\\u5730\\u7403\\uD83C\\uDF0D\"}";
		// const char *jsonstr = "\012444444444444444";
		// const char *jsonstr = "\"\"";
		// const char *jsonstr = "{\"nnnnnnnnnnnnnnnnnnn\012nnlnnnnnn\":0}";
		const char *jsonstr =
			"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
			"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
			"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
			"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
			"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
			"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
			"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
			"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
			"[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[";
		// const char jsonstr[] =
		// 	"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{"
		// 	"\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,"
		// 	"16.89,"
		// 	"\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":"
		// 	"16,"
		// 	"\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\","
		// 	"\"boolTrue\":"
		// 	"true,\"boolFalse\":false,\"null\":null}]}";

		// extern int LLVMFuzzerTestOneInput(const char *data, int32_t size);
		// LLVMFuzzerTestOneInput(jsonstr, strlen(jsonstr));

		// 解析json数据
		jsonRoot = RyanJsonParse(jsonstr);
		if (jsonRoot == NULL) { printf("%s:%d 序列化失败\r\n", __FILE__, __LINE__); }
		else
		{
			printf("jsonRoot: %p\r\n", jsonRoot);
			// RyanJsonChangeStringValue(jsonRoot, "hello");
			// RyanJsonChangeStrValue(jsonRoot, "hello");

			RyanJsonDeleteByKey(jsonRoot, "non_exist_key");

			// RyanJsonReplaceByKey(jsonRoot, "star2", RyanJsonDuplicate(jsonRoot));
			// RyanJson_t jsonRoot2 = RyanJsonCreateObject();
			// RyanJsonAddStringToObject(jsonRoot2, "star2", "NULL");
			// if (RyanJsonFalse == RyanJsonReplaceByKey(jsonRoot, "star2", jsonRoot2)) { RyanJsonDelete(jsonRoot2); }

			// RyanJson_t pJson2 = RyanJsonCreateObject();
			// if (RyanJsonFalse == RyanJsonReplaceByIndex(jsonRoot, RyanJsonGetSize(jsonRoot) - 1, pJson2))
			// {
			// 	RyanJsonDelete(pJson2);
			// }

			// printf(" %s\r\n", RyanJsonPrint(RyanJsonCreateString("NULL", "arrayString2222"), 10, RyanJsonFalse, NULL));

			printf("jsonRoot22: %p\r\n", jsonRoot);
			uint32_t len = 0;
			str = RyanJsonPrint(jsonRoot, 10, RyanJsonFalse, &len); // 以带格式方式将数据打印出来
			printf("strLen: %d, data: %s\r\n", len, str);

			// RyanJson_t pJsonDup = RyanJsonDuplicate(jsonRoot);
			// assert(NULL != pJsonDup);

			// // 判断复制json的size是否一致
			// assert(RyanJsonGetSize(jsonRoot) == RyanJsonGetSize(pJsonDup));
			// int32_t dupLen = 0;
			// char *jsonStrDup = RyanJsonPrint(pJsonDup, 100, RyanJsonFalse, &dupLen); // 以带格式方式将数据打印出来
			// if (len != dupLen || 0 != memcmp(str, jsonStrDup, len))
			// {
			// 	printf("len:%d, dupLen:%d\r\n", len, dupLen);
			// 	printf("jsonStr:%s \r\njsonDup:%s\r\n", str, jsonStrDup);
			// }

			// printf("jsonStr:%s \r\njsonDup:%s\r\n", str, jsonStrDup);
			// RyanJsonFree(jsonStrDup);
			// RyanJsonDelete(pJsonDup);

			RyanJsonFree(str);
			RyanJsonDelete(jsonRoot);
		}
	}

	RyanJsonExample();

	result = RyanJsonBaseTest();
	if (RyanJsonTrue != result)
	{
		printf("%s:%d RyanJsonTest fail\r\n", __FILE__, __LINE__);
		return -1;
	}

	printfTitle("RyanJson / cJSON / yyjson RFC8259标准测试");
	RFC8259JsonTest();

	printfTitle("RyanJson / cJSON / yyjson 内存对比程序");
	RyanJsonMemoryFootprintTest();
	printf("\r\nok\r\n");

	displayMem();
	return 0;
}

#endif // isEnableFuzzer
