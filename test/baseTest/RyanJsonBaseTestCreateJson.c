
#include "RyanJsonBaseTest.h"

RyanJsonBool_e RyanJsonBaseTestCreateJson(void)
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
	RyanJsonAddItemToObject(jsonRoot, "arrayInt", RyanJsonCreateIntArray(arrayInt, sizeof(arrayInt) / sizeof(arrayInt[0])));

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

	RyanJsonCheckCode(RyanJsonTrue == RyanJsonBaseTestCheckRoot(jsonRoot), {
		RyanJsonDelete(jsonRoot);
		return RyanJsonFalse;
	});
	RyanJsonDelete(jsonRoot);

	return RyanJsonTrue;
}
