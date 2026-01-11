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
#define RyanJsonStrlen             rt_strlen
#define RyanJsonStrcmp             rt_strcmp
#define RyanJsonSnprintf           rt_snprintf
#define RyanJsonPlatformAssert(EX) RT_ASSERT(EX)
#define RyanJsonMallocHeaderSize   12U
#define RyanJsonMallocAlign        RT_ALIGN_SIZE
#else
#include <assert.h>
#define RyanJsonMemset             memset
#define RyanJsonMemcpy             memcpy
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
#define RyanJsonNestingLimit 300U
#endif

// 当 RyanJsonPrint 剩余缓冲空间不足时申请的空间大小
#ifndef RyanJsonPrintfPreAlloSize
#define RyanJsonPrintfPreAlloSize (64U)
#endif

// 浮点数比较可调的绝对容差,一般场景 1e-8 足够了.
// 可以根据自己需求进行调整
// 容差必须大于 0.0,同时小于 1.0
#ifndef RyanJsonAbsTolerance
#define RyanJsonAbsTolerance 1e-8
#endif

// 用户需声明目标平台的 snprintf 是否支持科学计数法输出, (库内部是否使用 %g/%e)
#ifndef RyanJsonSnprintfSupportScientific
#define RyanJsonSnprintfSupportScientific false
#endif

// 用户需声明 double 序列化时的缓冲区大小
// 如果 snprintf 支持科学计数法，建议值 ≥ 32
// 如果不支持科学计数法，建议值 ≥ 330
#ifndef RyanJsonDoubleBufferSize
#if true == RyanJsonSnprintfSupportScientific
#define RyanJsonDoubleBufferSize 32
#else
// 不支持科学计数法的平台使用 ".17lf" 最大将会输出 330+ 字节的数据,对于此库来说占用太高了.
// 如果用户可以接受,就修改 RyanJsonDoubleBufferSize 宏,由你来决定RyanJson给 ".17lf" 提供多大缓冲区
#define RyanJsonDoubleBufferSize 64
#endif
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
#if 0 != RyanJsonMallocHeaderSize && RyanJsonMallocHeaderSize % 4 != 0
#error "RyanJsonMallocHeaderSize 必须是4的倍数"
#endif

#if 0 != RyanJsonMallocAlign && RyanJsonMallocAlign % 4 != 0
#error "RyanJsonMallocAlign 必须是4的倍数"
#endif

#if RyanJsonDoubleBufferSize < 8
#error "RyanJsonDoubleBufferSize 必须大于8"
#endif

#ifdef __cplusplus
}
#endif

#endif
