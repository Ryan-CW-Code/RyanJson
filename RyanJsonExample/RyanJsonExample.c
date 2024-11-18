
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "RyanJson.h"
#include "valloc.h"

/**
 * @brief 生成json示例
 *
 * @return int
 */
int createJsonExample()
{
    char *str = NULL;
    RyanJson_t jsonRoot, item;

    // 对象生成测试
    jsonRoot = RyanJsonCreateObject();
    RyanJsonAddIntToObject(jsonRoot, "inter", 16);
    RyanJsonAddDoubleToObject(jsonRoot, "double", 16.89);
    RyanJsonAddStringToObject(jsonRoot, "string", "hello");
    RyanJsonAddBoolToObject(jsonRoot, "boolTrue", RyanJsonTrue);
    RyanJsonAddBoolToObject(jsonRoot, "boolFalse", RyanJsonFalse);
    RyanJsonAddNullToObject(jsonRoot, "null");

    // 添加子对象
    item = RyanJsonCreateObject();
    RyanJsonAddIntToObject(item, "inter", 16);
    RyanJsonAddDoubleToObject(item, "double", 16.89);
    RyanJsonAddStringToObject(item, "string", "hello");
    RyanJsonAddBoolToObject(item, "boolTrue", RyanJsonTrue);
    RyanJsonAddBoolToObject(item, "boolFalse", RyanJsonFalse);
    RyanJsonAddNullToObject(item, "null");
    RyanJsonAddItemToObject(jsonRoot, "item", item); // 将上面创建的item子对象添加到root父对象

    // 添加子数组
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
    RyanJsonAddItemToObject(jsonRoot, "array", array); // 将上面创建的item子对象数组添加到root父对象

    // 添加对象数组
    RyanJson_t arrayItem = RyanJsonCreateArray();
    item = RyanJsonCreateObject();
    RyanJsonAddIntToObject(item, "inter", 16);
    RyanJsonAddDoubleToObject(item, "double", 16.89);
    RyanJsonAddStringToObject(item, "string", "hello");
    RyanJsonAddBoolToObject(item, "boolTrue", RyanJsonTrue);
    RyanJsonAddBoolToObject(item, "boolFalse", RyanJsonFalse);
    RyanJsonAddNullToObject(item, "null");
    RyanJsonAddItemToObject(arrayItem, "item", item); // 将item对象添加到arrayItem数组里面

    item = RyanJsonCreateObject();
    RyanJsonAddIntToObject(item, "inter", 16);
    RyanJsonAddDoubleToObject(item, "double", 16.89);
    RyanJsonAddStringToObject(item, "string", "hello");
    RyanJsonAddBoolToObject(item, "boolTrue", RyanJsonTrue);
    RyanJsonAddBoolToObject(item, "boolFalse", RyanJsonFalse);
    RyanJsonAddNullToObject(item, "null");
    RyanJsonAddItemToObject(arrayItem, "item", item); // 将item对象添加到arrayItem数组里面

    RyanJsonAddItemToObject(jsonRoot, "arrayItem", arrayItem); // 将arrayItem数组添加到root父对象

    uint32_t len = 0;
    str = RyanJsonPrint(jsonRoot, 250, RyanJsonTrue, &len); // 以带格式方式将数据打印出来
    printf("strLen: %d, data: %s\r\n", len, str);
    RyanJsonFree(str);

    RyanJsonDelete(jsonRoot);

    return 0;
}

/**
 * @brief 序列化json文本示例
 *
 * @return int
 */
int loadJsonExample()
{
    char *str = NULL;
    RyanJson_t jsonRoot;
    const char jsonstr[] = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}";

    // 解析json数据
    jsonRoot = RyanJsonParse(jsonstr);
    if (jsonRoot == NULL)
    {
        printf("%s:%d 序列化失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    // 将序列化的数据以无格式样式打印出来，并和原始数据进行对比
    str = RyanJsonPrint(jsonRoot, 250, RyanJsonFalse, NULL);
    if (strcmp(str, jsonstr) != 0)
    {
        printf("%s:%d 序列化与反序列化后的数据不对应\r\n", __FILE__, __LINE__);
        RyanJsonFree(str);
        RyanJsonDelete(jsonRoot);
        return -1;
    }
    RyanJsonFree(str);

    // 将序列化的数据以有格式样式打印出来
    uint32_t len = 0;
    str = RyanJsonPrint(jsonRoot, 250, RyanJsonTrue, &len);
    printf("strLen: %d, data: %s\r\n", len, str);
    RyanJsonFree(str);

    // 删除json对象
    RyanJsonDelete(jsonRoot);

    return 0;
}

/**
 * @brief 修改json示例
 *
 * @return int
 */
int changeJsonExample()
{
    char *str = NULL;
    RyanJson_t jsonRoot;
    const char *jsonstr = "{\"name\":\"Mash\",\"star\":4,\"hits\":[2,2,1,3]}";

    // 解析json数据
    jsonRoot = RyanJsonParse(jsonstr);
    if (jsonRoot == NULL)
    {
        printf("%s:%d 序列化失败\r\n", __FILE__, __LINE__);
        return -1;
    }

    RyanJsonChangeStringValue(RyanJsonGetObjectByKey(jsonRoot, "name"), "Ryan");
    if (0 != strcmp("Ryan", RyanJsonGetStringValue(RyanJsonGetObjectByKey(jsonRoot, "name"))))
    {
        printf("%s:%d 修改失败\r\n", __FILE__, __LINE__);
        RyanJsonDelete(jsonRoot);
        return -1;
    }

    RyanJsonReplaceByKey(jsonRoot, "star", RyanJsonCreateString("", "123456"));

    // 将序列化的数据以有格式样式打印出来
    uint32_t len = 0;
    str = RyanJsonPrint(jsonRoot, 250, RyanJsonTrue, &len);
    printf("strLen: %d, data: %s\r\n", len, str);
    RyanJsonFree(str);

    // 删除json对象
    RyanJsonDelete(jsonRoot);

    return 0;
}

int RyanJsonExample(void)
{
    RyanJsonInitHooks(v_malloc, v_free, v_realloc);

    printf("\r\n--------------------------- RyanJson 生成示例 --------------------------\r\n");
    createJsonExample();

    printf("\r\n--------------------------- RyanJson 序列化json文本示例 --------------------------\r\n");
    loadJsonExample();

    printf("\r\n--------------------------- RyanJson 修改json示例 --------------------------\r\n");
    changeJsonExample();

    return -1;
}
