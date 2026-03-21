#include "testCommon.h"
#include "valloc.h"
#include <string.h>

#include "rfc8259Embedded.h"

#define PrintfStrCmpEnable

#define RFC8259TotalCaseCount             319U
#define RFC8259RyanJsonPassCountStrict    317U
#define RFC8259RyanJsonPassCountNonStrict 319U

typedef RyanJsonBool_e (*rfc8259ParseFn_t)(const char *fileName, char *data, uint32_t len);

static uint32_t resolveMinifiedLen(const char *data, uint32_t len)
{
	if (NULL == data || 0U == len) { return 0U; }
	const char *nulPos = (const char *)memchr(data, '\0', len);
	if (NULL != nulPos) { return (uint32_t)(nulPos - data); }
	return len;
}

/* Parse, render back, compare semantics, etc. */
static void testFile(rfc8259ParseFn_t parseFn, const char *libName, uint32_t *passCountOut,
		     uint32_t *totalCountOut)
{
	uint32_t passCount = 0U;
	uint32_t usedCount = 0U;
	uint32_t fileCount = gRfc8259EmbeddedFileCount;

	if (0U == fileCount)
	{
		TEST_FAIL_MESSAGE("RFC8259 内嵌数据为空");
		return;
	}

	// 初始缓冲区
	uint32_t bufferCap = 4096U;
	char *data = (char *)malloc(bufferCap);
	if (NULL == data)
	{
		TEST_FAIL_MESSAGE("内存分配失败 (RFC8259 数据缓冲区)");
		return;
	}

	for (uint32_t i = 0U; i < fileCount; ++i)
	{
		const char *name = gRfc8259EmbeddedFiles[i].name;
		if (NULL == name || 0 == strlen(name)) { continue; }

		const unsigned char *embedded = gRfc8259EmbeddedFiles[i].data;
		uint32_t len = gRfc8259EmbeddedFiles[i].len;
		if (NULL == embedded) { continue; }

		if (len + 1 > bufferCap)
		{
			bufferCap = len + 128; // 预留一点空间
			char *newData = (char *)realloc(data, bufferCap);
			if (NULL == newData) { break; }
			data = newData;
		}
		memcpy(data, embedded, len);
		data[len] = '\0';

		unityTestLeakScope_t leakScope = unityTestLeakScopeBegin();
		RyanJsonBool_e status = parseFn(name, data, len);
		usedCount++;

		// 判定逻辑
		if (0 == strncmp("y_", name, 2))
		{
			if (RyanJsonTrue == status) { passCount++; }
			else
			{
				(void)testLog("[RFC8259][%s] 期望成功但失败: %s, 内容: %s\n", libName, name, data);
				(void)testLog("[RFC8259][FAIL] lib=%s file=%s expect=success actual=fail\n", libName, name);
			}
		}
		else if (0 == strncmp("n_", name, 2))
		{
			if (RyanJsonFalse == status) { passCount++; }
			else
			{
				(void)testLog("[RFC8259][%s] 期望失败但成功: %s, 内容: %s\n", libName, name, data);
				(void)testLog("[RFC8259][FAIL] lib=%s file=%s expect=fail actual=success\n", libName, name);
			}
		}
		else if (0 == strncmp("i_", name, 2))
		{
			passCount++;
		}

		// 内存泄漏检查
		if (leakScope.baseline != unityTestGetUse())
		{
			(void)testLog("[RFC8259][%s] 内存泄漏于文件: %s\n", libName, name);
			(void)testLog("[RFC8259][LEAK] lib=%s file=%s\n", libName, name);
		}
		unityTestLeakScopeEnd(leakScope, "RFC8259 内存泄漏");
	}

	free(data);

	if (NULL != passCountOut) { *passCountOut = passCount; }
	if (NULL != totalCountOut) { *totalCountOut = usedCount; }

	(void)testLog("\033[1;34m[\033[1;36m%s\033[1;34m] RFC 8259 Json 统计: \033[1;32m(%u/%u)\033[0m\r\n\r\n", libName, passCount,
		      usedCount);
	(void)testLog("[RFC8259][STATS] lib=%s pass=%u total=%u\n", libName, passCount, usedCount);
}

#include "testRfc8259Util.h"

static void checkJsonSemanticEquality(const char *libName, const char *fileName, char *data, uint32_t len, char *str,
				      uint32_t strLen, uint32_t *errorCount)
{
	if (0 != strcmp(data, str))
	{
		if (!RyanJsonValueSemanticEqual(data, len, str, strLen))
		{
			(*errorCount)++;
			(void)testLog("[RFC8259][DIFF] lib=%s file=%s\n", libName, (NULL != fileName) ? fileName : "<null>");
			(void)testLog("[RFC8259][%s] %d 数据不完全一致: %s -- 原始: %s -- 序列化: %s\n", libName, *errorCount,
				      (NULL != fileName) ? fileName : "<null>", data, str);
		}
	}
}

static RyanJsonBool_e RyanJsonParseData(const char *fileName, char *data, uint32_t len)
{
	RyanJson_t json = RyanJsonParseOptions(data, len, RyanJsonTrue, NULL);
	if (NULL == json) { return RyanJsonFalse; }

#ifdef PrintfStrCmpEnable
	int32_t strLen = 0;
	char *str = RyanJsonPrint(json, 60, RyanJsonFalse, &strLen);
	if (NULL == str)
	{
		(void)RyanJsonDelete(json);
		return RyanJsonFalse;
	}

	RyanJsonMinify(data, (int32_t)len);
	static uint32_t semanticErrorCount = 0;
	uint32_t minLen = resolveMinifiedLen(data, len);
	checkJsonSemanticEquality("RyanJson", fileName, data, minLen, str, strLen, &semanticErrorCount);

	RyanJsonFree(str);
#endif

	(void)RyanJsonDelete(json);
	return RyanJsonTrue;
}

static RyanJsonBool_e cJSONParseData(const char *fileName, char *data, uint32_t len)
{
	cJSON *json = cJSON_ParseWithLengthOpts(data, len + sizeof(""), NULL, RyanJsonTrue);
	if (NULL == json) { return RyanJsonFalse; }

#ifdef PrintfStrCmpEnable
	char *str = cJSON_PrintBuffered(json, 60, RyanJsonFalse);
	if (NULL == str)
	{
		(void)cJSON_Delete(json);
		return RyanJsonFalse;
	}

	cJSON_Minify(data);
	static uint32_t semanticErrorCount = 0;
	uint32_t minLen = resolveMinifiedLen(data, len);
	checkJsonSemanticEquality("cJSON", fileName, data, minLen, str, strlen(str), &semanticErrorCount);

	cJSON_free(str);
#endif

	(void)cJSON_Delete(json);
	return RyanJsonTrue;
}

static RyanJsonBool_e yyjsonParseData(const char *fileName, char *data, uint32_t len)
{
	yyjson_doc *doc = yyjson_read(data, len, 0);
	if (NULL == doc) { return RyanJsonFalse; }

#ifdef PrintfStrCmpEnable
	char *str = yyjson_write(doc, 0, NULL);
	if (NULL == str)
	{
		(void)yyjson_doc_free(doc);
		return RyanJsonFalse;
	}

	cJSON_Minify(data);
	static uint32_t semanticErrorCount = 0;
	uint32_t minLen = resolveMinifiedLen(data, len);
	checkJsonSemanticEquality("yyjson", fileName, data, minLen, str, strlen(str), &semanticErrorCount);

	free(str);
#endif

	(void)yyjson_doc_free(doc);
	return RyanJsonTrue;
}

static void testRfc8259RyanJson(void)
{
	cJSON_Hooks hooks = {.malloc_fn = unityTestMalloc, .free_fn = unityTestFree};
	(void)cJSON_InitHooks(&hooks);

	uint32_t passCount = 0;
	uint32_t totalCount = 0;
	testFile(RyanJsonParseData, "RyanJson", &passCount, &totalCount);

	char totalMsg[96] = {0};
	(void)snprintf(totalMsg, sizeof(totalMsg), "RFC8259 总用例数应为 %u", (unsigned)RFC8259TotalCaseCount);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(RFC8259TotalCaseCount, totalCount, totalMsg);

	uint32_t expectedPassCount =
		(true == RyanJsonStrictObjectKeyCheck) ? RFC8259RyanJsonPassCountStrict : RFC8259RyanJsonPassCountNonStrict;
	char expectedMsg[128] = {0};
	(void)snprintf(expectedMsg, sizeof(expectedMsg),
		       "RyanJson RFC8259 通过数不符合预期（非严格=%u，严格=%u）",
		       (unsigned)RFC8259RyanJsonPassCountNonStrict, (unsigned)RFC8259RyanJsonPassCountStrict);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expectedPassCount, passCount, expectedMsg);
}

static void testRfc8259Yyjson(void)
{
	testFile(yyjsonParseData, "yyjson", NULL, NULL);
}

static void testRfc8259Cjson(void)
{
	testFile(cJSONParseData, "cJSON", NULL, NULL);
}

void testRfc8259Runner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testRfc8259RyanJson);
	RUN_TEST(testRfc8259Yyjson);
	RUN_TEST(testRfc8259Cjson);
}
