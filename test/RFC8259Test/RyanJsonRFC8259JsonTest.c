#include "RyanJsonTest.h"
#include "valloc.h"

#define PrintfStrCmpEnable
#define TEST_FILE_PATH "./test/RFC8259JsonData"

typedef RyanJsonBool_e (*jsonParseData)(char *fileName, char *data, uint32_t len);

/* Read a file, parse, render back, etc. */
static RyanJsonBool_e testFile(const char *path, jsonParseData jsonParseDataHandle)
{
	DIR *dir = opendir(path);
	RyanJsonCheckReturnFalse(NULL != dir);

	struct dirent *entry = NULL;
	uint32_t count = 0;
	uint32_t used_count = 0;

	// 初始缓冲区
	uint32_t bufferCap = 4096;
	char *data = (char *)malloc(bufferCap);
	if (NULL == data)
	{
		(void)closedir(dir);
		return RyanJsonFalse;
	}

	while (NULL != (entry = readdir(dir))) // NOLINT(concurrency-mt-unsafe)
	{
		char *name = (char *)entry->d_name;
		if (NULL == name || 0 == strlen(name)) { continue; }
		if (0 == strcmp(name, ".") || 0 == strcmp(name, "..")) { continue; }

		char fullPath[512] = {0};
		int ret = snprintf(fullPath, sizeof(fullPath), "%s/%s", path, name);
		if (ret < 0 || ret >= (int)sizeof(fullPath)) { continue; }

		FILE *f = fopen(fullPath, "rb");
		if (NULL == f)
		{
			(void)printf("打开文件失败: %s\n", fullPath);
			continue;
		}

		if (0 != fseek(f, 0, SEEK_END))
		{
			(void)fclose(f);
			continue;
		}

		long fileSize = ftell(f);
		if (fileSize < 0)
		{
			(void)fclose(f);
			continue;
		}
		uint32_t len = (uint32_t)fileSize;

		if (0 != fseek(f, 0, SEEK_SET))
		{
			(void)fclose(f);
			continue;
		}

		// 必要时自动扩容
		if (len + 1 > bufferCap)
		{
			bufferCap = len + 128; // 预留一点空间
			char *newData = (char *)realloc(data, bufferCap);
			if (NULL == newData)
			{
				(void)fclose(f);
				break;
			}
			data = newData;
		}

		if (len != fread(data, 1, len, f))
		{
			(void)fclose(f);
			continue;
		}
		data[len] = '\0';
		(void)fclose(f);

		int32_t startUse = vallocGetUseByTlsf();
		RyanJsonBool_e status = jsonParseDataHandle(name, data, len);
		used_count++;

		// 判定逻辑优化
		if (0 == strncmp("y_", name, 2))
		{
			if (RyanJsonTrue == status) { count++; }
			else
			{
				(void)printf("应该成功，但是失败: %s, len: %u\n", data, len);
			}
		}
		else if (0 == strncmp("n_", name, 2))
		{
			if (RyanJsonFalse == status) { count++; }
			else
			{
				(void)printf("应该失败，但是成功: %s, len: %u\n", data, len);
			}
		}
		else if (0 == strncmp("i_", name, 2)) { count++; }

		if (startUse != vallocGetUseByTlsf())
		{
			int area = 0, use = 0;
			v_mcheck(&area, &use);
			(void)printf("内存泄漏 %s len: %u\r\n", data, len);
			(void)printf("|||----------->>> area = %d, size = %d\r\n", area, use);
			displayMem();
			break;
		}
	}

	free(data);
	(void)closedir(dir);

	(void)printf("RFC 8259 JSON: (%u/%u)\r\n", count, used_count);
	return RyanJsonTrue;
}

#include "RyanJsonRFC8259TestUtil.h"

static void checkJsonSemanticEquality(char *data, uint32_t len, char *str, uint32_t strLen, uint32_t *errorCount)
{
	if (0 != strcmp(data, str))
	{
		if (!RyanJsonValueSemanticEqual(data, len, str, strLen))
		{
			(*errorCount)++;
			(void)printf("%d 数据不完全一致 -- 原始: %s -- 序列化: %s\n", *errorCount, data, str);
		}
	}
}

/**
 * @brief RyanJson 测试程序
 *
 * @param fileName
 * @param data
 * @param len
 * @return int
 */
static RyanJsonBool_e RyanJsonParseData(char *fileName, char *data, uint32_t len)
{

	if (0 == strcmp(fileName, "n_structure_100000_opening_arrays.json") || 0 == strcmp(fileName, "n_structure_open_array_object.json"))
	{
		return RyanJsonFalse;
	}
	// printf("开始解析: %s\r\n", fileName);
	RyanJson_t json = RyanJsonParseOptions(data, len, RyanJsonTrue, NULL);
	RyanJsonCheckReturnFalse(NULL != json);

#ifdef PrintfStrCmpEnable
	int32_t strLen = 0;
	char *str = RyanJsonPrint(json, 60, RyanJsonFalse, &strLen);
	if (NULL == str)
	{
		printf("反序列化失败: [%s]\n", data);
		goto err;
	}

	RyanJsonMinify(data, (int32_t)len);
	static uint32_t alksdjfCOunt = 0;
	checkJsonSemanticEquality(data, len, str, strLen, &alksdjfCOunt);

	RyanJsonFree(str);
#endif

	(void)RyanJsonDelete(json);
	return RyanJsonTrue;

err:
	(void)RyanJsonDelete(json);
	return RyanJsonFalse;
}

/**
 * @brief cJson测试程序
 *
 * @param fileName
 * @param data
 * @param len
 * @return int
 */
static RyanJsonBool_e cJSONParseData(char *fileName, char *data, uint32_t len)
{

	if (0 == strcmp(fileName, "n_structure_100000_opening_arrays.json") || 0 == strcmp(fileName, "n_structure_open_array_object.json"))
	{
		return RyanJsonFalse;
	}

	cJSON *json = cJSON_ParseWithLengthOpts(data, len + sizeof(""), NULL, RyanJsonTrue);
	RyanJsonCheckReturnFalse(NULL != json);

#ifdef PrintfStrCmpEnable
	char *str = cJSON_PrintBuffered(json, 60, RyanJsonFalse);
	if (NULL == str)
	{
		printf("反序列化失败: [%s]\n", data);
		goto err;
	}

	cJSON_Minify(data);
	static uint32_t alksdjfCOunt = 0;
	checkJsonSemanticEquality(data, len, str, strlen(str), &alksdjfCOunt);

	cJSON_free(str);
#endif

	(void)cJSON_Delete(json);
	return RyanJsonTrue;
err:
	(void)cJSON_Delete(json);
	return RyanJsonFalse;
}

/**
 * @brief cJson测试程序
 *
 * @param fileName
 * @param data
 * @param len
 * @return int
 */
static RyanJsonBool_e yyjsonParseData(char *fileName, char *data, uint32_t len)
{
	if (0 == strcmp(fileName, "n_structure_100000_opening_arrays.json") || 0 == strcmp(fileName, "n_structure_open_array_object.json"))
	{
		return RyanJsonFalse;
	}

	yyjson_doc *doc = yyjson_read(data, len, 0);
	RyanJsonCheckReturnFalse(NULL != doc);

#ifdef PrintfStrCmpEnable
	char *str = yyjson_write(doc, 0, NULL);
	if (NULL == str)
	{
		printf("反序列化失败: [%s]\n", data);
		goto err;
	}

	cJSON_Minify(data);
	static uint32_t alksdjfCOunt = 0;
	checkJsonSemanticEquality(data, len, str, strlen(str), &alksdjfCOunt);

	free(str);
#endif

	(void)yyjson_doc_free(doc);
	return RyanJsonTrue;
err:
	(void)yyjson_doc_free(doc);
	return RyanJsonFalse;
}

// RFC 8259 JSON Test Suite
// https://github.com/nst/JSONTestSuite
static RyanJsonBool_e testRFC8259RyanJson(void) { return testFile(TEST_FILE_PATH, RyanJsonParseData); }

static RyanJsonBool_e testRFC8259cJSON(void) { return testFile(TEST_FILE_PATH, cJSONParseData); }

static RyanJsonBool_e testRFC8259yyjson(void) { return testFile(TEST_FILE_PATH, yyjsonParseData); }

RyanJsonBool_e RFC8259JsonTest(void)
{
	int32_t result = 0;
	uint32_t testRunCount = 0;
	uint64_t funcStartMs;

	cJSON_Hooks hooks = {.malloc_fn = v_malloc_tlsf, .free_fn = v_free_tlsf};
	(void)cJSON_InitHooks(&hooks);

	runTestWithLogAndTimer(testRFC8259RyanJson);
	runTestWithLogAndTimer(testRFC8259yyjson);
	runTestWithLogAndTimer(testRFC8259cJSON);

	return RyanJsonTrue;
}
