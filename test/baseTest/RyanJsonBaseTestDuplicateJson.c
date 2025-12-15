
#include "RyanJsonBaseTest.h"

RyanJsonBool_e RyanJsonBaseTestDuplicateJson()
{
	RyanJson_t json, dupItem, jsonRoot = NULL;
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
			 "\"boolFalse\":false,\"null\":null}]}";

	/**
	 * @brief 普通类型
	 *
	 */
	json = RyanJsonParse(jsonstr);
	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "inter"));
	if (RyanJsonFalse == RyanJsonCompare(dupItem, RyanJsonGetObjectToKey(json, "inter"))) { goto err; }
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
	if (RyanJsonFalse == RyanJsonCompare(dupItem, RyanJsonGetObjectToKey(json, "item"))) { goto err; }
	RyanJsonDelete(dupItem);

	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));
	RyanJsonAddItemToObject(json, "test", dupItem);
	if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "item"))) { goto err; }
	RyanJsonDelete(RyanJsonDetachByKey(json, "test"));

	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));
	RyanJsonAddItemToObject(json, "test", dupItem);
	if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "item"))) { goto err; }
	RyanJsonDelete(json);

	json = RyanJsonParse(jsonstr);
	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));
	RyanJsonAddItemToObject(json, "test", dupItem);
	if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "item"))) { goto err; }
	RyanJsonDelete(RyanJsonDetachByKey(json, "test"));
	RyanJsonDelete(json);

	/**
	 * @brief 数组类型
	 *
	 */
	json = RyanJsonParse(jsonstr);
	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "arrayItem"));
	if (RyanJsonFalse == RyanJsonCompare(dupItem, RyanJsonGetObjectToKey(json, "arrayItem"))) { goto err; }
	RyanJsonDelete(dupItem);

	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "arrayItem"));
	RyanJsonAddItemToObject(json, "test", dupItem);
	if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "arrayItem"))) { goto err; }
	RyanJsonDelete(RyanJsonDetachByKey(json, "test"));

	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "arrayItem"));
	RyanJsonAddItemToObject(json, "test", dupItem);
	if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "arrayItem"))) { goto err; }
	RyanJsonDelete(json);

	json = RyanJsonParse(jsonstr);
	dupItem = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "arrayItem"));
	RyanJsonAddItemToObject(json, "test", dupItem);
	if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "test"), RyanJsonGetObjectToKey(json, "arrayItem"))) { goto err; }
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
		if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(jsonRoot, "arrayItem"), dupItem)) { goto err; }
		RyanJsonReplaceByKey(json, "arrayItem", RyanJsonDuplicate(RyanJsonGetObjectByKey(dupItem, "item")));
		if (RyanJsonFalse == RyanJsonCompare(RyanJsonGetObjectToKey(json, "arrayItem"), RyanJsonGetObjectByKey(dupItem, "item")))
		{
			goto err;
		}
		RyanJsonDelete(dupItem);

		int newuse = vallocGetUse();
		if (i != 0 && newuse != use)
		{
			printf("%s:%d 内存泄漏\r\n", __FILE__, __LINE__);
			goto err;
		}
		use = newuse;
	}

	RyanJsonDelete(json);
	RyanJsonDelete(jsonRoot);
	return RyanJsonTrue;

err:
	RyanJsonDelete(json);
	RyanJsonDelete(jsonRoot);
	return RyanJsonFalse;
}
