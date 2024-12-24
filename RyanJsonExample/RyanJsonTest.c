
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "RyanJson.h"
#include "cJSON.h"
#include "valloc.h"

/* --------------------------------------- jsonTest ------------------------------------------- */
// !(fabs(RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(json, "double")) - 16.89) < 1e-6)
static RyanJsonBool compare_double(double a, double b)
{
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

static int rootNodeCheckTest(RyanJson_t json)
{
    if (!RyanJsonIsInt(RyanJsonGetObjectToKey(json, "inter")) || 16 != RyanJsonGetIntValue(RyanJsonGetObjectToKey(json, "inter")))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (!RyanJsonIsDouble(RyanJsonGetObjectToKey(json, "double")) || !compare_double(RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(json, "double")), 16.89))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (!RyanJsonIsString(RyanJsonGetObjectToKey(json, "string")) || strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(json, "string")), "hello"))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (!RyanJsonIsBool(RyanJsonGetObjectToKey(json, "boolTrue")) || RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "boolTrue")) != RyanJsonTrue)
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (!RyanJsonIsBool(RyanJsonGetObjectToKey(json, "boolFalse")) || RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "boolFalse")) != RyanJsonFalse)
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (!RyanJsonIsNull(RyanJsonGetObjectToKey(json, "null")))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int itemNodeCheckTest(RyanJson_t json)
{
    RyanJson_t item = RyanJsonGetObjectToKey(json, "item");
    if (0 != rootNodeCheckTest(item))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int arrayNodeCheckTest(RyanJson_t json)
{
    RyanJson_t item = NULL;

    // 判断是不是数组类型
    if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayInt")))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayDouble")))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayString")))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "array")))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    /**
     * @brief 检查弱类型数组
     *
     */
    //   array: [16, 16.89, "hello", true, false, null],
    if (!RyanJsonIsInt(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 0)) || 16 != RyanJsonGetIntValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 0)))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (!RyanJsonIsDouble(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 1)) || !compare_double(RyanJsonGetDoubleValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 1)), 16.89))
    {
        printf("%s:%d 解析失败 %f\r\n", __FILE__, __LINE__, RyanJsonGetDoubleValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 1)));
        return -1;
    }

    if (!RyanJsonIsString(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 2)) || strcmp(RyanJsonGetStringValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 2)), "hello"))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (!RyanJsonIsBool(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 3)) || RyanJsonGetBoolValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 3)) != RyanJsonTrue)
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (!RyanJsonIsBool(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 4)) || RyanJsonGetBoolValue(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 4)) != RyanJsonFalse)
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (!RyanJsonIsNull(RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "array"), 5)))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    /**
     * @brief 检查强类型数组
     *
     */
    for (uint8_t count = 0; count < RyanJsonGetSize(RyanJsonGetObjectToKey(json, "arrayInt")); count++)
    {
        item = RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "arrayInt"), count);
        if (!RyanJsonIsInt(item) || 16 != RyanJsonGetIntValue(item))
        {
            printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
            return -1;
        }
    }

    for (uint8_t count = 0; count < RyanJsonGetSize(RyanJsonGetObjectToKey(json, "arrayDouble")); count++)
    {
        item = RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "arrayDouble"), count);
        if (!RyanJsonIsDouble(item) || fabs(RyanJsonGetDoubleValue(item) - 16.8) < 0.001)
        {
            printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
            return -1;
        }
    }

    for (uint8_t count = 0; count < RyanJsonGetSize(RyanJsonGetObjectToKey(json, "arrayString")); count++)
    {
        item = RyanJsonGetObjectByIndex(RyanJsonGetObjectToKey(json, "arrayString"), count);
        if (!RyanJsonIsString(item) || strcmp(RyanJsonGetStringValue(item), "hello"))
        {
            printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
            return -1;
        }
    }

    if (6 != RyanJsonGetSize(RyanJsonGetObjectToKey(json, "array")))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

int arrayItemNodeCheckTest(RyanJson_t json)
{
    if (!RyanJsonIsArray(RyanJsonGetObjectToKey(json, "arrayItem")))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (0 != rootNodeCheckTest(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 0)))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (0 != rootNodeCheckTest(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 1)))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

/* --------------------------------------------------------------------- */

int loadJsonTest()
{
    char *str = NULL;
    RyanJson_t json;
    char jsonstr[] = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}";

    json = RyanJsonParse(jsonstr);
    if (json == NULL)
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    str = RyanJsonPrint(json, 250, RyanJsonFalse, NULL);
    if (strcmp(str, jsonstr) != 0)
    {
        printf("%s:%d 序列化与反序列化后的数据不对应\r\n", __FILE__, __LINE__);
        RyanJsonFree(str);
        RyanJsonDelete(json);
        return -1;
    }

    RyanJsonFree(str);

    if (0 != rootNodeCheckTest(json))
    {
        RyanJsonDelete(json);
        printf("%s:%d rootNodeCheckTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (0 != itemNodeCheckTest(json))
    {
        RyanJsonDelete(json);
        printf("%s:%d itemNodeCheckTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (0 != arrayNodeCheckTest(json))
    {
        RyanJsonDelete(json);
        printf("%s:%d arrayNodeCheckTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (0 != arrayItemNodeCheckTest(json))
    {
        RyanJsonDelete(json);
        printf("%s:%d arrayItemNodeCheckTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }
    RyanJsonDelete(json);

    /**
     * @brief 测试序列化错误json结构
     *
     */
    // \"inter\":16poi,  无效数字
    json = RyanJsonParse("{\"inter\":16poi,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}");
    if (json != NULL)
    {
        RyanJsonDelete(json);
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    // \"double\":16.8yu9,,  无效浮点数
    json = RyanJsonParse("{\"inter\":16,\"double\":16.8yu9,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}");
    if (json != NULL)
    {
        RyanJsonDelete(json);
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    // boolTrue 设置为 tru
    json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":tru,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}");
    if (json != NULL)
    {
        RyanJsonDelete(json);
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    // boolFalse 设置为 fale
    json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":fale,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}");
    if (json != NULL)
    {
        RyanJsonDelete(json);
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    // null 设置为 nul
    json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":nul,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}");
    if (json != NULL)
    {
        RyanJsonDelete(json);
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    // null 设置为 NULL
    json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":NULL,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}");
    if (json != NULL)
    {
        RyanJsonDelete(json);
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    // \"inter\":16后面少个,
    json = RyanJsonParse("{\"inter\":16\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}");
    if (json != NULL)
    {
        RyanJsonDelete(json);
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    // array数组项少一个,
    json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}");
    if (json != NULL)
    {
        RyanJsonDelete(json);
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    // \"item:{\"inter\":16,\"  少一个"
    json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item:{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}");
    if (json != NULL)
    {
        RyanJsonDelete(json);
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    // \"item\":{\"inter\":16,double\"  少一个"
    json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}");
    if (json != NULL)
    {
        RyanJsonDelete(json);
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    // \"item\":{\"inter\":16,\"\"double\"  多一个"
    json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}");
    if (json != NULL)
    {
        RyanJsonDelete(json);
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    // \"item\":{\"inter\":16\",\"double\"  多一个"
    json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16\",\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}");
    if (json != NULL)
    {
        RyanJsonDelete(json);
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    // \"arrayInt\":[16,16,16m,16,16]  无效数组数字
    json = RyanJsonParse("{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16m,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}");
    if (json != NULL)
    {
        RyanJsonDelete(json);
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }
    return 0;
}

int createJsonTest()
{
    RyanJson_t jsonRoot, item;

    // 对象生成测试
    jsonRoot = RyanJsonCreateObject();
    RyanJsonAddIntToObject(jsonRoot, "inter", 16);
    RyanJsonAddDoubleToObject(jsonRoot, "double", 16.89);
    RyanJsonAddStringToObject(jsonRoot, "string", "hello");
    RyanJsonAddBoolToObject(jsonRoot, "boolTrue", RyanJsonTrue);
    RyanJsonAddBoolToObject(jsonRoot, "boolFalse", RyanJsonFalse);
    RyanJsonAddNullToObject(jsonRoot, "null");

    /**
     * @brief 对象添加测试
     *
     */
    item = RyanJsonCreateObject();
    RyanJsonAddIntToObject(item, "inter", 16);
    RyanJsonAddDoubleToObject(item, "double", 16.89);
    RyanJsonAddStringToObject(item, "string", "hello");
    RyanJsonAddBoolToObject(item, "boolTrue", RyanJsonTrue);
    RyanJsonAddBoolToObject(item, "boolFalse", RyanJsonFalse);
    RyanJsonAddNullToObject(item, "null");
    RyanJsonAddItemToObject(jsonRoot, "item", item);

    /**
     * @brief 数组添加测试
     *
     */
    int arrayInt[] = {16, 16, 16, 16, 16};
    RyanJsonAddItemToObject(jsonRoot, "arrayInt",
                            RyanJsonCreateIntArray(arrayInt, sizeof(arrayInt) / sizeof(arrayInt[0])));

    double arrayDouble[] = {16.89, 16.89, 16.89, 16.89, 16.89};
    RyanJsonAddItemToObject(jsonRoot, "arrayDouble",
                            RyanJsonCreateDoubleArray(arrayDouble, sizeof(arrayDouble) / sizeof(arrayDouble[0])));

    const char *arrayString[] = {"hello", "hello", "hello", "hello", "hello"};
    RyanJsonAddItemToObject(jsonRoot, "arrayString",
                            RyanJsonCreateStringArray(arrayString, sizeof(arrayString) / sizeof(arrayString[0])));

    RyanJson_t array = RyanJsonCreateArray();
    RyanJsonAddIntToArray(array, 16);
    RyanJsonAddDoubleToArray(array, 16.89);
    RyanJsonAddStringToArray(array, "hello");
    RyanJsonAddBoolToArray(array, RyanJsonTrue);
    RyanJsonAddBoolToArray(array, RyanJsonFalse);
    RyanJsonAddNullToArray(array);
    RyanJsonAddItemToObject(jsonRoot, "array", array);

    /**
     * @brief 对象数组测试
     *
     */
    RyanJson_t arrayItem = RyanJsonCreateArray();
    item = RyanJsonCreateObject();
    RyanJsonAddIntToObject(item, "inter", 16);
    RyanJsonAddDoubleToObject(item, "double", 16.89);
    RyanJsonAddStringToObject(item, "string", "hello");
    RyanJsonAddBoolToObject(item, "boolTrue", RyanJsonTrue);
    RyanJsonAddBoolToObject(item, "boolFalse", RyanJsonFalse);
    RyanJsonAddNullToObject(item, "null");
    RyanJsonAddItemToObject(arrayItem, "item", item);

    item = RyanJsonCreateObject();
    RyanJsonAddIntToObject(item, "inter", 16);
    RyanJsonAddDoubleToObject(item, "double", 16.89);
    RyanJsonAddStringToObject(item, "string", "hello");
    RyanJsonAddBoolToObject(item, "boolTrue", RyanJsonTrue);
    RyanJsonAddBoolToObject(item, "boolFalse", RyanJsonFalse);
    RyanJsonAddNullToObject(item, "null");
    RyanJsonAddItemToObject(arrayItem, "item", item);

    RyanJsonAddItemToObject(jsonRoot, "arrayItem", arrayItem);

    if (0 != rootNodeCheckTest(jsonRoot))
    {
        RyanJsonDelete(jsonRoot);
        printf("%s:%d rootNodeCheckTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (0 != itemNodeCheckTest(jsonRoot))
    {
        RyanJsonDelete(jsonRoot);
        printf("%s:%d itemNodeCheckTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (0 != arrayNodeCheckTest(jsonRoot))
    {
        RyanJsonDelete(jsonRoot);
        printf("%s:%d arrayNodeCheckTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    if (0 != arrayItemNodeCheckTest(jsonRoot))
    {
        RyanJsonDelete(jsonRoot);
        printf("%s:%d arrayItemNodeCheckTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }
    RyanJsonDelete(jsonRoot);

    return 0;
}

int changeJsonTest()
{

    char jsonstr[] = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}";

    RyanJson_t json = RyanJsonParse(jsonstr);
    if (json == NULL)
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    /**
     * @brief 修改对应类型
     *
     */
    RyanJsonChangeIntValue(RyanJsonGetObjectToKey(json, "inter"), 20);
    if (!RyanJsonIsInt(RyanJsonGetObjectToKey(json, "inter")) || 20 != RyanJsonGetIntValue(RyanJsonGetObjectToKey(json, "inter")))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonChangeDoubleValue(RyanJsonGetObjectToKey(json, "double"), 20.89);
    if (!RyanJsonIsDouble(RyanJsonGetObjectToKey(json, "double")) || !compare_double(RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(json, "double")), 20.89))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonChangeStringValue(RyanJsonGetObjectToKey(json, "string"), "world");
    if (!RyanJsonIsString(RyanJsonGetObjectToKey(json, "string")) || strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(json, "string")), "world"))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(json, "boolTrue"), RyanJsonFalse);
    if (!RyanJsonIsBool(RyanJsonGetObjectToKey(json, "boolTrue")) || RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "boolTrue")) != RyanJsonFalse)
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(json, "boolFalse"), RyanJsonTrue);
    if (!RyanJsonIsBool(RyanJsonGetObjectToKey(json, "boolFalse")) || RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "boolFalse")) != RyanJsonTrue)
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    /* ---------------------------------- replace使用 -------------------------------------*/
    // 数组没有key, replace的子项不能有key, 函数内部没有做逻辑判断，会造成内存泄漏
    RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 0, RyanJsonCreateString(NULL, "arrayInt"));
    if (!RyanJsonIsString(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 0)) || strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 0)), "arrayInt"))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 0, RyanJsonCreateString(NULL, "arrayItem"));
    if (!RyanJsonIsString(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 0)) || strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 0)), "arrayItem"))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    // 对象必须包含key, 如果创建的对象key为null会引起内存错误
    RyanJsonReplaceByKey(json, "arrayString", RyanJsonCreateString("", "arrayString"));
    if (!RyanJsonIsString(RyanJsonGetObjectToKey(json, "arrayString")) || strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(json, "arrayString")), "arrayString"))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    // 修改数组节点为对象节点
    RyanJsonReplaceByKey(json, "arrayDouble", RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item")));
    if (!RyanJsonIsObject(RyanJsonGetObjectToKey(json, "arrayDouble")) || -1 == rootNodeCheckTest(RyanJsonGetObjectToKey(json, "arrayDouble")))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);

        goto err;
    }

    /**
     * @brief 对象子项删除测试
     *
     */
    RyanJsonDeleteByIndex(json, 0);
    if (RyanJsonGetObjectToKey(json, "inter"))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDeleteByKey(json, "double");
    if (RyanJsonGetObjectToKey(json, "double"))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    /**
     * @brief 数组对象子项删除测试
     *
     */
    RyanJsonDeleteByIndex(RyanJsonGetObjectToKey(json, "array"), 0);
    if (RyanJsonGetSize(RyanJsonGetObjectToKey(json, "array")) != 5)
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    // str = RyanJsonPrint(json, 1024, RyanJsonTrue, NULL);
    // printf("aa %s\r\n", str);
    // RyanJsonFree(str);
    RyanJsonDelete(json);
    return 0;

err:
    RyanJsonDelete(json);
    return -1;
}

int compareJsonTest()
{
    char jsonstr[] = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}";
    // char jsonstr[] = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}";

    RyanJson_t json = RyanJsonParse(jsonstr);
    RyanJson_t json2 = RyanJsonParse(jsonstr);

    // 比较函数
    if (RyanJsonTrue != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonAddStringToObject(json2, "test", "hello");
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonAddIntToObject(json2, "test", 1);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonAddDoubleToObject(json2, "test", 2.0);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonAddBoolToObject(json2, "test", RyanJsonTrue);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonAddNullToObject(json2, "test");
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonAddIntToArray(RyanJsonGetObjectToKey(json2, "arrayInt"), 2);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonAddDoubleToArray(RyanJsonGetObjectToKey(json2, "arrayDouble"), 2.0);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonAddStringToArray(RyanJsonGetObjectToKey(json2, "arrayString"), "hello");
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonAddItemToArray(RyanJsonGetObjectToKey(json2, "arrayItem"), RyanJsonCreateString("test", "hello"));
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonChangeKey(RyanJsonGetObjectToKey(json2, "inter"), "int2");
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonChangeIntValue(RyanJsonGetObjectToKey(json2, "inter"), 17);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonChangeDoubleValue(RyanJsonGetObjectToKey(json2, "double"), 20.89);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonChangeStringValue(RyanJsonGetObjectToKey(json2, "string"), "49");
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(json2, "boolTrue"), RyanJsonFalse);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(json2, "item", "boolTrue"), RyanJsonFalse);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonChangeIntValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "arrayInt"), 0), 17);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonChangeDoubleValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "arrayDouble"), 0), 20.89);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonChangeStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "arrayString"), 0), "20.89");
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonChangeIntValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "array"), 0), 17);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonChangeIntValue(RyanJsonGetObjectToKey(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json2, "arrayItem"), 0), "inter"), 17);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonDeleteByKey(json2, "arrayItem");
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonDeleteByIndex(RyanJsonGetObjectToKey(json2, "arrayInt"), 2);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json2);
    json2 = RyanJsonParse(jsonstr);
    RyanJsonDeleteByIndex(RyanJsonGetObjectToKey(json2, "arrayItem"), 0);
    if (RyanJsonFalse != RyanJsonCompare(json, json2))
    {
        printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
        goto err;
    }

    RyanJsonDelete(json);
    RyanJsonDelete(json2);
    return 0;

err:
    RyanJsonDelete(json);
    RyanJsonDelete(json2);
    return -1;
}

int duplicateTest()
{
    RyanJson_t json, dupItem, jsonRoot = NULL;
    char jsonstr[] = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}";

    /**
     * @brief 普通类型
     *
     */
    json = RyanJsonParse(jsonstr);
    dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "inter"));
    if (RyanJsonFalse == RyanJsonCompare(dupItem, RyanJsonGetObjectToKey(json, "inter")))
    {
        goto err;
    }
    RyanJsonDelete(dupItem);

    dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "inter"));
    RyanJsonAddItemToObject(json, "test", dupItem);
    if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test", "inter"), RyanJsonGetObjectToKey(json, "inter")))
    {
        goto err;
    }
    RyanJsonDelete(RyanJsonDetachByKey(json, "test"));

    dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "inter"));
    RyanJsonAddItemToObject(json, "test", dupItem);
    if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test", "inter"), RyanJsonGetObjectToKey(json, "inter")))
    {
        goto err;
    }
    RyanJsonDelete(json);

    json = RyanJsonParse(jsonstr);
    dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "inter"));
    RyanJsonAddItemToObject(json, "test", dupItem);
    if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test", "inter"), RyanJsonGetObjectToKey(json, "inter")))
    {
        goto err;
    }
    RyanJsonDelete(RyanJsonDetachByKey(json, "test"));
    RyanJsonDelete(json);

    /**
     * @brief 对象类型
     *
     */
    json = RyanJsonParse(jsonstr);
    dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));
    if (RyanJsonFalse == RyanJsonCompare(dupItem, RyanJsonGetObjectToKey(json, "item")))
    {
        goto err;
    }
    RyanJsonDelete(dupItem);

    dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));
    RyanJsonAddItemToObject(json, "test", dupItem);
    if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "item")))
    {
        goto err;
    }
    RyanJsonDelete(RyanJsonDetachByKey(json, "test"));

    dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));
    RyanJsonAddItemToObject(json, "test", dupItem);
    if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "item")))
    {
        goto err;
    }
    RyanJsonDelete(json);

    json = RyanJsonParse(jsonstr);
    dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));
    RyanJsonAddItemToObject(json, "test", dupItem);
    if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "item")))
    {
        goto err;
    }
    RyanJsonDelete(RyanJsonDetachByKey(json, "test"));
    RyanJsonDelete(json);

    /**
     * @brief 数组类型
     *
     */
    json = RyanJsonParse(jsonstr);
    dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "arrayItem"));
    if (RyanJsonFalse == RyanJsonCompare(dupItem, RyanJsonGetObjectToKey(json, "arrayItem")))
    {
        goto err;
    }
    RyanJsonDelete(dupItem);

    dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "arrayItem"));
    RyanJsonAddItemToObject(json, "test", dupItem);
    if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "arrayItem")))
    {
        goto err;
    }
    RyanJsonDelete(RyanJsonDetachByKey(json, "test"));

    dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "arrayItem"));
    RyanJsonAddItemToObject(json, "test", dupItem);
    if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "arrayItem")))
    {
        goto err;
    }
    RyanJsonDelete(json);

    json = RyanJsonParse(jsonstr);
    dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "arrayItem"));
    RyanJsonAddItemToObject(json, "test", dupItem);
    if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "arrayItem")))
    {
        goto err;
    }
    RyanJsonDelete(RyanJsonDetachByKey(json, "test"));
    RyanJsonDelete(json);

    json = RyanJsonParse(jsonstr);
    jsonRoot = RyanJsonCreateObject();
    RyanJsonAddBoolToObject(jsonRoot, "arrayItem", RyanJsonTrue);
    int use = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
        dupItem = RyanJsonParse(jsonstr);
        RyanJsonReplaceByKey(jsonRoot, "arrayItem", RyanJsonDuplicate(dupItem));
        if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(jsonRoot, "arrayItem"), dupItem))
        {
            goto err;
        }
        RyanJsonReplaceByKey(json, "arrayItem", RyanJsonDuplicate(RyanJsonGetObjectByKey(dupItem, "item")));
        if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "arrayItem"), RyanJsonGetObjectByKey(dupItem, "item")))
        {
            goto err;
        }
        RyanJsonDelete(dupItem);

        int newuse = 0;
        v_mcheck(NULL, &newuse);
        if (i != 0 && newuse != use)
        {
            printf("%s:%d 内存泄漏\r\n", __FILE__, __LINE__);
            goto err;
        }
        use = newuse;
    }

    RyanJsonDelete(json);
    RyanJsonDelete(jsonRoot);
    return 0;

err:
    RyanJsonDelete(json);
    RyanJsonDelete(jsonRoot);
    return -1;
}

int likeReferenceTest()
{

    // char *str = NULL;
    // char jsonstr[] = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}";
    // RyanJson_t json = RyanJsonParse(jsonstr);
    // RyanJson_t item = NULL;

    // // RyanJson_t adfasdf = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));

    // // RyanJsonAddItemToObject(json, "test", adfasdf);

    // // 这里做你想做的事,这里我选择打印出来
    // // str = RyanJsonPrint(json, 50, RyanJsonTrue, NULL);
    // // printf("item %s \r\n", str);
    // // RyanJsonFree(str);

    // for (int i = 0; i < 1; i++)
    // {
    //     // 分离test对象
    //     item = RyanJsonDetachByKey(json, "item");

    //     // if (RyanJsonIsKey(item))
    //     //     RyanJsonFree(RyanJsonGetKey(item));

    //     // RyanJsonFree(item);
    // }

    // RyanJsonAddItemToObject(json, "item", item);

    // str = RyanJsonPrint(json, 50, RyanJsonTrue, NULL);
    // printf("item %s \r\n", str);
    // RyanJsonFree(str);

    // RyanJsonDelete(json);

    return 0;
}

int forEachTest()
{
    char *str = NULL;
    char jsonstr[] = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}";

    RyanJson_t json = RyanJsonParse(jsonstr);
    RyanJson_t item = NULL;
    printf("arrayDouble: ");
    RyanJsonArrayForEach(RyanJsonGetObjectToKey(json, "arrayDouble"), item)
    {
        if (!RyanJsonIsDouble(item) || 16.89 != RyanJsonGetDoubleValue(item))
        {
            goto err;
        }
    }

    RyanJsonArrayForEach(RyanJsonGetObjectToKey(json, "arrayInt"), item)
    {
        if (!RyanJsonIsInt(item) || 16 != RyanJsonGetIntValue(item))
        {
            goto err;
        }
    }

    uint32_t strLen;
    RyanJsonObjectForEach(RyanJsonGetObjectToKey(json, "item"), item)
    {
        str = RyanJsonPrint(item, 50, RyanJsonTrue, &strLen);
        printf("item { %s : %s }  %d\r\n", RyanJsonGetKey(item), str, strLen);
        RyanJsonFree(str);
    }

    RyanJsonDelete(json);
    return 0;

err:
    RyanJsonDelete(json);
    return -1;
}

int RyanJsonTest(void)
{
    int result = 0;
    RyanJsonInitHooks(v_malloc, v_free, v_realloc);

    result = loadJsonTest(); // 从文本解析json测试
    if (0 != result)
    {
        printf("%s:%d loadJsonTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    result = createJsonTest(); // 生成json节点树测试
    if (0 != result)
    {
        printf("%s:%d loadJsonTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    result = changeJsonTest(); // 修改json节点测试,包含删除、分离
    if (0 != result)
    {
        printf("%s:%d loadJsonTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    result = compareJsonTest(); // 比较json节点树测试
    if (0 != result)
    {
        printf("%s:%d loadJsonTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    result = duplicateTest(); // 复制测试
    if (0 != result)
    {
        printf("%s:%d loadJsonTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    result = forEachTest();
    if (0 != result)
    {
        printf("%s:%d loadJsonTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    result = likeReferenceTest(); // 模仿 引用类型实现 示例
    if (0 != result)
    {
        printf("%s:%d loadJsonTest fail\r\n", __FILE__, __LINE__);
        return -1;
    }

    displayMem();

    return 0;
}
