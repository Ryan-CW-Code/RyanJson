#include "RyanJsonBaseTest.h"

/* --------------------------------------------------------------------- */

RyanJsonBool_e RyanJsonBaseTestReplaceJson()
{
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

	/* ---------------- 保留原有测试（并补充校验） ---------------- */

	// 数组替换测试：arrayInt 头
	RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 0, RyanJsonCreateString(NULL, "arrayInt"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 0);
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "arrayInt") == 0, { goto err; });
	}

	// 数组替换测试：arrayInt 尾
	{
		RyanJson_t arr = RyanJsonGetObjectToKey(json, "arrayInt");
		uint32_t last = RyanJsonGetSize(arr) - 1;
		RyanJsonReplaceByIndex(arr, last, RyanJsonCreateString(NULL, "arrayInt"));
		RyanJson_t v = RyanJsonGetObjectToIndex(arr, last);
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "arrayInt") == 0, { goto err; });
	}

	// 数组对象替换测试：arrayItem[0]
	RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 0, RyanJsonCreateString(NULL, "arrayItem"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 0);
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "arrayItem") == 0, { goto err; });
	}

	// 数组对象替换测试：arrayItem[1]
	RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 1, RyanJsonCreateString(NULL, "arrayItem"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayItem"), 1);
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "arrayItem") == 0, { goto err; });
	}

	// 对象字段替换：inter -> 999
	RyanJsonReplaceByKey(json, "inter", RyanJsonCreateInt("inter", 999));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "inter");
		RyanJsonCheckCode(RyanJsonIsInt(v), { goto err; });
		RyanJsonCheckCode(RyanJsonGetIntValue(v) == 999, { goto err; });
	}

	// 对象字段替换：double -> 123.45
	RyanJsonReplaceByKey(json, "double", RyanJsonCreateDouble("double", 123.45));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "double");
		RyanJsonCheckCode(RyanJsonIsDouble(v), { goto err; });
		RyanJsonCheckCode(RyanJsonGetDoubleValue(v) == 123.45, { goto err; });
	}

	// 对象字段替换：string -> "newString"
	RyanJsonReplaceByKey(json, "string", RyanJsonCreateString("string", "newString"));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "string");
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "newString") == 0, { goto err; });
	}

	// 对象字段替换：boolFalse -> true
	RyanJsonReplaceByKey(json, "boolFalse", RyanJsonCreateBool("boolFalse", RyanJsonTrue));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "boolFalse");
		RyanJsonCheckCode(RyanJsonIsBool(v), { goto err; });
		RyanJsonCheckCode(RyanJsonGetBoolValue(v) == RyanJsonTrue, { goto err; });
	}

	// 数组替换：arrayInt 中间元素 -> "midInt"
	RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 2, RyanJsonCreateString(NULL, "midInt"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayInt"), 2);
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "midInt") == 0, { goto err; });
	}

	RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "arrayString"), 1, RyanJsonCreateString(NULL, "headString"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "arrayString"), 1);
		printJsonDebug(v);
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "headString") == 0, { goto err; });
	}
	{
		RyanJson_t arr = RyanJsonGetObjectToKey(json, "arrayString");
		uint32_t last = RyanJsonGetSize(arr) - 1;
		RyanJsonReplaceByIndex(arr, last, RyanJsonCreateString(NULL, "tailString"));
		RyanJson_t v = RyanJsonGetObjectToIndex(arr, last);
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "tailString") == 0, { goto err; });
	}

	// 数组对象替换：arrayItem 尾部 -> "arrayItemTail"
	{
		RyanJson_t arr = RyanJsonGetObjectToKey(json, "arrayItem");
		uint32_t last = RyanJsonGetSize(arr) - 1;
		RyanJsonReplaceByIndex(arr, last, RyanJsonCreateString(NULL, "arrayItemTail"));
		RyanJson_t v = RyanJsonGetObjectToIndex(arr, last);
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "arrayItemTail") == 0, { goto err; });
	}

	// 嵌套对象替换：item.inter -> 111
	RyanJsonReplaceByKey(RyanJsonGetObjectToKey(json, "item"), "inter", RyanJsonCreateInt("inter", 111));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(RyanJsonGetObjectToKey(json, "item"), "inter");
		RyanJsonCheckCode(RyanJsonIsInt(v), { goto err; });
		RyanJsonCheckCode(RyanJsonGetIntValue(v) == 111, { goto err; });
	}

	// 嵌套对象替换：item.string -> "nestedReplace"
	RyanJsonReplaceByKey(RyanJsonGetObjectToKey(json, "item"), "string", RyanJsonCreateString("string", "nestedReplace"));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(RyanJsonGetObjectToKey(json, "item"), "string");
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "nestedReplace") == 0, { goto err; });
	}

	// 混合数组替换：各类型位置
	// 0:int -> "intReplaced"
	RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "array"), 0, RyanJsonCreateString(NULL, "intReplaced"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "array"), 0);
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "intReplaced") == 0, { goto err; });
	}
	// 1:double -> "doubleReplaced"
	RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "array"), 1, RyanJsonCreateString(NULL, "doubleReplaced"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "array"), 1);
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "doubleReplaced") == 0, { goto err; });
	}
	// 2:string -> "stringReplaced"
	RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "array"), 2, RyanJsonCreateString(NULL, "stringReplaced"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "array"), 2);
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "stringReplaced") == 0, { goto err; });
	}
	// 3:bool -> "boolReplaced"
	RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "array"), 3, RyanJsonCreateString(NULL, "boolReplaced"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "array"), 3);
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "boolReplaced") == 0, { goto err; });
	}
	// 5:null -> "nullReplaced"
	RyanJsonReplaceByIndex(RyanJsonGetObjectToKey(json, "array"), 5, RyanJsonCreateString(NULL, "nullReplaced"));
	{
		RyanJson_t v = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(json, "array"), 5);
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "nullReplaced") == 0, { goto err; });
	}

	// 对象替换测试：arrayString -> "arrayString2222"
	RyanJsonReplaceByKey(json, "arrayString", RyanJsonCreateString("", "arrayString2222"));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "arrayString");
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "arrayString2222") == 0, { goto err; });
	}

	// 修改数组节点为对象节点：arrayDouble -> duplicate(item)
	RyanJson_t duplicateJson = RyanJsonDuplicate(RyanJsonGetObjectToKey(json, "item"));
	RyanJsonReplaceByKey(json, "arrayDouble", duplicateJson);
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "arrayDouble");
		RyanJsonCheckCode(RyanJsonIsObject(v), { goto err; });
	}

	// 替换普通 key 的值：string2222 -> "world"
	RyanJsonReplaceByKey(json, "string2222", RyanJsonCreateString("string2222", "world"));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "string2222");
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "world") == 0, { goto err; });
	}

	// 替换布尔值：boolTrue -> false
	RyanJsonReplaceByKey(json, "boolTrue", RyanJsonCreateBool("boolTrue", RyanJsonFalse));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "boolTrue");
		RyanJsonCheckCode(RyanJsonIsBool(v), { goto err; });
		RyanJsonCheckCode(RyanJsonGetBoolValue(v) == RyanJsonFalse, { goto err; });
	}

	// 替换 null：null -> "notNull"
	RyanJsonReplaceByKey(json, "null", RyanJsonCreateString("null", "notNull"));
	{
		RyanJson_t v = RyanJsonGetObjectToKey(json, "null");
		RyanJsonCheckCode(RyanJsonIsString(v), { goto err; });
		RyanJsonCheckCode(strcmp(RyanJsonGetStringValue(v), "notNull") == 0, { goto err; });
	}

	char *str = RyanJsonPrint(json, 1024, RyanJsonTrue, NULL);
	RyanJsonFree(str);
	RyanJsonDelete(json);
	return RyanJsonTrue;

err:
	RyanJsonDelete(json);
	return RyanJsonFalse;
}
