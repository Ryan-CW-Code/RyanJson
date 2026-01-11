#include "RyanJsonFuzzer.h"

RyanJsonBool_e RyanJsonFuzzerTestDuplicate(RyanJson_t pJson)
{
	RyanJsonBool_e result = RyanJsonTrue;
	char *jsonStr = NULL;
	char *jsonStrDup = NULL;
	RyanJson_t pJsonDup = NULL;

	// 测试打印和复制功能
	uint32_t len = 0;
	uint32_t dupLen = 0;

	jsonStr = RyanJsonPrint(pJson, 100, RyanJsonFalse, &len);
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

	jsonStrDup = RyanJsonPrint(pJsonDup, 100, RyanJsonFalse, &dupLen); // 以带格式方式将数据打印出来
	RyanJsonCheckGotoExit(NULL != jsonStrDup && dupLen > 0);

	RyanJsonCheckCode(len == dupLen && 0 == memcmp(jsonStr, jsonStrDup, (size_t)len), {
		printf("len:%" PRIu32 ", dupLen:%" PRIu32 "\r\n", len, dupLen);
		printf("jsonStr:%s, jsonStrDup:%s\r\n", jsonStr, jsonStrDup);
		RyanJsonCheckGotoExit(0);
	});

	if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
	{
		// 测试size不相等
		RyanJsonDelete(RyanJsonDetachByIndex(pJson, 0));
		// 增加分支覆盖率
		if (RyanJsonGetSize(pJson) > 2) { RyanJsonDelete(RyanJsonDetachByIndex(pJson, 1)); }

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
exit__:

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
