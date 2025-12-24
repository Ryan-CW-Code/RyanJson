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

// 是否支持未对齐访问,未定义时会根据平台选择，
// 一般不用管，如果你明白你的需求就自己定义
// true 表示支持未对齐访问
// false 表示不支持未对齐访问
// UINT8_MAX 标识让RyanJson自动判断，但可能会漏掉支持对齐访问的平台
#ifndef RyanJsonUnalignedAccessSupported
#define RyanJsonUnalignedAccessSupported UINT8_MAX
#endif

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

/**
 * @brief 判断是否支持未对齐访问
 *
 */
#if UINT8_MAX == RyanJsonUnalignedAccessSupported
#undef RyanJsonUnalignedAccessSupported

// Cortex-M0/M0+/M1 属于 ARMv6-M
#if defined(__ARM_ARCH_6M__)
#define RyanJsonUnalignedAccessSupported false

// Cortex-M3/M4/M7 属于 ARMv7-M/EM
#elif defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#define RyanJsonUnalignedAccessSupported true

// Cortex-M23/M33 属于 ARMv8-M
#elif defined(__ARM_ARCH_8M_BASE__) || defined(__ARM_ARCH_8M_MAIN__)
#define RyanJsonUnalignedAccessSupported true

// Cortex-A/R 属于 ARMv7-A/R
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__)
#define RyanJsonUnalignedAccessSupported true

// ARM9/ARM11 等老核
#elif defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5E__)
#define RyanJsonUnalignedAccessSupported false

// ARMv8-A / ARM64
#elif defined(__aarch64__) || defined(__ARM_ARCH_8A__) || defined(__ARM_ARCH_9__)
#define RyanJsonUnalignedAccessSupported true

// RISC-V MCU 默认不支持未对齐访问
#elif defined(__riscv)
#define RyanJsonUnalignedAccessSupported false

// x86 / x86-64
#elif defined(__i386__) || defined(__x86_64__)
#define RyanJsonUnalignedAccessSupported true

// MIPS
#elif defined(__mips__)
#define RyanJsonUnalignedAccessSupported false

// PowerPC
#elif defined(__powerpc__) || defined(__ppc__)
#define RyanJsonUnalignedAccessSupported false

// SPARC
#elif defined(__sparc__)
#define RyanJsonUnalignedAccessSupported false

// SuperH
#elif defined(__sh__)
#define RyanJsonUnalignedAccessSupported false

// Alpha
#elif defined(__alpha__)
#define RyanJsonUnalignedAccessSupported true

// Itanium
#elif defined(__ia64__)
#define RyanJsonUnalignedAccessSupported false

#else
// 默认认为不支持未对齐访问
#define RyanJsonUnalignedAccessSupported false
#endif

#endif // UINT8_MAX == RyanJsonUnalignedAccessSupported

#if true != RyanJsonUnalignedAccessSupported
#define RyanJsonAlign sizeof(void *)
#else
#define RyanJsonAlign sizeof(uint8_t)
#endif

#ifdef __cplusplus
}
#endif

#endif
