#include "RyanJson.h"
#include "RyanJsonTest.h"
#include "tlsf.h"

extern void printJsonDebug(RyanJson_t json);
#define jsonLogByTest(fmt, ...) printf("%s:%d " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define LV_MEM_SIZE             (1024 * 1024)

static void printfTitle(char *title)
{
	printf("\r\n");
	printf("\r\n");
	printf("*****************************************************************************\r\n");
	printf("*************************** %s **************************\r\n", title);
	printf("*****************************************************************************\r\n");
}

static tlsf_t tlsfHandler;

static int32_t total2 = LV_MEM_SIZE, used2 = 0, available = 0;
bool tlsf_walker_callback(void *ptr, size_t size, int used, void *user)
{
	if (1 == used) { used2 += size + tlsf_alloc_overhead(); }
	return true;
}

void showMemoryInfo(void)
{
	int32_t total = 0, used = 0, max_used = 0;
	rt_memory_info22(tlsfHandler, &total, &used, &max_used);
	jsonLogByTest("total: %d, used: %d, max_used: %d, available: %d\r\n", total, used, max_used, total - used);

	used2 = 0, available = 0;
	total2 = total;
	tlsf_walk_pool(tlsf_get_pool(tlsfHandler), tlsf_walker_callback, NULL);
	jsonLogByTest("total2: %d, used2: %d, max_used2: %d, available2: %d\r\n", total2, used2, 0, total2 - used2);
}

int32_t vallocGetUseByTlsf(void)
{
	int32_t total = 0, used = 0, max_used = 0;
	rt_memory_info22(tlsfHandler, &total, &used, &max_used);
	return used;
}

void *v_malloc_tlsf(size_t size)
{
	if (size == 0) { return NULL; }

	return tlsf_malloc(tlsfHandler, size);
}

void v_free_tlsf(void *block)
{
	if (!block) { return; }

	tlsf_free(tlsfHandler, block);
}

void *v_realloc_tlsf(void *block, size_t size) { return tlsf_realloc(tlsfHandler, block, size); }

#ifndef isEnableFuzzer
int main(void)
{
	char *tlsfMemBuf = v_malloc(LV_MEM_SIZE);
	tlsfHandler = tlsf_create_with_pool((void *)tlsfMemBuf, LV_MEM_SIZE, LV_MEM_SIZE);
	jsonLogByTest("tlsf_size: %d\r\n", tlsf_size(tlsfHandler));

	RyanJsonBool_e result = RyanJsonFalse;
	RyanJsonInitHooks(v_malloc_tlsf, v_free_tlsf, v_realloc_tlsf);

	for (uint32_t i = 0; i < 1; i++)
	{
		char *str = NULL;
		RyanJson_t jsonRoot, item;

		// const char *jsonstr = "{\"emoji\":\"\\uD83D\\uDE00\"} ";
		const char *jsonstr = "{\"name\":\"Mash\",\"star\":4,\"hits\":[2,2,1,3]}";
		// const char *jsonstr = "{\"star\":4}";
		// const char *jsonstr = "\"name\"";
		// const char *jsonstr =
		// "{\"n\":0,\"q\":1,\"e\":1,\"p\":1,\"1u\":0,\"r\":0,\"w\":0.0,\"1w\":0,\"1x\":0,\"1y\":100.0,\"23\":0,"
		// 		      "\"29\":0,\"2h\":0,\"o\":30,\"1v\":12000,\"2c\":0}";

		// extern int LLVMFuzzerTestOneInput(const char *data, int32_t size);
		// LLVMFuzzerTestOneInput(jsonstr, strlen(jsonstr));

		// 解析json数据
		jsonRoot = RyanJsonParse(jsonstr);
		if (jsonRoot == NULL) { printf("%s:%d 序列化失败\r\n", __FILE__, __LINE__); }
		else
		{
			uint32_t len = 0;
			str = RyanJsonPrint(jsonRoot, 10, RyanJsonFalse, &len); // 以带格式方式将数据打印出来
			printf("strLen: %d, data: %s\r\n", len, str);

			RyanJsonFree(str);
			RyanJsonDelete(jsonRoot);
		}
	}

	showMemoryInfo();

	result = RyanJsonExample();
	if (RyanJsonTrue != result)
	{
		printf("%s:%d RyanJsonExample fail\r\n", __FILE__, __LINE__);
		return -1;
	}

	result = RyanJsonBaseTest();
	if (RyanJsonTrue != result)
	{
		printf("%s:%d RyanJsonBaseTest fail\r\n", __FILE__, __LINE__);
		return -1;
	}

	printfTitle("RyanJson / cJSON / yyjson RFC8259标准测试");
	RFC8259JsonTest();

	printfTitle("RyanJson / cJSON / yyjson 内存对比程序");
	RyanJsonMemoryFootprintTest();
	printf("\r\nok\r\n");

	showMemoryInfo();
	v_free(tlsfMemBuf);
	displayMem();
	return 0;
}

#endif // isEnableFuzzer
