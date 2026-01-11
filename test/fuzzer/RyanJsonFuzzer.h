#ifndef RyanJsonFuzzer_h
#define RyanJsonFuzzer_h

#ifdef __cplusplus
extern "C" {
#endif

#include "RyanJsonTest.h"
#include <signal.h>

/**
 * @brief 公共宏定义
 */
#define RyanJsonCheckGotoExit(EX)                                                                                                          \
	RyanJsonCheckCode(EX, {                                                                                                            \
		result = RyanJsonFalse;                                                                                                    \
		goto exit__;                                                                                                               \
	})

/**
 * @brief 全局变量声明
 */
extern RyanJsonBool_e isEnableRandomMemFail;

/**
 * @brief 内存钩子函数
 */
extern void *RyanJsonFuzzerMalloc(size_t size);
extern void RyanJsonFuzzerFree(void *block);
extern void *RyanJsonFuzzerRealloc(void *block, size_t size);

/**
 * @brief 解析打印测试函数
 */
extern RyanJsonBool_e RyanJsonFuzzerTestParse(RyanJson_t pJson, const char *data, uint32_t size);
extern RyanJsonBool_e RyanJsonFuzzerTestMinify(const char *data, uint32_t size);

/**
 * @brief 复制比较测试函数
 */
extern RyanJsonBool_e RyanJsonFuzzerTestDuplicate(RyanJson_t pJson);

/**
 * @brief 修改操作测试函数
 */
extern RyanJsonBool_e RyanJsonFuzzerTestModify(RyanJson_t pJson, uint32_t size);
extern RyanJsonBool_e RyanJsonFuzzerTestGet(RyanJson_t pJson, uint32_t size);
extern RyanJsonBool_e RyanJsonFuzzerVerifyGet(RyanJson_t lastJson, RyanJson_t pJson, uint32_t index, uint32_t size);

/**
 * @brief 创建操作测试函数
 */
extern RyanJson_t RyanJsonFuzzerCreateRandomNode(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonFuzzerTestCreate(RyanJson_t pJson, uint32_t size);
extern RyanJsonBool_e RyanJsonFuzzerTestReplace(RyanJson_t pJson, uint32_t size);
extern RyanJsonBool_e RyanJsonFuzzerTestDetach(RyanJson_t pJson, uint32_t size);
extern RyanJsonBool_e RyanJsonFuzzerTestDelete(RyanJson_t pJson, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif
