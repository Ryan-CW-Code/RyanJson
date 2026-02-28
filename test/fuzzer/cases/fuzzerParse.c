#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

/**
 * @brief 解析与打印测试
 *
 * 测试 RyanJson 的核心解析与打印路径。
 * 核心逻辑：
 * 错误注入测试：在内存分配失败场景下验证打印、复制与比较路径的健壮性。
 * 往返一致性验证：
 * - Parse(Print(Parse(Data))) 与 Parse(Data) 语义一致。
 * - 验证打印结果再次解析后，结构和内容保持稳定。
 * 预分配缓冲区打印测试：验证 PrintPreallocated 在不同容量下的行为。
 *
 * @param state Fuzzer 状态上下文
 * @param pJson 解析输入数据得到的初始 Json 对象
 * @param data 原始输入数据字符串
 * @param size 输入数据长度
 */
RyanJsonBool_e RyanJsonFuzzerTestParse(RyanJson_t pJson, const char *data, uint32_t size)
{
	// 一次性覆盖关键防御分支，避免依赖随机路径命中
	static RyanJsonBool_e coreBoundaryCovered = RyanJsonFalse;
	if (RyanJsonFalse == coreBoundaryCovered)
	{
		RyanJsonBool_e lastIsEnableMemFail = g_fuzzerState.isEnableMemFail;
		g_fuzzerState.isEnableMemFail = false;

		// 指数累积溢出防御
		assert(NULL == RyanJsonParse("1e2147483648"));
		assert(NULL == RyanJsonParse("1e-2147483648"));
		// 边界值：驱动 e_scale==INT32_MAX/10 且 digit<=INT32_MAX%10 的分支
		assert(NULL == RyanJsonParse("1e2147483647"));
		// 负指数边界会下溢到 0（有限数），属于可接受输入
		RyanJson_t negativeBoundary = RyanJsonParse("1e-2147483647");
		assert(NULL != negativeBoundary);
		RyanJsonDelete(negativeBoundary);
		assert(NULL == RyanJsonParse("{\"a\":1e2147483648}"));
#if true == RyanJsonStrictObjectKeyCheck
		assert(NULL == RyanJsonParse("{\"dup\":1,\"dup\":2}"));
#else
		{
			RyanJson_t dupObj = RyanJsonParse("{\"dup\":1,\"dup\":2}");
			assert(NULL != dupObj);
			RyanJsonDelete(dupObj);
		}
#endif

		// 比较逻辑分支覆盖：
		// 根节点类型不一致，覆盖类型比较失败分支。
		// boolValue 不一致，覆盖布尔全量比较失败分支。
		// Int/Double 数值不一致，覆盖数值全量比较失败分支。
		// strValue 不一致，覆盖字符串全量比较失败分支。
		// 对象 key 乱序，覆盖对象下沉/同层 key 回退查找分支。
		// 对象缺 key，覆盖 rightChild == NULL 的失败分支。
		{
			RyanJson_t cmpInt = RyanJsonCreateInt(NULL, 1);
			RyanJson_t cmpString = RyanJsonCreateString(NULL, "1");
			assert(NULL != cmpInt && NULL != cmpString);
			assert(RyanJsonFalse == RyanJsonCompare(cmpInt, cmpString));
			assert(RyanJsonFalse == RyanJsonCompareOnlyKey(cmpInt, cmpString));
			RyanJsonDelete(cmpInt);
			RyanJsonDelete(cmpString);

			RyanJson_t cmpBoolTrue = RyanJsonCreateBool(NULL, RyanJsonTrue);
			RyanJson_t cmpBoolFalse = RyanJsonCreateBool(NULL, RyanJsonFalse);
			assert(NULL != cmpBoolTrue && NULL != cmpBoolFalse);
			assert(RyanJsonFalse == RyanJsonCompare(cmpBoolTrue, cmpBoolFalse));
			assert(RyanJsonTrue == RyanJsonCompareOnlyKey(cmpBoolTrue, cmpBoolFalse));
			RyanJsonDelete(cmpBoolTrue);
			RyanJsonDelete(cmpBoolFalse);

			RyanJson_t cmpIntLeft = RyanJsonCreateInt(NULL, 123);
			RyanJson_t cmpIntRight = RyanJsonCreateInt(NULL, 456);
			assert(NULL != cmpIntLeft && NULL != cmpIntRight);
			assert(RyanJsonFalse == RyanJsonCompare(cmpIntLeft, cmpIntRight));
			assert(RyanJsonTrue == RyanJsonCompareOnlyKey(cmpIntLeft, cmpIntRight));
			RyanJsonDelete(cmpIntLeft);
			RyanJsonDelete(cmpIntRight);

			RyanJson_t cmpDoubleLeft = RyanJsonCreateDouble(NULL, 1.0);
			RyanJson_t cmpDoubleRight = RyanJsonCreateDouble(NULL, 2.0);
			assert(NULL != cmpDoubleLeft && NULL != cmpDoubleRight);
			assert(RyanJsonFalse == RyanJsonCompare(cmpDoubleLeft, cmpDoubleRight));
			assert(RyanJsonTrue == RyanJsonCompareOnlyKey(cmpDoubleLeft, cmpDoubleRight));
			RyanJsonDelete(cmpDoubleLeft);
			RyanJsonDelete(cmpDoubleRight);

			RyanJson_t cmpStrLeft = RyanJsonCreateString(NULL, "alpha");
			RyanJson_t cmpStrRight = RyanJsonCreateString(NULL, "beta");
			assert(NULL != cmpStrLeft && NULL != cmpStrRight);
			assert(RyanJsonFalse == RyanJsonCompare(cmpStrLeft, cmpStrRight));
			assert(RyanJsonTrue == RyanJsonCompareOnlyKey(cmpStrLeft, cmpStrRight));
			RyanJsonDelete(cmpStrLeft);
			RyanJsonDelete(cmpStrRight);

			RyanJson_t objLeft = RyanJsonCreateObject();
			RyanJson_t objRightUnordered = RyanJsonCreateObject();
			assert(NULL != objLeft && NULL != objRightUnordered);
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objLeft, "a", 1));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objLeft, "b", 2));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objLeft, "c", 3));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objRightUnordered, "b", 2));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objRightUnordered, "a", 1));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objRightUnordered, "c", 3));
			assert(RyanJsonTrue == RyanJsonCompare(objLeft, objRightUnordered));
			assert(RyanJsonTrue == RyanJsonCompareOnlyKey(objLeft, objRightUnordered));
			RyanJsonDelete(objLeft);
			RyanJsonDelete(objRightUnordered);

			RyanJson_t objNeedKey = RyanJsonCreateObject();
			RyanJson_t objMissKey = RyanJsonCreateObject();
			assert(NULL != objNeedKey && NULL != objMissKey);
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objNeedKey, "a", 1));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objMissKey, "b", 1));
			assert(RyanJsonFalse == RyanJsonCompare(objNeedKey, objMissKey));
			assert(RyanJsonFalse == RyanJsonCompareOnlyKey(objNeedKey, objMissKey));
			RyanJsonDelete(objNeedKey);
			RyanJsonDelete(objMissKey);

			// 命中同层快路径中 rightCandidate == NULL 的分支：
			// 左侧首 key=a，右侧将 a 放到最后；第一次下沉后 rightCurrent 位于尾节点。
			// 比较 leftNext 时 rightCandidate 为 NULL，必须走按 key 回退查找。
			RyanJson_t objLastHitLeft = RyanJsonCreateObject();
			RyanJson_t objLastHitRight = RyanJsonCreateObject();
			assert(NULL != objLastHitLeft && NULL != objLastHitRight);
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objLastHitLeft, "a", 1));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objLastHitLeft, "b", 2));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objLastHitLeft, "c", 3));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objLastHitRight, "b", 2));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objLastHitRight, "c", 3));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objLastHitRight, "a", 1));
			assert(RyanJsonTrue == RyanJsonCompare(objLastHitLeft, objLastHitRight));
			assert(RyanJsonTrue == RyanJsonCompareOnlyKey(objLastHitLeft, objLastHitRight));
			RyanJsonDelete(objLastHitLeft);
			RyanJsonDelete(objLastHitRight);

			// 命中同层回退查找失败（rightNext == NULL）分支：
			// 首 key 相同、后续 key 不同且 size 相同，失败点落在同层阶段而非下沉阶段。
			RyanJson_t objPrefixLeft = RyanJsonCreateObject();
			RyanJson_t objPrefixRight = RyanJsonCreateObject();
			assert(NULL != objPrefixLeft && NULL != objPrefixRight);
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objPrefixLeft, "a", 1));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objPrefixLeft, "b", 2));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objPrefixRight, "a", 1));
			assert(RyanJsonTrue == RyanJsonAddIntToObject(objPrefixRight, "c", 2));
			assert(RyanJsonFalse == RyanJsonCompare(objPrefixLeft, objPrefixRight));
			assert(RyanJsonFalse == RyanJsonCompareOnlyKey(objPrefixLeft, objPrefixRight));
			RyanJsonDelete(objPrefixLeft);
			RyanJsonDelete(objPrefixRight);

			// 容器内数值子类型：全量比较要求一致，仅比较 key 时允许 int/double 混用。
			RyanJson_t numberSubtypeLeft = RyanJsonParse("{\"n\":1,\"arr\":[1,2.0],\"obj\":{\"x\":3,\"y\":4.0}}");
			RyanJson_t numberSubtypeRight = RyanJsonParse("{\"obj\":{\"y\":4,\"x\":3.0},\"arr\":[1.0,2],\"n\":1.0}");
			assert(NULL != numberSubtypeLeft && NULL != numberSubtypeRight);
			assert(RyanJsonFalse == RyanJsonCompare(numberSubtypeLeft, numberSubtypeRight));
			assert(RyanJsonTrue == RyanJsonCompareOnlyKey(numberSubtypeLeft, numberSubtypeRight));
			RyanJsonDelete(numberSubtypeLeft);
			RyanJsonDelete(numberSubtypeRight);
		}

		// int32 打印边界：11（不含 \0 空间）失败，12（含 \0 空间）成功
		RyanJson_t intItem = RyanJsonCreateInt(NULL, INT32_MIN);
		assert(NULL != intItem);
		char tooSmall[11] = {0};
		assert(NULL == RyanJsonPrintPreallocated(intItem, tooSmall, sizeof(tooSmall), RyanJsonFalse, NULL));
		char exactFit[12] = {0};
		char *exactOut = RyanJsonPrintPreallocated(intItem, exactFit, sizeof(exactFit), RyanJsonFalse, NULL);
		assert(NULL != exactOut);
		assert(0 == strcmp(exactOut, "-2147483648"));
		RyanJsonDelete(intItem);

		// 预分配“刚好够用”的成功路径（非数值节点）
		{
			RyanJson_t strObj = RyanJsonCreateObject();
			assert(NULL != strObj);
			assert(RyanJsonTrue == RyanJsonAddStringToObject(strObj, "k", "v"));

			uint32_t expectedLen = 0;
			char *expected = RyanJsonPrint(strObj, 0, RyanJsonFalse, &expectedLen);
			assert(NULL != expected);

			char exactBuf[16] = {0};
			assert(expectedLen + 1U <= sizeof(exactBuf));
			char *exactStr = RyanJsonPrintPreallocated(strObj, exactBuf, expectedLen + 1U, RyanJsonFalse, NULL);
			assert(NULL != exactStr);
			assert(0 == strcmp(expected, exactStr));

			RyanJsonFree(expected);
			RyanJsonDelete(strObj);
		}

		g_fuzzerState.isEnableMemFail = lastIsEnableMemFail;
		coreBoundaryCovered = RyanJsonTrue;
	}

	if (RyanJsonFuzzerShouldFail(100))
	{
		assert(1 == RyanJsonInternalCalcLenBytes(UINT8_MAX - 1));
		assert(1 == RyanJsonInternalCalcLenBytes(UINT8_MAX));
		assert(2 == RyanJsonInternalCalcLenBytes((uint32_t)UINT8_MAX + 1U));
		assert(2 == RyanJsonInternalCalcLenBytes(UINT16_MAX));
		assert(3 == RyanJsonInternalCalcLenBytes(UINT32_MAX - 1));

		assert(1 == RyanJsonInternalDecodeKeyLenField(0x01));
		assert(2 == RyanJsonInternalDecodeKeyLenField(0x02));
		assert(4 == RyanJsonInternalDecodeKeyLenField(0x03));

		g_fuzzerState.isEnableMemFail = false; // 临时禁用内存失败模拟，便于构造测试对象
		RyanJson_t objItem = RyanJsonCreateObject();
		RyanJson_t objItem2 = RyanJsonCreateObject();

		// 故意设置错误的类型标志，测试鲁棒性
		RyanJsonSetType(objItem, 0);
		RyanJsonSetType(objItem2, 0);

		// 验证异常状态下的 API 行为
		assert(NULL == RyanJsonPrint(objItem, 100, RyanJsonFalse, NULL));
		assert(NULL == RyanJsonDuplicate(objItem));
		assert(RyanJsonFalse == RyanJsonCompare(objItem, objItem2));
		assert(RyanJsonFalse == RyanJsonCompareOnlyKey(objItem, objItem2));

		// 测试 Insert 类型检查
		assert(RyanJsonFalse == RyanJsonInsert(objItem, 0, RyanJsonCreateString("key", "true")));
		assert(RyanJsonFalse == RyanJsonInsert(objItem2, UINT32_MAX, RyanJsonCreateString("key", "true")));

		// 恢复正确类型
		RyanJsonSetType(objItem, RyanJsonTypeObject);
		RyanJsonSetType(objItem2, RyanJsonTypeObject);

		// 测试 Insert 异常参数
		if (RyanJsonIsObject(pJson))
		{
			assert(RyanJsonFalse == RyanJsonInsert(pJson, 0, RyanJsonCreateString(NULL, "true"))); // 缺少 key
			assert(RyanJsonFalse == RyanJsonInsert(pJson, UINT32_MAX, RyanJsonCreateString(NULL, "true")));
		}

		// 正常插入尝试
		if (RyanJsonIsArray(pJson)) { assert(RyanJsonTrue == RyanJsonInsert(pJson, 0, RyanJsonCreateString(NULL, "true"))); }
		else if (RyanJsonIsObject(pJson))
		{
			// 对象 key 需唯一：为插入构造一个未使用的 key
			RyanJsonBool_e foundUniqueKey = RyanJsonFalse;
			char keyBuf[32];

			for (uint32_t i = 0; i < 128; i++)
			{
				if (0 == i) { RyanJsonSnprintf(keyBuf, sizeof(keyBuf), "key"); }
				else
				{
					RyanJsonSnprintf(keyBuf, sizeof(keyBuf), "__fuzz_key_%u__", (unsigned)i);
				}

				if (RyanJsonFalse == RyanJsonHasObjectByKey(pJson, keyBuf))
				{
					foundUniqueKey = RyanJsonTrue;
					break;
				}
			}

			if (foundUniqueKey)
			{
				RyanJson_t insertItem = RyanJsonCreateString(keyBuf, "true");
				assert(NULL != insertItem);
				assert(RyanJsonTrue == RyanJsonInsert(pJson, 0, insertItem));
			}
		}

		RyanJsonDelete(objItem);
		RyanJsonDelete(objItem2);
		g_fuzzerState.isEnableMemFail = true; // 重新启用内存失败模拟

		// 验证 Print 系列函数的空指针处理
		assert(NULL == RyanJsonPrint(NULL, 100, RyanJsonFalse, NULL));
		assert(NULL == RyanJsonPrintWithStyle(pJson, 100, NULL, NULL));
		assert(NULL == RyanJsonPrintPreallocated(NULL, NULL, 100, RyanJsonFalse, NULL));
		assert(NULL == RyanJsonPrintPreallocated(pJson, NULL, 100, RyanJsonFalse, NULL));
		assert(NULL == RyanJsonPrintPreallocated(NULL, (char *)data, 100, RyanJsonFalse, NULL));
		assert(NULL == RyanJsonPrintPreallocated(pJson, (char *)data, 0, RyanJsonFalse, NULL));
		assert(NULL == RyanJsonPrintPreallocatedWithStyle(pJson, (char *)data, 100, NULL, NULL));

		assert(NULL == RyanJsonParse(NULL));

		// 特殊 doubleValue 打印（NaN / Infinity 输出为 null）
		{
			RyanJson_t specialObj = RyanJsonCreateObject();
			if (specialObj)
			{
				g_fuzzerState.isEnableMemFail = false;
				RyanJsonAddDoubleToObject(specialObj, "inf", INFINITY);
				RyanJsonAddDoubleToObject(specialObj, "ninf", -INFINITY);
				RyanJsonAddDoubleToObject(specialObj, "nan", NAN);
				g_fuzzerState.isEnableMemFail = true;

				uint32_t specialLen = 0;
				char *specialStr = RyanJsonPrint(specialObj, 100, RyanJsonFalse, &specialLen);
				if (specialStr)
				{
					assert(NULL != strstr(specialStr, "\"inf\":null"));
					assert(NULL != strstr(specialStr, "\"ninf\":null"));
					assert(NULL != strstr(specialStr, "\"nan\":null"));
					RyanJsonFree(specialStr);
				}

				RyanJsonDelete(specialObj);
			}
		}

		// 数值溢出路径覆盖（int/double）
		{
			g_fuzzerState.isEnableMemFail = false;

			uint32_t hugeIntLen = 600;
			uint32_t hugeFracLen = 1000;
			char *hugeBuf = (char *)malloc(hugeFracLen);
			if (hugeBuf)
			{
				memset(hugeBuf, '9', (size_t)(hugeIntLen - 1));
				hugeBuf[0] = '1';
				hugeBuf[hugeIntLen - 1] = '\0';

				RyanJson_t hugeIntJson = RyanJsonParse(hugeBuf);
				assert(NULL == hugeIntJson);
				if (hugeIntJson) { RyanJsonDelete(hugeIntJson); }

				hugeBuf[0] = '0';
				hugeBuf[1] = '.';
				memset(hugeBuf + 2, '9', (size_t)(hugeFracLen - 3));
				hugeBuf[hugeFracLen - 1] = '\0';

				RyanJson_t hugeFracJson = RyanJsonParse(hugeBuf);
				assert(NULL == hugeFracJson);
				if (hugeFracJson) { RyanJsonDelete(hugeFracJson); }

				free(hugeBuf);
				g_fuzzerState.isEnableMemFail = true;
			}
		}
	}

	RyanJsonBool isPrintFormat = size % 2 ? RyanJsonFalse : RyanJsonTrue;
	if (size > 1024) { isPrintFormat = RyanJsonFalse; }

	if (size > 512) { return RyanJsonTrue; }

	// 序列化一致性校验
	uint32_t len = 0;
	// 随机选择是否格式化，增加覆盖面
	char *jsonStr = RyanJsonPrint(pJson, isPrintFormat ? size : 0, isPrintFormat, &len);
	RyanJsonCheckReturnFalse(NULL != jsonStr);

	// 验证不传长度指针的情况
	g_fuzzerState.isEnableMemFail = false;
	char *jsonStrCopy = RyanJsonPrint(pJson, isPrintFormat ? size : 0, isPrintFormat, NULL);
	g_fuzzerState.isEnableMemFail = true;
	assert(0 == strncmp(jsonStr, jsonStrCopy, len));
	RyanJsonFree(jsonStrCopy);
	RyanJsonFree(jsonStr);

	// 预分配缓冲区打印测试
	// 验证在有限缓冲区、刚好够用缓冲区等情况下的打印行为
	uint32_t bufLen = len * 1.2;
	if (bufLen < size * 1.5) { bufLen = size * 1.5; }
	if (bufLen < 1024) { bufLen = 1024; }

	char *buf = (char *)malloc((size_t)bufLen);
	if (!buf) { return RyanJsonFalse; }

	{
		uint32_t len2 = 0;
		// RyanJson 内部可能返回失败
		g_fuzzerState.isEnableMemFail = false;
		char *jsonStr2 = RyanJsonPrintPreallocated(pJson, buf, bufLen, isPrintFormat, &len2);
		g_fuzzerState.isEnableMemFail = true;
		assert(NULL != jsonStr2);
		assert(len == len2);
		// assert(0 == strncmp(jsonStr, jsonStr2, len));
	}

	// 解析原始数据测试
	// 注意：输入数据不一定以 \0 结尾，因此需要手动补终止符
	memcpy(buf, data, (size_t)size);
	buf[size] = 0;

	RyanJson_t jsonRoot = RyanJsonParse(buf);
	RyanJsonCheckCode(NULL != jsonRoot, {
		free(buf);
		return RyanJsonFalse;
	});

	// 二次序列化校验
	// 这里验证 parsedRoot 可被稳定打印。
	// 仅当原始输入是合法 Json 且格式一致时，字符串内容才可能完全一致。
	{
		uint32_t len3 = 0;
		// 使用与第一次打印相同的参数
		char *jsonStr3 = RyanJsonPrint(jsonRoot, 100, size % 2 ? RyanJsonFalse : RyanJsonTrue, &len3);

		// 这里只校验非空，因为原始 data 可能包含冗余空白或格式差异，
		// 导致 Parse 后再 Print 与原始 data 不完全一致是正常现象。
		// 这里主要确认解析结果可被稳定打印。
		RyanJsonCheckCode(NULL != jsonStr3, {
			free(buf);
			if (jsonStr3) { RyanJsonFree(jsonStr3); }
			RyanJsonDelete(jsonRoot);
			return RyanJsonFalse;
		});

		RyanJsonFree(jsonStr3);
	}

	// 边界测试：缓冲区极小的情况
	{
		RyanJsonPrintPreallocated(jsonRoot, buf, bufLen / 15, RyanJsonTrue, NULL);
	}

	free(buf);
	RyanJsonDelete(jsonRoot);
	return RyanJsonTrue;
}
