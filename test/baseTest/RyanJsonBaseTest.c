#include "RyanJsonBaseTest.h"

static RyanJsonBool_e likeReferenceTest()
{

	// char *str = NULL;
	// char jsonstr[] =
	// "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}";
	// RyanJson_t json = RyanJsonParse(jsonstr);
	// RyanJson_t item = NULL;

	// // RyanJson_t adfasdf = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));

	// // RyanJsonAddItemToObject(json, "test", adfasdf);

	// // 这里做你想做的事,这里我选择打印出来
	// // str = RyanJsonPrint(json, 50, RyanJsonTrue, NULL);
	// // printf("item %s \r\n", str);
	// // RyanJsonFree(str);

	// for (int i = 0; i < 1; i++)
	// {
	//     // 分离test对象
	//     item = RyanJsonDetachByKey(json, "item");

	//     // if (RyanJsonIsKey(item))
	//     //     RyanJsonFree(RyanJsonGetKey(item));

	//     // RyanJsonFree(item);
	// }

	// RyanJsonAddItemToObject(json, "item", item);

	// str = RyanJsonPrint(json, 50, RyanJsonTrue, NULL);
	// printf("item %s \r\n", str);
	// RyanJsonFree(str);

	// RyanJsonDelete(json);

	return 0;
}

uint64_t platformUptimeMs(void)
{
	struct timespec ts;
	// CLOCK_MONOTONIC: 单调递增，不受系统时间修改影响，适合做耗时统计
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

RyanJsonBool_e RyanJsonBaseTest(void)
{
	int32_t result = 0;
	RyanJsonInitHooks(v_malloc, v_free, v_realloc);

	uint32_t testRunCount = 0;
	uint64_t funcStartMs;
#define runTestWithLogAndTimer(fun)                                                                                                        \
	do                                                                                                                                 \
	{                                                                                                                                  \
		testRunCount++;                                                                                                            \
		printf("┌── [TEST %d] 开始执行: " #fun "()\r\n", testRunCount);                                                            \
		funcStartMs = platformUptimeMs();                                                                                          \
		result = fun();                                                                                                            \
		printf("└── [TEST %" PRIu32 "] 结束执行: 返回值 = %" PRId32 " %s | 耗时: %" PRIu64 " ms\x1b[0m\r\n\r\n", testRunCount,     \
		       result, (result == RyanJsonTrue) ? "✅" : "❌", (platformUptimeMs() - funcStartMs));                                \
		RyanJsonCheckCodeNoReturn(RyanJsonTrue == result, { goto __exit; });                                                       \
	} while (0)

	runTestWithLogAndTimer(RyanJsonBaseTestChangeJson);    // JSON 修改功能的条件覆盖测试
	runTestWithLogAndTimer(RyanJsonBaseTestCompareJson);   // 节点比较与一致性验证
	runTestWithLogAndTimer(RyanJsonBaseTestCreateJson);    // 节点创建与结构正确性检查
	runTestWithLogAndTimer(RyanJsonBaseTestDeleteJson);    // JSON 删除功能的条件覆盖测试
	runTestWithLogAndTimer(RyanJsonBaseTestDetachJson);    // 节点分离操作的条件覆盖测试
	runTestWithLogAndTimer(RyanJsonBaseTestDuplicateJson); // 节点复制的深拷贝与浅拷贝验证
	runTestWithLogAndTimer(RyanJsonBaseTestForEachJson);   // 节点遍历与迭代稳定性测试
	runTestWithLogAndTimer(RyanJsonBaseTestLoadJson);      // JSON 文本解析与加载能力验证
	runTestWithLogAndTimer(RyanJsonBaseTestReplaceJson);   // 节点替换功能的条件覆盖测试

	// result = likeReferenceTest(); // 模仿 引用类型实现 示例
	// if (0 != result)
	// {
	// 	printf("%s:%d loadJsonTest fail\r\n", __FILE__, __LINE__);
	// 	return RyanJsonFalse;
	// }

	displayMem();
	return RyanJsonTrue;

__exit:
	displayMem();
	return RyanJsonFalse;
}
