#ifndef __RyanJsonConfig__
#define __RyanJsonConfig__

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
#else
#include <assert.h>
#define RyanJsonMemset             memset
#define RyanJsonMemcpy             memcpy
#define RyanJsonStrlen             strlen
#define RyanJsonStrcmp             strcmp
#define RyanJsonSnprintf           snprintf
#define RyanJsonPlatformAssert(EX) assert(EX)
#endif

// 是否启用assert
// #define RyanJsonEnableAssert

// 限制解析数组/对象中嵌套的深度
// RyanJson使用递归 序列化/反序列化 json
// 请根据单片机资源合理设置以防止堆栈溢出。
#ifndef RyanJsonNestingLimit
#define RyanJsonNestingLimit 3000
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

#ifdef __cplusplus
}
#endif

#endif
