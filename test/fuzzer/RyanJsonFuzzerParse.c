#include "RyanJsonFuzzer.h"

RyanJsonBool_e RyanJsonFuzzerTestParse(RyanJson_t pJson, const char *data, uint32_t size)
{
	isEnableRandomMemFail = RyanJsonFalse;
	RyanJson_t testItem = RyanJsonCreateObject();
	RyanJson_t testItem2 = RyanJsonCreateObject();
	RyanJsonSetType(testItem, 0);
	RyanJsonSetType(testItem2, 0);
	RyanJsonAssert(NULL == RyanJsonPrint(testItem, 100, RyanJsonFalse, NULL));
	RyanJsonAssert(NULL == RyanJsonDuplicate(testItem));
	RyanJsonAssert(RyanJsonFalse == RyanJsonCompare(testItem, testItem2));
	RyanJsonAssert(RyanJsonFalse == RyanJsonCompareOnlyKey(testItem, testItem2));

	// 测试pJson类型错误情况
	RyanJsonAssert(RyanJsonFalse == RyanJsonInsert(testItem, 0, RyanJsonCreateString("key", "true")));
	RyanJsonAssert(RyanJsonFalse == RyanJsonInsert(testItem2, UINT32_MAX, RyanJsonCreateString("key", "true")));

	RyanJsonSetType(testItem, RyanJsonTypeObject);
	RyanJsonSetType(testItem2, RyanJsonTypeObject);

	// 测试pJson为obj，但是item没有key的情况
	RyanJsonAssert(RyanJsonFalse == RyanJsonInsert(pJson, 0, RyanJsonCreateString(NULL, "true")));
	RyanJsonAssert(RyanJsonFalse == RyanJsonInsert(pJson, UINT32_MAX, RyanJsonCreateString(NULL, "true")));

	RyanJsonAssert(RyanJsonTrue == RyanJsonInsert(pJson, 0, RyanJsonCreateString("key", "true")));
	RyanJsonDelete(testItem);
	RyanJsonDelete(testItem2);
	isEnableRandomMemFail = RyanJsonTrue;

	RyanJsonAssert(NULL == RyanJsonPrint(NULL, 100, RyanJsonFalse, NULL));
	RyanJsonAssert(NULL == RyanJsonPrintPreallocated(NULL, NULL, 100, RyanJsonFalse, NULL));
	RyanJsonAssert(NULL == RyanJsonPrintPreallocated(pJson, NULL, 100, RyanJsonFalse, NULL));
	RyanJsonAssert(NULL == RyanJsonPrintPreallocated(NULL, (char *)data, 100, RyanJsonFalse, NULL));
	RyanJsonAssert(NULL == RyanJsonPrintPreallocated(pJson, (char *)data, 0, RyanJsonFalse, NULL));

	uint32_t len = 0;
	char *jsonStr = RyanJsonPrint(pJson, size % 5 ? 100 : 0, size % 2 ? RyanJsonFalse : RyanJsonTrue, &len);
	RyanJsonCheckReturnFalse(NULL != jsonStr && len > 0);

	char *jsonStrCopy = RyanJsonPrint(pJson, size % 5 ? 100 : 0, size % 2 ? RyanJsonFalse : RyanJsonTrue, NULL); // 不传递len
	RyanJsonAssert(0 == strncmp(jsonStr, jsonStrCopy, len));
	RyanJsonFree(jsonStr);
	RyanJsonFree(jsonStrCopy);

	uint32_t bufLen = len * 3;
	if (bufLen < size * 2) { bufLen = size * 2; }
	if (bufLen < 2048) { bufLen = 2048; }
	char *buf = (char *)malloc((size_t)bufLen);
	{
		uint32_t len2 = 0;
		char *jsonStr2 = RyanJsonPrintPreallocated(pJson, buf, bufLen, size % 2 ? RyanJsonFalse : RyanJsonTrue, &len2);
		// printf("len: %d, len2: %d, str: %s\r\n", len, len2, NULL == jsonStr2 ? "NULL" : jsonStr2);
		RyanJsonCheckCode(NULL != jsonStr2 && len == len2, {
			free(buf);
			return RyanJsonFalse;
		});
	}

	memcpy(buf, data, (size_t)size);
	buf[size] = 0;
	RyanJson_t jsonRoot = RyanJsonParse(buf);
	RyanJsonCheckCode(NULL != jsonRoot, {
		free(buf);
		return RyanJsonFalse;
	});

	// 测试多次打印结果是否一致
	{
		uint32_t len3 = 0;
		char *jsonStr3 = RyanJsonPrint(jsonRoot, 100, size % 2 ? RyanJsonFalse : RyanJsonTrue, &len3); // 以带格式方式将数据打印出来
		RyanJsonCheckCode(NULL != jsonStr3 && len == len3, {
			free(buf);
			if (jsonStr3) { RyanJsonFree(jsonStr3); }
			RyanJsonDelete(jsonRoot);
			return RyanJsonFalse;
		});

		RyanJsonFree(jsonStr3);
	}

	{
		RyanJsonPrintPreallocated(jsonRoot, buf, bufLen / 15, RyanJsonTrue, NULL);
	}

	free(buf);
	RyanJsonDelete(jsonRoot);
	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonFuzzerTestMinify(const char *data, uint32_t size)
{
	char *buf = (char *)malloc(size + 100);
	memcpy(buf, data, size);
	memset(buf + size, 0, 100);

	uint32_t size2 = RyanJsonMinify(buf, (int32_t)size);
	// 非法情况
	{
		RyanJsonCheckReturnFalse(0 == RyanJsonMinify(NULL, 0));
		RyanJsonCheckReturnFalse(0 == RyanJsonMinify(NULL, 10));
		RyanJsonCheckReturnFalse(0 == RyanJsonMinify(NULL, -10));
		RyanJsonCheckReturnFalse(0 == RyanJsonMinify(buf, -10));
	}

	// 内存泄漏就是上面出错了
	RyanJson_t pJson2 = RyanJsonParseOptions(buf, size2, size % 2 ? RyanJsonTrue : RyanJsonFalse, NULL);
	free(buf);
	if (NULL != pJson2)
	{
		uint32_t len = 0;
		char *jsonStr = RyanJsonPrint(pJson2, 100, RyanJsonFalse, &len); // 以带格式方式将数据打印出来
		RyanJsonCheckCode(NULL != jsonStr && len > 0, {
			RyanJsonDelete(pJson2);
			return RyanJsonFalse;
		});
		RyanJsonFree(jsonStr);
		RyanJsonDelete(pJson2);
	}
	else
	{
		return RyanJsonFalse;
	}

	return RyanJsonTrue;
}
