#include "RyanJsonFuzzer.h"

RyanJson_t RyanJsonFuzzerCreateRandomNode(RyanJson_t pJson)
{
	static int32_t count = 0;
	static int32_t count2 = 0;
	count++;
	char *key = "true";
	if (0 != count % 10 && 5 < count % 10) { key = NULL; }
	switch (count % 50)
	{
	case 0: return RyanJsonCreateArray();
	case 1: return RyanJsonCreateObject();
	case 2:
		count2++;
		if (0 == count2 % 10) { return RyanJsonDuplicate(pJson); }
		// fallthrough
	case 11:
	case 12:
	case 13: return RyanJsonCreateBool(key, RyanJsonTrue);
	case 20:
	case 21:
	case 22: return RyanJsonCreateInt(key, count);
	case 31:
	case 32:
	case 33: return RyanJsonCreateDouble(key, count * 1.123456789);

	default: return RyanJsonCreateString(key, "true");
	}
}

RyanJsonBool_e RyanJsonFuzzerTestCreate(RyanJson_t pJson, uint32_t size)
{
	// RyanJsonInsert的特殊情况
	RyanJsonAssert(RyanJsonFalse == RyanJsonInsert(NULL, UINT32_MAX, RyanJsonCreateString("key", "string")));
	RyanJsonAssert(RyanJsonFalse == RyanJsonInsert(pJson, UINT32_MAX, NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonInsert(NULL, 0, NULL));

	RyanJsonAssert(NULL == RyanJsonCreateString(NULL, NULL));
	RyanJsonAssert(NULL == RyanJsonCreateString("NULL", NULL));

	RyanJsonAssert(RyanJsonFalse == RyanJsonChangeStringValue(NULL, NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonChangeStringValue(pJson, NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonChangeStringValue(NULL, "NULL"));
	if (RyanJsonFalse == RyanJsonIsKey(pJson) && RyanJsonFalse == RyanJsonIsString(pJson)) // pJson类型错误
	{
		RyanJsonAssert(RyanJsonFalse == RyanJsonChangeStringValue(pJson, "NULL"));
	}

	RyanJsonAssert(RyanJsonFalse == RyanJsonChangeKey(NULL, NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonChangeKey(pJson, NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonChangeKey(NULL, "NULL"));
	if (RyanJsonFalse == RyanJsonIsKey(pJson) && RyanJsonFalse == RyanJsonIsString(pJson)) // pJson类型错误
	{
		RyanJsonAssert(RyanJsonFalse == RyanJsonChangeKey(pJson, "NULL"));
	}

	// 测试没有key但是有strValue的
	if (RyanJsonFalse == RyanJsonIsKey(pJson) && RyanJsonTrue == RyanJsonIsString(pJson)) { RyanJsonAssert(RyanJsonFalse == RyanJsonChangeKey(pJson, "NULL")); }

	RyanJsonAssert(RyanJsonFalse == RyanJsonChangeIntValue(NULL, 0));
	RyanJsonAssert(RyanJsonFalse == RyanJsonChangeDoubleValue(NULL, 0));

	RyanJsonAssert(RyanJsonFalse == RyanJsonAddItemToObject(NULL, NULL, NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonAddItemToObject(pJson, NULL, NULL));

	RyanJsonAssert(NULL == RyanJsonCreateIntArray(NULL, 0));
	RyanJsonAssert(NULL == RyanJsonCreateDoubleArray(NULL, 0));
	RyanJsonAssert(NULL == RyanJsonCreateStringArray(NULL, 0));

	RyanJsonAssert(RyanJsonFalse == RyanJsonHasObjectToKey(NULL, "0", "1", "2", "3"));
	RyanJsonAssert(RyanJsonFalse == RyanJsonHasObjectToIndex(NULL, 0, 1, 2, 3));
	RyanJsonAssert(RyanJsonFalse == RyanJsonHasObjectToKey(pJson, "0", "1", "2", "3"));
	RyanJsonAssert(RyanJsonFalse == RyanJsonHasObjectToIndex(pJson, 0, 1, 2, 3));

	char *key = "keyaaa";
	RyanJsonAddNullToObject(pJson, key);
	if (RyanJsonTrue == RyanJsonIsKey(pJson)) { key = RyanJsonGetKey(pJson); }
	if (RyanJsonTrue == RyanJsonIsBool(pJson)) { RyanJsonAddBoolToObject(pJson, key, RyanJsonGetBoolValue(pJson)); }
	if (RyanJsonTrue == RyanJsonIsNumber(pJson))
	{
		if (RyanJsonTrue == RyanJsonIsInt(pJson))
		{
			RyanJsonAddIntToObject(pJson, key, RyanJsonGetIntValue(pJson));
			int32_t arrayInt[] = {RyanJsonGetIntValue(pJson), RyanJsonGetIntValue(pJson), RyanJsonGetIntValue(pJson),
					  RyanJsonGetIntValue(pJson), RyanJsonGetIntValue(pJson)};
			RyanJsonAddItemToObject(pJson, (0 != size % 2) ? key : "arrayString",
						RyanJsonCreateIntArray(arrayInt, sizeof(arrayInt) / sizeof(arrayInt[0])));
		}
		if (RyanJsonTrue == RyanJsonIsDouble(pJson))
		{
			RyanJsonAddDoubleToObject(pJson, key, RyanJsonGetDoubleValue(pJson));
			double arrayDouble[] = {RyanJsonGetDoubleValue(pJson), RyanJsonGetDoubleValue(pJson), RyanJsonGetDoubleValue(pJson),
						RyanJsonGetDoubleValue(pJson), RyanJsonGetDoubleValue(pJson)};
			RyanJsonAddItemToObject(pJson, (0 != size % 2) ? key : "arrayString",
						RyanJsonCreateDoubleArray(arrayDouble, sizeof(arrayDouble) / sizeof(arrayDouble[0])));
		}
	}

	if (RyanJsonTrue == RyanJsonIsString(pJson))
	{
		RyanJsonAddStringToObject(pJson, key, RyanJsonGetStringValue(pJson));
		const char *arrayString[] = {RyanJsonGetStringValue(pJson), RyanJsonGetStringValue(pJson), RyanJsonGetStringValue(pJson),
					     RyanJsonGetStringValue(pJson), RyanJsonGetStringValue(pJson)};
		RyanJsonAddItemToObject(pJson, (0 != size % 2) ? key : "arrayString",
					RyanJsonCreateStringArray(arrayString, sizeof(arrayString) / sizeof(arrayString[0])));
	}

	if (RyanJsonTrue == RyanJsonIsArray(pJson) || RyanJsonTrue == RyanJsonIsObject(pJson))
	{
		RyanJson_t item;

		RyanJsonObjectForEach(pJson, item) { RyanJsonFuzzerTestCreate(item, size); }

		RyanJson_t pJson2 = RyanJsonFuzzerCreateRandomNode(pJson);
		RyanJsonAddItemToObject(pJson, key, pJson2);

		if (RyanJsonTrue == RyanJsonIsArray(pJson))
		{
			RyanJsonAddNullToArray(pJson);
			RyanJsonAddBoolToArray(pJson, 0 != size % 2 ? RyanJsonTrue : RyanJsonFalse);
			RyanJsonAddItemToArray(pJson, RyanJsonFuzzerCreateRandomNode(RyanJsonGetArrayValue(pJson)));
		}
	}

	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonFuzzerTestReplace(RyanJson_t pJson, uint32_t size)
{
	{
		isEnableRandomMemFail = RyanJsonFalse;
		RyanJson_t strItem = RyanJsonCreateString("", "NULL");
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByKey(NULL, NULL, NULL));
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByKey(pJson, NULL, NULL));
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByKey(NULL, "NULL", NULL));
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByKey(NULL, NULL, strItem));
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByKey(pJson, "NULL", NULL));
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByKey(NULL, "NULL", strItem));
		if (RyanJsonFalse == RyanJsonIsObject(pJson)) // pJson类型错误
		{
			RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByKey(pJson, "NULL", strItem));
		}

		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByIndex(NULL, 0, NULL));
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByIndex(pJson, 0, NULL));
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByIndex(NULL, 0, strItem));
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByIndex(pJson, 0, NULL));
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByIndex(NULL, 0, strItem));
		if (RyanJsonFalse == RyanJsonIsArray(pJson) && RyanJsonFalse == RyanJsonIsObject(pJson)) // pJson类型错误
		{
			RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByIndex(pJson, 0, strItem));
		}

		RyanJson_t objItem = RyanJsonCreateObject();
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByKey(objItem, "NULL", strItem));
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByIndex(objItem, 0, strItem));

		RyanJsonAddItemToObject(objItem, "item", RyanJsonCreateObject());
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByKey(objItem, "NULL222", strItem));
		RyanJsonAssert(RyanJsonFalse == RyanJsonReplaceByIndex(objItem, INT32_MAX, strItem));
		isEnableRandomMemFail = RyanJsonTrue;

		RyanJsonDelete(objItem);
		RyanJsonDelete(strItem);
	}

	// 只处理数组或对象
	if (RyanJsonFalse == RyanJsonIsArray(pJson) && RyanJsonFalse == RyanJsonIsObject(pJson)) { return RyanJsonTrue; }

	// 递归替换子节点
	RyanJson_t item = NULL;
	RyanJson_t LastItem = NULL;
	RyanJsonObjectForEach(pJson, item)
	{
		if (RyanJsonTrue != RyanJsonFuzzerTestReplace(item, size)) { return RyanJsonFalse; }
		LastItem = item;
	}

	// 只有非根节点才做替换

	// 按 key 替换（仅对象）
	// 不要动第一个节点
	if (RyanJsonTrue == RyanJsonIsObject(pJson))
	{
		if (NULL != LastItem && RyanJsonTrue == RyanJsonIsKey(LastItem))
		{
			RyanJson_t newNode = RyanJsonFuzzerCreateRandomNode(pJson);
			if (RyanJsonFalse == RyanJsonReplaceByKey(pJson, RyanJsonGetKey(LastItem), newNode))
			{
				if (NULL != newNode) { RyanJsonDelete(newNode); }
				return RyanJsonFalse;
			}
		}
	}

	// 按 index 替换
	{
		uint32_t idx = RyanJsonGetSize(pJson) % size;
		RyanJson_t newNode = RyanJsonFuzzerCreateRandomNode(pJson);
		if (RyanJsonFalse == RyanJsonReplaceByIndex(pJson, (0 != size % 25) ? idx : 0, newNode))
		{
			if (NULL != newNode) { RyanJsonDelete(newNode); }
			return RyanJsonFalse;
		}
	}

	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonFuzzerTestDetach(RyanJson_t pJson, uint32_t size)
{
	RyanJsonAssert(RyanJsonFalse == RyanJsonDetachByKey(NULL, NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonDetachByKey(pJson, NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonDetachByKey(NULL, "NULL"));
	if (RyanJsonFalse == RyanJsonIsObject(pJson)) // pJson类型错误
	{
		RyanJsonAssert(NULL == RyanJsonDetachByKey(pJson, "NULL"));
	}

	RyanJsonAssert(NULL == RyanJsonDetachByIndex(NULL, 10));
	if (RyanJsonFalse == RyanJsonIsArray(pJson) && RyanJsonFalse == RyanJsonIsObject(pJson)) // pJson类型错误
	{
		RyanJsonAssert(NULL == RyanJsonDetachByIndex(pJson, 0));
	}

	if (RyanJsonFalse == RyanJsonIsArray(pJson) && RyanJsonFalse == RyanJsonIsObject(pJson)) { return RyanJsonTrue; }

	// 递归遍历子节点
	RyanJson_t item = NULL;
	RyanJson_t LastItem = NULL;
	RyanJsonObjectForEach(pJson, item)
	{
		RyanJsonFuzzerTestDetach(item, size);
		LastItem = item;
	}

	// 只有非根节点才做 detach

	// 按 key 分离（仅对象）
	if (RyanJsonTrue == RyanJsonIsObject(pJson))
	{
		if (NULL != LastItem && RyanJsonTrue == RyanJsonIsKey(LastItem))
		{
			RyanJson_t detached = RyanJsonDetachByKey(pJson, RyanJsonGetKey(LastItem));
			if (NULL != detached) { RyanJsonDelete(detached); }

			// RyanJsonAssert(RyanJsonFalse == RyanJsonDetachByKey(pJson, RyanJsonGetKey(LastItem)));
		}
	}

	// 按 index 分离
	{
		uint32_t idx = RyanJsonGetSize(pJson) % size;
		RyanJson_t detached = RyanJsonDetachByIndex(pJson, (0 != size % 25) ? idx : 0);
		if (NULL != detached) { RyanJsonDelete(detached); }
	}

	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonFuzzerTestDelete(RyanJson_t pJson, uint32_t size)
{
	if (RyanJsonFalse == RyanJsonIsArray(pJson) && RyanJsonFalse == RyanJsonIsObject(pJson)) { return RyanJsonTrue; }

	// -------- 测试错误的 delete 调用 --------
	// Key 删除错误用例
	RyanJsonDeleteByKey(pJson, "non_exist_key");
	RyanJsonDeleteByKey(NULL, "some_key");
	RyanJsonDeleteByKey(pJson, NULL);
	RyanJsonDeleteByKey(NULL, NULL);

	// Index 删除错误用例
	RyanJsonDeleteByIndex(pJson, RyanJsonGetSize(pJson)); // 越界
	RyanJsonDeleteByIndex(NULL, (RyanJsonGetSize(pJson) % size));
	RyanJsonDeleteByIndex(pJson, (uint32_t)(-(int32_t)size)); // 负数
	RyanJsonDeleteByIndex(NULL, (uint32_t)(-(int32_t)size));

	// 递归遍历子节点
	RyanJson_t item = NULL;
	RyanJson_t LastItem = NULL;
	RyanJsonObjectForEach(pJson, item)
	{
		RyanJsonFuzzerTestDelete(item, size);
		LastItem = item;
	}

	// -------- 正常删除逻辑（保护根节点） --------

	// 按 key 删除（仅对象）
	if (RyanJsonTrue == RyanJsonIsObject(pJson))
	{
		if (NULL != LastItem && RyanJsonTrue == RyanJsonIsKey(LastItem))
		{

			// printf("key is %d %s\r\n", RyanJsonGetType(LastItem),
			//        RyanJsonGetKey(LastItem) == NULL ? "NULL" : RyanJsonGetKey(LastItem));
			RyanJsonDeleteByKey(pJson, RyanJsonGetKey(LastItem));
		}
	}

	// 按 index 删除
	uint32_t idx = RyanJsonGetSize(pJson) % size;
	RyanJsonDeleteByIndex(pJson, (0 != size % 25) ? idx : 0);

	return RyanJsonTrue;
}
