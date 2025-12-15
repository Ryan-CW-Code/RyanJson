#ifndef __RyanJsonConfig__
#define __RyanJsonConfig__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RT_VER_NUM
#define RyanJsonMemset             rt_memset
#define RyanJsonMemcpy             rt_memcpy
#define RyanJsonStrlen             rt_strlen
#define RyanJsonStrcmp             rt_strcmp
#define RyanJsonSnprintf           rt_snprintf
#define RyanJsonPlatformAssert(EX) RT_ASSERT(EX)
#else
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <inttypes.h>
#include <assert.h>
#include "valloc.h"
#define RyanJsonMemset             memset
#define RyanJsonMemcpy             memcpy
#define RyanJsonStrlen             strlen
#define RyanJsonStrcmp             strcmp
#define RyanJsonSnprintf           snprintf
#define RyanJsonPlatformAssert(EX) assert(EX)
#endif

// 是否启用assert
// #define RyanJsonEnableAssert

// 是否支持未对齐访问,未定义时会根据平台选择，
// 一般不用管，如果你明白你的需求就自己定义
// #define RyanJsonAlignUnalignedAccessSupported 0

// 限制解析数组/对象中嵌套的深度
// RyanJson使用递归 序列化/反序列化 json
// 请根据单片机资源合理设置以防止堆栈溢出。
#ifndef RyanJsonNestingLimit
#define RyanJsonNestingLimit 900000000
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
 * @brief 判断是否支持未对齐访问
 *
 */
#ifndef RyanJsonAlignUnalignedAccessSupported

#if defined(__ARM_ARCH_6M__) // Cortex-M0/M0+/M1 属于 ARMv6-M
#define RyanJsonAlignUnalignedAccessSupported 0
#elif defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__) // Cortex-M3/M4/M7 属于 ARMv7-M/EM
#define RyanJsonAlignUnalignedAccessSupported 1
#elif defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__) // Cortex-M23/M33 属于 ARMv8-M
#define RyanJsonAlignUnalignedAccessSupported 1
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) // Cortex-A/R 属于 ARMv7-A/R
#define RyanJsonAlignUnalignedAccessSupported 1
#elif defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5E__) // ARM9/ARM11 等老核
#define RyanJsonAlignUnalignedAccessSupported 0
#elif defined(__riscv) // RISC-V MCU 默认不支持未对齐访问
#define RyanJsonAlignUnalignedAccessSupported 0
#elif defined(__i386__) || defined(__x86_64__) // x86 / x86-64
#define RyanJsonAlignUnalignedAccessSupported 1
#else
// 默认认为不支持未对齐访问
#define RyanJsonAlignUnalignedAccessSupported 0
#endif

#endif // RyanJsonAlignUnalignedAccessSupported

#if 0 == RyanJsonAlignUnalignedAccessSupported
#define RyanJsonAlign sizeof(void *)
#else
#define RyanJsonAlign sizeof(uint8_t)
#endif

#ifdef __cplusplus
}
#endif

#endif
