#ifndef RyanJson
#define RyanJson

#ifdef __cplusplus
extern "C" {
#endif

#include "RyanJsonConfig.h"

/**
 * @brief Json 错误处理块
 *
 */
#define RyanJsonCheckCodeNoReturn(EX, code)                                                                                                \
	if (!(EX))                                                                                                                         \
	{                                                                                                                                  \
		jsonLog("\r\n%s:%d Check failed: %s\n", __FILE__, __LINE__, #EX);                                                          \
		code                                                                                                                       \
	}

#define RyanJsonCheckCode(EX, code) RyanJsonCheckCodeNoReturn(EX, code)

#define RyanJsonCheckReturnFalse(EX) RyanJsonCheckCode(EX, return RyanJsonFalse;)
#define RyanJsonCheckReturnNull(EX)  RyanJsonCheckCode(EX, return NULL;)

// !没有使能assert时RyanJsonCheckAssert不执行的
#ifdef RyanJsonEnableAssert
#define RyanJsonCheckAssert(EX) RyanJsonCheckCode(EX, RyanJsonAssert(NULL &&#EX);)
#else
#define RyanJsonCheckAssert(EX) ((void)0)
#endif

// Json 的最基础节点，所有 Json 元素都由该节点表示。
// 结构体中仅包含固定的 next 指针，用于单向链表串联。
// 其余数据（flag、key、stringValue、numberValue、doubleValue 等）均通过动态内存分配管理。
struct RyanJsonNode
{
	struct RyanJsonNode *next; // 单链表节点指针

	/**
	 * @brief RyanJson 节点结构体
	 * 每个节点由链表连接，包含元数据标识 (Flag) 与动态载荷存储区。
	 *
	 * 内存布局：
	 * [ next指针 | flag(1字节) | padding/指针空间 | 动态载荷区 ]
	 *
	 * @brief 节点元数据标识 (Flag)
	 * 紧跟 next 指针后，利用 1 字节位域描述节点类型及存储状态。
	 *
	 * flag 位分布定义：
	 * bit7   bit6   bit5   bit4   bit3   bit2   bit1   bit0
	 * -----------------------------------------------------
	 * strMode KeyLen KeyLen HasKey NumExt Type2  Type1  Type0
	 *
	 * 各位含义：
	 * - bit0-2 : 节点类型
	 *            000=Unknown, 001=Null, 010=Bool, 011=Number,
	 *            100=String, 101=Array, 110=Object, 111=Reserved
	 *
	 * - bit3   : 扩展位
	 *            Bool 类型：0=false, 1=true
	 *            Number 类型：0=int(4字节), 1=double(8字节)
	 *
	 * - bit4   : 是否包含 Key
	 *            0=无 Key（数组元素）
	 *            1=有 Key（对象成员）
	 *
	 * - bit5-6 : Key 长度字段字节数
	 *            00=1字节 (≤UINT8_MAX)
	 *            01=2字节 (≤UINT16_MAX)
	 *            10=3字节 (≤UINT24_MAX)
	 *            11=4字节 (≤UINT32_MAX)
	 *
	 * - bit7   : 表示key / strValue 存储模式
	 *            0:inline 模式, 1=ptr 模式
	 *
	 * @brief 动态载荷存储区
     * 目的：
     * - 在保持 API 易用性和稳定性的同时，最大限度减少 malloc 调用次数。
     * - 尤其在嵌入式平台，malloc 代价高昂：不仅有堆头部空间浪费，还会产生内存碎片。
     * - 通过利用结构体内的对齐填充 (Padding) 和指针空间，形成一个灵活的缓冲区。
     *
     * 存储策略：
     * 利用结构体内存对齐产生的 Padding（如 Flag 后的空隙）以及原本用于存储指针的空间，形成一个缓冲区
     * 若节点包含 key / strValue，则可能有两种方案：
     * 1. inline 模式 (小数据优化)
     *    - 当 (KeyLen + Key + Value) 的总长度 ≤ 阈值时，直接存储在结构体内部。
     *    - 阈值计算公式：
     *        阈值 = Padding + sizeof(void*) + (malloc头部空间的一半)，再向上对齐到字节边界。
     *      举例：
     *        - 内存对齐：4字节
     *        - malloc头部空间：8字节
     *        - 可用空间 = 3 (flag后padding) + 4 (指针空间) + 4 (malloc头部一半)
     *        - 向上对齐后得到阈值12字节
     *    - 存储布局：
     *        [ KeyLen | Key | Value ]
     *      起始地址即为 flag 之后，数据紧凑排列，无需额外 malloc。
     *
     * 2. ptr 模式 (大数据)
     *    - 当数据长度 > 阈值时，结构体存储一个指针，指向独立的堆区。
     *    - 存储布局：
     *        [ KeyLen | *ptr ] -> (ptr指向) [ Key | Value ]
     *    - KeyLen 的大小由 flag 中的长度字段决定 (最多 4 字节)。
     *    - 这样保证大数据不会撑爆结构体，同时保持 API 一致性。

     * 其他类型的存储：
	 * - null / bool : 由 flag 位直接表示，无需额外空间。
	 * - number      : 根据 flag 扩展位决定存储 int(4字节) 或 double(8字节)。
	 * - object      : 动态分配空间存储子节点，采用链表结构。
     *
     * 设计考量：
     * - malloc 在嵌入式平台的开销：
     *    * RTT 最小内存管理算法中，malloc 头部约 12 字节(可以考虑tlsf算法头部空间仅4字节，内存碎片也控制的很好，适合物联网应用)。
     *    * 一个 RyanJson 节点本身可能只有个位数字节，头部空间就让内存占用翻倍。
     * - 因此：
     *    * 小数据尽量 inline 存储，避免二次 malloc。
     *    * 大数据 fallback 到 ptr 模式，保证灵活性。
     * - 修改场景：
     *    * 理想情况：节点结构体后面直接跟 key/strValue，修改时释放并重新申请节点。
     *    * 但这样 changKey/changStrValue 接口改动太大，用户层需要修改指针，代价高。
     *    * 实际策略：提供就地修改接口。
     *        - 若新值长度 ≤ 原有 inline 缓冲区，直接覆盖。
     *        - 若超过阈值，自动切换到 ptr 模式，用户层无需关心。
	 *
     * 链表结构示例：
     *   {
     *       "name": "RyanJson",
     *   next (
     *       "version": "xxx",
     *   next (
     *       "repository": "https://github.com/Ryan-CW-Code/RyanJson",
     *   next (
     *       "keywords": [
     *           "json",
     *       next (
     *           "streamlined",
     *       next (
     *           "parser"
     *       ))
     *       ],
     *   next (
     *       "others": { ... }
     *   }
	 */
};

typedef struct RyanJsonNode *RyanJson_t;

typedef enum
{
	// 类型标志 占用8字节,剩余一个备用
	RyanJsonTypeNull = 1,
	RyanJsonTypeBool = 2,
	RyanJsonTypeNumber = 3,
	RyanJsonTypeString = 4,
	RyanJsonTypeArray = 5,
	RyanJsonTypeObject = 6,
} RyanjsonType_e;

#define RyanJsonFalse (false)
#define RyanJsonTrue  (true)

// !兼容之前的类型定义
typedef bool RyanJsonBool_e;
typedef RyanJsonBool_e RyanJsonBool;

// 内存钩子函数
typedef void *(*RyanJsonMalloc_t)(size_t size);
typedef void (*RyanJsonFree_t)(void *block);
typedef void *(*RyanJsonRealloc_t)(void *block, size_t size);

/**
 * !!! 较底层接口, 不推荐用户使用，除非用户知道这些接口意义
 * !!! 一定要看这里，这里的接口不推荐使用
 */
#define RyanJsonGetMask(bits)                           (((1U << (bits)) - 1))
#define RyanJsonGetPayloadPtr(pJson)                    ((uint8_t *)(pJson) + sizeof(struct RyanJsonNode))
#define RyanJsonGetPayloadFlagField(pJson, shift, mask) (((*RyanJsonGetPayloadPtr(pJson)) >> (shift)) & (mask))
#define RyanJsonSetPayloadFlagField(pJson, shift, mask, value)                                                                             \
	((*RyanJsonGetPayloadPtr(pJson)) =                                                                                                 \
		 ((*RyanJsonGetPayloadPtr(pJson)) & ~((mask) << (shift))) | ((uint8_t)(((value) & (mask)) << (shift))))

#define RyanJsonGetType(pJson)       ((RyanjsonType_e)RyanJsonGetPayloadFlagField((pJson), 0, RyanJsonGetMask(3)))
#define RyanJsonSetType(pJson, type) (RyanJsonSetPayloadFlagField((pJson), 0, RyanJsonGetMask(3), (RyanjsonType_e)(type)))

// bool跟number用一个字段，因为bool和number类型不会同时存在
#define RyanJsonGetPayloadBoolValueByFlag(pJson)             RyanJsonGetPayloadFlagField((pJson), 3, RyanJsonGetMask(1))
#define RyanJsonSetPayloadBoolValueByFlag(pJson, value)      RyanJsonSetPayloadFlagField((pJson), 3, RyanJsonGetMask(1), (value))
#define RyanJsonGetPayloadNumberIsDoubleByFlag(pJson)        RyanJsonGetPayloadFlagField((pJson), 3, RyanJsonGetMask(1))
#define RyanJsonSetPayloadNumberIsDoubleByFlag(pJson, value) RyanJsonSetPayloadFlagField((pJson), 3, RyanJsonGetMask(1), (value))

#define RyanJsonGetPayloadWhiteKeyByFlag(pJson)        RyanJsonGetPayloadFlagField((pJson), 4, RyanJsonGetMask(1))
#define RyanJsonSetPayloadWhiteKeyByFlag(pJson, value) RyanJsonSetPayloadFlagField((pJson), 4, RyanJsonGetMask(1), (value))

// ! 使用超过8字节后一定要注意 RyanJsonSetPayloadFlagField 目前限制uint8_t类型
// flag空间不够的时候可以把这个字段弃用，用redis的listpack方法将key和keyLen一起表示，内存占用也挺好，但是复杂度高，有空间就保持现在这样
#define RyanJsonGetPayloadEncodeKeyLenByFlag(pJson)        ((uint8_t)RyanJsonGetPayloadFlagField((pJson), 5, RyanJsonGetMask(2)) + 1)
#define RyanJsonSetPayloadEncodeKeyLenByFlag(pJson, value) RyanJsonSetPayloadFlagField((pJson), 5, RyanJsonGetMask(2), (value))

#define RyanJsonGetPayloadStrIsPtrByFlag(pJson)        RyanJsonGetPayloadFlagField((pJson), 7, RyanJsonGetMask(1))
#define RyanJsonSetPayloadStrIsPtrByFlag(pJson, value) RyanJsonSetPayloadFlagField((pJson), 7, RyanJsonGetMask(1), (value))

extern RyanJsonBool_e RyanJsonInsert(RyanJson_t pJson, uint32_t index, RyanJson_t item);
/**
 * !!!上面的接口不推荐使用
 *
 */

/**
 * @brief json对象函数
 */
extern RyanJsonBool_e RyanJsonInitHooks(RyanJsonMalloc_t userMalloc, RyanJsonFree_t userFree, RyanJsonRealloc_t userRealloc);
extern RyanJson_t RyanJsonParseOptions(const char *text, uint32_t size, RyanJsonBool_e requireNullTerminator,
				       const char **parseEndPtr);                                             // 需用户释放内存
#define RyanJsonParse(text) RyanJsonParseOptions((text), (uint32_t)RyanJsonStrlen(text), RyanJsonFalse, NULL) // 需用户释放内存
extern void RyanJsonDelete(RyanJson_t pJson);
extern void RyanJsonFree(void *block);

/**
 * @brief 打印json对象函数
 */
extern char *RyanJsonPrint(RyanJson_t pJson, uint32_t preset, RyanJsonBool_e format, uint32_t *len); // 需用户释放内存
extern char *RyanJsonPrintPreallocated(RyanJson_t pJson, char *buffer, uint32_t length, RyanJsonBool_e format, uint32_t *len);

/**
 * @brief json杂项函数
 */
extern RyanJson_t RyanJsonDuplicate(RyanJson_t pJson); // 需用户释放内存
extern uint32_t RyanJsonMinify(char *text, int32_t textLen);
extern RyanJsonBool_e RyanJsonCompare(RyanJson_t leftJson, RyanJson_t rightJson);
extern uint32_t RyanJsonGetSize(RyanJson_t pJson);
#define RyanJsonGetArraySize(pJson) RyanJsonGetSize(pJson)

/**
 * @brief 添加相关函数
 */
extern RyanJson_t RyanJsonCreateObject(void);                                  // 如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateNull(const char *key);                         // 如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateBool(const char *key, RyanJsonBool_e boolean); // 如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateInt(const char *key, int32_t number);          // 如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateDouble(const char *key, double number);        // 如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateString(const char *key, const char *string);   // 如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateArray(void);                                   // 如果没有添加到父json, 则需释放内存

/**
 * @brief 分离相关函数
 */
extern RyanJson_t RyanJsonDetachByIndex(RyanJson_t pJson, uint32_t index); // 需用户释放内存
extern RyanJson_t RyanJsonDetachByKey(RyanJson_t pJson, const char *key);  // 需用户释放内存

/**
 * @brief 删除相关函数
 */
extern RyanJsonBool_e RyanJsonDeleteByIndex(RyanJson_t pJson, uint32_t index);
extern RyanJsonBool_e RyanJsonDeleteByKey(RyanJson_t pJson, const char *key);

/**
 * @brief 查询函数
 */
extern RyanJson_t RyanJsonGetObjectByKey(RyanJson_t pJson, const char *key);
extern RyanJson_t RyanJsonGetObjectByIndex(RyanJson_t pJson, uint32_t index);

// 工具宏
#define RyanJsonMakeBool(ex) ((ex) ? RyanJsonTrue : RyanJsonFalse)

/**
 * @brief 查询函数
 */
#define RyanJsonHasObjectByKey(pJson, key)     RyanJsonMakeBool(RyanJsonGetObjectByKey(pJson, key))
#define RyanJsonHasObjectByIndex(pJson, index) RyanJsonMakeBool(RyanJsonGetObjectByIndex(pJson, index))

/**
 * @brief RyanJson 类型判断接口
 */
extern RyanJsonBool_e RyanJsonIsKey(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsNull(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsBool(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsNumber(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsString(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsArray(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsObject(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsInt(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsDouble(RyanJson_t pJson);

/**
 * @brief 取值宏
 * !取值宏使用前一定要RyanJsonIsXXXX类型判断函数做好判断,否则会内存访问越界
 */
extern char *RyanJsonGetKey(RyanJson_t pJson);
extern char *RyanJsonGetStringValue(RyanJson_t pJson);
extern int32_t RyanJsonGetIntValue(RyanJson_t pJson);
extern double RyanJsonGetDoubleValue(RyanJson_t pJson);
extern RyanJson_t RyanJsonGetObjectValue(RyanJson_t pJson);
extern RyanJson_t RyanJsonGetArrayValue(RyanJson_t pJson);
#define RyanJsonGetBoolValue(pJson) RyanJsonGetPayloadBoolValueByFlag(pJson)

/**
 * @brief 添加相关函数
 * ! add函数使用前建议RyanJsonIsXXXX宏判断是否是对象 / 数组,否则会内存访问越界
 * ! add函数内部会处理失败情况，如果返回false，不需要用户手动释放内存
 */
#define RyanJsonAddNullToObject(pJson, key)           RyanJsonInsert(pJson, UINT32_MAX, RyanJsonCreateNull(key))
#define RyanJsonAddBoolToObject(pJson, key, boolean)  RyanJsonInsert(pJson, UINT32_MAX, RyanJsonCreateBool(key, boolean))
#define RyanJsonAddIntToObject(pJson, key, number)    RyanJsonInsert(pJson, UINT32_MAX, RyanJsonCreateInt(key, number))
#define RyanJsonAddDoubleToObject(pJson, key, number) RyanJsonInsert(pJson, UINT32_MAX, RyanJsonCreateDouble(key, number))
#define RyanJsonAddStringToObject(pJson, key, string) RyanJsonInsert(pJson, UINT32_MAX, RyanJsonCreateString(key, string))
extern RyanJsonBool_e RyanJsonAddItemToObject(RyanJson_t pJson, const char *key, RyanJson_t item);

#define RyanJsonAddNullToArray(pJson)           RyanJsonAddNullToObject(pJson, NULL)
#define RyanJsonAddBoolToArray(pJson, boolean)  RyanJsonAddBoolToObject(pJson, NULL, boolean)
#define RyanJsonAddIntToArray(pJson, number)    RyanJsonAddIntToObject(pJson, NULL, number)
#define RyanJsonAddDoubleToArray(pJson, number) RyanJsonAddDoubleToObject(pJson, NULL, number)
#define RyanJsonAddStringToArray(pJson, string) RyanJsonAddStringToObject(pJson, NULL, string)
#define RyanJsonAddItemToArray(pJson, item)     RyanJsonAddItemToObject(pJson, NULL, item)

/**
 * @brief 遍历函数
 */
#define RyanJsonArrayForEach(pJson, item)  for ((item) = RyanJsonGetArrayValue(pJson); NULL != (item); (item) = (item)->next)
#define RyanJsonObjectForEach(pJson, item) for ((item) = RyanJsonGetObjectValue(pJson); NULL != (item); (item) = (item)->next)

/**
 * @brief 修改相关函数
 * !修改函数没有对入参做校验，使用前请做使用RyanJsonIsXXXX类型判断宏做好判断,否则会内存访问越界
 */
extern RyanJsonBool_e RyanJsonChangeKey(RyanJson_t pJson, const char *key);
extern RyanJsonBool_e RyanJsonChangeStringValue(RyanJson_t pJson, const char *strValue);
extern RyanJsonBool_e RyanJsonChangeIntValue(RyanJson_t pJson, int32_t number);
extern RyanJsonBool_e RyanJsonChangeDoubleValue(RyanJson_t pJson, double number);
#define RyanJsonChangeBoolValue(pJson, boolean) RyanJsonSetPayloadBoolValueByFlag(pJson, boolean)

// 这是change方法的补充，当需要修改value类型时，使用此函数
// 请参考 changeJsonTest 示例，严格按照规则来使用
/**
 * @brief
 * !这是change方法的补充，当需要修改value类型时，使用此函数，请参考 RyanJsonBaseTestChangeJson 示例，严格按照规则来使用
 */
extern RyanJsonBool_e RyanJsonReplaceByKey(RyanJson_t pJson, const char *key, RyanJson_t item);
extern RyanJsonBool_e RyanJsonReplaceByIndex(RyanJson_t pJson, uint32_t index, RyanJson_t item); // object对象也可以使用，但是不推荐

#ifdef __cplusplus
}
#endif

#endif
