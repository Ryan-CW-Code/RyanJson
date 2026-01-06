#ifndef RyanJsonConfig
#define RyanJsonConfig

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <inttypes.h>

#ifdef RT_VER_NUM
#include "rtthread.h"
#define RyanJsonMemset             rt_memset
#define RyanJsonMemcpy             rt_memcpy
// 平台没有memove或者memove实现性能低的，也可以注释掉这个宏,交给RyanJson内部实现的memmove,RyanJson内部会尽量的使用memcpy
#define RyanJsonMemmove            rt_memmove
#define RyanJsonStrlen             rt_strlen
#define RyanJsonStrcmp             rt_strcmp
#define RyanJsonSnprintf           rt_snprintf
#define RyanJsonPlatformAssert(EX) RT_ASSERT(EX)
#define RyanJsonMallocHeaderSize   12U
#define RyanJsonMallocAlign        (uint32_t)(RT_ALIGN_SIZE)
#else
#include <assert.h>
#define RyanJsonMemset             memset
#define RyanJsonMemcpy             memcpy
#define RyanJsonMemmove            memmove
#define RyanJsonStrlen             strlen
#define RyanJsonStrcmp             strcmp
#define RyanJsonSnprintf           snprintf
#define RyanJsonPlatformAssert(EX) assert(EX)
#define RyanJsonMallocHeaderSize   8U
#define RyanJsonMallocAlign        4U
#endif

// 是否启用assert
// #define RyanJsonEnableAssert

#ifndef RyanJsonMallocAlign
#define RyanJsonMallocAlign 8U
#endif

//
#ifndef RyanJsonMallocHeaderSize
#define RyanJsonMallocHeaderSize 8U
#endif

// 限制解析数组/对象中嵌套的深度
// RyanJson使用递归 序列化/反序列化 json
// 请根据单片机资源合理设置以防止堆栈溢出。
#ifndef RyanJsonNestingLimit
#define RyanJsonNestingLimit 500U
#endif

// 当 RyanJsonPrint 剩余缓冲空间不足时申请的空间大小
#ifndef RyanJsonPrintfPreAlloSize
#define RyanJsonPrintfPreAlloSize (64U)
#endif

/**
 * @brief 调试相关配置
 *
 */
// #define jsonLog(fmt, ...) printf("%s:%d " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define jsonLog(...)

#ifdef RyanJsonEnableAssert
#define RyanJsonAssert(EX) RyanJsonPlatformAssert(EX)
#else
#define RyanJsonAssert(EX) (void)(EX)
#endif

/**
 * @brief 检查宏是否合法
 *
 */
#if RyanJsonMallocHeaderSize < 4
#error "RyanJsonMallocHeaderSize 必须大于或等于4"
#endif

#if RyanJsonMallocAlign % 4 != 0
#error "RyanJsonMallocAlign 必须是4的倍数"
#endif

#ifdef __cplusplus
}
#endif

#endif
