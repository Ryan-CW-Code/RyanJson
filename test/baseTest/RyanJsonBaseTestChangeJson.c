
#include "RyanJsonBaseTest.h"

/* --------------------------------------------------------------------- */

RyanJsonBool_e RyanJsonBaseTestChangeJson()
{

	char jsonstr[] = "{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":"
			 "{\"inter\":16,\"double\":16."
			 "89,\"string\":\"hello\","
			 "\"boolTrue\":true,\"boolFalse\":false,\"null\":null},\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,"
			 "16.89,16.89,16.89],"
			 "\"arrayString\":[\"hello\",\"hello\","
			 "\"hello\",\"hello\",\"hello\"],\"array\":[16,16.89,\"hello\",true,false,null],\"arrayItem\":[{\"inter\":16,"
			 "\"double\":16.89,\"string\":"
			 "\"hello\",\"boolTrue\":true,"
			 "\"boolFalse\":false,\"null\":null},{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,"
			 "\"boolFalse\":false,\"null\":null}],"
			 "\"string2222\":\"hello\"}";

	RyanJson_t json = RyanJsonParse(jsonstr);
	RyanJsonCheckReturnFlase(NULL != json);

	/**
	 * @brief 修改对应类型
	 *
	 */
	RyanJsonChangeIntValue(RyanJsonGetObjectToKey(json, "inter"), 20);
	RyanJsonCheckCode(RyanJsonIsInt(RyanJsonGetObjectToKey(json, "inter")) &&
				  20 == RyanJsonGetIntValue(RyanJsonGetObjectToKey(json, "inter")),
			  { goto err; });

	RyanJsonChangeDoubleValue(RyanJsonGetObjectToKey(json, "double"), 20.89);
	if (!RyanJsonIsDouble(RyanJsonGetObjectToKey(json, "double")) ||
	    !compare_double(RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(json, "double")), 20.89))
	{
		RyanJsonCheckCode(NULL, { goto err; });
	}

	RyanJsonChangeStringValue(RyanJsonGetObjectToKey(json, "string"), "world");
	if (!RyanJsonIsString(RyanJsonGetObjectToKey(json, "string")) ||
	    strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(json, "string")), "world"))
	{
		RyanJsonCheckCode(NULL, { goto err; });
	}

	RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(json, "boolTrue"), RyanJsonFalse);
	if (!RyanJsonIsBool(RyanJsonGetObjectToKey(json, "boolTrue")) ||
	    RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "boolTrue")) != RyanJsonFalse)
	{
		RyanJsonCheckCode(NULL, { goto err; });
	}

	RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(json, "boolFalse"), RyanJsonTrue);
	if (!RyanJsonIsBool(RyanJsonGetObjectToKey(json, "boolFalse")) ||
	    RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "boolFalse")) != RyanJsonTrue)
	{
		RyanJsonCheckCode(NULL, { goto err; });
	}

	RyanJsonChangeKey(RyanJsonGetObjectToKey(json, "inter"), "inter");
	RyanJsonChangeKey(RyanJsonGetObjectToKey(json, "double"), "double22222222");
	RyanJsonChangeKey(RyanJsonGetObjectToKey(json, "string2222"), "string333");
	RyanJsonChangeStringValue(RyanJsonGetObjectToKey(json, "string333"), "stringValye333");
	if (!RyanJsonIsBool(RyanJsonGetObjectToKey(json, "boolFalse")) ||
	    RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "boolFalse")) != RyanJsonTrue)
	{
		RyanJsonCheckCode(NULL, { goto err; });
	}

	/* ---------------------------------- replace使用 -------------------------------------*/
	{
		// 数组没有key, replace的子项不能有key, 函数内部没有做逻辑判断，会造成内存泄漏
		RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 0, RyanJsonCreateString(NULL, "arrayInt"));
		if (!RyanJsonIsString(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 0)) ||
		    strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 0)), "arrayInt"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayInt"),
				       RyanJsonGetSize(RyanJsonGetObjectToKey(json, "arrayInt")) - 1,
				       RyanJsonCreateString(NULL, "arrayInt"));
		if (!RyanJsonIsString(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayInt"),
							       RyanJsonGetSize(RyanJsonGetObjectToKey(json, "arrayInt")) - 1)) ||
		    strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayInt"),
									   RyanJsonGetSize(RyanJsonGetObjectToKey(json, "arrayInt")) - 1)),
			   "arrayInt"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 0, RyanJsonCreateString(NULL, "arrayItem"));
		if (!RyanJsonIsString(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 0)) ||
		    strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 0)), "arrayItem"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 1, RyanJsonCreateString(NULL, "arrayItem"));
		if (!RyanJsonIsString(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 1)) ||
		    strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 1)), "arrayItem"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		// 对象必须包含key, 如果创建的对象key为null会引起内存错误
		RyanJsonReplaceByKey(json, "arrayString", RyanJsonCreateString("", "arrayString2222"));
		if (!RyanJsonIsString(RyanJsonGetObjectToKey(json, "arrayString")) ||
		    strcmp(RyanJsonGetStringValue(RyanJsonGetObjectToKey(json, "arrayString")), "arrayString2222"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		// 修改数组节点为对象节点
		RyanJson_t duplicateJson = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));
		printfJsonaaaa(duplicateJson);

		// RyanJson_t item = RyanJsonCreateObject2("key");
		// // RyanJson_t item = RyanJsonCreateObject();
		// RyanJsonAddIntToObject(item, "inter", 16);
		// RyanJsonAddDoubleToObject(item, "double", 16.89);
		// RyanJsonAddStringToObject(item, "string", "hello");
		// RyanJsonAddBoolToObject(item, "boolTrue", RyanJsonTrue);
		// RyanJsonAddBoolToObject(item, "boolFalse", RyanJsonFalse);
		// RyanJsonAddNullToObject(item, "null");

		// RyanJsonReplaceByKey(json, "arrayDouble", item);
		RyanJsonReplaceByKey(json, "arrayDouble", duplicateJson);
		if (!RyanJsonIsObject(RyanJsonGetObjectToKey(json, "arrayDouble")) ||
		    -1 == rootNodeCheckTest(RyanJsonGetObjectToKey(json, "arrayDouble")))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}
	}

	/**
	 * @brief 对象子项删除测试
	 *
	 */
	{
		RyanJsonDeleteByIndex(json, 0);
		if (RyanJsonGetObjectToKey(json, "inter"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}

		RyanJsonDeleteByKey(json, "double");
		if (RyanJsonGetObjectToKey(json, "double"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}
	}

	/**
	 * @brief 数组对象子项删除测试
	 *
	 */
	{
		RyanJsonDeleteByIndex(RyanJsonGetObjectToKey(json, "array"), 0);
		if (RyanJsonGetSize(RyanJsonGetObjectToKey(json, "array")) != 5)
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}
	}

	/**
	 * @brief 对象子项分离测试
	 *
	 */
	{
		RyanJson_t json2 = RyanJsonParse(jsonstr);
		RyanJsonDelete(RyanJsonDetachByIndex(json, 0));
		if (RyanJsonGetObjectToKey(json, "inter"))
		{
			RyanJsonCheckCode(NULL, { goto err; });
		}
		RyanJsonDelete(json2);

		json2 = RyanJsonParse(jsonstr);
		RyanJsonDelete(RyanJsonDetachByKey(json, "inter"));
		if (RyanJsonGetObjectToKey(json, "inter"))
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
