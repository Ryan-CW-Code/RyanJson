#ifndef RyanJsonFuzzer_h
#define RyanJsonFuzzer_h

#ifdef __cplusplus
extern "C" {
#endif

#include "testCommon.h"
#include "RyanJsonInternal.h"
#include <signal.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief RyanJson Fuzzer 架构说明
 *
 * 架构分层：
 * - Driver（`utils/fuzzerDriver.c`）：负责状态初始化、PRNG 与故障注入概率控制。
 * - Entry（`test/fuzzer/entry.c`）：作为 `LLVMFuzzerTestOneInput` 入口，负责初始化和主流程调度。
 * - SelfTest（`utils/fuzzerSelfTest.c`）：集中放置一次性确定性断言，避免每轮 fuzz 重复执行。
 * - Cases（`cases/`）：承载与输入相关的运行期测试逻辑。
 *
 * 当前原则：
 * - 热路径只保留与当前输入相关的行为。
 * - 与输入无关的防御分支覆盖移到 self-test。
 * - 只有在“运行期结构上很难稳定命中”时，才允许最小范围的确定性补齐。
 * - PRNG 仍由输入驱动，保证同一输入触发同一路径。
 */

/**
 * @brief Fuzzer 运行状态上下文
 *
 * 只保留运行期真正需要的全局状态，避免挂未使用的输入缓存字段。
 */
typedef struct
{
	uint32_t seed;        // PRNG 种子，由当前输入混合初始化
	bool isEnableMemFail; // 是否启用内存分配失败
} RyanJsonFuzzerState;

/**
 * @brief 驱动层接口
 * 定义在 utils/fuzzerDriver.c
 */
extern void RyanJsonFuzzerInit(const uint8_t *data, size_t size);
extern uint32_t RyanJsonFuzzerNextRand();
extern RyanJsonBool_e RyanJsonFuzzerShouldFail(uint32_t probability);

/**
 * @brief 公共宏定义
 */
#define RyanJsonCheckGotoExit(EX)                                                                                                          \
	RyanJsonCheckCode(EX, {                                                                                                            \
		result = RyanJsonFalse;                                                                                                    \
		goto exit__;                                                                                                               \
	})

/*
 * 临时关闭随机 OOM，执行需要稳定断言的代码块。
 * 这个宏只用于“已经确定要验证某条路径的后置状态”。
 * 不用于把运行期理论可达的路径整体搬进 self-test。
 */
#define fuzzTestWithMemFail(func)                                                                                                          \
	do                                                                                                                                 \
	{                                                                                                                                  \
		RyanJsonBool_e lastIsEnableMemFail;                                                                                        \
		RyanJsonFuzzerMemFailPush(lastIsEnableMemFail, RyanJsonFalse);                                                             \
		func;                                                                                                                      \
		RyanJsonFuzzerMemFailPop(lastIsEnableMemFail);                                                                             \
	} while (0)

/**
 * @brief 全局变量声明
 */
extern RyanJsonFuzzerState g_fuzzerState;

/**
 * @brief 内存钩子函数
 */
extern void *RyanJsonFuzzerMalloc(size_t size);
extern void RyanJsonFuzzerFree(void *block);
extern void *RyanJsonFuzzerRealloc(void *block, size_t size);

/**
 * @brief Fuzzer 入口自检（一次性）
 */
extern void RyanJsonFuzzerSelfTestOnce(void);
extern void RyanJsonFuzzerSelfTestParseCases(void);
extern void RyanJsonFuzzerSelfTestMinifyCases(void);
extern void RyanJsonFuzzerSelfTestCreateCases(void);
extern void RyanJsonFuzzerSelfTestModifyCases(void);
extern void RyanJsonFuzzerSelfTestGetCases(void);
extern void RyanJsonFuzzerSelfTestReplaceCases(void);
extern void RyanJsonFuzzerSelfTestDuplicateCases(void);

/**
 * @brief 解析与基础功能测试
 * 覆盖 Json 解析、压缩和基础打印功能。
 */
extern RyanJsonBool_e RyanJsonFuzzerTestParse(RyanJson_t pJson, const char *data, uint32_t size);
extern RyanJsonBool_e RyanJsonFuzzerTestMinify(const char *data, uint32_t size);

/**
 * @brief 深度复制与比较测试
 * 验证复制结果与原 Object 的一致性。
 */
extern RyanJsonBool_e RyanJsonFuzzerTestDuplicate(RyanJson_t pJson);

/**
 * @brief 结构修改与访问测试
 * 测试对 Json 结构的修改（改值、改 key）以及数据访问安全性。
 */
extern RyanJsonBool_e RyanJsonFuzzerTestModify(RyanJson_t pJson, uint32_t size);
extern RyanJsonBool_e RyanJsonFuzzerTestGet(RyanJson_t pJson, uint32_t size);
extern RyanJsonBool_e RyanJsonFuzzerVerifyGet(RyanJson_t lastJson, RyanJson_t pJson, uint32_t index, uint32_t size);

/**
 * @brief 结构变异与增删测试
 * 测试节点的创建、替换、分离和删除，模拟复杂的 DOM 操作序列。
 */
extern RyanJson_t RyanJsonFuzzerCreateRandomNode(RyanJson_t pJson);
extern RyanJson_t RyanJsonFuzzerCreateRandomNodeWithKey(RyanJson_t pJson, const char *key);
extern RyanJsonBool_e RyanJsonFuzzerTestCreate(RyanJson_t pJson, uint32_t size);
extern RyanJsonBool_e RyanJsonFuzzerTestReplace(RyanJson_t pJson, uint32_t size);
extern RyanJsonBool_e RyanJsonFuzzerTestDetach(RyanJson_t pJson, uint32_t size);
extern RyanJsonBool_e RyanJsonFuzzerTestDelete(RyanJson_t pJson, uint32_t size);

/* 保存当前故障注入开关，并切换到新状态。 */
#define RyanJsonFuzzerMemFailPush(saved, enable)                                                                                           \
	do                                                                                                                                 \
	{                                                                                                                                  \
		(saved) = g_fuzzerState.isEnableMemFail;                                                                                   \
		g_fuzzerState.isEnableMemFail = (enable);                                                                                  \
	} while (0)

/* 恢复调用前的故障注入状态，避免影响后续用例。 */
#define RyanJsonFuzzerMemFailPop(saved)                                                                                                    \
	do                                                                                                                                 \
	{                                                                                                                                  \
		g_fuzzerState.isEnableMemFail = (saved);                                                                                   \
	} while (0)

#ifdef __cplusplus
}
#endif

#endif
