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

RyanJsonBool_e RyanJsonBaseTest(void)
{
	int32_t result = 0;
	uint32_t testRunCount = 0;
	uint64_t funcStartMs;

	runTestWithLogAndTimer(RyanJsonBaseTestChangeJson);    // 验证 JSON 动态更新及存储模式切换逻辑
	runTestWithLogAndTimer(RyanJsonBaseTestCompareJson);   // 验证节点及其属性的深度一致性比较逻辑
	runTestWithLogAndTimer(RyanJsonBaseTestCreateJson);    // 验证全类型节点的构造与初始化逻辑
	runTestWithLogAndTimer(RyanJsonBaseTestDeleteJson);    // 验证节点及其子项的递归内存回收逻辑
	runTestWithLogAndTimer(RyanJsonBaseTestDetachJson);    // 验证节点的分离操作及其所属权转移逻辑
	runTestWithLogAndTimer(RyanJsonBaseTestDuplicateJson); // 验证对象的深拷贝 (Deep Copy) 完整性逻辑
	runTestWithLogAndTimer(RyanJsonBaseTestForEachJson);   // 验证数组与对象迭代器的稳定性与边界情况
	runTestWithLogAndTimer(RyanJsonBaseTestLoadJson);      // 验证复杂 JSON 文本解析与内存映射的健壮性
	runTestWithLogAndTimer(RyanJsonBaseTestReplaceJson);   // 验证节点就地替换与成员管理机制的有效性

	// 验证节点属性一致性
	runTestWithLogAndTimer(RyanJsonBaseTestEqualityBool);   // 验证布尔值一致性
	runTestWithLogAndTimer(RyanJsonBaseTestEqualityDouble); // 验证浮点数一致性
	runTestWithLogAndTimer(RyanJsonBaseTestEqualityInt);    // 验证整数一致性
	runTestWithLogAndTimer(RyanJsonBaseTestEqualityString); // 验证字符串一致性

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
