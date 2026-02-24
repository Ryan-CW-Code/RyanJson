#include "RyanJson.h"

/**
 * @brief Json 构建示例
 *
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e createJsonExample(void)
{
	char *str = NULL;
	RyanJson_t jsonRoot, item;

	// 构建根对象
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
	RyanJsonAddItemToObject(jsonRoot, "item", item); // 把子对象挂到根对象

	// 添加数字子数组
	int32_t arrayInt[] = {16, 16, 16, 16, 16};
	RyanJsonAddItemToObject(jsonRoot, "arrayInt", RyanJsonCreateIntArray(arrayInt, sizeof(arrayInt) / sizeof(arrayInt[0])));

	// 添加浮点数子数组
	double arrayDouble[] = {16.89, 16.89, 16.89, 16.89, 16.89};
	RyanJsonAddItemToObject(jsonRoot, "arrayDouble",
				RyanJsonCreateDoubleArray(arrayDouble, sizeof(arrayDouble) / sizeof(arrayDouble[0])));

	// 添加字符串子数组
	const char *arrayString[] = {"hello", "hello", "hello", "hello", "hello"};
	RyanJsonAddItemToObject(jsonRoot, "arrayString",
				RyanJsonCreateStringArray(arrayString, sizeof(arrayString) / sizeof(arrayString[0])));

	// 添加杂项数组
	RyanJson_t array = RyanJsonCreateArray();
	RyanJsonAddIntToArray(array, 16);
	RyanJsonAddDoubleToArray(array, 16.89);
	RyanJsonAddStringToArray(array, "hello");
	RyanJsonAddBoolToArray(array, RyanJsonTrue);
	RyanJsonAddBoolToArray(array, RyanJsonFalse);
	RyanJsonAddNullToArray(array);
	RyanJsonAddItemToObject(jsonRoot, "array", array); // 把混合数组挂到根对象

	// 添加对象数组
	RyanJson_t arrayItem = RyanJsonCreateArray();
	item = RyanJsonCreateObject();
	RyanJsonAddIntToObject(item, "inter", 16);
	RyanJsonAddDoubleToObject(item, "double", 16.89);
	RyanJsonAddStringToObject(item, "string", "hello");
	RyanJsonAddBoolToObject(item, "boolTrue", RyanJsonTrue);
	RyanJsonAddBoolToObject(item, "boolFalse", RyanJsonFalse);
	RyanJsonAddNullToObject(item, "null");
	RyanJsonAddItemToObject(arrayItem, "item", item); // 将对象加入数组

	item = RyanJsonCreateObject();
	RyanJsonAddIntToObject(item, "inter", 16);
	RyanJsonAddDoubleToObject(item, "double", 16.89);
	RyanJsonAddStringToObject(item, "string", "hello");
	RyanJsonAddBoolToObject(item, "boolTrue", RyanJsonTrue);
	RyanJsonAddBoolToObject(item, "boolFalse", RyanJsonFalse);
	RyanJsonAddNullToObject(item, "null");
	RyanJsonAddItemToObject(arrayItem, "item", item); // 将对象加入数组

	RyanJsonAddItemToObject(jsonRoot, "arrayItem", arrayItem); // 把对象数组挂到根对象

	uint32_t len = 0;
	str = RyanJsonPrint(jsonRoot, 250, RyanJsonTrue, &len); // 格式化打印
	printf("strLen: %" PRIu32 ", data: %s\r\n", len, str);
	RyanJsonFree(str);

	RyanJsonDelete(jsonRoot);
	return RyanJsonTrue;
}

/**
 * @brief Json 解析示例
 *
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e loadJsonExample(void)
{
	char *str = NULL;
	RyanJson_t jsonRoot = NULL;
	static const char *jsonstr =
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
		"\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\","
		"\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,"
		"16.89,16.89,16.89,16.89],\"arrayString\":[\"hello\","
		"\"hello\",\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{"
		"\"inter\":16,\"double\":16.89,\"string\":\"hello\","
		"\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\","
		"\"boolTrue\":true,\"boolFalse\":false,\"null\":null}]}";

	// 解析 Json 文本
	jsonRoot = RyanJsonParse(jsonstr);
	if (NULL == jsonRoot)
	{
		printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
		return RyanJsonFalse;
	}

	// 读取 int32_t 数据
	int32_t inter = RyanJsonGetIntValue(RyanJsonGetObjectByKey(jsonRoot, "inter"));
	if (16 != inter)
	{
		printf("%s:%d 读取int失败\r\n", __FILE__, __LINE__);
		RyanJsonDelete(jsonRoot);
		return RyanJsonFalse;
	}

	// 读取 double 数据
	double doubleValue = RyanJsonGetDoubleValue(RyanJsonGetObjectByKey(jsonRoot, "double"));
	if (RyanJsonFalse == RyanJsonCompareDouble(doubleValue, 16.89))
	{
		printf("%s:%d 读取double失败\r\n", __FILE__, __LINE__);
		RyanJsonDelete(jsonRoot);
		return RyanJsonFalse;
	}

	// 读取 string 数据
	const char *strValue = RyanJsonGetStringValue(RyanJsonGetObjectByKey(jsonRoot, "string"));
	if (0 != strcmp(strValue, "hello"))
	{
		printf("%s:%d 读取string失败\r\n", __FILE__, __LINE__);
		RyanJsonDelete(jsonRoot);
		return RyanJsonFalse;
	}

	// 读取 bool 数据
	RyanJsonBool_e boolValue = RyanJsonGetBoolValue(RyanJsonGetObjectByKey(jsonRoot, "boolTrue"));
	if (RyanJsonTrue != boolValue)
	{
		printf("%s:%d 读取bool失败\r\n", __FILE__, __LINE__);
		RyanJsonDelete(jsonRoot);
		return RyanJsonFalse;
	}

	// 读取 null 数据
	if (RyanJsonTrue != RyanJsonIsNull(RyanJsonGetObjectByKey(jsonRoot, "null")))
	{
		printf("%s:%d 读取null失败\r\n", __FILE__, __LINE__);
		RyanJsonDelete(jsonRoot);
		return RyanJsonFalse;
	}

	// 将解析结果以无格式重新输出，并与原始文本对比
	str = RyanJsonPrint(jsonRoot, 250, RyanJsonFalse, NULL);
	if (0 != strcmp(str, jsonstr))
	{
		printf("%s:%d 解析后再序列化结果不一致  %s\r\n", __FILE__, __LINE__, str);
		RyanJsonFree(str);
		RyanJsonDelete(jsonRoot);
		return RyanJsonFalse;
	}
	RyanJsonFree(str);

	// 将解析结果以格式化样式打印
	uint32_t len = 0;
	str = RyanJsonPrint(jsonRoot, 250, RyanJsonTrue, &len);
	printf("strLen: %" PRIu32 ", data: %s\r\n", len, str);
	RyanJsonFree(str);

	// 释放 Json 对象
	RyanJsonDelete(jsonRoot);

	return RyanJsonTrue;
}

/**
 * @brief Json 修改示例
 *
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e changeJsonExample(void)
{
	char *str = NULL;
	RyanJson_t jsonRoot;
	const char *jsonstr = "{\"name\":\"Mash\",\"star\":4,\"doubleKey\":4.4,\"boolKey\":true,\"hits\":[2,2,1,3]}";

	// 解析 Json 文本
	jsonRoot = RyanJsonParse(jsonstr);
	if (jsonRoot == NULL)
	{
		printf("%s:%d 解析失败\r\n", __FILE__, __LINE__);
		return RyanJsonFalse;
	}

	// 修改 key
	RyanJsonChangeKey(RyanJsonGetObjectByKey(jsonRoot, "name"), "name2");
	if (0 != strcmp("name2", RyanJsonGetKey(RyanJsonGetObjectByKey(jsonRoot, "name2"))))
	{
		printf("%s:%d 修改失败\r\n", __FILE__, __LINE__);
		RyanJsonDelete(jsonRoot);
		return RyanJsonFalse;
	}

	// 修改 strValue
	RyanJsonChangeStringValue(RyanJsonGetObjectByKey(jsonRoot, "name2"), "Ryan");
	if (0 != strcmp("Ryan", RyanJsonGetStringValue(RyanJsonGetObjectByKey(jsonRoot, "name2"))))
	{
		printf("%s:%d 修改失败\r\n", __FILE__, __LINE__);
		RyanJsonDelete(jsonRoot);
		return RyanJsonFalse;
	}

	// 修改 intValue
	RyanJsonChangeIntValue(RyanJsonGetObjectByKey(jsonRoot, "star"), 5);
	if (5 != RyanJsonGetIntValue(RyanJsonGetObjectByKey(jsonRoot, "star")))
	{
		printf("%s:%d 修改失败\r\n", __FILE__, __LINE__);
		RyanJsonDelete(jsonRoot);
		return RyanJsonFalse;
	}

	// 修改 doubleValue
	RyanJsonChangeDoubleValue(RyanJsonGetObjectByKey(jsonRoot, "doubleKey"), 5.5);
	if (RyanJsonFalse == RyanJsonCompareDouble(RyanJsonGetDoubleValue(RyanJsonGetObjectByKey(jsonRoot, "doubleKey")), 5.5))
	{
		printf("%s:%d 修改失败\r\n", __FILE__, __LINE__);
		RyanJsonDelete(jsonRoot);
		return RyanJsonFalse;
	}

	// 修改 boolValue
	RyanJsonChangeBoolValue(RyanJsonGetObjectByKey(jsonRoot, "boolKey"), RyanJsonFalse);
	if (RyanJsonFalse != RyanJsonGetBoolValue(RyanJsonGetObjectByKey(jsonRoot, "boolKey")))
	{
		printf("%s:%d 修改失败\r\n", __FILE__, __LINE__);
		RyanJsonDelete(jsonRoot);
		return RyanJsonFalse;
	}

	// 替换节点（同时改变节点类型）
	RyanJsonReplaceByKey(jsonRoot, "star", RyanJsonCreateString("", "123456"));

	// 将修改结果以格式化样式打印
	uint32_t len = 0;
	str = RyanJsonPrint(jsonRoot, 250, RyanJsonTrue, &len);
	printf("strLen: %" PRIu32 ", data: %s\r\n", len, str);
	RyanJsonFree(str);

	// 释放 Json 对象
	RyanJsonDelete(jsonRoot);

	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonExample(void)
{
	RyanJsonInitHooks(malloc, free, NULL);

	printf("\r\n--------------------------- RyanJson 生成示例 --------------------------\r\n");
	RyanJsonCheckReturnFalse(RyanJsonTrue == createJsonExample());

	printf("\r\n--------------------------- RyanJson 解析json文本示例 --------------------------\r\n");
	RyanJsonCheckReturnFalse(RyanJsonTrue == loadJsonExample());

	printf("\r\n--------------------------- RyanJson 修改json示例 --------------------------\r\n");
	RyanJsonCheckReturnFalse(RyanJsonTrue == changeJsonExample());

	// 更多功能请查看 RyanJson.h 与 test 目录下的测试用例

	return RyanJsonTrue;
}
