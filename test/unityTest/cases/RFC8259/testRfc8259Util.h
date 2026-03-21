#ifndef RYAN_JSON_RFC8259_TEST_UTIL_H
#define RYAN_JSON_RFC8259_TEST_UTIL_H

#include <stdint.h>

/**
 * @brief 提取一元素 Array 的唯一元素；若不是一元素 Array 返回 0
 */
int32_t RyanJsonExtractSingleArrayElement(const char *s, uint32_t len, const char **elem, uint32_t *elemLen);

/**
 * @brief 值级语义比较：String（去引号并 normalize）、Number（含科学计数法）、Bool、Null
 */
int32_t RyanJsonScalarSemanticEqual(const char *a, uint32_t aLen, const char *b, uint32_t bLen);

/**
 * @brief Json 语义比较：支持单元素 Array 剥离后进行标量比较
 */
int32_t RyanJsonValueSemanticEqual(const char *a, uint32_t aLen, const char *b, uint32_t bLen);

/**
 * @brief 将 Json 字符串字面量文本规范化为 UTF-8 字节序列
 */
int32_t RyanJsonNormalizeString(const char *in, uint32_t inLen, unsigned char **out, uint32_t *outLen);

#endif // RYAN_JSON_RFC8259_TEST_UTIL_H
