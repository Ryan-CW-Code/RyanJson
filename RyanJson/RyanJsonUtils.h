
#ifndef __RyanJsonUtils__
#define __RyanJsonUtils__

#ifdef __cplusplus
extern "C" {
#endif

#include "RyanJson.h"

// 语法糖，根据传入的numbers数组创建一个int类型的数组。如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateIntArray(const int32_t *numbers, int32_t count);
// 语法糖，根据传入的numbers数组创建一个double类型的数组。如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateDoubleArray(const double *numbers, int32_t count);
// 语法糖，根据传入的strings数组创建一个string类型的数组。如果没有添加到父json, 则需释放内存
extern RyanJson_t RyanJsonCreateStringArray(const char **strings, int32_t count);

extern RyanJsonBool_e RyanJsonCompareOnlyKey(RyanJson_t a, RyanJson_t b);

/**
 * @brief 查询函数
 */
extern RyanJson_t RyanJsonGetObjectByIndexs(RyanJson_t pJson, int32_t index, ...);
extern RyanJson_t RyanJsonGetObjectByKeys(RyanJson_t pJson, const char *key, ...);
#define RyanJsonGetObjectToKey(pJson, key, ...)     RyanJsonGetObjectByKeys(pJson, (key), ##__VA_ARGS__, NULL)
#define RyanJsonGetObjectToIndex(pJson, index, ...) RyanJsonGetObjectByIndexs(pJson, (index), ##__VA_ARGS__, INT32_MIN)

#define RyanJsonHasObjectToKey(pJson, key, ...)     RyanJsonMakeBool(RyanJsonGetObjectByKeys(pJson, key, ##__VA_ARGS__, NULL))
#define RyanJsonHasObjectToIndex(pJson, index, ...) RyanJsonMakeBool(RyanJsonGetObjectByIndexs(pJson, index, ##__VA_ARGS__, INT32_MIN))

#ifdef __cplusplus
}
#endif

#endif
