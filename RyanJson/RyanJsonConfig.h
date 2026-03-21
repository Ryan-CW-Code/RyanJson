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
#include <stdarg.h>

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
#define RyanJsonMallocAlign        8U
#endif

/**
 * @brief RyanJsonEnableAssert: 启用库内断言（RyanJsonAssert / RyanJsonCheckAssert）。
 * @note 默认关闭（注释状态）。
 */
// #define RyanJsonEnableAssert

/**
 * @brief RyanJsonMallocAlign: 内存对齐粒度（字节）。
 * @note 默认值为 8（平台未提前定义时）。
 * @note 必须是 4 的倍数。
 */
#ifndef RyanJsonMallocAlign
#define RyanJsonMallocAlign 8U
#endif

/**
 * @brief RyanJsonMallocHeaderSize: 分配器头部开销（字节），用于内联阈值估算。
 * @note 默认值为 8（平台未提前定义时）。
 * @note 必须是 4 的倍数。
 */
#ifndef RyanJsonMallocHeaderSize
#define RyanJsonMallocHeaderSize 8U
#endif

/**
 * @brief RyanJsonPrintfPreAlloSize: RyanJsonPrint 缓冲区不足时，每次扩容的预分配大小。
 * @note 默认值为 64 字节。
 */
#ifndef RyanJsonPrintfPreAlloSize
#define RyanJsonPrintfPreAlloSize (64U)
#endif

/**
 * @brief RyanJsonAbsTolerance: 浮点比较的绝对容差。
 * @note 默认值为 1e-8（常见场景足够）。
 * @note 建议范围为 (0.0, 1.0)。
 */
#ifndef RyanJsonAbsTolerance
#define RyanJsonAbsTolerance 1e-8
#endif

/**
 * @brief RyanJsonStrictObjectKeyCheck: 控制 Object 是否允许重复 key。
 * @note true 时 Parse/Insert/ReplaceByIndex 拒绝重复 key。
 * @note false 时允许重复 key，但按 key 的 API 通常只命中第一个节点。
 * @note 默认值为 false。
 */
#ifndef RyanJsonStrictObjectKeyCheck
#define RyanJsonStrictObjectKeyCheck false
#endif

/**
 * @brief RyanJsonDefaultAddAtHead: 控制 Add 系列接口（Array/Object）的默认插入方向。
 * @note false 为尾插（保持业务顺序，超大链表时查尾为 O(N)）。
 * @note true 为头插（O(1)，但遍历/打印时新元素会在前面）。
 * @note 默认值为 false。
 */
#ifndef RyanJsonDefaultAddAtHead
#define RyanJsonDefaultAddAtHead false
#endif

/**
 * @brief RyanJsonSnprintfSupportScientific: 声明目标平台 snprintf 是否支持科学计数法（%g/%e）。
 * @note 该配置会影响 Double 序列化策略与 RyanJsonDoubleBufferSize 默认值。
 * @note 默认值为 true。
 */
#ifndef RyanJsonSnprintfSupportScientific
#define RyanJsonSnprintfSupportScientific true
#endif

/**
 * @brief RyanJsonDoubleBufferSize: Double 序列化临时缓冲区大小。
 * @note 若支持科学计数法：默认 32（建议 >= 32）。
 * @note 若不支持科学计数法：默认 64（理论上完整输出可到 330+，可按需求增大）。
 */
#ifndef RyanJsonDoubleBufferSize
#if true == RyanJsonSnprintfSupportScientific
#define RyanJsonDoubleBufferSize 32
#else
// 不支持科学计数法的平台使用 ".17lf" 最大将会输出 330+ 字节的数据，对于此库来说占用太高。
// 如果用户可以接受，就修改 RyanJsonDoubleBufferSize 宏，由你来决定 RyanJson 给 ".17lf" 提供多大缓冲区。
#define RyanJsonDoubleBufferSize 64
#endif
#endif

/**
 * @brief jsonLog: 内部调试日志钩子。
 * @note 默认为空实现。
 * @note 可按需取消下一行注释替换为 printf 等实现。
 */
// #define jsonLog(fmt, ...) printf("%s:%d " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define jsonLog(...)

#ifdef RyanJsonEnableAssert
#define RyanJsonAssert(EX) RyanJsonPlatformAssert(EX)
#else
#define RyanJsonAssert(EX)
#endif

/**
 * @brief 配置合法性检查（编译期）。
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

#if true != RyanJsonStrictObjectKeyCheck && false != RyanJsonStrictObjectKeyCheck
#error "RyanJsonStrictObjectKeyCheck 必须是 true 或 false"
#endif

#if true != RyanJsonDefaultAddAtHead && false != RyanJsonDefaultAddAtHead
#error "RyanJsonDefaultAddAtHead 必须是 true 或 false"
#endif

/**
 * @brief RyanJsonInlineStringSize: key/短字符串内联阈值（单位：字节）。
 * @note 用户可在包含 RyanJson.h 前自行定义。
 * @note C99 预处理器无法对包含 sizeof 的表达式做 #if 校验。
 */
#ifdef RyanJsonInlineStringSize
#if RyanJsonInlineStringSize <= 1U
#error "RyanJsonInlineStringSize 必须大于1"
#endif
#endif

/**
 * @brief RyanJsonAddPosition: Add 系列接口的默认插入索引（用于 RyanJsonInsert）。
 * @note 0 表示头插，UINT32_MAX 表示尾插（按追加语义处理）。
 * @note 默认值由 RyanJsonDefaultAddAtHead 自动推导。
 */
#ifndef RyanJsonAddPosition
#if true == RyanJsonDefaultAddAtHead
#define RyanJsonAddPosition 0U
#else
#define RyanJsonAddPosition UINT32_MAX
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
