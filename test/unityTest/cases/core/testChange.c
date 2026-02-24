#include "testBase.h"

static void testChangeEdgeCases(void)
{
	// 测试 NULL 输入处理
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeBoolValue(NULL, true), "ChangeBoolValue(NULL) 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeIntValue(NULL, 1), "ChangeIntValue(NULL) 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeDoubleValue(NULL, 1.0), "ChangeDoubleValue(NULL) 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeStringValue(NULL, "test"), "ChangeStringValue(NULL) 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeKey(NULL, "key"), "ChangeKey(NULL) 应返回 False");

	// 测试类型不匹配处理
	RyanJson_t intNode = RyanJsonCreateInt(NULL, 1);
	RyanJson_t doubleNode = RyanJsonCreateDouble(NULL, 1.0);
	RyanJson_t boolNode = RyanJsonCreateBool(NULL, RyanJsonTrue);
	RyanJson_t strNode = RyanJsonCreateString(NULL, "s");
	RyanJson_t keyedStrNode = RyanJsonCreateString("k", "v");
	RyanJson_t keyedBoolNode = RyanJsonCreateBool("flag", RyanJsonTrue);
	TEST_ASSERT_NOT_NULL(intNode);
	TEST_ASSERT_NOT_NULL(doubleNode);
	TEST_ASSERT_NOT_NULL(boolNode);
	TEST_ASSERT_NOT_NULL(strNode);
	TEST_ASSERT_NOT_NULL(keyedStrNode);
	TEST_ASSERT_NOT_NULL(keyedBoolNode);

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeIntValue(doubleNode, 3), "ChangeIntValue(非Int节点) 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeDoubleValue(intNode, 3.14), "ChangeDoubleValue(非Double节点) 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeBoolValue(intNode, RyanJsonTrue), "ChangeBoolValue(非Bool节点) 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeStringValue(intNode, "x"), "ChangeStringValue(非String节点) 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeKey(strNode, "k2"), "ChangeKey(无Key的String节点) 应返回 False");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeKey(keyedStrNode, NULL), "ChangeKey(key!=NULL 前置条件) 应返回 False");

	// 失败后原值应保持不变
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonGetBoolValue(boolNode), "bool 初始值错误");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeIntValue(boolNode, 9), "ChangeIntValue(Bool节点) 应返回 False");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonGetBoolValue(boolNode), "失败后 bool 值不应变化");

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, RyanJsonGetIntValue(intNode), "int 初始值错误");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeDoubleValue(intNode, 8.8), "ChangeDoubleValue(Int节点) 应返回 False");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, RyanJsonGetIntValue(intNode), "失败后 int 值不应变化");

	TEST_ASSERT_EQUAL_STRING_MESSAGE("v", RyanJsonGetStringValue(keyedStrNode), "string 初始值错误");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonChangeBoolValue(keyedStrNode, RyanJsonFalse), "ChangeBoolValue(String节点) 应返回 False");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("v", RyanJsonGetStringValue(keyedStrNode), "失败后 string 值不应变化");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("k", RyanJsonGetKey(keyedStrNode), "失败后 key 不应变化");

	// 有 key 的非字符串节点允许改 key，且值不应改变
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeKey(keyedBoolNode, "flag2"), "ChangeKey(Bool+Key 节点) 应成功");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("flag2", RyanJsonGetKey(keyedBoolNode), "Bool 节点改 key 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonGetBoolValue(keyedBoolNode), "Bool 节点改 key 后 value 不应变化");

	RyanJsonDelete(intNode);
	RyanJsonDelete(doubleNode);
	RyanJsonDelete(boolNode);
	RyanJsonDelete(strNode);
	RyanJsonDelete(keyedStrNode);
	RyanJsonDelete(keyedBoolNode);
}

static void testChangeValueStress(void)
{
	RyanJson_t root = RyanJsonCreateObject();
	RyanJsonAddStringToObject(root, "k", "v");

	// 高频修改 strValue
	// 变为超长
	char *longStr = (char *)malloc(1000);
	memset(longStr, 'A', 999);
	longStr[999] = '\0';
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(RyanJsonGetObjectByKey(root, "k"), longStr));
	TEST_ASSERT_EQUAL_STRING(longStr, RyanJsonGetStringValue(RyanJsonGetObjectByKey(root, "k")));

	// 变为特殊字符
	const char *specials = "\t\n\r\b\f\"\\/";
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(RyanJsonGetObjectByKey(root, "k"), specials));
	TEST_ASSERT_EQUAL_STRING(specials, RyanJsonGetStringValue(RyanJsonGetObjectByKey(root, "k")));

	// 变为空串
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(RyanJsonGetObjectByKey(root, "k"), ""));
	TEST_ASSERT_EQUAL_STRING("", RyanJsonGetStringValue(RyanJsonGetObjectByKey(root, "k")));

	// 高频修改 key
	// 变为超长
	TEST_ASSERT_TRUE(RyanJsonChangeKey(RyanJsonGetObjectByKey(root, "k"), longStr));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, longStr));

	// 变为空 key
	TEST_ASSERT_TRUE(RyanJsonChangeKey(RyanJsonGetObjectByKey(root, longStr), ""));
	TEST_ASSERT_TRUE(RyanJsonHasObjectByKey(root, ""));

	free(longStr);
	RyanJsonDelete(root);
}

static void testChangeScalarAndStorageMode(void)
{
	char jsonstr[] =
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null,"
		"\"item\":{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"\"arrayInt\":[16,16,16,16,16],\"arrayDouble\":[16.89,16.89,16.89,16.89,16.89],"
		"\"arrayString\":[\"hello\",\"hello\",\"hello\",\"hello\",\"hello\"],"
		"\"array\":[16,16.89,\"hello\",true,false,null],"
		"\"arrayItem\":[{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null},"
		"{\"inter\":16,\"double\":16.89,\"string\":\"hello\",\"boolTrue\":true,\"boolFalse\":false,\"null\":null}],"
		"\"string2222\":\"hello\",\"0\":\"1\",\"nameaaaaaaaaaaaaaaaaaaaaaaaaaaaa\":\"Mash\",\"2\":\"3\",\"name\":"
		"\"Mashaaaaaaaaaaaaaaaaaaaaaaaa\"}";

	RyanJson_t jsonRoot = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL_MESSAGE(jsonRoot, "解析基础 Json 失败");

	/**
	 * @brief 修改基本类型
	 */
	// 修改整数
	RyanJsonChangeIntValue(RyanJsonGetObjectToKey(jsonRoot, "inter"), 20);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(RyanJsonGetObjectToKey(jsonRoot, "inter")), "字段 'inter' 修改后不是整数");
	TEST_ASSERT_EQUAL_INT_MESSAGE(20, RyanJsonGetIntValue(RyanJsonGetObjectToKey(jsonRoot, "inter")), "字段 'inter' 值不匹配");

	// 修改浮点数
	RyanJsonChangeDoubleValue(RyanJsonGetObjectToKey(jsonRoot, "double"), 20.89);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDouble(RyanJsonGetObjectToKey(jsonRoot, "double")), "字段 'double' 修改后不是浮点数");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareDouble(RyanJsonGetDoubleValue(RyanJsonGetObjectToKey(jsonRoot, "double")), 20.89),
				 "字段 'double' 值不匹配");

	// 修改 Key (inline 模式，不超过长度)
	RyanJsonChangeKey(RyanJsonGetObjectByKey(jsonRoot, "0"), "type");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("type", RyanJsonGetKey(RyanJsonGetObjectToKey(jsonRoot, "type")), "字段 'type' 的 Key 不匹配");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("1", RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "type")), "字段 'type' 的内容不匹配");

	// 修改 Key (从 inline 转 ptr 模式)
	RyanJsonChangeKey(RyanJsonGetObjectByKey(jsonRoot, "type"), "type000000000000000");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("type000000000000000", RyanJsonGetKey(RyanJsonGetObjectToKey(jsonRoot, "type000000000000000")),
					 "长 Key 模式下的 Key 不匹配");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("1", RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "type000000000000000")),
					 "长 Key 模式下的内容不匹配");

	// 修改 Key (从 ptr 转 inline 模式)
	RyanJsonChangeKey(RyanJsonGetObjectByKey(jsonRoot, "nameaaaaaaaaaaaaaaaaaaaaaaaaaaaa"), "na");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("na", RyanJsonGetKey(RyanJsonGetObjectToKey(jsonRoot, "na")), "回退到 inline 模式的 Key 不匹配");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("Mash", RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "na")),
					 "回退到 inline 模式的内容不匹配");

	// 修改 Value (inline 模式，不超过长度)
	RyanJsonChangeStringValue(RyanJsonGetObjectByKey(jsonRoot, "2"), "type");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("2", RyanJsonGetKey(RyanJsonGetObjectToKey(jsonRoot, "2")), "字段 '2' 的 Key 不匹配");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("type", RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "2")), "字段 '2' 的新内容不匹配");

	// 修改 Value (从 ptr 转 inline 模式)
	RyanJsonChangeStringValue(RyanJsonGetObjectByKey(jsonRoot, "name"), "Ma");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("name", RyanJsonGetKey(RyanJsonGetObjectToKey(jsonRoot, "name")), "字段 'name' 的 Key 不匹配");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("Ma", RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "name")),
					 "回退到 inline 模式的 Value 不匹配");

	// 修改 Value (从 ptr 转 ptr 模式)
	RyanJsonChangeStringValue(RyanJsonGetObjectByKey(jsonRoot, "name"), "Mashaaaaaaaaaaaaaaaaaaaaaaaa");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("name", RyanJsonGetKey(RyanJsonGetObjectToKey(jsonRoot, "name")), "字段 'name' 的 Key 不匹配");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("Mashaaaaaaaaaaaaaaaaaaaaaaaa", RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "name")),
					 "长 Value 模式下的内容不匹配");

	// 修改字符串
	RyanJsonChangeStringValue(RyanJsonGetObjectToKey(jsonRoot, "string"), "world");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsString(RyanJsonGetObjectToKey(jsonRoot, "string")), "字段 'string' 修改后不是字符串");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("world", RyanJsonGetStringValue(RyanJsonGetObjectToKey(jsonRoot, "string")),
					 "字段 'string' 内容不匹配");

	// 修改 boolValue (true -> false)
	RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(jsonRoot, "boolTrue"), RyanJsonFalse);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(RyanJsonGetObjectToKey(jsonRoot, "boolTrue")), "字段 'boolTrue' 类型错误");
	TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonFalse, RyanJsonGetBoolValue(RyanJsonGetObjectToKey(jsonRoot, "boolTrue")),
				      "字段 'boolTrue' 值错误");

	// 修改 boolValue (false -> true)
	RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(jsonRoot, "boolFalse"), RyanJsonTrue);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsBool(RyanJsonGetObjectToKey(jsonRoot, "boolFalse")), "字段 'boolFalse' 类型错误");
	TEST_ASSERT_EQUAL_INT_MESSAGE(RyanJsonTrue, RyanJsonGetBoolValue(RyanJsonGetObjectToKey(jsonRoot, "boolFalse")),
				      "字段 'boolFalse' 值错误");

	/**
	 * @brief 修改数组元素 (arrayInt)
	 */
	RyanJsonChangeIntValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayInt"), 0), 99);
	TEST_ASSERT_EQUAL_INT_MESSAGE(99, RyanJsonGetIntValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayInt"), 0)),
				      "数组元素修改失败");

	/**
	 * @brief 修改数组元素 (arrayDouble)
	 */
	RyanJsonChangeDoubleValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayDouble"), 1), 99.99);
	TEST_ASSERT_TRUE_MESSAGE(
		RyanJsonCompareDouble(RyanJsonGetDoubleValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayDouble"), 1)),
				      99.99),
		"数组浮点元素修改失败");

	/**
	 * @brief 修改数组元素 (arrayString)
	 */
	RyanJsonChangeStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayString"), 2), "changedString");
	TEST_ASSERT_EQUAL_STRING_MESSAGE(
		"changedString", RyanJsonGetStringValue(RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayString"), 2)),
		"数组字符串元素修改失败");

	/**
	 * @brief 修改嵌套对象
	 */
	RyanJson_t nestedObj = RyanJsonGetObjectToKey(jsonRoot, "item");
	RyanJsonChangeStringValue(RyanJsonGetObjectToKey(nestedObj, "string"), "nestedWorld");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("nestedWorld", RyanJsonGetStringValue(RyanJsonGetObjectToKey(nestedObj, "string")),
					 "嵌套对象修改失败");

	/**
	 * @brief 修改数组对象中的字段 (arrayItem[0].inter -> 123)
	 */
	RyanJson_t arrayItem0 = RyanJsonGetObjectToIndex(RyanJsonGetObjectToKey(jsonRoot, "arrayItem"), 0);
	RyanJsonChangeIntValue(RyanJsonGetObjectToKey(arrayItem0, "inter"), 123);
	TEST_ASSERT_EQUAL_INT_MESSAGE(123, RyanJsonGetIntValue(RyanJsonGetObjectToKey(arrayItem0, "inter")), "数组中对象的字段修改失败");

	char *str = RyanJsonPrint(jsonRoot, 1024, RyanJsonTrue, NULL);
	RyanJsonFree(str);
	RyanJsonDelete(jsonRoot);

	/**
	 * @brief 验证创建后的内存模式库切换 (Create -> Change)
	 */
	RyanJson_t json = RyanJsonCreateString("key", "val");
	TEST_ASSERT_NOT_NULL(json);
	TEST_ASSERT_EQUAL_STRING_MESSAGE("val", RyanJsonGetStringValue(json), "Inline 模式初次设置失败");

	// 修改为长字符串 (触发自动切换到 ptr 模式)
	const char *longVal = "This is a very long string that definitely exceeds the inline threshold of RyanJson structure.";
	RyanJsonChangeStringValue(json, longVal);
	TEST_ASSERT_EQUAL_STRING_MESSAGE(longVal, RyanJsonGetStringValue(json), "切换到 Ptr 模式后值错误");

	// 修改回短字符串
	RyanJsonChangeStringValue(json, "new");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("new", RyanJsonGetStringValue(json), "切回短字符串后值错误");

	// 修改 Key 为长 Key
	const char *longKey = "a_very_long_key_name_to_trigger_ptr_mode_for_key_storage_in_ryanjson";
	RyanJsonChangeKey(json, longKey);
	TEST_ASSERT_EQUAL_STRING_MESSAGE(longKey, RyanJsonGetKey(json), "Key 切换到 Ptr 模式后错误");

	RyanJsonDelete(json);
}

static void testChangeInlineCalcBoundary(void)
{
	/**
	 * 这个测试的目标：
	 * 验证 RyanJsonInternalChangeString 的容量判断在“临界长度”处是否正确；
	 * 验证 value 和 key 两条路径都会发生 inline <-> ptr 的双向切换；
	 * 不依赖固定常量（例如 keyLen=255），而是基于当前编译配置动态计算边界。
	 *
	 * 背景：
	 * RyanJsonInternalChangeString 的判定核心是：
	 *   if ((mallocSize + keyLenFieldBytes) <= RyanJsonInlineStringSize) => inline
	 *   else => ptr
	 *
	 * 因此这里分别构造：
	 * - 恰好满足 <= 的输入（应保持/切回 inline）
	 * - 超过 1 字节的输入（应切到 ptr）
	 */
	uint32_t inlineSize = RyanJsonInlineStringSize;
	uint32_t fixedKeyLen = 1;
	uint32_t fixedValueLen = 1;

	/**
	 * value 边界计算（固定 key="k"）：
	 * 需要占用的总 inline 空间 = keyFieldLen + keyLen + '\0' + valueLen + '\0'
	 * 先算出不含 value 内容本体时的固定开销 baseNeedForValue，
	 * 再反推出 value 能放进 inline 的最大长度 maxInlineValueLen。
	 */
	uint32_t keyFieldLenForValue = RyanJsonInternalDecodeKeyLenField(RyanJsonInternalCalcLenBytes(fixedKeyLen));
	uint32_t baseNeedForValue = keyFieldLenForValue + fixedKeyLen + 1 + 1; // keyField + key + '\0' + value '\0'
	TEST_ASSERT_TRUE_MESSAGE(inlineSize >= baseNeedForValue, "inline 大小异常，无法完成边界测试");

	uint32_t maxInlineValueLen = inlineSize - baseNeedForValue;
	// valueInline: 恰好命中 inline 临界；valuePtr: 比临界多 1 字节，必须走 ptr
	char *valueInline = (char *)malloc((size_t)maxInlineValueLen + 1U);
	char *valuePtr = (char *)malloc((size_t)maxInlineValueLen + 2U);
	TEST_ASSERT_NOT_NULL(valueInline);
	TEST_ASSERT_NOT_NULL(valuePtr);
	memset(valueInline, 'v', maxInlineValueLen);
	valueInline[maxInlineValueLen] = '\0';
	memset(valuePtr, 'p', maxInlineValueLen + 1U);
	valuePtr[maxInlineValueLen + 1U] = '\0';

	RyanJson_t valueNode = RyanJsonCreateString("k", "");
	TEST_ASSERT_NOT_NULL(valueNode);
	// 临界长度：应为 inline
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(valueNode, valueInline));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonGetPayloadStrIsPtrByFlag(valueNode), "value 临界长度应为 inline");
	// 超临界：应切到 ptr
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(valueNode, valuePtr));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonGetPayloadStrIsPtrByFlag(valueNode), "value 超过临界长度应切到 ptr");
	// 回退到临界：应能从 ptr 回到 inline（覆盖回切逻辑）
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(valueNode, valueInline));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonGetPayloadStrIsPtrByFlag(valueNode), "value 回退到临界长度应切回 inline");

	/**
	 * key 边界计算（固定 value="v"）：
	 * keyLen 变化时，keyLenField 可能从 1/2/4 字节变化，
	 * 所以不能硬编码某个 keyLen，必须遍历找到“当前配置下最大可 inline 的 key 长度”。
	 */
	uint32_t maxInlineKeyLen = 0;
	for (uint32_t keyLen = 0; keyLen <= inlineSize; keyLen++)
	{
		uint32_t keyFieldLen = RyanJsonInternalDecodeKeyLenField(RyanJsonInternalCalcLenBytes(keyLen));
		uint32_t need = keyFieldLen + keyLen + 1 + fixedValueLen + 1;
		if (need <= inlineSize) { maxInlineKeyLen = keyLen; }
	}

	// 交叉校验：maxInlineKeyLen 应满足 inline；maxInlineKeyLen+1 应超过 inline
	uint32_t inlineNeed =
		RyanJsonInternalDecodeKeyLenField(RyanJsonInternalCalcLenBytes(maxInlineKeyLen)) + maxInlineKeyLen + 1 + fixedValueLen + 1;
	uint32_t ptrNeed = RyanJsonInternalDecodeKeyLenField(RyanJsonInternalCalcLenBytes(maxInlineKeyLen + 1U)) + (maxInlineKeyLen + 1U) +
			   1 + fixedValueLen + 1;
	TEST_ASSERT_TRUE_MESSAGE(inlineNeed <= inlineSize, "inline key 临界值计算错误");
	TEST_ASSERT_TRUE_MESSAGE(ptrNeed > inlineSize, "ptr key 临界值计算错误");

	// keyInline: 恰好临界；keyPtr: 超临界 1 字节
	char *keyInline = (char *)malloc((size_t)maxInlineKeyLen + 1U);
	char *keyPtr = (char *)malloc((size_t)maxInlineKeyLen + 2U);
	TEST_ASSERT_NOT_NULL(keyInline);
	TEST_ASSERT_NOT_NULL(keyPtr);
	memset(keyInline, 'k', maxInlineKeyLen);
	keyInline[maxInlineKeyLen] = '\0';
	memset(keyPtr, 'K', maxInlineKeyLen + 1U);
	keyPtr[maxInlineKeyLen + 1U] = '\0';

	RyanJson_t keyNode = RyanJsonCreateString("", "v");
	TEST_ASSERT_NOT_NULL(keyNode);
	// key 临界长度：应为 inline
	TEST_ASSERT_TRUE(RyanJsonChangeKey(keyNode, keyInline));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonGetPayloadStrIsPtrByFlag(keyNode), "key 临界长度应为 inline");
	// key 超临界：应切到 ptr
	TEST_ASSERT_TRUE(RyanJsonChangeKey(keyNode, keyPtr));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonGetPayloadStrIsPtrByFlag(keyNode), "key 超过临界长度应切到 ptr");
	// key 回退临界：应切回 inline
	TEST_ASSERT_TRUE(RyanJsonChangeKey(keyNode, keyInline));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonGetPayloadStrIsPtrByFlag(keyNode), "key 回退到临界长度应切回 inline");

	// 释放临时资源，避免单测内存泄漏
	RyanJsonDelete(valueNode);
	RyanJsonDelete(keyNode);
	free(valueInline);
	free(valuePtr);
	free(keyInline);
	free(keyPtr);
}

void testChangeRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testChangeEdgeCases);
	RUN_TEST(testChangeValueStress);
	RUN_TEST(testChangeScalarAndStorageMode);
	RUN_TEST(testChangeInlineCalcBoundary);
}
