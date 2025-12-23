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
extern void printJsonDebug(RyanJson_t json);
int main(void)
{

	RyanJsonBool_e result = RyanJsonFalse;
	RyanJsonInitHooks(v_malloc, v_free, v_realloc);

	for (uint32_t i = 0; i < 1; i++)
	{

		char *str = NULL;
		RyanJson_t jsonRoot, item;

		// const char *jsonstr =
		// 	"{\"emoji\":\"\\uD83D\\uDE00\"} ";
		// const char *jsonstr = "{\"name\":\"Mash\",\"star\":4,\"hits\":[2,2,1,3]}";
		const char *jsonstr = "[1";

		// extern int LLVMFuzzerTestOneInput(const char *data, int32_t size);
		// LLVMFuzzerTestOneInput(jsonstr, strlen(jsonstr));

		// 解析json数据
		// jsonRoot = RyanJsonParse(jsonstr);
		// if (jsonRoot == NULL) { printf("%s:%d 序列化失败\r\n", __FILE__, __LINE__); }
		// else
		// {
		// 	uint32_t len = 0;
		// 	str = RyanJsonPrint(jsonRoot, 10, RyanJsonFalse, &len); // 以带格式方式将数据打印出来
		// 	printf("strLen: %d, data: %s\r\n", len, str);

		// 	RyanJsonFree(str);
		// 	RyanJsonDelete(jsonRoot);
		// }
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
