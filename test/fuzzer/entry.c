#include "RyanJsonFuzzer.h"
#include <string.h>

/**
 * @brief LLVM LibFuzzer 主入口
 *
 * 每轮 Fuzz 迭代都会调用该函数。
 *
 * 主要流程：
 * - 初始化 Fuzzer 状态与随机源。
 * - 检查首字节是否为 `0xFF`，决定执行 API 序列模式或解析模式。
 * - 注册内存 Hook，确保测试过程中资源可控并可回收。
 */
int LLVMFuzzerTestOneInput(const char *data, uint32_t size)
{
	// 初始化覆盖率/随机状态
	uint8_t magicByte = 0;
	if (size > 0) { magicByte = (uint8_t)data[0]; }
	if (0xFF == magicByte) { RyanJsonFuzzerInit((const uint8_t *)data + 1, size - 1); }

	if (0 == g_fuzzerState.seed) { RyanJsonFuzzerInit((const uint8_t *)data, size); }
	g_fuzzerState.isEnableMemFail = true;

	assert(RyanJsonFalse == RyanJsonInitHooks(NULL, RyanJsonFuzzerFree, RyanJsonFuzzerRealloc));
	assert(RyanJsonFalse == RyanJsonInitHooks(RyanJsonFuzzerMalloc, NULL, RyanJsonFuzzerRealloc));
	assert(RyanJsonFalse == RyanJsonInitHooks(NULL, NULL, NULL));

	assert(RyanJsonTrue == RyanJsonInitHooks(RyanJsonFuzzerMalloc, RyanJsonFuzzerFree, 0 != size % 2 ? NULL : RyanJsonFuzzerRealloc));

	assert(NULL == RyanJsonParseOptions(NULL, 100, RyanJsonFalse, NULL));
	assert(NULL == RyanJsonParseOptions(data, 0, RyanJsonFalse, NULL));

	const char *parseEndPtr = NULL;
	RyanJson_t pJson = RyanJsonParseOptions(data, size, 0 != size % 3 ? RyanJsonTrue : RyanJsonFalse, &parseEndPtr);
	if (NULL != pJson)
	{
		RyanJsonFuzzerTestMinify(data, size);
		RyanJsonFuzzerTestParse(pJson, data, size);
		RyanJsonFuzzerTestGet(pJson, size);

		RyanJsonFuzzerTestDuplicate(pJson);
		RyanJsonCheckCode(RyanJsonFuzzerTestModify(pJson, size), { goto exit__; });
		RyanJsonCheckCode(RyanJsonFuzzerTestCreate(pJson, size), { goto exit__; });
		RyanJsonCheckCode(RyanJsonFuzzerTestDelete(pJson, size), { goto exit__; });
		RyanJsonCheckCode(RyanJsonFuzzerTestReplace(pJson, size), { goto exit__; });

		// 测试分离
		{
			g_fuzzerState.isEnableMemFail = false;
			RyanJson_t pJson2 = RyanJsonDuplicate(pJson);
			g_fuzzerState.isEnableMemFail = true;
			RyanJsonDelete(pJson);

			RyanJsonFuzzerTestDetach(pJson2, size);
			RyanJsonDelete(pJson2);
		}
	}

	return 0;

exit__:
	RyanJsonDelete(pJson);
	return 0;
}
