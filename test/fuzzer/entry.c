#include "RyanJsonFuzzer.h"
#include <limits.h>

/**
 * @brief LLVM LibFuzzer 主入口
 *
 * 每轮 Fuzz 迭代都会调用该函数。
 *
 * 主要流程：
 * - 初始化 Fuzzer 状态与随机源。
 * - 先执行一次与输入无关的确定性自检。
 * - 注册内存 Hook，确保测试过程中资源可控并可回收。
 */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	if (NULL == data) { return 0; }

	// RyanJson 各测试入口统一使用 uint32_t 长度，先在入口处做一次收敛。
	uint32_t inputSize = (size > (size_t)UINT32_MAX) ? UINT32_MAX : (uint32_t)size;

	RyanJsonFuzzerInit(data, inputSize);
	// 与输入无关的确定性覆盖只在首轮执行一次，避免拖慢后续 10 万次跑批。
	RyanJsonFuzzerSelfTestOnce();
	g_fuzzerState.isEnableMemFail = true;

	// 用输入奇偶切换 realloc Hook，可稳定覆盖“有/无 realloc”两套资源路径。
	assert(RyanJsonTrue ==
	       RyanJsonInitHooks(RyanJsonFuzzerMalloc, RyanJsonFuzzerFree, 0 != inputSize % 2 ? NULL : RyanJsonFuzzerRealloc));

	// 用输入驱动 strict 选项，保证同一份样本在“宽松尾部/严格尾部”两类解析语义下都能逐步积累覆盖。
	RyanJson_t pJson = RyanJsonParseOptions((const char *)data, inputSize, 0 != inputSize % 3 ? RyanJsonTrue : RyanJsonFalse, NULL);
	if (NULL != pJson)
	{
		// 执行顺序保持“文本相关 -> 只读访问 -> 原地变异 -> 所有权迁移”。
		// 这样前面的 case 更像观察者，后面的 case 再消费和改写树结构，减少互相污染。
		RyanJsonFuzzerTestMinify((const char *)data, inputSize);
		RyanJsonFuzzerTestParse(pJson, (const char *)data, inputSize);
		RyanJsonFuzzerTestGet(pJson, inputSize);

		RyanJsonFuzzerTestDuplicate(pJson);
		RyanJsonCheckCode(RyanJsonFuzzerTestModify(pJson, inputSize), { goto exit__; });
		RyanJsonCheckCode(RyanJsonFuzzerTestCreate(pJson, inputSize), { goto exit__; });
		RyanJsonCheckCode(RyanJsonFuzzerTestDelete(pJson, inputSize), { goto exit__; });
		RyanJsonCheckCode(RyanJsonFuzzerTestReplace(pJson, inputSize), { goto exit__; });

		{
			RyanJsonBool_e lastIsEnableMemFail;
			RyanJsonFuzzerMemFailPush(lastIsEnableMemFail, RyanJsonFalse);
			// detach 会改变树所有权；先复制一份，避免与前面的变异操作互相干扰。
			// 这里特意关闭 OOM，是因为我们想验证 detach 语义本身，而不是重复打一遍 Duplicate 的失败路径。
			RyanJson_t pJson2 = RyanJsonDuplicate(pJson);
			RyanJsonFuzzerMemFailPop(lastIsEnableMemFail);
			RyanJsonDelete(pJson);

			RyanJsonFuzzerTestDetach(pJson2, inputSize);
			RyanJsonDelete(pJson2);
		}
	}

	return 0;

exit__:
	RyanJsonDelete(pJson);
	return 0;
}
