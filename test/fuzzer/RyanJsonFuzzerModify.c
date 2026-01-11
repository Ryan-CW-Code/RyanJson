#include "RyanJsonFuzzer.h"

RyanJsonBool_e RyanJsonFuzzerTestModify(RyanJson_t pJson, uint32_t size)
{
	RyanJsonIsNull(pJson);

	if (RyanJsonIsKey(pJson))
	{
		char *key = (char *)malloc(strlen(RyanJsonGetKey(pJson)) + 1);
		if (key)
		{
			memcpy(key, RyanJsonGetKey(pJson), strlen(RyanJsonGetKey(pJson)));
			key[strlen(RyanJsonGetKey(pJson))] = 0;

			RyanJsonChangeKey(pJson, "key");
			RyanJsonChangeKey(pJson, key);
			free(key);
		}
	}
	if (RyanJsonIsBool(pJson)) { RyanJsonChangeBoolValue(pJson, !RyanJsonGetBoolValue(pJson)); }
	if (RyanJsonIsNumber(pJson))
	{
		if (RyanJsonIsInt(pJson))
		{
			int32_t value = RyanJsonGetIntValue(pJson);
			RyanJsonChangeIntValue(pJson, (int32_t)size);
			RyanJsonChangeIntValue(pJson, value);
		}
		if (RyanJsonIsDouble(pJson))
		{
			double value = RyanJsonGetDoubleValue(pJson);
			RyanJsonChangeDoubleValue(pJson, size * 1.123456789);
			RyanJsonChangeDoubleValue(pJson, value);
		}
	}

	if (RyanJsonIsString(pJson))
	{
		char *value = (char *)malloc(strlen(RyanJsonGetStringValue(pJson)) + 1);
		if (value)
		{
			memcpy(value, RyanJsonGetStringValue(pJson), strlen(RyanJsonGetStringValue(pJson)));
			value[strlen(RyanJsonGetStringValue(pJson))] = 0;

			RyanJsonChangeStringValue(pJson, "hello world");
			RyanJsonChangeStringValue(pJson, "h");
			RyanJsonChangeStringValue(pJson, value);

			free(value);
		}
	}

	if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
	{
		RyanJson_t item;
		RyanJsonArrayForEach(pJson, item)
		{
			RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonFuzzerTestModify(item, size));
		}
	}

	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonFuzzerVerifyGet(RyanJson_t lastJson, RyanJson_t pJson, uint32_t index, uint32_t size)
{
	RyanJsonIsNull(pJson);

	RyanJsonAssert(NULL == RyanJsonGetKey(NULL));
	RyanJsonAssert(NULL == RyanJsonGetStringValue(NULL));
	RyanJsonAssert(0 == RyanJsonGetIntValue(NULL));
	RyanJsonAssert(RyanJsonTrue == RyanJsonCompareDouble(RyanJsonGetDoubleValue(NULL), 0.0));
	RyanJsonAssert(NULL == RyanJsonGetObjectValue(NULL));
	RyanJsonAssert(NULL == RyanJsonGetArrayValue(NULL));

	RyanJsonAssert(NULL == RyanJsonGetObjectByKey(NULL, NULL));
	RyanJsonAssert(NULL == RyanJsonGetObjectByKey(pJson, NULL));
	RyanJsonAssert(NULL == RyanJsonGetObjectByKey(NULL, "NULL"));

	RyanJsonAssert(NULL == RyanJsonGetObjectByKeys(NULL, NULL));
	RyanJsonAssert(NULL == RyanJsonGetObjectByKeys(pJson, NULL));
	RyanJsonAssert(NULL == RyanJsonGetObjectByKeys(NULL, "NULL"));
	if (!RyanJsonIsObject(pJson)) // pJson类型错误
	{
		RyanJsonAssert(NULL == RyanJsonGetObjectByKey(pJson, "NULL"));
	}

	RyanJsonAssert(NULL == RyanJsonGetObjectByIndex(NULL, 10));
	if (!RyanJsonIsArray(pJson) && !RyanJsonIsObject(pJson)) // pJson类型错误
	{
		RyanJsonAssert(NULL == RyanJsonGetObjectByIndex(pJson, 0));
	}

	if (RyanJsonIsKey(pJson)) { RyanJsonGetObjectToKey(lastJson, RyanJsonGetKey(pJson)); }
	else
	{
		RyanJsonGetObjectToIndex(lastJson, index);
	}

	if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
	{
		RyanJson_t item;
		RyanJsonObjectForEach(pJson, item) { RyanJsonFuzzerVerifyGet(pJson, item, index, size); }
	}

	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonFuzzerTestGet(RyanJson_t pJson, uint32_t size)
{
	RyanJsonAssert(RyanJsonFalse == RyanJsonIsKey(NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonIsNull(NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonIsBool(NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonIsNumber(NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonIsString(NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonIsArray(NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonIsObject(NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonIsInt(NULL));
	RyanJsonAssert(RyanJsonFalse == RyanJsonIsDouble(NULL));

	if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
	{
		RyanJson_t item;
		uint32_t index = 0;
		RyanJsonObjectForEach(pJson, item)
		{
			RyanJsonFuzzerVerifyGet(pJson, item, index, size);
			index++;
		}
	}
	return RyanJsonTrue;
}
