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

	const char *s = key;
	RyanJson_t nextItem = RyanJsonGetObjectByKey(pJson, s);
	RyanJsonCheckReturnNull(NULL != nextItem && RyanJsonIsKey(nextItem));

	va_list args;
	va_start(args, key);
	s = va_arg(args, const char *);
	while (nextItem && NULL != s)
	{
		nextItem = RyanJsonGetObjectByKey(nextItem, s);
		s = va_arg(args, char *);
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
RyanJson_t RyanJsonGetObjectByIndexs(RyanJson_t pJson, int32_t index, ...)
{
	RyanJsonCheckReturnNull(NULL != pJson && index >= 0);

	int32_t i = index;
	RyanJson_t nextItem = RyanJsonGetObjectByIndex(pJson, i);
	RyanJsonCheckReturnNull(NULL != nextItem);

	va_list args;
	va_start(args, index);
	i = va_arg(args, int32_t);
	while (nextItem && INT32_MIN != i)
	{
		nextItem = RyanJsonGetObjectByIndex(nextItem, i);
		i = va_arg(args, int32_t);
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
RyanJson_t RyanJsonCreateIntArray(const int32_t *numbers, int32_t count)
{
	RyanJsonCheckReturnNull(NULL != numbers && count > 0);

	RyanJson_t pJson = RyanJsonCreateArray();
	for (int32_t i = 0; pJson && i < count; i++) { RyanJsonAddIntToArray(pJson, numbers[i]); }
	return pJson;
}

/**
 * @brief 创建一个double类型的数组json对象
 *
 * @param numbers
 * @param count
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateDoubleArray(const double *numbers, int32_t count)
{
	RyanJsonCheckReturnNull(NULL != numbers && count > 0);

	RyanJson_t pJson = RyanJsonCreateArray();
	for (int32_t i = 0; pJson && i < count; i++) { RyanJsonAddDoubleToArray(pJson, numbers[i]); }
	return pJson;
}

/**
 * @brief 创建一个string类型的数组json对象
 *
 * @param strings
 * @param count
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateStringArray(const char **strings, int32_t count)
{
	RyanJsonCheckReturnNull(NULL != strings && count > 0);

	RyanJson_t pJson = RyanJsonCreateArray();
	for (int32_t i = 0; pJson && i < count; i++) { RyanJsonAddStringToArray(pJson, strings[i]); }
	return pJson;
}

/**
 * @brief 递归比较两个 pJson 对象key是否相等。
 * 此接口效率较低, 谨慎使用
 * @param a
 * @param b
 * @return RyanJsonBool_e
 */
RyanJsonBool_e RyanJsonCompareOnlyKey(RyanJson_t a, RyanJson_t b)
{
	if (NULL == a || NULL == b) { return RyanJsonFalse; }

	// 相同的对象相等
	if (a == b) { return RyanJsonTrue; }

	if (RyanJsonGetType(a) != RyanJsonGetType(b)) { return RyanJsonFalse; }

	switch (RyanJsonGetType(a))
	{
	case RyanJsonTypeBool:
	case RyanJsonTypeNull:
	case RyanJsonTypeNumber:
	case RyanJsonTypeString: return RyanJsonTrue;

	case RyanJsonTypeArray: {
		if (RyanJsonGetSize(a) != RyanJsonGetSize(b)) { return RyanJsonFalse; }

		for (int32_t count = 0; count < RyanJsonGetSize(a); count++)
		{
			if (RyanJsonTrue != RyanJsonCompareOnlyKey(RyanJsonGetObjectByIndex(a, count), RyanJsonGetObjectByIndex(b, count)))
			{
				return RyanJsonFalse;
			}
		}
		return RyanJsonTrue;
	}

	case RyanJsonTypeObject: {
		RyanJson_t a_element, b_element;
		if (RyanJsonGetSize(a) != RyanJsonGetSize(b)) { return RyanJsonFalse; }

		RyanJsonObjectForEach(a, a_element)
		{
			b_element = RyanJsonGetObjectByKey(b, RyanJsonGetKey(a_element));
			if (NULL == b_element) { return RyanJsonFalse; }

			if (RyanJsonTrue != RyanJsonCompareOnlyKey(a_element, b_element)) { return RyanJsonFalse; }
		}

		return RyanJsonTrue;
	}
	}

	return RyanJsonFalse;
}
