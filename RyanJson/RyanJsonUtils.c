#include "RyanJsonUtils.h"

/**
 * @brief 连续通过 key 获取json对象的子项
 *
 * @param pJson
 * @param key
 * @param ... 可变参，连续输入key，直到NULL结束
 * @return RyanJson_t
 */
RyanJson_t RyanJsonGetObjectByKeys(RyanJson_t pJson, const char *key, ...)
{
	RyanJsonCheckReturnNull(NULL != pJson && NULL != key);

	const char *nextKey = key;
	RyanJson_t nextItem = RyanJsonGetObjectByKey(pJson, nextKey);
	RyanJsonCheckReturnNull(NULL != nextItem && RyanJsonIsKey(nextItem));

	va_list args;
	va_start(args, key);
	nextKey = va_arg(args, const char *);
	while (nextItem && NULL != nextKey)
	{
		nextItem = RyanJsonGetObjectByKey(nextItem, nextKey);
		nextKey = va_arg(args, char *);
	}
	va_end(args);

	return nextItem;
}

/**
 * @brief 连续通过 索引 获取json对象的子项
 *
 * @param pJson
 * @param index
 * @param ... 可变参，连续输入索引，直到INT_MIN结束
 * @return RyanJson_t
 */
RyanJson_t RyanJsonGetObjectByIndexs(RyanJson_t pJson, uint32_t index, ...)
{
	RyanJsonCheckReturnNull(NULL != pJson);

	uint32_t nextIndex = index;
	RyanJson_t nextItem = RyanJsonGetObjectByIndex(pJson, nextIndex);
	RyanJsonCheckReturnNull(NULL != nextItem);

	va_list args;
	va_start(args, index);
	nextIndex = va_arg(args, uint32_t);
	while (nextItem && nextIndex > 0)
	{
		nextItem = RyanJsonGetObjectByIndex(nextItem, nextIndex);
		nextIndex = va_arg(args, uint32_t);
	}
	va_end(args);

	return nextItem;
}

/**
 * @brief 创建一个int类型的数组json对象
 *
 * @param numbers 数组的地址必须为int类型
 * @param count 数组的长度
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateIntArray(const int32_t *numbers, uint32_t count)
{
	RyanJsonCheckReturnNull(NULL != numbers);

	RyanJson_t pJson = RyanJsonCreateArray();
	for (uint32_t i = 0; pJson && i < count; i++) { RyanJsonAddIntToArray(pJson, numbers[i]); }
	return pJson;
}

/**
 * @brief 创建一个double类型的数组json对象
 *
 * @param numbers
 * @param count
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateDoubleArray(const double *numbers, uint32_t count)
{
	RyanJsonCheckReturnNull(NULL != numbers);

	RyanJson_t pJson = RyanJsonCreateArray();
	for (uint32_t i = 0; pJson && i < count; i++) { RyanJsonAddDoubleToArray(pJson, numbers[i]); }
	return pJson;
}

/**
 * @brief 创建一个string类型的数组json对象
 *
 * @param strings
 * @param count
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateStringArray(const char **strings, uint32_t count)
{
	RyanJsonCheckReturnNull(NULL != strings);

	RyanJson_t pJson = RyanJsonCreateArray();
	for (uint32_t i = 0; pJson && i < count; i++) { RyanJsonAddStringToArray(pJson, strings[i]); }
	return pJson;
}

/**
 * @brief 递归比较两个 pJson 对象key是否相等。
 * 此接口效率较低, 谨慎使用
 * @param leftJson
 * @param rightJson
 * @return RyanJsonBool_e
 */
RyanJsonBool_e RyanJsonCompareOnlyKey(RyanJson_t leftJson, RyanJson_t rightJson)
{
	if (NULL == leftJson || NULL == rightJson) { return RyanJsonFalse; }

	// 相同的对象相等
	if (leftJson == rightJson) { return RyanJsonTrue; }

	if (RyanJsonGetType(leftJson) != RyanJsonGetType(rightJson)) { return RyanJsonFalse; }

	switch (RyanJsonGetType(leftJson))
	{
	case RyanJsonTypeBool:
	case RyanJsonTypeNull:
	case RyanJsonTypeNumber:
	case RyanJsonTypeString: return RyanJsonTrue;

	case RyanJsonTypeArray: {
		if (RyanJsonGetSize(leftJson) != RyanJsonGetSize(rightJson)) { return RyanJsonFalse; }

		RyanJson_t item;
		uint32_t itemIndex = 0;
		RyanJsonArrayForEach(leftJson, item)
		{
			if (RyanJsonTrue != RyanJsonCompareOnlyKey(item, RyanJsonGetObjectByIndex(rightJson, itemIndex)))
			{
				return RyanJsonFalse;
			}
			itemIndex++;
		}
		return RyanJsonTrue;
	}

	case RyanJsonTypeObject: {
		if (RyanJsonGetSize(leftJson) != RyanJsonGetSize(rightJson)) { return RyanJsonFalse; }

		RyanJson_t item;
		RyanJsonObjectForEach(leftJson, item)
		{
			if (RyanJsonTrue != RyanJsonCompareOnlyKey(item, RyanJsonGetObjectByKey(rightJson, RyanJsonGetKey(item))))
			{
				return RyanJsonFalse;
			}
		}
		return RyanJsonTrue;
	}
	}

	return RyanJsonFalse;
}
