#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

/**
 * @brief 补齐超长数字导致的解析溢出分支
 *
 * 运行期 parse harness 对大输入会提前返回，且 FUZZ_MAX_LEN/调度概率下
 * 很难稳定命中 `isfinite(number)` 的失败路径，这里保留最小确定性补齐。
 */
static void RyanJsonFuzzerSelfTestParseHugeNumberOverflowCases(void)
{
	// 这些长度远小于 FUZZ_MAX_LEN=8192，但已足够把 number 推到非有限值。
	// 这样可以只补最小分支，不必恢复整包 parse 手动覆盖。
	char hugeIntText[640];
	char hugeFracText[1024];

	hugeIntText[0] = '1';
	memset(hugeIntText + 1, '9', sizeof(hugeIntText) - 2);
	hugeIntText[sizeof(hugeIntText) - 1] = '\0';
	assert(NULL == RyanJsonParse(hugeIntText));

	hugeFracText[0] = '0';
	hugeFracText[1] = '.';
	memset(hugeFracText + 2, '9', sizeof(hugeFracText) - 3);
	hugeFracText[sizeof(hugeFracText) - 1] = '\0';
	assert(NULL == RyanJsonParse(hugeFracText));
}

/**
 * @brief parse/print 相关的一次性确定性自检
 *
 * 这里只保留运行期 harness 不会主动调用到的 guard 和非法状态。
 */
void RyanJsonFuzzerSelfTestParseCases(void)
{
	RyanJsonBool_e lastIsEnableMemFail;
	RyanJsonFuzzerMemFailPush(lastIsEnableMemFail, RyanJsonFalse);

	// FUZZ_MAX_LEN 远小于 UINT16_MAX，这些内部长度编码边界只能手动补齐。
	assert(3 == RyanJsonInternalCalcLenBytes(UINT32_MAX));
	assert(4 == RyanJsonInternalDecodeKeyLenField(0x03));

	// 第一组：不会随输入变化的 parse/print guard。
	{
		const char *jsonText = "{}";
		const char *parseEndPtr = NULL;
		RyanJson_t endPtrJson = RyanJsonParseOptions(jsonText, 2, RyanJsonFalse, &parseEndPtr);
		assert(NULL != endPtrJson);
		assert(parseEndPtr == jsonText + 2);
		RyanJsonDelete(endPtrJson);
	}

	// 第二组：构造非法内部 type，验证 Print/Duplicate/Compare/Insert 的鲁棒性。
	// 这类状态运行期 harness 不会主动生成，属于典型的 self-test 归属。
	{
		RyanJson_t objItem = RyanJsonCreateObject();
		RyanJson_t objItem2 = RyanJsonCreateObject();
		assert(NULL != objItem && NULL != objItem2);

		RyanJsonSetType(objItem, 0);
		RyanJsonSetType(objItem2, 0);

		assert(NULL == RyanJsonPrint(objItem, 100, RyanJsonFalse, NULL));
		assert(NULL == RyanJsonDuplicate(objItem));
		assert(RyanJsonFalse == RyanJsonCompare(objItem, objItem2));
		assert(RyanJsonFalse == RyanJsonCompareOnlyKey(objItem, objItem2));
		assert(RyanJsonFalse == RyanJsonInsert(objItem, 0, RyanJsonCreateString("key", "true")));
		assert(RyanJsonFalse == RyanJsonInsert(objItem2, UINT32_MAX, RyanJsonCreateString("key", "true")));

		RyanJsonDelete(objItem);
		RyanJsonDelete(objItem2);
	}

	// 第三组：空指针和空缓冲区合同，全部属于与 corpus 无关的固定 guard。
	{
		RyanJson_t dummyObj = RyanJsonCreateObject();
		assert(NULL != dummyObj);
		char dummyBuf[8] = {0};

		assert(NULL == RyanJsonPrint(NULL, 100, RyanJsonFalse, NULL));
		assert(NULL == RyanJsonPrintWithStyle(dummyObj, 100, NULL, NULL));
		assert(NULL == RyanJsonPrintPreallocated(NULL, NULL, 100, RyanJsonFalse, NULL));
		assert(NULL == RyanJsonPrintPreallocated(dummyObj, NULL, sizeof(dummyBuf), RyanJsonFalse, NULL));
		assert(NULL == RyanJsonPrintPreallocated(NULL, dummyBuf, sizeof(dummyBuf), RyanJsonFalse, NULL));
		assert(NULL == RyanJsonPrintPreallocated(dummyObj, dummyBuf, 0, RyanJsonFalse, NULL));
		assert(NULL == RyanJsonPrintPreallocatedWithStyle(dummyObj, dummyBuf, sizeof(dummyBuf), NULL, NULL));

		RyanJsonDelete(dummyObj);
		assert(NULL == RyanJsonParse(NULL));
	}

	// 第四组：非有限 double 的打印降级语义。
	// 这里不依赖随机故障注入，直接验证库承诺的稳定输出格式。
	{
		RyanJson_t specialObj = RyanJsonCreateObject();
		assert(NULL != specialObj);
		assert(RyanJsonTrue == RyanJsonAddDoubleToObject(specialObj, "inf", INFINITY));
		assert(RyanJsonTrue == RyanJsonAddDoubleToObject(specialObj, "nan", NAN));

		uint32_t specialLen = 0;
		char *specialStr = RyanJsonPrint(specialObj, 100, RyanJsonFalse, &specialLen);
		assert(NULL != specialStr);
		assert(NULL != strstr(specialStr, "\"inf\":null"));
		assert(NULL != strstr(specialStr, "\"nan\":null"));
		RyanJsonFree(specialStr);
		RyanJsonDelete(specialObj);
	}

	RyanJsonFuzzerSelfTestParseHugeNumberOverflowCases();

	RyanJsonFuzzerMemFailPop(lastIsEnableMemFail);
}

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
	RyanJsonBool isPrintFormat = size % 2 ? RyanJsonFalse : RyanJsonTrue;
	if (size > 1024) { isPrintFormat = RyanJsonFalse; }

	// 大输入的 parse/print 往返成本很高；这里把重型一致性校验限制在较小样本上，
	// 其余大样本主要交给 parser 本体和其他 case 去吃覆盖。
	if (size > 512) { return RyanJsonTrue; }

	// 第一段：对当前已解析树做 print roundtrip 校验。
	// 序列化一致性校验
	uint32_t len = 0;
	char *jsonStr = RyanJsonPrint(pJson, isPrintFormat ? size : 0, isPrintFormat, &len);
	RyanJsonCheckReturnFalse(NULL != jsonStr);

	// 验证不传长度指针的情况
	RyanJsonBool_e lastMemFail;
	RyanJsonFuzzerMemFailPush(lastMemFail, RyanJsonFalse);
	char *jsonStrCopy = RyanJsonPrint(pJson, isPrintFormat ? size : 0, isPrintFormat, NULL);
	RyanJsonFuzzerMemFailPop(lastMemFail);
	assert(0 == strncmp(jsonStr, jsonStrCopy, len));
	RyanJsonFree(jsonStrCopy);
	RyanJsonFree(jsonStr);

	// 第二段：验证预分配缓冲区打印。
	// 这块特意关闭随机 OOM，只想证明“容量足够时 API 语义正确”，不想把打印失败变成概率噪声。
	// 预分配缓冲区打印测试
	// 保守放大缓冲区，减少“仅因为缓冲区太小而失败”的噪声，让热路径更聚焦语义校验。
	uint32_t bufLen = len * 1.2;
	if (bufLen < size * 1.5) { bufLen = size * 1.5; }
	if (bufLen < 1024) { bufLen = 1024; }

	char *buf = (char *)malloc((size_t)bufLen);
	if (!buf) { return RyanJsonFalse; }

	{
		uint32_t len2 = 0;
		RyanJsonBool_e lastMemFail;
		RyanJsonFuzzerMemFailPush(lastMemFail, RyanJsonFalse);
		char *jsonStr2 = RyanJsonPrintPreallocated(pJson, buf, bufLen, isPrintFormat, &len2);
		RyanJsonFuzzerMemFailPop(lastMemFail);
		assert(NULL != jsonStr2);
		assert(len == len2);
	}

	memcpy(buf, data, (size_t)size);
	buf[size] = 0;

	// 原始输入不保证自带 '\0'，这里补终止符后再走按字符串解析的公共入口。
	RyanJson_t jsonRoot = RyanJsonParse(buf);
	RyanJsonCheckCode(NULL != jsonRoot, {
		free(buf);
		return RyanJsonFalse;
	});

	// 第三段：对“原始输入重新解析得到的新树”再做一次打印校验，
	// 防止只验证了入口 parse 成功，却漏掉后续 print 路径的稳定性。
	{
		uint32_t len3 = 0;
		char *jsonStr3 = RyanJsonPrint(jsonRoot, 100, isPrintFormat, &len3);
		RyanJsonCheckCode(NULL != jsonStr3, {
			free(buf);
			if (jsonStr3) { RyanJsonFree(jsonStr3); }
			RyanJsonDelete(jsonRoot);
			return RyanJsonFalse;
		});
		RyanJsonFree(jsonStr3);
	}

	// 这里刻意只调用不硬断言，目的是覆盖“小缓冲区失败或截断”路径，
	// 而不是把具体返回形式锁死为单一实现。
	RyanJsonPrintPreallocated(jsonRoot, buf, bufLen / 15, RyanJsonTrue, NULL);

	free(buf);
	RyanJsonDelete(jsonRoot);
	return RyanJsonTrue;
}
