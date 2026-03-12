#include "RyanJsonFuzzer.h"

/**
 * @brief 一次性校验最基础的全局前置条件
 *
 * 这里只放与输入无关、且每轮重复执行没有价值的断言，
 * 例如 Hook 初始化、空指针保护和基础类型判定。
 */
static void RyanJsonFuzzerSelfTestInitHooks(void)
{
	// 这组断言的目标不是覆盖输入空间，而是锁死 fuzzer 运行时最基础的前置条件：
	// hooks 必须能拒绝非法组合，也必须能成功安装合法组合。
	assert(RyanJsonFalse == RyanJsonInitHooks(NULL, RyanJsonFuzzerFree, RyanJsonFuzzerRealloc));
	assert(RyanJsonFalse == RyanJsonInitHooks(RyanJsonFuzzerMalloc, NULL, RyanJsonFuzzerRealloc));
	assert(RyanJsonFalse == RyanJsonInitHooks(NULL, NULL, NULL));
	assert(RyanJsonTrue == RyanJsonInitHooks(RyanJsonFuzzerMalloc, RyanJsonFuzzerFree, RyanJsonFuzzerRealloc));

	// ParseOptions 的空指针/零长度保护同样属于全局 guard。
	// 这类路径与 corpus 内容无关，放在一次性自检比放热路径更稳定。
	assert(NULL == RyanJsonParseOptions(NULL, 100, RyanJsonFalse, NULL));
	assert(NULL == RyanJsonParseOptions("{}", 0, RyanJsonFalse, NULL));

	// 全局异常测试
	assert(RyanJsonFalse == RyanJsonIsKey(NULL));
	assert(RyanJsonFalse == RyanJsonIsNull(NULL));
	assert(RyanJsonFalse == RyanJsonIsBool(NULL));
	assert(RyanJsonFalse == RyanJsonIsNumber(NULL));
	assert(RyanJsonFalse == RyanJsonIsString(NULL));
	assert(RyanJsonFalse == RyanJsonIsArray(NULL));
	assert(RyanJsonFalse == RyanJsonIsObject(NULL));
	assert(RyanJsonFalse == RyanJsonIsInt(NULL));
	assert(RyanJsonFalse == RyanJsonIsDouble(NULL));
}

/**
 * @brief Fuzzer 自检入口（仅执行一次）
 *
 * 用于放置与输入无关的确定性断言，避免每轮重复执行。
 */
void RyanJsonFuzzerSelfTestOnce(void)
{
	static RyanJsonBool_e selfTestOnce = RyanJsonFalse;
	// 先置位再执行，避免自检内部若再次间接进入入口时重复执行。
	if (RyanJsonTrue == selfTestOnce) { return; }
	selfTestOnce = RyanJsonTrue;

	RyanJsonBool_e lastIsEnableMemFail;
	// 自检目标是稳定覆盖固定 guard；随机 OOM 会把断言变成概率事件。
	RyanJsonFuzzerMemFailPush(lastIsEnableMemFail, RyanJsonFalse);

	// 调用顺序保持“全局前置条件 -> Parse/文本预处理 -> 结构变异类 case”。
	// 这样后续维护时更容易判断某条手动覆盖到底应该归属哪一类 self-test。
	RyanJsonFuzzerSelfTestInitHooks();
	RyanJsonFuzzerSelfTestParseCases();
	RyanJsonFuzzerSelfTestMinifyCases();
	RyanJsonFuzzerSelfTestCreateCases();
	RyanJsonFuzzerSelfTestModifyCases();
	RyanJsonFuzzerSelfTestGetCases();
	RyanJsonFuzzerSelfTestReplaceCases();
	RyanJsonFuzzerSelfTestDuplicateCases();

	RyanJsonFuzzerMemFailPop(lastIsEnableMemFail);
}
