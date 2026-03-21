#include "../../common/testCommon.h"

static void *yyMalloc(void *ctx, size_t size)
{
	(void)ctx;
	return unityTestMalloc(size);
}

static void *yyRealloc(void *ctx, void *ptr, size_t oldSize, size_t size)
{
	(void)ctx;
	(void)oldSize;
	return unityTestRealloc(ptr, size);
}

static void yyFree(void *ctx, void *ptr)
{
	(void)ctx;
	unityTestFree(ptr);
}

static void RyanJsonMemoryFootprint(const char *jsonstr, int32_t *footprint)
{
	unityTestLeakScope_t leakScope = unityTestLeakScopeBegin();
	RyanJsonInitHooks(unityTestMalloc, unityTestFree, unityTestRealloc);

	RyanJson_t json = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "RyanJson 解析失败");

	*footprint = unityTestGetUse() - leakScope.baseline;
	RyanJsonDelete(json);
	unityTestLeakScopeEnd(leakScope, "RyanJson 释放后 TLSF used 未回到基线");
}

static void cJSONMemoryFootprint(const char *jsonstr, int32_t *footprint)
{
	unityTestLeakScope_t leakScope = unityTestLeakScopeBegin();
	cJSON_Hooks hooks = {.malloc_fn = unityTestMalloc, .free_fn = unityTestFree};
	cJSON_InitHooks(&hooks);

	cJSON *json = cJSON_Parse(jsonstr);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "cJSON 解析失败");

	*footprint = unityTestGetUse() - leakScope.baseline;
	cJSON_Delete(json);
	unityTestLeakScopeEnd(leakScope, "cJSON 释放后 TLSF used 未回到基线");
}

static void yyjsonMemoryFootprint(const char *jsonstr, int32_t *footprint)
{
	unityTestLeakScope_t leakScope = unityTestLeakScopeBegin();
	yyjson_alc yyalc = {yyMalloc, yyRealloc, yyFree, NULL};

	// 先解析成只读文档
	yyjson_doc *doc = yyjson_read_opts(jsonstr, strlen(jsonstr), YYJSON_READ_NOFLAG, &yyalc, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(doc, "yyjson 解析只读文档失败");

	// 从只读文档拷贝为可变文档
	yyjson_mut_doc *mdoc = yyjson_doc_mut_copy(doc, &yyalc);
	yyjson_doc_free(doc);
	TEST_ASSERT_NOT_NULL_MESSAGE(mdoc, "yyjson 拷贝可变文档失败");

	*footprint = unityTestGetUse() - leakScope.baseline;
	yyjson_mut_doc_free(mdoc);
	unityTestLeakScopeEnd(leakScope, "yyjson 释放后 TLSF used 未回到基线");
}

static void printfJsonCompare(const char *id, const char *title, const char *jsonstr)
{
	int32_t ryanJsonCount = 0;
	int32_t cJSONCount = 0;
	int32_t yyjsonCount = 0;

	RyanJsonMemoryFootprint(jsonstr, &ryanJsonCount);
	cJSONMemoryFootprint(jsonstr, &cJSONCount);
	yyjsonMemoryFootprint(jsonstr, &yyjsonCount);

	(void)testLog("%s 原始长度: %lu, 模拟分配(header=%lu,align=%lu) 峰值占用 -> RyanJson:%ld, cJSON:%ld, yyjson:%ld\r\n", title,
		      (unsigned long)strlen(jsonstr), (unsigned long)RyanJsonTestAllocHeaderSize, (unsigned long)RyanJsonTestAllocAlignSize,
		      (long)ryanJsonCount, (long)cJSONCount, (long)yyjsonCount);
	(void)testLog("[MEM][COMPARE] id=%s strict=%d head=%d sci=%d len=%lu header=%lu align=%lu ryanjson=%ld cjson=%ld yyjson=%ld\r\n",
		      (NULL != id) ? id : "unknown",
		      (int)RyanJsonStrictObjectKeyCheck,
		      (int)RyanJsonDefaultAddAtHead,
		      (int)RyanJsonSnprintfSupportScientific,
		      (unsigned long)strlen(jsonstr),
		      (unsigned long)RyanJsonTestAllocHeaderSize,
		      (unsigned long)RyanJsonTestAllocAlignSize,
		      (long)ryanJsonCount,
		      (long)cJSONCount,
		      (long)yyjsonCount);

	TEST_ASSERT_TRUE_MESSAGE(ryanJsonCount > 0 && cJSONCount > 0 && yyjsonCount > 0, "内存统计值必须大于 0");
	TEST_ASSERT_TRUE_MESSAGE(ryanJsonCount < cJSONCount, "RyanJson 内存占用应低于 cJSON");
	if (ryanJsonCount >= yyjsonCount)
	{
		(void)testLog("提示: 当前模拟参数下 RyanJson(%ld) 未低于 yyjson(%ld)\r\n", (long)ryanJsonCount, (long)yyjsonCount);
	}

	if (cJSONCount > 0 && yyjsonCount > 0)
	{
		double saveVsCjson = 100.0 - ((double)ryanJsonCount * 100.0) / (double)cJSONCount;
		double saveVsYyjson = 100.0 - ((double)ryanJsonCount * 100.0) / (double)yyjsonCount;
		(void)testLog("内存优化程度 -> 比 cJSON 节省: \033[1;32m%.2f%%\033[0m, 比 yyjson 节省: \033[1;32m%.2f%%\033[0m\r\n",
			      saveVsCjson, saveVsYyjson);
	}
}

static void testMixedJsonMemory(void)
{
	static const char jsonstr[] =
		"{\"item1\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
		"\"item\":{\"inter\":16,"
		"\"double\":16.89,\"string\":\"hello\","
		"\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,"
		"16.89,16.89],"
		"\"arrayString\":[\"hello\",\"hello\",\"hello\","
		"\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,"
		"\"string\":\"hello\","
		"\"boolTrue\":true,\"boolFalse\":false,"
		"\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]"
		"},\"item2\":{"
		"\"inter\":16,\"double\":16.89,\"string\":"
		"\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":"
		"\"hello\",\"boolTrue\":"
		"true,\"boolFalse\":false,\"null\":null},"
		"\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\","
		"\"hello\",\"hello\","
		"\"hello\"],\"array\":[16,16.89,\"hello\","
		"true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":"
		"false,\"null\":null},{"
		"\"inter\":16,\"double\":16.89,\"string\":"
		"\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]},\"item3\":{\"inter\":16,\"double\":16.89,\"string\":"
		"\"hello\",\"boolTrue\":"
		"true,\"boolFalse\":false,\"null\":null,"
		"\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"\"arrayInt\":[16,16,16,16,"
		"16],\"arrayDouble\":[16.89,16.89,16.89,"
		"16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,"
		"false,null],"
		"\"arrayItem\":[{\"inter\":16,\"double\":16.89,"
		"\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":"
		"\"hello\",\"boolTrue\":"
		"true,\"boolFalse\":false,\"null\":null}]}"
		",\"item4\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
		"\"item\":{\"inter\":16,"
		"\"double\":16.89,\"string\":\"hello\","
		"\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,"
		"16.89,16.89],"
		"\"arrayString\":[\"hello\",\"hello\",\"hello\","
		"\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,"
		"\"string\":\"hello\","
		"\"boolTrue\":true,\"boolFalse\":false,"
		"\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]"
		"}}";
	printfJsonCompare("mixed", "混合对象", jsonstr);
}

static void testObjectJsonMemory(void)
{
	static const char jsonstr[] =
		"{\"message\":\"success感谢又拍云(upyun.com)提供CDN赞助\",\"status\":200,\"date\":\"20230822\",\"time\":\"2023-08-22 "
		"09:44:54\",\"cityInfo\":{\"city\":\"郑州市\",\"citykey\":\"101180101\",\"parent\":\"河南\",\"updateTime\":\"07:46\"},"
		"\"data\":{\"shidu\":"
		"\"85%\",\"pm25\":20,\"pm10\":56,"
		"\"quality\":\"良\",\"wendu\":\"29\",\"ganmao\":\"极少数敏感人群应减少户外活动\",\"forecast\":[{\"date\":\"22\",\"high\":"
		"\"高温 "
		"35℃\",\"low\":\"低温 "
		"23℃\",\"ymd\":\"2023-08-22\",\"week\":\"星期二\",\"sunrise\":\"05:51\",\"sunset\":\"19:05\",\"aqi\":78,\"fx\":\"东南风\","
		"\"fl\":\"2级\","
		"\"type\":\"晴\",\"notice\":"
		"\"愿你拥有比阳光明媚的心情\"},{\"date\":\"23\",\"high\":\"高温 33℃\",\"low\":\"低温 "
		"23℃\",\"ymd\":\"2023-08-23\",\"week\":\"星期三\",\"sunrise\":\"05:52\",\"sunset\":\"19:04\",\"aqi\":71,\"fx\":\"南风\","
		"\"fl\":\"2级\","
		"\"type\":\"中雨\",\"notice\":"
		"\"记得随身携带雨伞哦\"},{\"date\":\"24\",\"high\":\"高温 31℃\",\"low\":\"低温 "
		"21℃\",\"ymd\":\"2023-08-24\",\"week\":\"星期四\",\"sunrise\":\"05:52\",\"sunset\":\"19:03\",\"aqi\":74,\"fx\":\"东风\","
		"\"fl\":\"2级\","
		"\"type\":\"晴\",\"notice\":"
		"\"愿你拥有比阳光明媚的心情\"},{\"date\":\"25\",\"high\":\"高温 30℃\",\"low\":\"低温 "
		"23℃\",\"ymd\":\"2023-08-25\",\"week\":\"星期五\",\"sunrise\":\"05:53\",\"sunset\":\"19:02\",\"aqi\":93,\"fx\":\"东风\","
		"\"fl\":\"1级\","
		"\"type\":\"小雨\",\"notice\":"
		"\"雨虽小，注意保暖别感冒\"},{\"date\":\"26\",\"high\":\"高温 25℃\",\"low\":\"低温 "
		"22℃\",\"ymd\":\"2023-08-26\",\"week\":\"星期六\",\"sunrise\":\"05:54\",\"sunset\":\"19:00\",\"aqi\":80,\"fx\":\"东北风\","
		"\"fl\":\"1级\","
		"\"type\":\"阴\",\"notice\":"
		"\"不要被阴云遮挡住好心情\"},{\"date\":\"27\",\"high\":\"高温 27℃\",\"low\":\"低温 "
		"20℃\",\"ymd\":\"2023-08-27\",\"week\":\"星期日\",\"sunrise\":\"05:55\",\"sunset\":\"18:59\",\"aqi\":74,\"fx\":\"西北风\","
		"\"fl\":\"1级\","
		"\"type\":\"阴\",\"notice\":"
		"\"不要被阴云遮挡住好心情\"},{\"date\":\"28\",\"high\":\"高温 30℃\",\"low\":\"低温 "
		"20℃\",\"ymd\":\"2023-08-28\",\"week\":\"星期 "
		"一\",\"sunrise\":\"05:55\",\"sunset\":\"18:58\",\"aqi\":80,\"fx\":\"东北风\",\"fl\":\"2级\",\"type\":\"多云\",\"notice\":"
		"\"阴晴之间，谨防紫外线侵扰\"},{\"date\":\"29\",\"high\":"
		"\"高温 30℃\",\"low\":\"低温 "
		"20℃\",\"ymd\":\"2023-08-29\",\"week\":\"星期二\",\"sunrise\":\"05:56\",\"sunset\":\"18:56\",\"aqi\":80,\"fx\":\"东北风\","
		"\"fl\":\"2级\","
		"\"type\":\"多云\",\"notice\":"
		"\"阴晴之间，谨防紫外线侵扰\"},{\"date\":\"30\",\"high\":\"高温 31℃\",\"low\":\"低温 "
		"20℃\",\"ymd\":\"2023-08-30\",\"week\":\"星期三\",\"sunrise\":\"05:57\",\"sunset\":\"18:55\",\"aqi\":92,\"fx\":\"南风\","
		"\"fl\":\"1级\","
		"\"type\":\"晴\",\"notice\":"
		"\"愿你拥有比阳光明媚的心情\"},{\"date\":\"31\",\"high\":\"高温 33℃\",\"low\":\" 低温 "
		"22℃\",\"ymd\":\"2023-08-31\",\"week\":\"星期四\",\"sunrise\":\"05:57\",\"sunset\":\"18:54\",\"aqi\":91,\"fx\":\"南风\","
		"\"fl\":\"1级\","
		"\"type\":\"晴\",\"notice\":"
		"\"愿你拥有比阳光明媚的心情\"},{\"date\":\"01\",\"high\":\"高温 34℃\",\"low\":\"低温 "
		"23℃\",\"ymd\":\"2023-09-01\",\"week\":\"星期五\",\"sunrise\":\"05:58\",\"sunset\":\"18:52\",\"aqi\":91,\"fx\":\"西风\","
		"\"fl\":\"1级\","
		"\"type\":\"晴\",\"notice\":"
		"\"愿你拥有比阳光明媚的心情\"},{\"date\":\"02\",\"high\":\"高温 36℃\",\"low\":\"低温 "
		"25℃\",\"ymd\":\"2023-09-02\",\"week\":\"星期六\",\"sunrise\":\"05:59\",\"sunset\":\"18:51\",\"aqi\":78,\"fx\":\"南风\","
		"\"fl\":\"1级\","
		"\"type\":\"阴\",\"notice\":"
		"\"不要被阴云遮挡住好心情\"},{\"date\":\"03\",\"high\":\"高温 35℃\",\"low\":\"低温 "
		"24℃\",\"ymd\":\"2023-09-03\",\"week\":\"星期日\",\"sunrise\":\"06:00\",\"sunset\":\"18:50\",\"aqi\":82,\"fx\":\"东北风\","
		"\"fl\":\"1级\","
		"\"type\":\"晴\",\"notice\":"
		"\"愿你拥有比阳光明媚的心情\"},{\"date\":\"04\",\"high\":\"高温 35℃\",\"low\":\"低温 "
		"25℃\",\"ymd\":\"2023-09-04\",\"week\":\"星期一\",\"sunrise\":\"06:00\",\"sunset\":\"18:48\",\"aqi\":88,\"fx\":\"南风\","
		"\"fl\":\"2级\","
		"\"type\":\"晴\",\"notice\":"
		"\"愿你拥有比阳光明媚的心情\"},{\"date\":\"05\",\"high\":\"高温 35℃\",\"low\":\"低温 "
		"25℃\",\"ymd\":\"2023-09-05\",\"week\":\"星期二\",\"sunrise\":\"06:01\",\"sunset\":\"18:47\",\"aqi\":58,\"fx\":\"南风\","
		"\"fl\":\"2级\","
		"\"type\":\"阴\",\"notice\":"
		"\"不要被阴云遮挡住好心情\"}],\"yesterday\":{\"date\":\"21\",\"high\":\"高温 34℃\",\"low\":\"低温 "
		"24℃\",\"ymd\":\"2023-08-21\",\"week\":\" "
		"星期一\",\"sunrise\":\"05:50\",\"sunset\":\"19:07\",\"aqi\":60,\"fx\":\"西风\",\"fl\":\"2级\",\"type\":\"小雨\","
		"\"notice\":"
		"\"雨虽小，注意保暖别感冒\"}}}";
	printfJsonCompare("weather_object", "经典天气对象", jsonstr);
}

static void testArrayJsonMemory(void)
{
	static const char jsonstr[] =
		"{\"item1\":{\"arrayInt\":[16,16,16,16,16,16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89,16.89,16.89,16."
		"89,16.89,16.89],"
		"\"arrayString\":[\"hello\",\"hello\","
		"\"hello\",\"hello\",\"hello\",\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,"
		"false,null,16,16.89,"
		"\"hello\",true,false,null]},\"item2\":{"
		"\"arrayInt\":[16,16,16,16,16,16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89,16.89,16.89,16.89,16.89,16."
		"89],\"arrayString\":["
		"\"hello\",\"hello\",\"hello\",\"hello\","
		"\"hello\",\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null,16,16.89,"
		"\"hello\",true,false,"
		"null]},\"item3\":{\"arrayInt\":[16,16,16,"
		"16,16,16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89,16.89,16.89,16.89,16.89,16.89],\"arrayString\":["
		"\"hello\",\"hello\","
		"\"hello\",\"hello\",\"hello\",\"hello\","
		"\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null,16,16.89,\"hello\",true,false,"
		"null]},\"item4\":{"
		"\"arrayInt\":[16,16,16,16,16,16,16,16,16,16],"
		"\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89,16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\","
		"\"hello\",\"hello\","
		"\"hello\",\"hello\",\"hello\",\"hello\","
		"\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null,16,16.89,\"hello\",true,false,null]}}";
	printfJsonCompare("deep_array", "深度数组", jsonstr);
}

static void testSmallMixedJsonMemory(void)
{
	static const char jsonstr[] =
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}";
	printfJsonCompare("small_mixed", "小型混合对象", jsonstr);
}

static void testSmallStringJsonMemory(void)
{
	static const char jsonstr[] =
		"{\"inter\":\"16\",\"double\":\"16.89\",\"string\":\"hello\",\"boolTrue\":\"true\",\"boolFalse\":\"false\",\"null\":"
		"\"null\"}";
	printfJsonCompare("small_string", "小型字符串对象", jsonstr);
}

static void testCompressedBusinessJsonMemory(void)
{
	static const char jsonstr[] =
		"{\"0\":\"0\",\"1\":\"189774523\",\"2\":{\"7\":\"3\",\"8\":\"103\",\"9\":\"37\",\"20\":\"0\",\"26\":\"37\",\"27\":"
		"\"367\",\"28\":\"367\",\"s\":\"0\",\"t\":\"0\",\"a\":\"24.98\",\"2a\":\"0\",\"1p\":\"23628\"},\"3\":\"0\",\"22\":"
		"\"epmgrow1105\",\"23\":\"0\",\"29\":\"0\",\"i\":\"4\",\"b\":\"900\",\"c\":\"1\",\"rsrp\":\"-111\",\"rsrq\":\"-4\","
		"\"sinr\":\"0\",\"soc\":\"XXXXXXX\",\"j\":\"0\",\"g\":\"898604asdf0210\",\"h\":\"866968798839\",\"d\":\"1.3.5."
		"00.20991231\",\"f\":\"0\",\"k\":\"1\",\"l\":\"20000\",\"m\":\"20000\",\"u\":\"0\",\"v\":\"0\",\"e\":\"1\",\"w\":\"0."
		"00\",\"n\":\"0\",\"2h\":\"0\",\"o\":\"30\",\"1v\":\"12000\",\"2c\":\"0\",\"p\":\"1\",\"q\":\"1\",\"x\":\"0\",\"y\":"
		"\"167\",\"r\":\"0\",\"1x\":\"0\",\"1w\":\"0\",\"1y\":\"100.00\",\"1u\":\"0\"}";
	printfJsonCompare("compressed_business", "压缩业务对象", jsonstr);
}

void testMemoryRunner(void)
{
	UnitySetTestFile(__FILE__);
	unityTestSetAllocSimulation(1U);

	RUN_TEST(testMixedJsonMemory);
	RUN_TEST(testObjectJsonMemory);
	RUN_TEST(testArrayJsonMemory);
	RUN_TEST(testSmallMixedJsonMemory);
	RUN_TEST(testSmallStringJsonMemory);
	RUN_TEST(testCompressedBusinessJsonMemory);

	unityTestSetAllocSimulation(0U);
}
