#ifndef __RyanJson__
#define __RyanJson__

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
#ifdef RyanJsonEnableAssert
#define RyanJsonCheckAssert(EX) RyanJsonCheckCode(EX, RyanJsonAssert(NULL && "RyanJsonCheckAssert");)
#else
#define RyanJsonCheckAssert(EX) (void)(EX)
#endif

// Json 的最基础节点，所有 Json 元素都由该节点表示。
// 结构体中仅包含固定的 next 指针，用于单向链表串联。
// 其余数据（flag、key、stringValue、numberValue、doubleValue 等）均通过动态内存分配管理。
struct RyanJsonNode
{
	struct RyanJsonNode *next; // 单链表节点指针

	/*
	 * 在 next 后紧跟一个字节的 flag，用于描述节点的核心信息：
	 *
	 * 位分布如下：
	 * bit7   bit6   bit5   bit4   bit3   bit2   bit1   bit0
	 * -----------------------------------------------------
	 * 保留   KeyLen KeyLen HasKey NumExt Type2 Type1 Type0
	 *
	 * 各位含义：
	 * - bit0-2 : 节点类型
	 *            000=Unknown, 001=Null, 010=Bool, 011=Number,
	 *            100=String, 101=Array, 110=Object, 111=Reserved
	 *
	 * - bit3   : 扩展位
	 *            Bool 类型：0=false, 1=true
	 *            Number 类型：0=int, 1=double
	 *
	 * - bit4   : 是否包含 Key
	 *            0=无 Key（数组元素）
	 *            1=有 Key（对象成员）
	 *
	 * - bit5-6 : Key 长度字段字节数
	 *            00=1字节 (≤255)
	 *            01=2字节 (≤65535)
	 *            10=3字节 (≤16M)
	 *            11=4字节 (≤UINT32_MAX)
	 *
	 * - bit7   : 保留位（未来可用于压缩标记、特殊类型等）
	 */

	/*
	 * flag 后若节点包含 key / strValue，则跟随一个指针，
	 * 指向存储区：[ keyLen | key | stringValue ]
	 * 其中 keyLen 的大小由 flag 中的长度信息决定（最多 4 字节）。
	 *
	 * 在指针之后，根据节点类型存储具体数据：
	 * - null / bool : 由 flag 表示
	 * - string      : 由上述指针指向
	 * - number      : 根据 flag 决定存储 int(4字节) 或 double(8字节)
	 * - object      : 动态分配空间存储子节点，链表结构如下：
	 *
	 *   {
	 *       "name": "RyanJson",
	 *   next (
	 *       "version": "xxx",
	 *   next (
	 *       "repository": "https://github.com/Ryan-CW-Code/RyanJson",
	 *   next (
	 *       "keywords": ["json", "streamlined", "parser"],
	 *   next (
	 *       "others": { ... }
	 *   )))
	 *   }
	 */

	/*
	 * 设计特点：
	 * - 一个 Json 节点最多 malloc 两次（一次节点本身，一次可选的 key/stringValue），
	 *   对嵌入式系统非常友好，减少 malloc 头部开销, 尽可能的减少内存碎片。
	 *
	 * - key 和 stringValue 必须通过指针管理：
	 *   * 如果直接放在节点里，虽然只需一次 malloc，
	 *     但修改场景会遇到替换/释放困难。
	 *   * 用户可能传递的 Json 对象不是指针，无法直接替换节点，
	 *     要求应用层传递指针会增加侵入性，不符合“应用层无需修改”的目标。
	 *
	 * - 因此采用指针方式，保证灵活性和低侵入性。
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
extern RyanJsonBool_e RyanJsonInsert(RyanJson_t pJson, uint32_t index, RyanJson_t item);
/**
 * !!!上面的接口不推荐使用
 *
 */

/**
 * @brief json对象函数
 */
extern RyanJsonBool_e RyanJsonInitHooks(RyanJsonMalloc_t _malloc, RyanJsonFree_t _free, RyanJsonRealloc_t _realloc);
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
