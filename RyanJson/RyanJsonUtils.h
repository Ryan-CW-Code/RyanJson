#ifndef __RyanJsonUtils__
#define __RyanJsonUtils__

#ifdef __cplusplus
extern "C" {
#endif

#include "RyanJson.h"
#include <stdarg.h>

// 语法糖，根据传入的numbers数组创建一个int类型的数组。如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateIntArray(const int32_t *numbers, uint32_t count);
// 语法糖，根据传入的numbers数组创建一个double类型的数组。如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateDoubleArray(const double *numbers, uint32_t count);
// 语法糖，根据传入的strings数组创建一个string类型的数组。如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateStringArray(const char **strings, uint32_t count);

extern RyanJsonBool_e RyanJsonCompareOnlyKey(RyanJson_t leftJson, RyanJson_t rightJson);

/**
 * @brief 查询函数，此接口较为底层，请使用下发的宏定义调用
 */
extern RyanJson_t RyanJsonGetObjectByIndexs(RyanJson_t pJson, uint32_t index, ...);
extern RyanJson_t RyanJsonGetObjectByKeys(RyanJson_t pJson, const char *key, ...);

/**
 * @brief 可使用此宏进行嵌套式查找，例如 RyanJsonGetObjectToKey(json, "test", "inter")
 * 
 */
#define RyanJsonGetObjectToKey(pJson, key, ...)     RyanJsonGetObjectByKeys(pJson, (key), ##__VA_ARGS__, NULL)

/**
 * @brief 可使用此宏进行嵌套式查找，例如 RyanJsonGetObjectToIndex(json, 0, 2)
 * 
 */
#define RyanJsonGetObjectToIndex(pJson, index, ...) RyanJsonGetObjectByIndexs(pJson, (index), ##__VA_ARGS__, UINT32_MAX)

#define RyanJsonHasObjectToKey(pJson, key, ...)     RyanJsonMakeBool(RyanJsonGetObjectByKeys(pJson, key, ##__VA_ARGS__, NULL))
#define RyanJsonHasObjectToIndex(pJson, index, ...) RyanJsonMakeBool(RyanJsonGetObjectByIndexs(pJson, index, ##__VA_ARGS__, UINT32_MAX))

#ifdef __cplusplus
}
#endif

#endif
