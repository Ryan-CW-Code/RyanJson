#include "testBase.h"

static void testStandardTopLevelScalars(void)
{
	// 标准 Json 允许顶层是任意 Json 值，而不仅是 Object/Array。
	RyanJson_t json = NULL;
	RyanJson_t roundtrip = NULL;
	char *printed = NULL;

	json = RyanJsonParse(" \n\ttrue\r ");
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "顶层 true 解析失败");
	TEST_ASSERT_TRUE(RyanJsonIsBool(json));
	TEST_ASSERT_TRUE(RyanJsonGetBoolValue(json));
	printed = RyanJsonPrint(json, 32, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	TEST_ASSERT_EQUAL_STRING("true", printed);
	roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(json, roundtrip));
	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(json);

	json = RyanJsonParse("false");
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "顶层 false 解析失败");
	TEST_ASSERT_TRUE(RyanJsonIsBool(json));
	TEST_ASSERT_FALSE(RyanJsonGetBoolValue(json));
	printed = RyanJsonPrint(json, 32, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	TEST_ASSERT_EQUAL_STRING("false", printed);
	roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(json, roundtrip));
	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(json);

	json = RyanJsonParse("null");
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "顶层 null 解析失败");
	TEST_ASSERT_TRUE(RyanJsonIsNull(json));
	printed = RyanJsonPrint(json, 32, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	TEST_ASSERT_EQUAL_STRING("null", printed);
	roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(json, roundtrip));
	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(json);

	json = RyanJsonParse("\"root string\"");
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "顶层 String 解析失败");
	TEST_ASSERT_TRUE(RyanJsonIsString(json));
	TEST_ASSERT_EQUAL_STRING("root string", RyanJsonGetStringValue(json));
	printed = RyanJsonPrint(json, 64, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	TEST_ASSERT_EQUAL_STRING("\"root string\"", printed);
	roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(json, roundtrip));
	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(json);

	json = RyanJsonParse("-12.5");
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "顶层 double 解析失败");
	TEST_ASSERT_TRUE(RyanJsonIsDouble(json));
	TEST_ASSERT_TRUE(RyanJsonCompareDouble(-12.5, RyanJsonGetDoubleValue(json)));
	printed = RyanJsonPrint(json, 32, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(json, roundtrip));
	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(json);
}

static void testStandardStringEscapesRoundtrip(void)
{
	// 覆盖 RFC 8259 String 基础转义字符的解码与往返。
	const char *jsonText = "{\"s\":\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"}";
	const char *expect = "\"\\/\b\f\n\r\t";

	RyanJson_t json = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "String 转义解析失败");

	RyanJson_t s = RyanJsonGetObjectToKey(json, "s");
	TEST_ASSERT_NOT_NULL(s);
	TEST_ASSERT_TRUE(RyanJsonIsString(s));
	TEST_ASSERT_EQUAL_STRING_MESSAGE(expect, RyanJsonGetStringValue(s), "String 转义解码结果不符合预期");

	char *printed = RyanJsonPrint(json, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(printed, "String 转义打印失败");

	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL_MESSAGE(roundtrip, "String 转义往返解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(json, roundtrip), "String 转义往返后 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(json);
}

static void testStandardUnicodeRoundtrip(void)
{
	// 覆盖 BMP 转义、UTF-16 代理对与 UTF-8 直写的值语义一致性。
	const char *escapedText = "{\"latin\":\"\\u0041\\u00DF\\u6771\",\"pair\":\"\\uD834\\uDD1E\",\"emoji\":\"\\uD83D\\uDE03\"}";
	const char *utf8Text = "{\"latin\":\"A\xC3\x9F\xE6\x9D\xB1\",\"pair\":\"\xF0\x9D\x84\x9E\",\"emoji\":\"\xF0\x9F\x98\x83\"}";

	RyanJson_t escaped = RyanJsonParse(escapedText);
	RyanJson_t utf8 = RyanJsonParse(utf8Text);

	TEST_ASSERT_NOT_NULL_MESSAGE(escaped, "Unicode 转义文本解析失败");
	TEST_ASSERT_NOT_NULL_MESSAGE(utf8, "UTF-8 文本解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(escaped, utf8), "Unicode 转义与 UTF-8 文本语义应一致");

	TEST_ASSERT_EQUAL_STRING("A\xC3\x9F\xE6\x9D\xB1", RyanJsonGetStringValue(RyanJsonGetObjectToKey(escaped, "latin")));
	TEST_ASSERT_EQUAL_STRING("\xF0\x9D\x84\x9E", RyanJsonGetStringValue(RyanJsonGetObjectToKey(escaped, "pair")));
	TEST_ASSERT_EQUAL_STRING("\xF0\x9F\x98\x83", RyanJsonGetStringValue(RyanJsonGetObjectToKey(escaped, "emoji")));

	char *printed = RyanJsonPrint(escaped, 160, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(printed, "Unicode 打印失败");

	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL_MESSAGE(roundtrip, "Unicode 往返解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(escaped, roundtrip), "Unicode 往返后 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(utf8);
	RyanJsonDelete(escaped);
}

static void testStandardLiteralArrayTypeMatrix(void)
{
	// 覆盖 Array 中标准字面量与容器的类型边界，避免落回复杂 key 语义。
	const char *jsonText = "[null,true,false,\"true\",\"false\",\"null\",{\"k\":\"v\"},[\"x\"],0]";
	RyanJson_t json = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "字面量类型矩阵解析失败");
	TEST_ASSERT_TRUE(RyanJsonIsArray(json));
	TEST_ASSERT_EQUAL_UINT32(9U, RyanJsonGetArraySize(json));

	RyanJson_t n0 = RyanJsonGetObjectByIndex(json, 0);
	RyanJson_t n1 = RyanJsonGetObjectByIndex(json, 1);
	RyanJson_t n2 = RyanJsonGetObjectByIndex(json, 2);
	RyanJson_t n3 = RyanJsonGetObjectByIndex(json, 3);
	RyanJson_t n4 = RyanJsonGetObjectByIndex(json, 4);
	RyanJson_t n5 = RyanJsonGetObjectByIndex(json, 5);
	RyanJson_t n6 = RyanJsonGetObjectByIndex(json, 6);
	RyanJson_t n7 = RyanJsonGetObjectByIndex(json, 7);
	RyanJson_t n8 = RyanJsonGetObjectByIndex(json, 8);

	TEST_ASSERT_TRUE(RyanJsonIsNull(n0));
	TEST_ASSERT_TRUE(RyanJsonIsBool(n1));
	TEST_ASSERT_TRUE(RyanJsonIsBool(n2));
	TEST_ASSERT_TRUE(RyanJsonIsString(n3));
	TEST_ASSERT_TRUE(RyanJsonIsString(n4));
	TEST_ASSERT_TRUE(RyanJsonIsString(n5));
	TEST_ASSERT_TRUE(RyanJsonIsObject(n6));
	TEST_ASSERT_TRUE(RyanJsonIsArray(n7));
	TEST_ASSERT_TRUE(RyanJsonIsInt(n8));

	TEST_ASSERT_TRUE(RyanJsonGetBoolValue(n1));
	TEST_ASSERT_FALSE(RyanJsonGetBoolValue(n2));
	TEST_ASSERT_EQUAL_STRING("true", RyanJsonGetStringValue(n3));
	TEST_ASSERT_EQUAL_STRING("false", RyanJsonGetStringValue(n4));
	TEST_ASSERT_EQUAL_STRING("null", RyanJsonGetStringValue(n5));
	TEST_ASSERT_EQUAL_STRING("v", RyanJsonGetStringValue(RyanJsonGetObjectToKey(n6, "k")));
	TEST_ASSERT_EQUAL_STRING("x", RyanJsonGetStringValue(RyanJsonGetObjectByIndex(n7, 0)));
	TEST_ASSERT_EQUAL_INT(0, RyanJsonGetIntValue(n8));

	char *pretty = RyanJsonPrint(json, 256, RyanJsonTrue, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(pretty, "字面量类型矩阵格式化打印失败");

	RyanJson_t roundtrip = RyanJsonParse(pretty);
	TEST_ASSERT_NOT_NULL_MESSAGE(roundtrip, "字面量类型矩阵格式化往返解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(json, roundtrip), "字面量类型矩阵往返 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(pretty);
	RyanJsonDelete(json);
}

static void testStandardNestedWhitespaceRoundtrip(void)
{
	// 经典嵌套结构 + 大量标准空白字符的往返语义。
	const char *jsonText =
		"  { \"Image\" : { \"Width\" : 800 , \"Height\" : 600 , \"Title\" : \"View\" , "
		"\"Thumbnail\" : { \"Url\" : \"http://www.example.com/image/481989943\" , \"Height\" : 125 , \"Width\" : \"100\" } , "
		"\"Animated\" : false , \"IDs\" : [ 116 , 943 , 234 , 38793 ] } }  ";

	RyanJson_t json = RyanJsonParse(jsonText);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "嵌套标准 Json 解析失败");

	TEST_ASSERT_EQUAL_INT(800, RyanJsonGetIntValue(RyanJsonGetObjectToKey(json, "Image", "Width")));
	TEST_ASSERT_EQUAL_INT(600, RyanJsonGetIntValue(RyanJsonGetObjectToKey(json, "Image", "Height")));
	TEST_ASSERT_EQUAL_STRING("View", RyanJsonGetStringValue(RyanJsonGetObjectToKey(json, "Image", "Title")));
	TEST_ASSERT_FALSE(RyanJsonGetBoolValue(RyanJsonGetObjectToKey(json, "Image", "Animated")));
	TEST_ASSERT_EQUAL_STRING("100", RyanJsonGetStringValue(RyanJsonGetObjectToKey(json, "Image", "Thumbnail", "Width")));
	TEST_ASSERT_EQUAL_UINT32(4U, RyanJsonGetArraySize(RyanJsonGetObjectToKey(json, "Image", "IDs")));

	char *pretty = RyanJsonPrint(json, 512, RyanJsonTrue, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(pretty, "格式化打印失败");

	RyanJson_t roundtrip = RyanJsonParse(pretty);
	TEST_ASSERT_NOT_NULL_MESSAGE(roundtrip, "格式化打印后的 Json 重新解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(json, roundtrip), "嵌套标准 Json 往返后 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(pretty);
	RyanJsonDelete(json);
}

static void testStandardTopLevelEmptyContainersRoundtrip(void)
{
	RyanJson_t obj = RyanJsonParse("{}");
	RyanJson_t arr = RyanJsonParse("[]");
	TEST_ASSERT_NOT_NULL_MESSAGE(obj, "顶层空 Object 解析失败");
	TEST_ASSERT_NOT_NULL_MESSAGE(arr, "顶层空 Array 解析失败");
	TEST_ASSERT_TRUE(RyanJsonIsObject(obj));
	TEST_ASSERT_TRUE(RyanJsonIsArray(arr));
	TEST_ASSERT_EQUAL_UINT32(0U, RyanJsonGetSize(obj));
	TEST_ASSERT_EQUAL_UINT32(0U, RyanJsonGetArraySize(arr));

	char *objPrinted = RyanJsonPrint(obj, 16, RyanJsonFalse, NULL);
	char *arrPrinted = RyanJsonPrint(arr, 16, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(objPrinted);
	TEST_ASSERT_NOT_NULL(arrPrinted);
	TEST_ASSERT_EQUAL_STRING("{}", objPrinted);
	TEST_ASSERT_EQUAL_STRING("[]", arrPrinted);

	RyanJson_t objRoundtrip = RyanJsonParse(objPrinted);
	RyanJson_t arrRoundtrip = RyanJsonParse(arrPrinted);
	TEST_ASSERT_NOT_NULL(objRoundtrip);
	TEST_ASSERT_NOT_NULL(arrRoundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(obj, objRoundtrip));
	TEST_ASSERT_TRUE(RyanJsonCompare(arr, arrRoundtrip));

	RyanJsonDelete(objRoundtrip);
	RyanJsonDelete(arrRoundtrip);
	RyanJsonFree(objPrinted);
	RyanJsonFree(arrPrinted);
	RyanJsonDelete(obj);
	RyanJsonDelete(arr);
}

static void testStandardUnicodeLineSeparatorInString(void)
{
	// 覆盖 U+2028 在 String 中的值语义与往返。
	const char *utf8Text = "{\"s\":\"\xE2\x80\xA8\"}";
	const char *escapedText = "{\"s\":\"\\u2028\"}";

	RyanJson_t utf8 = RyanJsonParse(utf8Text);
	RyanJson_t escaped = RyanJsonParse(escapedText);
	TEST_ASSERT_NOT_NULL_MESSAGE(utf8, "UTF-8 行分隔符解析失败");
	TEST_ASSERT_NOT_NULL_MESSAGE(escaped, "转义行分隔符解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(utf8, escaped), "UTF-8 与转义形式应语义一致");

	char *printed = RyanJsonPrint(utf8, 32, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(printed, "行分隔符打印失败");
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL_MESSAGE(roundtrip, "行分隔符往返解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(utf8, roundtrip), "行分隔符往返 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(escaped);
	RyanJsonDelete(utf8);
}

static void testStandardRejectNonFiniteNumbers(void)
{
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("NaN"), "标准 Json 不允许 NaN");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("Infinity"), "标准 Json 不允许 Infinity");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("-Infinity"), "标准 Json 不允许 -Infinity");
	TEST_ASSERT_NULL_MESSAGE(RyanJsonParse("{\"n\":NaN}"), "Object 中的 NaN 应解析失败");
}

void testStandardValueRoundtripRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testStandardTopLevelScalars);
	RUN_TEST(testStandardStringEscapesRoundtrip);
	RUN_TEST(testStandardUnicodeRoundtrip);
	RUN_TEST(testStandardLiteralArrayTypeMatrix);
	RUN_TEST(testStandardNestedWhitespaceRoundtrip);
	RUN_TEST(testStandardTopLevelEmptyContainersRoundtrip);
	RUN_TEST(testStandardUnicodeLineSeparatorInString);
	RUN_TEST(testStandardRejectNonFiniteNumbers);
}
