#include "testBase.h"

static void testForEachEdgeCases(void)
{
	RyanJson_t item = NULL;

	// 遍历 NULL Object (应该安全跳过循环)
	int32_t count = 0;
	RyanJsonArrayForEach(NULL, item)
	{
		count++;
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, count, "遍历 NULL Array 应不执行循环");

	count = 0;
	RyanJsonObjectForEach(NULL, item)
	{
		count++;
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, count, "遍历 NULL Object 应不执行循环");

	// 遍历非容器 Object (应该同上)
	RyanJson_t num = RyanJsonCreateInt("num", 1);
	count = 0;
	RyanJsonArrayForEach(num, item)
	{
		count++;
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, count, "遍历非容器 Array 应不执行循环");

	count = 0;
	RyanJsonObjectForEach(num, item)
	{
		count++;
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, count, "遍历非容器 Object 应不执行循环");
	RyanJsonDelete(num);

	// 循环中断测试 (break)
	RyanJson_t arr = RyanJsonCreateArray();
	TEST_ASSERT_NOT_NULL(arr);
	RyanJsonAddIntToArray(arr, 1);
	RyanJsonAddIntToArray(arr, 2);
	RyanJsonAddIntToArray(arr, 3);

	count = 0;
	RyanJsonArrayForEach(arr, item)
	{
		count++;
		if (RyanJsonGetIntValue(item) == 2) { break; }
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(2, count, "循环 break 测试失败");
	RyanJsonDelete(arr);
}

static void testForEachIterativeTraversals(void)
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
			 "\"boolFalse\":false,\"null\":null}]}";

	RyanJson_t json = RyanJsonParse(jsonstr);
	TEST_ASSERT_NOT_NULL_MESSAGE(json, "解析 Json 失败");

	RyanJson_t item = NULL;

	// 遍历 arrayDouble Array 测试
	RyanJsonArrayForEach(RyanJsonGetObjectToKey(json, "arrayDouble"), item)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDouble(item), "Array 元素不是 Double 类型");
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompareDouble(16.89, RyanJsonGetDoubleValue(item)), "Array 元素值不正确");
	}

	// 遍历 arrayInt Array 测试
	RyanJsonArrayForEach(RyanJsonGetObjectToKey(json, "arrayInt"), item)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsInt(item), "Array 元素不是 Int 类型");
		TEST_ASSERT_EQUAL_INT_MESSAGE(16, RyanJsonGetIntValue(item), "Array 元素值不正确");
	}

	// 遍历 item Object 测试
	RyanJsonObjectForEach(RyanJsonGetObjectToKey(json, "item"), item)
	{
		TEST_ASSERT_NOT_NULL_MESSAGE(RyanJsonGetKey(item), "Object 键值为空");
		char *str = RyanJsonPrint(item, 128, RyanJsonTrue, NULL);
		TEST_ASSERT_NOT_NULL_MESSAGE(str, "遍历项打印失败");
		RyanJsonFree(str);
	}

	RyanJsonDelete(json);
}

static void testForEachComplexSummaryWithMixedArrayMacros(void)
{
	// 复杂链路：
	// Create(root/events/summary) -> Add*ToArray(混合类型) -> ArrayForEach(统计+改写)
	// -> ObjectForEach(汇总校验) -> Print/Parse Roundtrip。
	// 目标：
	// - 覆盖 AddNull/Bool/Double/StringToArray 与 AddItemToArray 的协同路径；
	// - 覆盖遍历过程中“按类型分支修改节点”是否会引发连锁结构破坏；
	// - 覆盖 ObjectForEach 在汇总 Object 上的稳定遍历语义。
	RyanJson_t root = RyanJsonCreateObject();
	RyanJson_t events = RyanJsonCreateArray();
	RyanJson_t summary = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(root);
	TEST_ASSERT_NOT_NULL(events);
	TEST_ASSERT_NOT_NULL(summary);

	// 构造混合类型 Array：Null / Bool / Double / String / Object
	TEST_ASSERT_TRUE(RyanJsonAddNullToArray(events));
	TEST_ASSERT_TRUE(RyanJsonAddBoolToArray(events, RyanJsonTrue));
	TEST_ASSERT_TRUE(RyanJsonAddDoubleToArray(events, 12.5));
	TEST_ASSERT_TRUE(RyanJsonAddStringToArray(events, "evt"));

	RyanJson_t payloadObj = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(payloadObj);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(payloadObj, "type", "obj"));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(payloadObj, "score", 7));
	TEST_ASSERT_TRUE(RyanJsonAddItemToArray(events, payloadObj));

	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(summary, "nullCnt", 0));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(summary, "boolCnt", 0));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(summary, "doubleCnt", 0));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(summary, "stringCnt", 0));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(summary, "objectCnt", 0));

	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, "events", events));
	TEST_ASSERT_TRUE(RyanJsonAddItemToObject(root, "summary", summary));

	// 挂载完成后重新取回容器，避免后续误用旧缓存指针。
	events = RyanJsonGetObjectToKey(root, "events");
	summary = RyanJsonGetObjectToKey(root, "summary");
	TEST_ASSERT_NOT_NULL(events);
	TEST_ASSERT_NOT_NULL(summary);
	TEST_ASSERT_EQUAL_UINT32(5U, RyanJsonGetArraySize(events));

	uint32_t idx = 0;
	int32_t nullIndex = -1;
	RyanJson_t item = NULL;
	RyanJsonArrayForEach(events, item)
	{
		if (RyanJsonIsNull(item))
		{
			TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(summary, "nullCnt"),
								RyanJsonGetIntValue(RyanJsonGetObjectToKey(summary, "nullCnt")) + 1));
			// 记录待替换位置。不要在 ArrayForEach 内直接替换当前节点，避免破坏迭代指针。
			nullIndex = (int32_t)idx;
		}
		else if (RyanJsonIsBool(item))
		{
			TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(summary, "boolCnt"),
								RyanJsonGetIntValue(RyanJsonGetObjectToKey(summary, "boolCnt")) + 1));
		}
		else if (RyanJsonIsDouble(item))
		{
			TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(summary, "doubleCnt"),
								RyanJsonGetIntValue(RyanJsonGetObjectToKey(summary, "doubleCnt")) + 1));
			TEST_ASSERT_TRUE(RyanJsonChangeDoubleValue(item, 25.0));
		}
		else if (RyanJsonIsString(item))
		{
			TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(summary, "stringCnt"),
								RyanJsonGetIntValue(RyanJsonGetObjectToKey(summary, "stringCnt")) + 1));
		}
		else if (RyanJsonIsObject(item))
		{
			TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(summary, "objectCnt"),
								RyanJsonGetIntValue(RyanJsonGetObjectToKey(summary, "objectCnt")) + 1));
			TEST_ASSERT_TRUE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(item, "score"), 9));
		}
		else
		{
			TEST_FAIL_MESSAGE("events 中出现未预期类型");
		}
		idx++;
	}
	TEST_ASSERT_TRUE_MESSAGE(nullIndex >= 0, "应至少命中一个 null 元素");
	TEST_ASSERT_TRUE(RyanJsonReplaceByIndex(events, (uint32_t)nullIndex, RyanJsonCreateString(NULL, "null->string")));

	// 不能依赖索引顺序：
	// 在 RyanJsonDefaultAddAtHead=true/false 两种模式下，Add*ToArray 的相对位置不同。
	// 这里改为顺序无关的语义校验，确保“类型/值/数量”正确即可。
	uint32_t boolCnt = 0;
	uint32_t doubleCnt = 0;
	uint32_t stringCnt = 0;
	uint32_t objectCnt = 0;
	uint32_t hitEvt = 0;
	uint32_t hitNullToString = 0;
	RyanJsonArrayForEach(events, item)
	{
		if (RyanJsonIsBool(item))
		{
			boolCnt++;
			TEST_ASSERT_TRUE(RyanJsonGetBoolValue(item));
		}
		else if (RyanJsonIsDouble(item))
		{
			doubleCnt++;
			TEST_ASSERT_TRUE(RyanJsonCompareDouble(25.0, RyanJsonGetDoubleValue(item)));
		}
		else if (RyanJsonIsString(item))
		{
			const char *s = RyanJsonGetStringValue(item);
			TEST_ASSERT_NOT_NULL(s);
			stringCnt++;
			if (strcmp(s, "evt") == 0) { hitEvt++; }
			else if (strcmp(s, "null->string") == 0) { hitNullToString++; }
			else
			{
				TEST_FAIL_MESSAGE("events 中 String 值不在预期集合");
			}
		}
		else if (RyanJsonIsObject(item))
		{
			objectCnt++;
			TEST_ASSERT_EQUAL_INT(9, RyanJsonGetIntValue(RyanJsonGetObjectToKey(item, "score")));
		}
		else
		{
			TEST_FAIL_MESSAGE("events 中出现未预期类型");
		}
	}
	TEST_ASSERT_EQUAL_UINT32(1U, boolCnt);
	TEST_ASSERT_EQUAL_UINT32(1U, doubleCnt);
	TEST_ASSERT_EQUAL_UINT32(2U, stringCnt);
	TEST_ASSERT_EQUAL_UINT32(1U, objectCnt);
	TEST_ASSERT_EQUAL_UINT32(1U, hitEvt);
	TEST_ASSERT_EQUAL_UINT32(1U, hitNullToString);

	int32_t summaryTotal = 0;
	RyanJsonObjectForEach(summary, item)
	{
		TEST_ASSERT_TRUE(RyanJsonIsInt(item));
		summaryTotal += RyanJsonGetIntValue(item);
	}
	TEST_ASSERT_EQUAL_INT(5, summaryTotal);

	char *printed = RyanJsonPrint(root, 192, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

static void testForEachObjectTraversalWithCrossContainerMove(void)
{
	// 复杂链路：
	// Parse(wait/done) -> ObjectForEach(wait) 收集条件 -> 复制挂载到 done
	// -> 按收集 key 回删 wait -> ArrayForEach(done) 二次校验。
	// 目标：
	// - 验证 Object 遍历驱动的“条件迁移”不会造成容器链表损坏；
	// - 验证迁移后再删除源节点，目标容器数据保持稳定；
	// - 验证复杂链路下 key/index 查询语义一致。
	const char *source = "{\"wait\":{\"a\":{\"done\":false},\"b\":{\"done\":true},\"c\":{\"done\":true}},\"done\":[]}";
	RyanJson_t root = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL(root);

	RyanJson_t wait = RyanJsonGetObjectToKey(root, "wait");
	RyanJson_t done = RyanJsonGetObjectToKey(root, "done");
	TEST_ASSERT_NOT_NULL(wait);
	TEST_ASSERT_NOT_NULL(done);
	TEST_ASSERT_TRUE(RyanJsonIsObject(wait));
	TEST_ASSERT_TRUE(RyanJsonIsArray(done));

	char movedKeys[3][8] = {{0}};
	uint32_t movedCount = 0;
	RyanJson_t item = NULL;
	RyanJsonObjectForEach(wait, item)
	{
		RyanJson_t doneNode = RyanJsonGetObjectToKey(item, "done");
		TEST_ASSERT_NOT_NULL(doneNode);
		if (RyanJsonGetBoolValue(doneNode))
		{
			TEST_ASSERT_TRUE_MESSAGE(movedCount < 3U, "movedKeys 缓冲区不足");
			RyanJsonSnprintf(movedKeys[movedCount], sizeof(movedKeys[movedCount]), "%s", RyanJsonGetKey(item));
			movedCount++;
			TEST_ASSERT_TRUE(RyanJsonAddItemToArray(done, RyanJsonDuplicate(item)));
		}
	}
	TEST_ASSERT_EQUAL_UINT32(2U, movedCount);
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(done));

	for (uint32_t i = 0; i < movedCount; i++)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonDeleteByKey(wait, movedKeys[i]), "按收集 key 回删 wait 失败");
	}

	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(wait, "a"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(wait, "b"));
	TEST_ASSERT_FALSE(RyanJsonHasObjectToKey(wait, "c"));
	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetSize(wait));

	uint32_t doneTraverseCount = 0;
	RyanJsonArrayForEach(done, item)
	{
		doneTraverseCount++;
		TEST_ASSERT_TRUE(RyanJsonGetBoolValue(RyanJsonGetObjectToKey(item, "done")));
	}
	TEST_ASSERT_EQUAL_UINT32(2U, doneTraverseCount);

	char *printed = RyanJsonPrint(root, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

void testForEachRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testForEachEdgeCases);
	RUN_TEST(testForEachIterativeTraversals);
	RUN_TEST(testForEachComplexSummaryWithMixedArrayMacros);
	RUN_TEST(testForEachObjectTraversalWithCrossContainerMove);
}
