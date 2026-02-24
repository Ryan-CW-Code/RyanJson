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
 * - Driver（`fuzzerDriver`）：负责状态管理、伪随机数生成和确定性控制。
 * - Runner（`entry.c`）：作为 `LLVMFuzzerTestOneInput` 入口，负责初始化、模式分发和清理。
 * - Case（`cases/`）：承载具体测试逻辑，覆盖 Parse/Create/Modify/Structure 等模块。
 *
 * 确定性与可复现性：
 * - 以输入数据为种子驱动 PRNG，保证同一输入触发同一随机路径。
 * - `FuzzerState` 记录当前种子、输入缓冲区和读取位置。
 * - 通过 PRNG 控制内存失败注入，稳定覆盖 OOM 路径。
 *
 * 分发策略：
 * - 若首字节为 `0xFF`，进入 API 序列模式（预留扩展）。
 * - 其他输入进入解析模式，执行 Parse/Minify 与后续结构化变异测试。
 * - 该策略对普通 Json corpus 透明兼容。
 *
 * 目录说明：
 * - `runner/`：入口调度代码。
 * - `cases/`：各类测试用例。
 * - `utils/`：内存钩子、生成器、驱动实现。
 * - `include/`：公共声明。
 */

/**
 * @brief Fuzzer 运行状态上下文
 * 用于在各个测试用例间传递状态，保证无全局副作用，实现可重入和确定性。
 */
typedef struct
{
	uint32_t seed;        // PRNG 种子，由 Input Data 初始化
	const uint8_t *data;  // 原始输入数据指针
	size_t size;          // 原始输入数据大小
	size_t pos;           // 当前读取位置
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

#define fuzzTestWithMemFail(func)                                                                                                          \
	do                                                                                                                                 \
	{                                                                                                                                  \
		uint32_t lastIsEnableMemFail = g_fuzzerState.isEnableMemFail;                                                              \
		g_fuzzerState.isEnableMemFail = false;                                                                                     \
		func;                                                                                                                      \
		g_fuzzerState.isEnableMemFail = true;                                                                                      \
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
 * @brief 解析与基础功能测试
 * 覆盖 Json 解析、压缩和基础打印功能。
 */
extern RyanJsonBool_e RyanJsonFuzzerTestParse(RyanJson_t pJson, const char *data, uint32_t size);
extern RyanJsonBool_e RyanJsonFuzzerTestMinify(const char *data, uint32_t size);

/**
 * @brief 深度复制与比较测试
 * 验证复制结果与原对象的一致性。
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

#ifdef __cplusplus
}
#endif

#endif
