
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
		{code};                                                                                                                    \
	}

#define RyanJsonCheckCode(EX, code) RyanJsonCheckCodeNoReturn(EX, { {code}; });

#define RyanJsonCheckReturnFlase(EX) RyanJsonCheckCode(EX, { return RyanJsonFalse; })
#define RyanJsonCheckReturnNull(EX)  RyanJsonCheckCode(EX, { return NULL; })
#define RyanJsonCheckAssert(EX)      RyanJsonCheckCode(EX, { RyanJsonAssert(NULL && "RyanJsonCheckAssert"); })

// Json的最基础节点，所有Json元素都由该节点表示。
// 结构体中仅包含固定的 next 指针，用于单向链表串联。
// 其余数据（flag、key、stringValue、numberValue、doubleValue 等）均通过动态内存分配管理。
//
// 在 next 后紧跟一个字节的 flag，用于描述节点的核心信息：
//   - 节点类型（null / bool / number / string / object）
//   - 是否包含 key
//   - Bool 类型的取值（true/false）
//   - Number 类型的类别（整数 / 浮点数）
//   - Key 的长度（占用 1~4 字节）
//
// flag 后若节点包含 key 或字符串值，则跟随一个指针，指向存储区：
//   [ keyLen | key | stringValue ]
//   其中 keyLen 的大小由 flag 中的长度信息决定（最多 4 字节）。
//
// 在指针之后，根据节点类型存储具体数据：
//   - null / bool：由 flag 表示
//   - string：由上述指针指向
//   - number：根据 flag 决定存储 int 或 double，来申请空间
//   - object：动态分配空间存储子节点，申请一个指针空间
//
// 整个设计通过一个字节的 flag 高度复用信息，保证结构紧凑且灵活。
// 设计特点：
//   - 一个 Json 节点最多 malloc 两次（一次节点本身，一次可选的 key/stringValue），
//     对嵌入式系统非常友好，减少 malloc 头部开销。
//   - key 和 stringValue 必须通过指针管理：
//       * 如果直接将key 和 stringValue放在节点里，虽然只需一次 malloc，但部分修改场景会遇到问题
//       * 如何找到前置节点的问题，需要找到前置节点，替换修改过节点，然后释放当前节点。（这个最开始已经实现了，关键是下面这条）
//       * 用户可能传递的 Json 对象不是指针，无法直接替换节点。要求应用层传递指针会增加侵入性，改动太大，不符合“应用层无需修改”的更新目标。
//   - 因此采用指针方式，保证灵活性和低侵入性。

struct RyanJsonNode
{
	struct RyanJsonNode *next; // 单链表node节点
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
 * !!!较底层接口, 不推荐用户使用，除非用户知道这些接口意义
 * !!!一定要看这里，这里的接口不推荐使用
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
extern RyanJsonBool_e RyanJsonInsert(RyanJson_t pJson, int32_t index, RyanJson_t item);
extern void *RyanJsonGetValue(RyanJson_t pJson);
extern RyanJson_t RyanJsonCreateItem(const char *key, RyanJson_t item);
/**
 * !!!上面的接口不推荐使用
 *
 */

/**
 * @brief json对象函数
 */
extern RyanJsonBool_e RyanJsonInitHooks(RyanJsonMalloc_t _malloc, RyanJsonFree_t _free, RyanJsonRealloc_t _realloc);
extern RyanJson_t RyanJsonParseOptions(const char *text, int32_t size, RyanJsonBool_e requireNullTerminator,
				       const char **parseEndPtr);                                            // 需用户释放内存
#define RyanJsonParse(text) RyanJsonParseOptions((text), (int32_t)RyanJsonStrlen(text), RyanJsonFalse, NULL) // 需用户释放内存

/**
 * @brief 打印json对象函数
 */
extern char *RyanJsonPrint(RyanJson_t pJson, int32_t preset, RyanJsonBool_e format, int32_t *len); // 需用户释放内存
extern char *RyanJsonPrintPreallocated(RyanJson_t pJson, char *buffer, int32_t length, RyanJsonBool_e format, int32_t *len);

/**
 * @brief json杂项函数
 */
extern RyanJson_t RyanJsonDuplicate(RyanJson_t pJson); // 需用户释放内存
extern int32_t RyanJsonGetSize(RyanJson_t pJson);      // 获取Json中子项个数
extern int32_t RyanJsonMinify(char *text, int32_t textLen);

extern void RyanJsonDelete(RyanJson_t pJson);
extern void RyanJsonFree(void *block);

extern RyanJsonBool_e RyanJsonCompare(RyanJson_t a, RyanJson_t b);

/**
 * @brief 添加 / 删除相关函数
 */
extern RyanJson_t RyanJsonCreateObject(void);                                  // 如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateNull(const char *key);                         // 如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateBool(const char *key, RyanJsonBool_e boolean); // 如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateInt(const char *key, int32_t number);          // 如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateDouble(const char *key, double number);        // 如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateString(const char *key, const char *string);   // 如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateArray(void);                                   // 如果没有添加到父json, 则需释放内存

// 语法糖，根据传入的numbers数组创建一个int类型的数组。如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateIntArray(const int32_t *numbers, int32_t count);
// 语法糖，根据传入的numbers数组创建一个double类型的数组。如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateDoubleArray(const double *numbers, int32_t count);
// 语法糖，根据传入的strings数组创建一个string类型的数组。如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateStringArray(const char **strings, int32_t count);

extern RyanJson_t RyanJsonDetachByIndex(RyanJson_t pJson, int32_t index); // 需用户释放内存
extern RyanJson_t RyanJsonDetachByKey(RyanJson_t pJson, const char *key); // 需用户释放内存
extern RyanJsonBool_e RyanJsonDeleteByIndex(RyanJson_t pJson, int32_t index);
extern RyanJsonBool_e RyanJsonDeleteByKey(RyanJson_t pJson, const char *key);

// 工具宏
#define RyanJsonMakeBool(ex) ((ex) ? RyanJsonTrue : RyanJsonFalse)

/**
 * @brief 查询函数
 */
extern RyanJson_t RyanJsonGetObjectByKey(RyanJson_t pJson, const char *key);
extern RyanJson_t RyanJsonGetObjectByIndex(RyanJson_t pJson, int32_t index);

#define RyanJsonHasObjectByKey(pJson, key)     RyanJsonMakeBool(RyanJsonGetObjectByKey(pJson, key))
#define RyanJsonHasObjectByIndex(pJson, index) RyanJsonMakeBool(RyanJsonGetObjectByIndex(pJson, index))

#define RyanJsonIsKey(pJson)    RyanJsonMakeBool(RyanJsonGetPayloadWhiteKeyByFlag(pJson))
#define RyanJsonIsNull(pJson)   RyanJsonMakeBool(RyanJsonGetType(pJson) == RyanJsonTypeNull)
#define RyanJsonIsBool(pJson)   RyanJsonMakeBool(RyanJsonGetType(pJson) == RyanJsonTypeBool)
#define RyanJsonIsNumber(pJson) RyanJsonMakeBool(RyanJsonGetType(pJson) == RyanJsonTypeNumber)
#define RyanJsonIsInt(pJson)    RyanJsonMakeBool(RyanJsonIsNumber(pJson) && (RyanJsonFalse == RyanJsonGetPayloadNumberIsDoubleByFlag(pJson)))
#define RyanJsonIsDouble(pJson) RyanJsonMakeBool(RyanJsonIsNumber(pJson) && (RyanJsonTrue == RyanJsonGetPayloadNumberIsDoubleByFlag(pJson)))
#define RyanJsonIsString(pJson) RyanJsonMakeBool(RyanJsonGetType(pJson) == RyanJsonTypeString)
#define RyanJsonIsArray(pJson)  RyanJsonMakeBool(RyanJsonGetType(pJson) == RyanJsonTypeArray)
#define RyanJsonIsObject(pJson) RyanJsonMakeBool(RyanJsonGetType(pJson) == RyanJsonTypeObject)

//! get value函数使用前一定要RyanJsonIsXXXX宏做好判断,否则会内存访问越界
extern char *RyanJsonGetKey(RyanJson_t pJson);
extern char *RyanJsonGetStringValue(RyanJson_t pJson);
#define RyanJsonGetBoolValue(pJson)   RyanJsonGetPayloadBoolValueByFlag(pJson)
#define RyanJsonGetIntValue(pJson)    (*(int32_t *)RyanJsonGetValue(pJson))
#define RyanJsonGetDoubleValue(pJson) (*(double *)RyanJsonGetValue(pJson))
#define RyanJsonGetArrayValue(pJson)  (*(RyanJson_t *)RyanJsonGetValue(pJson))
#define RyanJsonGetObjectValue(pJson) (*(RyanJson_t *)RyanJsonGetValue(pJson))

#define RyanJsonGetArraySize(pJson) RyanJsonGetSize(pJson)

//! add函数使用前建议RyanJsonIsXXXX宏判断是否是对象 / 数组,否则会内存访问越界
//! add函数内部会处理失败情况，如果返回false，不需要用户手动释放内存
#define RyanJsonAddNullToObject(pJson, key)           RyanJsonInsert(pJson, INT32_MAX, RyanJsonCreateNull(key))
#define RyanJsonAddBoolToObject(pJson, key, boolean)  RyanJsonInsert(pJson, INT32_MAX, RyanJsonCreateBool(key, boolean))
#define RyanJsonAddIntToObject(pJson, key, number)    RyanJsonInsert(pJson, INT32_MAX, RyanJsonCreateInt(key, number))
#define RyanJsonAddDoubleToObject(pJson, key, number) RyanJsonInsert(pJson, INT32_MAX, RyanJsonCreateDouble(key, number))
#define RyanJsonAddStringToObject(pJson, key, string) RyanJsonInsert(pJson, INT32_MAX, RyanJsonCreateString(key, string))
// #define RyanJsonAddItemToObject(pJson, key, item)     RyanJsonInsert(pJson, INT32_MAX, RyanJsonCreateItem(key, item))
extern RyanJsonBool_e RyanJsonAddItemToObject(RyanJson_t pJson, char *key, RyanJson_t item);

#define RyanJsonAddNullToArray(pJson)           RyanJsonAddNullToObject(pJson, NULL)
#define RyanJsonAddBoolToArray(pJson, boolean)  RyanJsonAddBoolToObject(pJson, NULL, boolean)
#define RyanJsonAddIntToArray(pJson, number)    RyanJsonAddIntToObject(pJson, NULL, number)
#define RyanJsonAddDoubleToArray(pJson, number) RyanJsonAddDoubleToObject(pJson, NULL, number)
#define RyanJsonAddStringToArray(pJson, string) RyanJsonAddStringToObject(pJson, NULL, string)
#define RyanJsonAddItemToArray(pJson, item)     RyanJsonAddItemToObject(pJson, NULL, item)

// 遍历函数
#define RyanJsonArrayForEach(pJson, item)  for ((item) = RyanJsonGetArrayValue(pJson); NULL != (item); (item) = (item)->next)
#define RyanJsonObjectForEach(pJson, item) for ((item) = RyanJsonGetObjectValue(pJson); NULL != (item); (item) = (item)->next)

/**
 * @brief change函数
 * !change函数没有对入参做校验，使用前请做使用RyanJsonIsXXXX宏做好判断,否则会内存访问越界
 */
extern RyanJsonBool_e RyanJsonChangeKey(RyanJson_t pJson, const char *key);
extern RyanJsonBool_e RyanJsonChangeStringValue(RyanJson_t pJson, const char *strValue);
#define RyanJsonChangeBoolValue(pJson, boolean)  RyanJsonSetPayloadBoolValueByFlag(pJson, boolean)
#define RyanJsonChangeIntValue(pJson, number)    (RyanJsonGetIntValue(pJson) = (number))
#define RyanJsonChangeDoubleValue(pJson, number) (RyanJsonGetDoubleValue(pJson) = (number))

// 这是change方法的补充，当需要修改value类型时，使用此函数
// 请参考 changeJsonTest 示例，严格按照规则来使用
extern RyanJsonBool_e RyanJsonReplaceByKey(RyanJson_t pJson, const char *key, RyanJson_t item);
extern RyanJsonBool_e RyanJsonReplaceByIndex(RyanJson_t pJson, int32_t index, RyanJson_t item); // object对象也可以使用，但是不推荐

#ifdef __cplusplus
}
#endif

#endif
