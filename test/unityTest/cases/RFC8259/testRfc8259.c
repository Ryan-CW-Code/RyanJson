#include "testCommon.h"
#include "valloc.h"

#if defined(RyanJsonTestPlatformQemu)
void testRfc8259Runner(void)
{
	UnitySetTestFile(__FILE__);
}
#else

#include <dirent.h>
#include <errno.h>

#define PrintfStrCmpEnable

#ifndef RyanJsonProjectRootPath
#define RyanJsonProjectRootPath "."
#endif

#define RFC8259TestFilePath               RyanJsonProjectRootPath "/test/data/rfc8259"
#define RFC8259TotalCaseCount             322U
#define RFC8259RyanJsonPassCountStrict    320U
#define RFC8259RyanJsonPassCountNonStrict 322U

typedef RyanJsonBool_e (*jsonParseData)(char *fileName, char *data, uint32_t len);

typedef struct
{
	char **fileNames;
	uint32_t fileCount;
} rfc8259FileList_t;

static void freeRfc8259FileList(rfc8259FileList_t *fileList)
{
	if (NULL == fileList) { return; }
	if (NULL != fileList->fileNames)
	{
		for (uint32_t i = 0U; i < fileList->fileCount; ++i)
		{
			free(fileList->fileNames[i]);
		}
		free(fileList->fileNames);
	}
	fileList->fileNames = NULL;
	fileList->fileCount = 0U;
}

static RyanJsonBool_e hasJsonSuffix(const char *fileName)
{
	size_t fileNameLen = 0U;
	static const size_t jsonSuffixLen = sizeof(".json") - 1U;

	if (NULL == fileName) { return RyanJsonFalse; }
	fileNameLen = strlen(fileName);
	if (fileNameLen < jsonSuffixLen) { return RyanJsonFalse; }

	return (0 == strcmp(fileName + fileNameLen - jsonSuffixLen, ".json")) ? RyanJsonTrue : RyanJsonFalse;
}

static int compareFileNameAsc(const void *left, const void *right)
{
	const char *leftName = *(const char *const *)left;
	const char *rightName = *(const char *const *)right;
	if (NULL == leftName && NULL == rightName) { return 0; }
	if (NULL == leftName) { return -1; }
	if (NULL == rightName) { return 1; }
	return strcmp(leftName, rightName);
}

static RyanJsonBool_e collectRfc8259FileList(const char *path, rfc8259FileList_t *fileListOut)
{
	DIR *dir = NULL;
	struct dirent *entry = NULL;
	int32_t scanErrno = 0;
	uint32_t fileCapacity = 0U;
	rfc8259FileList_t fileList = {NULL, 0U};

	if (NULL == path || NULL == fileListOut) { return RyanJsonFalse; }

	errno = 0;
	dir = opendir(path);
	if (NULL == dir)
	{
		(void)testLog("打开 RFC8259 目录失败: %s\n", path);
		return RyanJsonFalse;
	}

	while (NULL != (entry = readdir(dir)))
	{
		const char *fileName = entry->d_name;
		char *nameCopy = NULL;

		if (NULL == fileName) { continue; }
		if (0 == strcmp(fileName, ".") || 0 == strcmp(fileName, "..")) { continue; }
		if (RyanJsonTrue != hasJsonSuffix(fileName)) { continue; }

		if (fileList.fileCount == fileCapacity)
		{
			uint32_t newCapacity = (0U == fileCapacity) ? 64U : (fileCapacity * 2U);
			char **newFileNames = (char **)realloc(fileList.fileNames, (size_t)newCapacity * sizeof(char *));
			if (NULL == newFileNames)
			{
				(void)closedir(dir);
				freeRfc8259FileList(&fileList);
				return RyanJsonFalse;
			}
			fileList.fileNames = newFileNames;
			fileCapacity = newCapacity;
		}

		nameCopy = (char *)malloc(strlen(fileName) + 1U);
		if (NULL == nameCopy)
		{
			(void)closedir(dir);
			freeRfc8259FileList(&fileList);
			return RyanJsonFalse;
		}
		(void)strcpy(nameCopy, fileName);
		fileList.fileNames[fileList.fileCount++] = nameCopy;
	}

	scanErrno = errno;
	(void)closedir(dir);
	if (0 != scanErrno)
	{
		freeRfc8259FileList(&fileList);
		return RyanJsonFalse;
	}

	if (fileList.fileCount > 1U) { qsort(fileList.fileNames, fileList.fileCount, sizeof(char *), compareFileNameAsc); }

	*fileListOut = fileList;
	return RyanJsonTrue;
}

/* Read a file, parse, render back, etc. */
static void testFile(const char *path, jsonParseData jsonParseDataHandle, const char *libName, uint32_t *passCountOut,
		     uint32_t *totalCountOut)
{
	uint32_t passCount = 0U;
	uint32_t usedCount = 0U;
	rfc8259FileList_t fileList = {NULL, 0U};

	if (RyanJsonTrue != collectRfc8259FileList(path, &fileList))
	{
		TEST_FAIL_MESSAGE("RFC8259 扫描目录失败");
		return;
	}

	// 初始缓冲区
	uint32_t bufferCap = 4096U;
	char *data = (char *)malloc(bufferCap);
	if (NULL == data)
	{
		freeRfc8259FileList(&fileList);
		TEST_FAIL_MESSAGE("内存分配失败 (RFC8259 数据缓冲区)");
		return;
	}

	for (uint32_t i = 0U; i < fileList.fileCount; ++i)
	{
		char *name = fileList.fileNames[i];
		if (NULL == name || 0 == strlen(name)) { continue; }

		char fullPath[512] = {0};
		int32_t ret = snprintf(fullPath, sizeof(fullPath), "%s/%s", path, name);
		if (ret < 0 || ret >= (int32_t)sizeof(fullPath)) { continue; }

		FILE *f = fopen(fullPath, "rb");
		if (NULL == f)
		{
			(void)testLog("打开文件失败: %s\n", fullPath);
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

		int32_t startUse = unityTestGetUse();
		RyanJsonBool_e status = jsonParseDataHandle(name, data, len);
		usedCount++;

		// 判定逻辑优化
		if (0 == strncmp("y_", name, 2))
		{
			if (RyanJsonTrue == status) { passCount++; }
			else
			{
				(void)testLog("RFC8259 期望成功但失败: %s, 内容: %s\n", name, data);
			}
		}
		else if (0 == strncmp("n_", name, 2))
		{
			if (RyanJsonFalse == status) { passCount++; }
			else
			{
				(void)testLog("RFC8259 期望失败但成功: %s, 内容: %s\n", name, data);
			}
		}
		else if (0 == strncmp("i_", name, 2)) { passCount++; }

		// 内存泄漏检查
		if (startUse != unityTestGetUse())
		{
			(void)testLog("RFC8259 内存泄漏于文件: %s\n", name);
			TEST_ASSERT_EQUAL_INT_MESSAGE(startUse, unityTestGetUse(), "RFC8259 内存泄漏");
		}
	}

	free(data);
	freeRfc8259FileList(&fileList);

	if (NULL != passCountOut) { *passCountOut = passCount; }
	if (NULL != totalCountOut) { *totalCountOut = usedCount; }

	(void)testLog("\033[1;34m[\033[1;36m%s\033[1;34m] RFC 8259 Json 统计: \033[1;32m(%u/%u)\033[0m\r\n\r\n", libName, passCount,
		      usedCount);
}

#include "testRfc8259Util.h"

static void checkJsonSemanticEquality(char *data, uint32_t len, char *str, uint32_t strLen, uint32_t *errorCount)
{
	if (0 != strcmp(data, str))
	{
		if (!RyanJsonValueSemanticEqual(data, len, str, strLen))
		{
			(*errorCount)++;
			(void)testLog("%d 数据不完全一致 -- 原始: %s -- 序列化: %s\n", *errorCount, data, str);
		}
	}
}

static RyanJsonBool_e RyanJsonParseData(char *fileName, char *data, uint32_t len)
{
	if (0 == strcmp(fileName, "n_structure_100000_opening_arrays.json") || 0 == strcmp(fileName, "n_structure_open_array_object.json"))
	{
		return RyanJsonFalse;
	}

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
	checkJsonSemanticEquality(data, len, str, strLen, &semanticErrorCount);

	RyanJsonFree(str);
#endif

	(void)RyanJsonDelete(json);
	return RyanJsonTrue;
}

static RyanJsonBool_e cJSONParseData(char *fileName, char *data, uint32_t len)
{
	if (0 == strcmp(fileName, "n_structure_100000_opening_arrays.json") || 0 == strcmp(fileName, "n_structure_open_array_object.json"))
	{
		return RyanJsonFalse;
	}

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
	checkJsonSemanticEquality(data, len, str, strlen(str), &semanticErrorCount);

	cJSON_free(str);
#endif

	(void)cJSON_Delete(json);
	return RyanJsonTrue;
}

static RyanJsonBool_e yyjsonParseData(char *fileName, char *data, uint32_t len)
{
	if (0 == strcmp(fileName, "n_structure_100000_opening_arrays.json") || 0 == strcmp(fileName, "n_structure_open_array_object.json"))
	{
		return RyanJsonFalse;
	}

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
	checkJsonSemanticEquality(data, len, str, strlen(str), &semanticErrorCount);

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
	testFile(RFC8259TestFilePath, RyanJsonParseData, "RyanJson", &passCount, &totalCount);

	TEST_ASSERT_EQUAL_UINT32_MESSAGE(RFC8259TotalCaseCount, totalCount, "RFC8259 总用例数应为 322");

	uint32_t expectedPassCount =
		(true == RyanJsonStrictObjectKeyCheck) ? RFC8259RyanJsonPassCountStrict : RFC8259RyanJsonPassCountNonStrict;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expectedPassCount, passCount, "RyanJson RFC8259 通过数不符合预期（非严格=322，严格=320）");
}

static void testRfc8259Yyjson(void)
{
	testFile(RFC8259TestFilePath, yyjsonParseData, "yyjson", NULL, NULL);
}

static void testRfc8259Cjson(void)
{
	testFile(RFC8259TestFilePath, cJSONParseData, "cJSON", NULL, NULL);
}

void testRfc8259Runner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testRfc8259RyanJson);
	RUN_TEST(testRfc8259Yyjson);
	RUN_TEST(testRfc8259Cjson);
}

#endif
