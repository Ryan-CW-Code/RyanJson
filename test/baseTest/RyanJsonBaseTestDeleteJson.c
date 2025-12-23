#include "RyanJsonBaseTest.h"

/* --------------------------------------------------------------------- */

RyanJsonBool_e RyanJsonBaseTestDeleteJson()
{
	// 保持原始 jsonStr，不做修改
	char jsonstr[] =
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,\"item\":"
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],"
		"\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],"
		"\"array\":[16,16.89,\"hello\",true,false,null],"
		"\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}],"
		"\"string2222\":\"hello\"}";

	RyanJson_t json = RyanJsonParse(jsonstr);
	RyanJsonCheckReturnFalse(NULL != json);

	/**
	 * @brief 场景 1：删除对象中的节点（头、中、尾）
	 */
	{
		// 删除中间节点 (double)
		RyanJsonDeleteByKey(json, "double");
		RyanJsonCheckCode(NULL == RyanJsonGetObjectToKey(json, "double"), { goto err; });

		// 删除头部节点 (inter)
		RyanJsonDeleteByIndex(json, 0);
		RyanJsonCheckCode(NULL == RyanJsonGetObjectToKey(json, "inter"), { goto err; });

		// 删除尾部节点 (string2222)
		uint32_t lastIndex = RyanJsonGetSize(json) - 1;
		RyanJsonDeleteByIndex(json, lastIndex);
		RyanJsonCheckCode(NULL == RyanJsonGetObjectToKey(json, "string2222"), { goto err; });
	}

	/**
	 * @brief 场景 2：删除数组中的元素（arrayInt）
	 */
	{
		RyanJson_t array = RyanJsonGetObjectToKey(json, "arrayInt");

		// 删除数组首位
		RyanJsonDeleteByIndex(array, 0);
		RyanJsonCheckCode(RyanJsonGetSize(array) == 4, { goto err; });

		// 删除数组中间元素
		RyanJsonDeleteByIndex(array, 1);
		RyanJsonCheckCode(RyanJsonGetSize(array) == 3, { goto err; });

		// 删除数组尾部元素
		uint32_t lastIndex = RyanJsonGetSize(array) - 1;
		RyanJsonDeleteByIndex(array, lastIndex);
		RyanJsonCheckCode(RyanJsonGetSize(array) == 2, { goto err; });
	}

	/**
	 * @brief 场景 3：深层嵌套删除（item）
	 */
	{
		RyanJsonDeleteByKey(json, "item");
		RyanJsonCheckCode(NULL == RyanJsonGetObjectToKey(json, "item"), { goto err; });
	}

	/**
	 * @brief 场景 4：数组对象元素删除（arrayItem）
	 */
	{
		RyanJson_t arrObj = RyanJsonGetObjectToKey(json, "arrayItem");

		// 删除第一个对象
		RyanJsonDeleteByIndex(arrObj, 0);
		RyanJsonCheckCode(RyanJsonGetSize(arrObj) == 1, { goto err; });

		// 删除最后一个对象
		RyanJsonDeleteByIndex(arrObj, 0);
		RyanJsonCheckCode(RyanJsonGetSize(arrObj) == 0, { goto err; });
	}

	/**
	 * @brief 场景 5：特殊类型删除（null / bool）
	 */
	{
		RyanJsonDeleteByKey(json, "null");
		RyanJsonCheckCode(NULL == RyanJsonGetObjectToKey(json, "null"), { goto err; });

		RyanJsonDeleteByKey(json, "boolTrue");
		RyanJsonCheckCode(NULL == RyanJsonGetObjectToKey(json, "boolTrue"), { goto err; });

		RyanJsonDeleteByKey(json, "boolFalse");
		RyanJsonCheckCode(NULL == RyanJsonGetObjectToKey(json, "boolFalse"), { goto err; });
	}

	/**
	 * @brief 场景 6：异常路径覆盖（健壮性）
	 */
	{
		RyanJsonDeleteByKey(json, "non_exist"); // 删除不存在的 Key
		RyanJsonDeleteByIndex(NULL, 0);         // 在 NULL 上操作
	}

	char *str = RyanJsonPrint(json, 1024, RyanJsonTrue, NULL);
	RyanJsonFree(str);
	RyanJsonDelete(json);
	return RyanJsonTrue;

err:
	RyanJsonDelete(json);
	return RyanJsonFalse;
}
