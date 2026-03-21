#ifndef RyanJsonInternal_h
#define RyanJsonInternal_h

#ifdef __cplusplus
extern "C" {
#endif

#include "RyanJson.h"

#ifndef RyanJsonInternalApi
#define RyanJsonInternalApi extern
#endif

#define RyanJsonFlagSize               sizeof(uint8_t)
#define RyanJsonKeyFeidLenMaxSize      sizeof(uint32_t)
#define RyanJsonAlign(size, align)     (((size) + (align) - 1) & ~((align) - 1))
#define RyanJsonAlignDown(size, align) ((size) & ~((align) - 1))
#define _checkType(info, type)         (RyanJsonGetType(info) == (type))
#define RyanJsonUnused(x)              (void)(x)

#ifndef RyanJsonInlineStringSize
/**
 * @brief RyanJsonInlineStringSize 默认值（单位：字节，与历史版本等价）。
 * @details
 * 该宏用于计算“节点内联字符串区可用容量”（不含 flag 字节）。
 *
 * 当前实现：
 * rawSize = sizeof(void*) - RyanJsonFlagSize
 *         + sizeof(void*)
 *         + (RyanJsonMallocHeaderSize / 2)
 *         + RyanJsonFlagSize
 * inlineSize = RyanJsonAlign(rawSize, RyanJsonMallocAlign) - RyanJsonFlagSize
 *
 * 各项用途：
 * - sizeof(void*) - RyanJsonFlagSize：
 *   flag 占用一个字节，32 位 4 字节对齐场景下，减去 flag 占用的 1 字节后，剩余部分正好是一个指针槽的净可用载荷。
 * - + sizeof(void*)：
 *   char * 最少占用一个指针大小，所以再加一个指针大小
 * - + (RyanJsonMallocHeaderSize / 2)：
 *   再加上半个 header 大小的补偿，内存占用和字节内联的一个平衡点
 * - + RyanJsonFlagSize：
 *   在对齐前把前面扣掉的 flag 补回，便于统一按总尺寸做 align。
 * - RyanJsonAlign(..., RyanJsonMallocAlign)：
 *   将总尺寸按 RyanJsonMallocAlign 向上对齐。
 * - - RyanJsonFlagSize：
 *   对齐后再扣掉 flag，得到最终“字符串可用字节数”。
 *
 * 其中 rawSize 里的 `-RyanJsonFlagSize` 与 `+RyanJsonFlagSize` 会抵消，
 * 因此与历史公式完全等价：
 * inlineSize = RyanJsonAlign(2 * sizeof(void*) + (RyanJsonMallocHeaderSize / 2), RyanJsonMallocAlign)
 *            - RyanJsonFlagSize
 *
 * 32 位示例（sizeof(void *)=4, RyanJsonFlagSize=1, RyanJsonMallocAlign=4）：
 * - RyanJsonMallocHeaderSize=8:  Align(12, 4) - 1 = 11
 * - RyanJsonMallocHeaderSize=12: Align(14, 4) - 1 = 15
 *
 * 64 位示例（sizeof(void *)=8, RyanJsonFlagSize=1, RyanJsonMallocAlign=8）：
 * - RyanJsonMallocHeaderSize=12: Align(22, 8) - 1 = 23
 */
#define RyanJsonInlineStringSize                                                                                                           \
	(RyanJsonAlign((sizeof(void *) - RyanJsonFlagSize + sizeof(void *) + (RyanJsonMallocHeaderSize / 2) + RyanJsonFlagSize),           \
		       RyanJsonMallocAlign) -                                                                                              \
	 RyanJsonFlagSize)
// static uint32_t RyanJsonInlineStringSize(void)
// {
// 	// 先计算“两个指针槽 + 补偿值”的基础尺寸（暂不做对齐）
// 	uint32_t baseSize = sizeof(void *) - RyanJsonFlagSize;     // 一个指针槽，先扣掉 flag
// 	baseSize += sizeof(void *) + RyanJsonMallocHeaderSize / 2; // 再加一个指针槽和半个 header 补偿
// 	// 再对齐：对齐前把 flag 加回，对齐后再减回，得到最终可用字节数
// 	return (uint32_t)(RyanJsonAlign(baseSize + RyanJsonFlagSize, RyanJsonMallocAlign) - RyanJsonFlagSize);
// }
#endif

// 该结构字段语义需与 struct RyanJsonNode 保持一致
typedef struct
{
	const char *key;
	const char *strValue;

	RyanjsonType_e type;
	RyanJsonBool_e boolIsTrueFlag;
	RyanJsonBool_e numberIsDoubleFlag;
} RyanJsonNodeInfo_t;

RyanJsonInternalApi RyanJsonMalloc_t jsonMalloc;
RyanJsonInternalApi RyanJsonFree_t jsonFree;
RyanJsonInternalApi RyanJsonRealloc_t jsonRealloc;

RyanJsonInternalApi uint8_t *RyanJsonInternalGetStrPtrModeBuf(RyanJson_t pJson);
RyanJsonInternalApi void RyanJsonInternalSetStrPtrModeBuf(RyanJson_t pJson, uint8_t *heapPtr);
RyanJsonInternalApi uint8_t *RyanJsonInternalGetStrPtrModeBufAt(RyanJson_t pJson, uint32_t index);
RyanJsonInternalApi uint8_t RyanJsonInternalDecodeKeyLenField(uint8_t encoded);
RyanJsonInternalApi uint8_t RyanJsonInternalCalcLenBytes(uint32_t len);
RyanJsonInternalApi uint32_t RyanJsonInternalGetKeyLen(RyanJson_t pJson);
RyanJsonInternalApi void *RyanJsonInternalGetValue(RyanJson_t pJson);

RyanJsonInternalApi RyanJson_t RyanJsonInternalNewNode(RyanJsonNodeInfo_t *info);
RyanJsonInternalApi void RyanJsonInternalListInsertAfter(RyanJson_t parent, RyanJson_t prev, RyanJson_t item);
RyanJsonInternalApi RyanJson_t RyanJsonInternalGetParent(RyanJson_t pJson);
RyanJsonInternalApi RyanJsonBool_e RyanJsonInternalChangeString(RyanJson_t pJson, RyanJsonBool_e isNew, const char *key,
								const char *strValue);
RyanJsonInternalApi RyanJson_t RyanJsonInternalCreateObjectAndKey(const char *key);
RyanJsonInternalApi RyanJson_t RyanJsonInternalCreateArrayAndKey(const char *key);
/**
 * @brief 内部接口：仅用于容器 children 指针改写。
 * @note 调用方需保证 pJson 为 Array/Object 且非 NULL。
 */
RyanJsonInternalApi RyanJsonBool_e RyanJsonInternalChangeObjectValue(RyanJson_t pJson, RyanJson_t objValue);
RyanJsonInternalApi RyanJsonBool_e RyanJsonInternalStrEq(const char *s1, const char *s2);
RyanJsonInternalApi void *RyanJsonInternalExpandRealloc(void *block, uint32_t oldSize, uint32_t newSize); // 跨模块使用时保留

RyanJsonInternalApi RyanJsonBool_e RyanJsonInternalParseDoubleRaw(const uint8_t *currentPtr, uint32_t remainSize, double *numberValuePtr);

#ifdef RyanJsonLinuxTestEnv
#undef RyanJsonSnprintf
RyanJsonInternalApi RyanJsonBool_e RyanJsonFuzzerShouldFail(uint32_t probability);
RyanJsonInternalApi int32_t RyanJsonSnprintf(char *buf, size_t size, const char *fmt, ...);
RyanJsonInternalApi uint32_t RyanJsonRandRange(uint32_t min, uint32_t max);
#endif

#ifdef __cplusplus
}
#endif

#endif
