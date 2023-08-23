
#ifndef __RyanJson__
#define __RyanJson__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <math.h>

    typedef enum
    {
        // 类型标志 占用8字节,剩余一个备用
        RyanJsonTypeUnknow = 1 << 0,
        RyanJsonTypeNull = 1 << 1,
        RyanJsonTypeBool = 1 << 2,
        RyanJsonTypeNumber = 1 << 3,
        RyanJsonTypeString = 1 << 4,
        RyanJsonTypeArray = 1 << 5,
        RyanJsonTypeObject = 1 << 6,
    } RyanjsonType_e;

    typedef enum
    {
        // flag标志
        RyanJsonValueBoolTrueFlag = 1 << 8,
        RyanJsonValueNumberIntFlag = 1 << 9,
        RyanJsonWithKeyFlag = 1 << 10
    } RyanJsonInfoFlag_e;

    typedef enum
    {
        RyanJsonFalse = 0,
        RyanJsonTrue = 1
    } RyanJsonBool;

    struct RyanJsonNode
    {
        uint32_t info;             // 包含类型，key等标志
        struct RyanJsonNode *next; // 单链表node节点

        // [char *key] 有key的json节点, 会动态创建指针

        // 有value值的节点, 会动态创建指针
        // [int32_t value / double value / char* value / RyanJson_t item]
    };

    typedef struct RyanJsonNode *RyanJson_t;

    // 内存钩子函数
    typedef void *(*malloc_t)(size_t size);
    typedef void (*free_t)(void *block);
    typedef void *(*realloc_t)(void *block, size_t size);

// 限制解析数组/对象中嵌套的深度
// RyanJson使用递归 序列化/反序列化 json
// 请根据单片机资源合理设置以防止堆栈溢出。
#ifndef RyanJsonNestingLimit
#define RyanJsonNestingLimit 30000
#endif

/**
 * @brief 较底层接口, 不推荐用户使用，除非用户知道这些接口意义
 */
#define RyanJsonGetInfo(pJson) ((pJson) ? ((pJson)->info) : 0)
#define RyanJsonGetType(pJson) ((uint8_t)RyanJsonGetInfo(pJson))
    RyanJsonBool RyanJsonInsert(RyanJson_t pJson, int32_t index, RyanJson_t item);
    void *RyanJsonGetValue(RyanJson_t pJson);
    RyanJson_t RyanJsonGetObjectByIndexs(RyanJson_t pJson, int32_t index, ...);
    RyanJson_t RyanJsonGetObjectByKeys(RyanJson_t pJson, char *key, ...);
    RyanJsonBool RyanJsonReapplyString(char **dst, const char *src);
    RyanJson_t RyanJsonCreateItem(const char *key, RyanJson_t item);

    /**
     * @brief json对象函数
     */
    RyanJsonBool RyanJsonInitHooks(malloc_t _malloc, free_t _free, realloc_t _realloc);
    RyanJson_t RyanJsonParseOptions(const char *text, uint32_t size, RyanJsonBool require_null_terminated, const char **return_parse_end); // 需用户释放内存
    static inline RyanJson_t RyanJsonParse(const char *text)                                                                               // 需用户释放内存
    {
        return RyanJsonParseOptions(text, strlen(text), RyanJsonFalse, NULL);
    }

    char *RyanJsonPrint(RyanJson_t pJson, uint32_t preset, RyanJsonBool format, uint32_t *len); // 需用户释放内存
    char *RyanJsonPrintPreallocated(RyanJson_t pJson, char *buffer, uint32_t length, RyanJsonBool format, uint32_t *len);

    RyanJson_t RyanJsonDuplicate(RyanJson_t pJson); // 需用户释放内存
    uint32_t RyanJsonGetSize(RyanJson_t pJson);
    void RyanJsonMinify(char *text);

    void RyanJsonDelete(RyanJson_t pJson);
    void RyanJsonFree(void *block);

    RyanJsonBool RyanJsonCompare(RyanJson_t a, RyanJson_t b);
    RyanJsonBool RyanJsonCompareOnlyKey(RyanJson_t a, RyanJson_t b);

    /**
     * @brief 添加 / 删除相关函数
     */
    RyanJson_t RyanJsonCreateObject();                              // 如果没有添加到父json, 则需释放内存
    RyanJson_t RyanJsonCreateNull(char *key);                       // 如果没有添加到父json, 则需释放内存
    RyanJson_t RyanJsonCreateBool(char *key, RyanJsonBool boolean); // 如果没有添加到父json, 则需释放内存
    RyanJson_t RyanJsonCreateInt(char *key, int32_t number);        // 如果没有添加到父json, 则需释放内存
    RyanJson_t RyanJsonCreateDouble(char *key, double number);      // 如果没有添加到父json, 则需释放内存
    RyanJson_t RyanJsonCreateString(char *key, const char *string); // 如果没有添加到父json, 则需释放内存

    RyanJson_t RyanJsonCreateArray();                                           // 如果没有添加到父json, 则需释放内存
    RyanJson_t RyanJsonCreateIntArray(const int32_t *numbers, int32_t count);   // 如果没有添加到父json, 则需释放内存
    RyanJson_t RyanJsonCreateDoubleArray(const double *numbers, int32_t count); // 如果没有添加到父json, 则需释放内存
    RyanJson_t RyanJsonCreateStringArray(const char **strings, int32_t count);  // 如果没有添加到父json, 则需释放内存

    RyanJson_t RyanJsonDetachByIndex(RyanJson_t pJson, int32_t index); // 需用户释放内存
    RyanJson_t RyanJsonDetachByKey(RyanJson_t pJson, const char *key); // 需用户释放内存
    RyanJsonBool RyanJsonDeleteByIndex(RyanJson_t pJson, int32_t index);
    RyanJsonBool RyanJsonDeleteByKey(RyanJson_t pJson, const char *key);

#define RyanJsonAddNullToObject(pJson, key) RyanJsonInsert(pJson, INT_MAX, RyanJsonCreateNull(key))
#define RyanJsonAddBoolToObject(pJson, key, boolean) RyanJsonInsert(pJson, INT_MAX, RyanJsonCreateBool(key, boolean))
#define RyanJsonAddIntToObject(pJson, key, number) RyanJsonInsert(pJson, INT_MAX, RyanJsonCreateInt(key, number))
#define RyanJsonAddDoubleToObject(pJson, key, number) RyanJsonInsert(pJson, INT_MAX, RyanJsonCreateDouble(key, number))
#define RyanJsonAddStringToObject(pJson, key, string) RyanJsonInsert(pJson, INT_MAX, RyanJsonCreateString(key, string))
#define RyanJsonAddItemToObject(pJson, key, item) RyanJsonInsert(pJson, INT_MAX, RyanJsonCreateItem(key, item))

#define RyanJsonAddNullToArray(pJson) RyanJsonAddNullToObject(pJson, NULL)
#define RyanJsonAddBoolToArray(pJson, boolean) RyanJsonAddBoolToObject(pJson, NULL, boolean)
#define RyanJsonAddIntToArray(pJson, number) RyanJsonAddIntToObject(pJson, NULL, number)
#define RyanJsonAddDoubleToArray(pJson, number) RyanJsonAddDoubleToObject(pJson, NULL, number)
#define RyanJsonAddStringToArray(pJson, string) RyanJsonAddStringToObject(pJson, NULL, string)
#define RyanJsonAddItemToArray(pJson, item) RyanJsonAddItemToObject(pJson, NULL, item)

    /**
     * @brief 查询函数
     */
    RyanJson_t RyanJsonGetObjectByKey(RyanJson_t pJson, const char *key);
    RyanJson_t RyanJsonGetObjectByIndex(RyanJson_t pJson, int32_t index);
#define RyanJsonGetObjectToKey(pJson, key, ...) RyanJsonGetObjectByKeys(pJson, key, ##__VA_ARGS__, NULL)
#define RyanJsonGetObjectToIndex(pJson, index, ...) RyanJsonGetObjectByIndexs(pJson, index, ##__VA_ARGS__, INT_MIN)

#define RyanJsonHasObjectByKey(pJson, key) (RyanJsonGetObjectByKey(pJson, key) ? RyanJsonTrue : RyanJsonFalse)
#define RyanJsonHasObjectByIndex(pJson, key) (RyanJsonGetObjectByIndex(pJson, index) ? RyanJsonTrue : RyanJsonFalse)
#define RyanJsonHasObjectToKey(pJson, key, ...) (RyanJsonGetObjectByKeys(pJson, key, ##__VA_ARGS__, NULL) ? RyanJsonTrue : RyanJsonFalse)
#define RyanJsonHasObjectToIndex(pJson, key, ...) (RyanJsonGetObjectByIndexs(pJson, index, ##__VA_ARGS__, INT_MIN) ? RyanJsonTrue : RyanJsonFalse)

#define returnJsonBool(ex) ((ex) ? RyanJsonTrue : RyanJsonFalse)
#define RyanJsonIsKey(pJson) returnJsonBool(RyanJsonGetInfo(pJson) & RyanJsonWithKeyFlag)
#define RyanJsonIsNull(pJson) returnJsonBool(RyanJsonGetType(pJson) & RyanJsonTypeNull)
#define RyanJsonIsBool(pJson) returnJsonBool(RyanJsonGetType(pJson) & RyanJsonTypeBool)
#define RyanJsonIsNumber(pJson) returnJsonBool(RyanJsonGetType(pJson) & RyanJsonTypeNumber)
#define RyanJsonIsInt(pJson) returnJsonBool(RyanJsonGetType(pJson) & RyanJsonTypeNumber && RyanJsonGetInfo(pJson) & RyanJsonValueNumberIntFlag)
#define RyanJsonIsDouble(pJson) returnJsonBool(RyanJsonGetType(pJson) & RyanJsonTypeNumber && !(RyanJsonGetInfo(pJson) & RyanJsonValueNumberIntFlag))
#define RyanJsonIsString(pJson) returnJsonBool(RyanJsonGetType(pJson) & RyanJsonTypeString)
#define RyanJsonIsArray(pJson) returnJsonBool(RyanJsonGetType(pJson) & RyanJsonTypeArray)
#define RyanJsonIsObject(pJson) returnJsonBool(RyanJsonGetType(pJson) & RyanJsonTypeObject)

//! get函数使用前建议RyanJsonIsXXXX宏做好判断
#define RyanJsonGetKey(pJson) (*(char **)((RyanJson_t)(pJson) + 1))
#define RyanJsonGetNullValue(pJson) (NULL)
#define RyanJsonGetBoolValue(pJson) (RyanJsonGetInfo(pJson) & RyanJsonValueBoolTrueFlag ? RyanJsonTrue : RyanJsonFalse)
#define RyanJsonGetIntValue(pJson) (*(int32_t *)RyanJsonGetValue(pJson))
#define RyanJsonGetDoubleValue(pJson) (*(double *)RyanJsonGetValue(pJson))
#define RyanJsonGetStringValue(pJson) (*(char **)RyanJsonGetValue(pJson))
#define RyanJsonGetArrayValue(pJson) (*(RyanJson_t *)RyanJsonGetValue(pJson))
#define RyanJsonGetObjectValue(pJson) (*(RyanJson_t *)RyanJsonGetValue(pJson))

#define RyanJsonArrayForEach(pJson, item) for ((item) = RyanJsonGetArrayValue(pJson); NULL != (item); (item) = (item)->next)
#define RyanJsonObjectForEach(pJson, item) for ((item) = RyanJsonGetObjectValue(pJson); NULL != (item); (item) = (item)->next)

    /**
     * @brief change函数
     * !change函数没有对入参做校验，使用前请做使用RyanJsonIsXXXX宏做好判断
     */
#define RyanJsonChangeKey(pJson, key) (RyanJsonReapplyString(&RyanJsonGetKey(pJson), key))
#define RyanJsonChangeStringValue(pJson, string) (RyanJsonReapplyString(&RyanJsonGetStringValue(pJson), string))
#define RyanJsonChangeBoolValue(pJson, boolean) ((boolean) == RyanJsonTrue) ? ((pJson)->info |= (RyanJsonValueBoolTrueFlag)) : ((pJson)->info &= (~RyanJsonValueBoolTrueFlag))
#define RyanJsonChangeIntValue(pJson, number) (RyanJsonGetIntValue(pJson) = (number))
#define RyanJsonChangeDoubleValue(pJson, number) (RyanJsonGetDoubleValue(pJson) = (number))

    // 这是change方法的补充，当需要修改value类型时，使用此函数
    // 请参考 changeJsonTest 示例，严格按照规则来使用
    RyanJsonBool RyanJsonReplaceByKey(RyanJson_t pJson, const char *key, RyanJson_t item);
    RyanJsonBool RyanJsonReplaceByIndex(RyanJson_t pJson, int32_t index, RyanJson_t item); // object对象也可以使用，但是不推荐

#ifdef __cplusplus
}
#endif

#endif
