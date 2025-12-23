#include "RyanJsonBaseTest.h"

/* --------------------------------------------------------------------- */

RyanJsonBool_e RyanJsonBaseTestDetachJson()
{
	char jsonstr[] =
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":"
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],"
		"\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],"
		"\"array\":[16,16.89,\"hello\",true,false,null],"
		"\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{"
		"\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,"
		"\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,"
		"\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}],"
		"\"string2222\":\"hello\"}";

	RyanJson_t json = RyanJsonParse(jsonstr);
	RyanJsonCheckReturnFalse(NULL != json);

	/**
	 * @brief 对象子项分离测试（头、中、尾）
	 */
	{
		RyanJson_t json2 = RyanJsonParse(jsonstr);

		// 头部 (第一个 key: inter)
		RyanJsonDelete(RyanJsonDetachByIndex(json, 0));
		if (RyanJsonGetObjectToKey(json, "inter"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		// 中间 (double)
		RyanJsonDelete(RyanJsonDetachByKey(json, "double"));
		if (RyanJsonGetObjectToKey(json, "double"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		// 尾部 (最后一个 key: string2222)
		uint32_t lastIndex = RyanJsonGetSize(json) - 1;
		RyanJsonDelete(RyanJsonDetachByIndex(json, lastIndex));
		if (RyanJsonGetObjectToKey(json, "string2222"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonDelete(json2);
	}

	/**
	 * @brief 数组元素分离测试 (arrayInt 头、中、尾)
	 */
	{
		RyanJson_t arr = RyanJsonGetObjectByKey(json, "arrayInt");
		RyanJson_t json2 = RyanJsonParse(jsonstr);

		// 头部
		RyanJsonDelete(RyanJsonDetachByIndex(arr, 0));
		if (RyanJsonGetObjectToIndex(arr, 0) == NULL)
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		// 中间
		RyanJsonDelete(RyanJsonDetachByIndex(arr, 2));
		if (RyanJsonGetObjectToIndex(arr, 2) == NULL)
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		// 尾部
		uint32_t lastIndex = RyanJsonGetSize(arr) - 1;
		RyanJsonDelete(RyanJsonDetachByIndex(arr, lastIndex));
		if (NULL != RyanJsonGetObjectToIndex(arr, lastIndex))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonDelete(json2);
	}

	/**
	 * @brief 数组元素分离测试 (arrayDouble 头、中、尾)
	 */
	{
		RyanJson_t arr = RyanJsonGetObjectByKey(json, "arrayDouble");
		RyanJson_t json2 = RyanJsonParse(jsonstr);

		RyanJsonDelete(RyanJsonDetachByIndex(arr, 0));
		if (RyanJsonGetObjectToIndex(arr, 0) == NULL)
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonDelete(RyanJsonDetachByIndex(arr, 2));
		if (RyanJsonGetObjectToIndex(arr, 2) == NULL)
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		uint32_t lastIndex = RyanJsonGetSize(arr) - 1;
		RyanJsonDelete(RyanJsonDetachByIndex(arr, lastIndex));
		if (RyanJsonGetObjectToIndex(arr, lastIndex) != NULL)
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonDelete(json2);
	}

	/**
	 * @brief 数组元素分离测试 (arrayString 头、中、尾)
	 */
	{
		RyanJson_t arr = RyanJsonGetObjectByKey(json, "arrayString");
		RyanJson_t json2 = RyanJsonParse(jsonstr);

		RyanJsonDelete(RyanJsonDetachByIndex(arr, 0));
		if (RyanJsonGetObjectToIndex(arr, 0) == NULL)
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonDelete(RyanJsonDetachByIndex(arr, 2));
		if (RyanJsonGetObjectToIndex(arr, 2) == NULL)
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		uint32_t lastIndex = RyanJsonGetSize(arr) - 1;
		RyanJsonDelete(RyanJsonDetachByIndex(arr, lastIndex));
		if (RyanJsonGetObjectToIndex(arr, lastIndex) != NULL)
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonDelete(json2);
	}

	/**
	 * @brief 嵌套对象分离测试 (item)
	 */
	{
		RyanJson_t json2 = RyanJsonParse(jsonstr);
		RyanJsonDelete(RyanJsonDetachByKey(json, "item"));
		if (RyanJsonGetObjectToKey(json, "item"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}
		RyanJsonDelete(json2);
	}

	/**
	 * @brief 数组对象元素分离测试 (arrayItem 头、中、尾)
	 */
	{
		RyanJson_t arr = RyanJsonGetObjectByKey(json, "arrayItem");
		RyanJson_t json2 = RyanJsonParse(jsonstr);

		RyanJsonDelete(RyanJsonDetachByIndex(arr, 0));
		if (RyanJsonGetObjectToIndex(arr, 0) == NULL)
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonDelete(RyanJsonDetachByIndex(arr, 1));
		if (RyanJsonGetObjectToIndex(arr, 1) == NULL)
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		uint32_t lastIndex = RyanJsonGetSize(arr) - 1;
		RyanJsonDelete(RyanJsonDetachByIndex(arr, lastIndex));
		if (RyanJsonGetObjectToIndex(arr, lastIndex) != NULL)
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonDelete(json2);
	}

	/**
	 * @brief 特殊类型分离测试（null / bool）
	 */
	{
		RyanJson_t json2 = RyanJsonParse(jsonstr);

		RyanJsonDelete(RyanJsonDetachByKey(json, "null"));
		if (RyanJsonGetObjectToKey(json, "null"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonDelete(RyanJsonDetachByKey(json, "boolTrue"));
		if (RyanJsonGetObjectToKey(json, "boolTrue"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonDelete(json2);
	}

	char *str = RyanJsonPrint(json, 1024, RyanJsonTrue, NULL);
	RyanJsonFree(str);
	RyanJsonDelete(json);
	return RyanJsonTrue;

err:
	RyanJsonDelete(json);
	return RyanJsonFalse;
}
