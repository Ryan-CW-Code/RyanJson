#include "RyanJsonTest.h"
#include <signal.h>

// cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
// ./build/linux/x86/release/RyanJson -dict=./test/fuzzer/RyanJsonFuzzer.dict
// ./build/linux/x86/release/RyanJson -dict=./test/fuzzer/RyanJsonFuzzer.dict -timeout=10 -runs=999999 -max_len=8192 -print_final_stats=0
// -verbosity=0 test/fuzzer/input
// ./build/linux/x86/release/RyanJson -dict=./test/fuzzer/RyanJsonFuzzer.dict -timeout=10 -runs=99999 -verbosity=0 -max_len=8192

// llvm-profdata merge -sparse default.profraw -o default.profdata
// 文本报告
// llvm-cov report ./build/linux/x86/release/RyanJson -instr-profile=default.profdata
// html报告
// llvm-cov show ./build/linux/x86/release/RyanJson -instr-profile=default.profdata -format=html -output-dir=coverage_report
// llvm-cov show ./build/linux/x86/release/RyanJson \
//   -instr-profile=default.profdata \
//   -format=html \
//   -output-dir=coverage_report \
//   -compilation-dir=/home/ryan/linux/appDev/RyanJson \
//   -path-equivalence=/home/ryan/linux/appDev/RyanJson,./RyanJson

// llvm-cov show ./build/linux/x86/release/RyanJson -instr-profile=default.profdata -format=html -output-dir=coverage_report
// llvm-cov show ./build/linux/x86/release/RyanJson -instr-profile=default.profdata -name=RyanJson -format=html > coverage_report.html
// llvm-cov show ./build/linux/x86/release/RyanJson -instr-profile=default.profdata -name=FuzzMe

// RyanJsonBool_e RyanJson
#define RyanJsonCheckGotoExit(EX)                                                                                                          \
	RyanJsonCheckCode(EX, {                                                                                                            \
		result = RyanJsonFalse;                                                                                                    \
		goto __exit;                                                                                                               \
	})
RyanJsonBool_e isadfa = RyanJsonTrue;

static void RyanPrintJsonToLog22(RyanJson_t pJson)
{
	int32_t strLen = 0;
	char *jsonStr = RyanJsonPrint(pJson, 512, RyanJsonFalse, &strLen);
	if (NULL == jsonStr) { return; }

	for (int32_t i = 0; i < strLen; i++)
	{
		int32_t aaa = strLen - (i * 100);
		if (aaa > 100) { printf("%.*s", 100, jsonStr + (i * 100)); }
		else
		{
			if (aaa) { printf("%.*s", aaa, jsonStr + (i * 100)); }
			printf("\r\n");
			break;
		}
	}
	RyanJsonFree(jsonStr);
}

static RyanJsonBool_e RyanJsonFuzzerTestByParseAndPrint(RyanJson_t pJson, const char *data, int32_t size)
{

	int32_t len = 0;
	char *jsonStr = RyanJsonPrint(pJson, 100, size % 2 ? RyanJsonFalse : RyanJsonTrue, &len); // 以带格式方式将数据打印出来
	// printf("len222222222222222222: %d\r\n", len);
	RyanJsonCheckReturnFlase(NULL != jsonStr && len > 0);
	RyanJsonFree(jsonStr);

	{
		int32_t bufLen = len * 3;
		if (bufLen < 8192) { bufLen = 8192; }
		char *buf = malloc((size_t)bufLen);

		int32_t len2 = 0;
		char *jsonStr2 = RyanJsonPrintPreallocated(pJson, buf, bufLen, size % 2 ? RyanJsonFalse : RyanJsonTrue, &len2);
		// printf("len: %d, len2: %d, str: %s\r\n", len, len2, NULL == jsonStr2 ? "NULL" : jsonStr2);
		if (buf)
		{
			RyanJsonCheckCode(NULL != jsonStr2 && len == len2, {
				free(buf);
				return RyanJsonFalse;
			});

			free(buf);
		}
	}

	{
		int32_t bufLen = size * 3;
		char *buf = malloc((size_t)bufLen);
		if (buf)
		{
			memset(buf, 0, (size_t)bufLen);
			memcpy(buf, data, (size_t)size);
			RyanJson_t jsonRoot = RyanJsonParse(buf);
			RyanJsonCheckCode(NULL != jsonRoot, {
				free(buf);
				return RyanJsonFalse;
			});
			free(buf);

			int32_t len3 = 0;
			char *jsonStr3 =
				RyanJsonPrint(jsonRoot, 100, size % 2 ? RyanJsonFalse : RyanJsonTrue, &len3); // 以带格式方式将数据打印出来
			// printf("len222222222222222222: %d\r\n", len);
			RyanJsonCheckCode(NULL != jsonStr3 && len == len3, {
				if (jsonStr3) { RyanJsonFree(jsonStr3); }
				RyanJsonDelete(jsonRoot);
				return RyanJsonFalse;
			});

			RyanJsonFree(jsonStr3);

			RyanJsonDelete(jsonRoot);
		}
	}

	return RyanJsonTrue;
}

static RyanJsonBool_e RyanJsonFuzzerTestByDup(RyanJson_t pJson)
{
	RyanJsonBool_e result = RyanJsonTrue;
	char *jsonStr = NULL;
	char *jsonStrDup = NULL;
	RyanJson_t pJsonDup = NULL;

	// 测试打印和复制功能
	int32_t len = 0;

	jsonStr = RyanJsonPrint(pJson, 100, RyanJsonFalse, &len); // 以带格式方式将数据打印出来
	RyanJsonCheckGotoExit(NULL != jsonStr && len > 0);

	pJsonDup = RyanJsonDuplicate(pJson);
	RyanJsonCheckGotoExit(NULL != pJsonDup);

	// 测试dup失败情况
	RyanJsonCheckGotoExit(NULL == RyanJsonDuplicate(NULL));

	// 判断复制json的size是否一致
	RyanJsonCheckGotoExit(0 == RyanJsonGetSize(NULL));
	RyanJsonCheckGotoExit(RyanJsonGetSize(pJson) == RyanJsonGetSize(pJsonDup));
	RyanJsonCompare(pJson, pJsonDup);
	RyanJsonCompareOnlyKey(pJson, pJsonDup);
	// assert(RyanJsonTrue == RyanJsonCompare(pJson, pJsonDup)); // 大浮点数判断容易出错
	// RyanJsonCheckGotoExit(RyanJsonTrue == RyanJsonCompareOnlyKey(pJson, pJsonDup)); // 重复key也会失败

	// 测试compare特殊情况
	RyanJsonCheckGotoExit(RyanJsonTrue == RyanJsonCompare(pJson, pJson));
	RyanJsonCheckGotoExit(RyanJsonFalse == RyanJsonCompare(NULL, pJsonDup));
	RyanJsonCheckGotoExit(RyanJsonFalse == RyanJsonCompare(pJson, NULL));
	RyanJsonCheckGotoExit(RyanJsonFalse == RyanJsonCompare(NULL, NULL));

	// 测试compareKey特殊情况
	RyanJsonCheckGotoExit(RyanJsonTrue == RyanJsonCompareOnlyKey(pJson, pJson));
	RyanJsonCheckGotoExit(RyanJsonFalse == RyanJsonCompareOnlyKey(NULL, pJsonDup));
	RyanJsonCheckGotoExit(RyanJsonFalse == RyanJsonCompareOnlyKey(pJson, NULL));
	RyanJsonCheckGotoExit(RyanJsonFalse == RyanJsonCompareOnlyKey(NULL, NULL));

	int32_t dupLen = 0;
	jsonStrDup = RyanJsonPrint(pJsonDup, 100, RyanJsonFalse, &dupLen); // 以带格式方式将数据打印出来
	RyanJsonCheckGotoExit(NULL != jsonStrDup && dupLen > 0);

	RyanJsonCheckCode(len == dupLen && 0 == memcmp(jsonStr, jsonStrDup, (size_t)len), {
		printf("len:%d, dupLen:%d\r\n", len, dupLen);
		printf("jsonStr:%s, jsonStrDup:%s\r\n", jsonStr, jsonStrDup);
		RyanJsonCheckGotoExit(0);
	});

	if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
	{
		// 测试size不相等
		RyanJsonDelete(RyanJsonDetachByIndex(pJson, 0));

		if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
		{
			// 改变key
			RyanJson_t item;
			RyanJsonObjectForEach(pJson, item)
			{
				if (RyanJsonIsKey(item))
				{
					RyanJsonChangeKey(item, "key12231123");
					break;
				}
			}

			// 改变value
			RyanJsonObjectForEach(pJson, item)
			{
				if (RyanJsonIsBool(item))
				{
					RyanJsonChangeBoolValue(item, !RyanJsonGetBoolValue(item));
					break;
				}
			}

			// 改变obj的key
			RyanJsonObjectForEach(pJson, item)
			{
				if (RyanJsonIsKey(item) && RyanJsonIsObject(item))
				{
					RyanJsonChangeKey(item, "key12231123");
					break;
				}
			}
		}

		RyanJsonCompare(pJson, pJsonDup);
		RyanJsonCompareOnlyKey(pJson, pJsonDup);
	}
__exit:

	if (jsonStr)
	{
		RyanJsonFree(jsonStr);
		jsonStr = NULL;
	}
	if (pJsonDup)
	{
		RyanJsonDelete(pJsonDup);
		pJsonDup = NULL;
	}
	if (jsonStrDup)
	{
		RyanJsonFree(jsonStrDup);
		jsonStrDup = NULL;
	}

	return result;
}

static RyanJsonBool_e RyanJsonFuzzerTestByForEachChange(RyanJson_t pJson, int32_t size)
{
	RyanJsonIsNull(pJson);

	if (RyanJsonIsKey(pJson))
	{
		char *key = malloc(strlen(RyanJsonGetKey(pJson)) + 1);
		if (key)
		{

			memset(key, 0, strlen(RyanJsonGetKey(pJson)) + 1);
			memcpy(key, RyanJsonGetKey(pJson), strlen(RyanJsonGetKey(pJson)));

			RyanJsonChangeKey(pJson, "key");
			RyanJsonChangeKey(pJson, key);
			free(key);
		}
	}
	if (RyanJsonIsBool(pJson)) { RyanJsonChangeBoolValue(pJson, !RyanJsonGetBoolValue(pJson)); }
	if (RyanJsonIsNumber(pJson))
	{
		if (RyanJsonIsInt(pJson))
		{
			int32_t value = RyanJsonGetIntValue(pJson);
			RyanJsonChangeIntValue(pJson, size);
			RyanJsonChangeIntValue(pJson, value);
		}
		if (RyanJsonIsDouble(pJson))
		{
			double value = RyanJsonGetDoubleValue(pJson);
			RyanJsonChangeDoubleValue(pJson, size * 1.123456789);
			RyanJsonChangeDoubleValue(pJson, value);
		}
	}

	if (RyanJsonIsString(pJson))
	{
		char *value = malloc(strlen(RyanJsonGetStringValue(pJson)) + 1);
		if (value)
		{
			memset(value, 0, strlen(RyanJsonGetStringValue(pJson)) + 1);
			memcpy(value, RyanJsonGetStringValue(pJson), strlen(RyanJsonGetStringValue(pJson)));

			RyanJsonChangeStringValue(pJson, "hello world");
			RyanJsonChangeStringValue(pJson, value);

			free(value);
		}
	}

	if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
	{
		RyanJson_t item;
		if (RyanJsonIsArray(pJson))
		{
			RyanJsonArrayForEach(pJson, item)
			{ RyanJsonCheckReturnFlase(RyanJsonTrue == RyanJsonFuzzerTestByForEachChange(item, size)); }
		}
		else
		{
			RyanJsonObjectForEach(pJson, item)
			{ RyanJsonCheckReturnFlase(RyanJsonTrue == RyanJsonFuzzerTestByForEachChange(item, size)); }
		}
	}

	return RyanJsonTrue;
}

static RyanJsonBool_e RyanJsonFuzzerTestByForEachGet2(RyanJson_t lastJson, RyanJson_t pJson, int32_t index, int32_t size)
{
	RyanJsonIsNull(pJson);

	if (RyanJsonIsKey(pJson)) { RyanJsonGetObjectToKey(lastJson, RyanJsonGetKey(pJson)); }
	else
	{
		RyanJsonGetObjectToIndex(lastJson, index);
	}

	if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
	{
		RyanJson_t item;
		RyanJsonObjectForEach(pJson, item) { RyanJsonFuzzerTestByForEachGet2(pJson, item, index, size); }
	}

	return RyanJsonTrue;
}

static RyanJsonBool_e RyanJsonFuzzerTestByForEachGet(RyanJson_t pJson, int32_t size)
{
	if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
	{
		RyanJson_t item;
		int32_t index = 0;
		RyanJsonObjectForEach(pJson, item)
		{
			RyanJsonFuzzerTestByForEachGet2(pJson, item, index, size);
			index++;
		}
	}
	return RyanJsonTrue;
}

static RyanJson_t RyanJsonFuzzerTestalkdfjald(RyanJson_t pJson)
{
	static int32_t count = 0;
	static int32_t count2 = 0;
	count++;
	char *key = "true";
	if (count % 10 > 5) { key = NULL; }
	switch (count % 50)
	{
	case 0: return RyanJsonCreateArray();
	case 1: return RyanJsonCreateObject();
	case 2:
		count2++;
		if (0 == count2 % 10) { return RyanJsonDuplicate(pJson); }
	case 11:
	case 12:
	case 13: return RyanJsonCreateBool(key, RyanJsonTrue);
	case 20:
	case 21:
	case 22: return RyanJsonCreateInt(key, count);
	case 31:
	case 32:
	case 33: return RyanJsonCreateDouble(key, count * 1.123456789);

	default: return RyanJsonCreateString(key, "true");
	}
}

static RyanJsonBool_e RyanJsonFuzzerTestByForEachCreate(RyanJson_t pJson, int32_t size)
{
	// RyanJsonInsert的特殊情况
	RyanJsonCheckReturnFlase(RyanJsonFalse == RyanJsonInsert(NULL, INT32_MAX, RyanJsonCreateString("key", "string")));
	RyanJsonCheckReturnFlase(RyanJsonFalse == RyanJsonInsert(pJson, -10, RyanJsonCreateString("key", "string")));
	RyanJsonCheckReturnFlase(RyanJsonFalse == RyanJsonInsert(pJson, INT32_MAX, NULL));
	RyanJsonCheckReturnFlase(RyanJsonFalse == RyanJsonInsert(NULL, -10, NULL));

	char *key = "keyaaa";
	RyanJsonAddNullToObject(pJson, key);
	if (RyanJsonIsKey(pJson)) { key = RyanJsonGetKey(pJson); }
	if (RyanJsonIsBool(pJson)) { RyanJsonAddBoolToObject(pJson, key, RyanJsonGetBoolValue(pJson)); }
	if (RyanJsonIsNumber(pJson))
	{
		if (RyanJsonIsInt(pJson))
		{
			RyanJsonAddIntToObject(pJson, key, RyanJsonGetIntValue(pJson));
			int arrayInt[] = {RyanJsonGetIntValue(pJson), RyanJsonGetIntValue(pJson), RyanJsonGetIntValue(pJson),
					  RyanJsonGetIntValue(pJson), RyanJsonGetIntValue(pJson)};
			RyanJsonAddItemToObject(pJson, (size % 2) ? key : "arrayString",
						RyanJsonCreateIntArray(arrayInt, sizeof(arrayInt) / sizeof(arrayInt[0])));
		}
		if (RyanJsonIsDouble(pJson))
		{
			RyanJsonAddDoubleToObject(pJson, key, RyanJsonGetDoubleValue(pJson));
			double arrayDouble[] = {RyanJsonGetDoubleValue(pJson), RyanJsonGetDoubleValue(pJson), RyanJsonGetDoubleValue(pJson),
						RyanJsonGetDoubleValue(pJson), RyanJsonGetDoubleValue(pJson)};
			RyanJsonAddItemToObject(pJson, (size % 2) ? key : "arrayString",
						RyanJsonCreateDoubleArray(arrayDouble, sizeof(arrayDouble) / sizeof(arrayDouble[0])));
		}
	}

	if (RyanJsonIsString(pJson))
	{
		RyanJsonAddStringToObject(pJson, key, RyanJsonGetStringValue(pJson));
		const char *arrayString[] = {RyanJsonGetStringValue(pJson), RyanJsonGetStringValue(pJson), RyanJsonGetStringValue(pJson),
					     RyanJsonGetStringValue(pJson), RyanJsonGetStringValue(pJson)};
		RyanJsonAddItemToObject(pJson, (size % 2) ? key : "arrayString",
					RyanJsonCreateStringArray(arrayString, sizeof(arrayString) / sizeof(arrayString[0])));
	}

	if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
	{
		RyanJson_t item;

		RyanJsonObjectForEach(pJson, item) { RyanJsonFuzzerTestByForEachCreate(item, size); }

		RyanJson_t pJson2 = RyanJsonFuzzerTestalkdfjald(pJson);
		RyanJsonAddItemToObject(pJson, key, pJson2);

		if (RyanJsonIsArray(pJson))
		{
			RyanJsonAddNullToArray(pJson);
			RyanJsonAddBoolToArray(pJson, size % 2 ? RyanJsonTrue : RyanJsonFalse);
			RyanJsonAddItemToArray(pJson, RyanJsonFuzzerTestalkdfjald(RyanJsonGetArrayValue(pJson)));
		}
	}

	return RyanJsonTrue;
}

/**
 * @brief 测试 Json 的 Replace 功能（保护根节点）
 *
 * @param pJson 待测试的 Json 节点
 * @param size  用于计算 index 的模数
 * @param isFirst 是否为第一次调用（根节点）
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e RyanJsonFuzzerTestByForEachReplace(RyanJson_t pJson, int32_t size)
{
	// 只处理数组或对象
	if (!(RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))) { return RyanJsonTrue; }

	// 递归替换子节点
	RyanJson_t item = NULL;
	RyanJson_t LastItem = NULL;
	RyanJsonObjectForEach(pJson, item)
	{
		if (RyanJsonTrue != RyanJsonFuzzerTestByForEachReplace(item, size)) { return RyanJsonFalse; }
		LastItem = item;
	}

	// 只有非根节点才做替换

	// 按 key 替换（仅对象）
	// 不要动第一个节点
	if (RyanJsonIsObject(pJson))
	{
		if (LastItem && RyanJsonIsKey(LastItem))
		{
			RyanJson_t newNode = RyanJsonFuzzerTestalkdfjald(pJson);
			if (RyanJsonFalse == RyanJsonReplaceByKey(pJson, RyanJsonGetKey(LastItem), newNode))
			{
				if (newNode) { RyanJsonDelete(newNode); }
				return RyanJsonFalse;
			}
		}
	}

	// 按 index 替换
	{
		int32_t idx = RyanJsonGetSize(pJson) % size;
		RyanJson_t newNode = RyanJsonFuzzerTestalkdfjald(pJson);
		if (RyanJsonFalse == RyanJsonReplaceByIndex(pJson, (size % 25) ? idx : 0, newNode))
		{
			if (newNode) { RyanJsonDelete(newNode); }
			return RyanJsonFalse;
		}
	}

	return RyanJsonTrue;
}

/**
 * @brief 测试 Json 的 Detach 分离功能（保护根节点）
 *
 * @param pJson 待测试的 Json 节点
 * @param size  用于计算 index 的模数
 * @param isFirst 是否为第一次调用（根节点）
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e RyanJsonFuzzerTestByForEachDetach(RyanJson_t pJson, int32_t size)
{
	if (!(RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))) { return RyanJsonTrue; }

	// 递归遍历子节点
	RyanJson_t item = NULL;
	RyanJson_t LastItem = NULL;
	RyanJsonObjectForEach(pJson, item)
	{
		RyanJsonFuzzerTestByForEachDetach(item, size);
		LastItem = item;
	}

	// 只有非根节点才做 detach

	// 按 key 分离（仅对象）
	if (RyanJsonIsObject(pJson))
	{
		if (LastItem && RyanJsonIsKey(LastItem))
		{
			RyanJson_t detached = RyanJsonDetachByKey(pJson, RyanJsonGetKey(LastItem));
			if (detached) { RyanJsonDelete(detached); }
		}
	}

	// 按 index 分离
	{
		int32_t idx = RyanJsonGetSize(pJson) % size;
		RyanJson_t detached = RyanJsonDetachByIndex(pJson, (size % 25) ? idx : 0);
		if (detached) { RyanJsonDelete(detached); }
	}

	return RyanJsonTrue;
}

/**
 * @brief 测试 Json 的 Delete 功能（保护根节点）
 *
 * @param pJson 待测试的 Json 节点
 * @param size  用于计算 index 的模数
 * @param isFirst 是否为第一次调用（根节点）
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e RyanJsonFuzzerTestByForEachDelete(RyanJson_t pJson, int32_t size)
{
	if (!(RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))) { return RyanJsonTrue; }

	// -------- 测试错误的 delete 调用 --------
	// Key 删除错误用例
	RyanJsonDeleteByKey(pJson, "non_exist_key");
	RyanJsonDeleteByKey(NULL, "some_key");
	RyanJsonDeleteByKey(pJson, NULL);
	RyanJsonDeleteByKey(NULL, NULL);

	// Index 删除错误用例
	RyanJsonDeleteByIndex(pJson, RyanJsonGetSize(pJson)); // 越界
	RyanJsonDeleteByIndex(NULL, (RyanJsonGetSize(pJson) % size));
	RyanJsonDeleteByIndex(pJson, -size); // 负数
	RyanJsonDeleteByIndex(NULL, -size);

	// 递归遍历子节点
	RyanJson_t item = NULL;
	RyanJson_t LastItem = NULL;
	RyanJsonObjectForEach(pJson, item)
	{
		RyanJsonFuzzerTestByForEachDelete(item, size);
		LastItem = item;
	}

	// -------- 正常删除逻辑（保护根节点） --------

	// 按 key 删除（仅对象）
	if (RyanJsonIsObject(pJson))
	{
		if (LastItem && RyanJsonIsKey(LastItem))
		{
			// RyanPrintJsonToLog22(pJson);

			// printf("key is %d %s\r\n", RyanJsonGetType(LastItem),
			//        RyanJsonGetKey(LastItem) == NULL ? "NULL" : RyanJsonGetKey(LastItem));
			RyanJsonDeleteByKey(pJson, RyanJsonGetKey(LastItem));
		}
	}

	// 按 index 删除
	int32_t idx = RyanJsonGetSize(pJson) % size;
	RyanJsonDeleteByIndex(pJson, (size % 25) ? idx : 0);

	return RyanJsonTrue;
}

static RyanJsonBool_e RyanJsonFuzzerTestByMinify(const char *data, int32_t size)
{
	char *buf = malloc(size + 100);
	memset(buf, 0, size + 100);
	memcpy(buf, data, size);

	int32_t size2 = RyanJsonMinify(buf, size);
	// 非法情况
	{
		RyanJsonCheckReturnFlase(0 == RyanJsonMinify(NULL, 0));
		RyanJsonCheckReturnFlase(0 == RyanJsonMinify(NULL, 10));
		RyanJsonCheckReturnFlase(0 == RyanJsonMinify(NULL, -10));
		RyanJsonCheckReturnFlase(0 == RyanJsonMinify(buf, -10));
	}
	RyanJson_t pJson2 = RyanJsonParseOptions(buf, size2, size % 2 ? RyanJsonTrue : RyanJsonFalse, NULL);
	free(buf);
	if (NULL != pJson2)
	{
		int32_t len = 0;
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

static void *RyanJsonFuzzerMalloc(size_t size)
{
	static int32_t count = 0;
	count++;
	if (RyanJsonTrue == isadfa)
	{
		if (0 == count % 709) { return NULL; }
	}
	return (char *)v_malloc(size);
}

static void RyanJsonFuzzerFree(void *block) { v_free(block); }

static void *RyanJsonFuzzerRealloc(void *block, size_t size)
{
	static int32_t count = 0;
	count++;
	if (RyanJsonTrue == isadfa)
	{
		if (0 == count % 708) { return NULL; }
	}
	return (char *)v_realloc(block, size);
}

// 需要模拟内存故障
// tokey需要增加s测试
int LLVMFuzzerTestOneInput(const char *data, int32_t size)
{

	// !检查分支覆盖率的时候要把这个取消掉,否则不知道是这个测试用例还是Fuzzer触发的，期望的是Fuzzer触发
	// {
	// 	// 执行基础测试
	// 	static bool isFirst = true;
	// 	if (true == isFirst)
	// 	{
	// 		RyanJsonBool_e result = RyanJsonBaseTest();
	// 		if (RyanJsonTrue != result)
	// 		{
	// 			printf("%s:%d RyanJsonTest fail\r\n", __FILE__, __LINE__);
	// 			return -1;
	// 		}

	// 		result = RFC8259JsonTest();
	// 		// if (RyanJsonTrue != result)
	// 		// {
	// 		// 	printf("%s:%d RFC8259JsonTest fail\r\n", __FILE__, __LINE__);
	// 		// 	return -1;
	// 		// }
	// 		isFirst = false;
	// 	}
	// }

	// for (int i = 0; i < size; i++) { printf("%c", size, data[i]); }
	// printf("\r\n");

	RyanJsonInitHooks(RyanJsonFuzzerMalloc, RyanJsonFuzzerFree, size % 2 ? NULL : RyanJsonFuzzerRealloc);

	const char *parseEndPtr = NULL;
	RyanJson_t pJson = RyanJsonParseOptions(data, size, size % 2 ? RyanJsonTrue : RyanJsonFalse, &parseEndPtr);
	if (NULL != pJson)
	{
		assert(NULL != parseEndPtr && parseEndPtr - data <= size);

		{
			isadfa = RyanJsonFalse;
			RyanJson_t pJson2 = RyanJsonDuplicate(pJson);
			isadfa = RyanJsonTrue;
			RyanJsonCheckCode(RyanJsonFuzzerTestByForEachDelete(pJson2, size), {
				RyanJsonDelete(pJson2);
				goto __exit;
			});
			RyanJsonDelete(pJson2);
		}

		{
			isadfa = RyanJsonFalse;
			RyanJson_t pJson2 = RyanJsonDuplicate(pJson);
			isadfa = RyanJsonTrue;
			RyanJsonCheckCode(RyanJsonFuzzerTestByForEachDetach(pJson2, size), {
				RyanJsonDelete(pJson2);
				goto __exit;
			});
			RyanJsonDelete(pJson2);
		}

		RyanJsonFuzzerTestByMinify(data, size);
		RyanJsonFuzzerTestByParseAndPrint(pJson, data, size);
		RyanJsonFuzzerTestByForEachGet(pJson, size);

		RyanJsonFuzzerTestByDup(pJson);
		RyanJsonCheckCode(RyanJsonFuzzerTestByForEachChange(pJson, size), { goto __exit; });
		RyanJsonCheckCode(RyanJsonFuzzerTestByForEachCreate(pJson, size), { goto __exit; });

		RyanJsonCheckCode(RyanJsonFuzzerTestByForEachReplace(pJson, size), { goto __exit; });

		// 测试打印和复制功能
		RyanJsonDelete(pJson);
	}

	// cJSON_Hooks hooks = {.malloc_fn = v_malloc, .free_fn = v_free};
	// cJSON_InitHooks(&hooks);

	// cJSON *json = cJSON_ParseWithLength(data, size);
	// if (json)
	// {

	// 	char *str2 = cJSON_Print(json);
	// 	cJSON_free(str2);
	// 	cJSON_Delete(json);
	// }

	// RyanJsonMinify(data);
	return 0;

__exit:
	RyanJsonDelete(pJson);
	return 0;
}
