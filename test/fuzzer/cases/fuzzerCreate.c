#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

/**
 * @brief 创建与插入测试
 *
 * 测试 RyanJson 的节点创建、数据类型设置以及对象/数组的插入操作。
 * 覆盖场景：
 * 异常参数注入：测试空指针、无效 key、类型不匹配等错误处理。
 * 基础类型创建：测试 Bool/Int/Double/String 及其数组的创建与添加。
 * 递归结构构建：递归地向对象/数组中添加随机生成的子节点。
 * 边界插入测试：覆盖数组头部与中间位置插入。
 *
 * @param state Fuzzer 状态上下文
 * @param pJson 当前正在操作的 Json 节点
 * @param size 输入数据大小，用于控制递归深度和随机决策
 */
RyanJsonBool_e RyanJsonFuzzerTestCreate(RyanJson_t pJson, uint32_t size)
{
	// 覆盖 RyanJsonIsDetachedItem 的防御分支：
	// next==NULL 但 IsLast==1 属于非法游离态，应返回 false。
	// 该路径只需覆盖一次，避免每轮 fuzz 重复创建节点。
	static RyanJsonBool_e detachedFlagBranchCovered = RyanJsonFalse;
	if (RyanJsonFalse == detachedFlagBranchCovered)
	{
		RyanJsonBool_e lastIsEnableMemFail = g_fuzzerState.isEnableMemFail;
		g_fuzzerState.isEnableMemFail = false;

		RyanJson_t malformedDetached = RyanJsonCreateInt(NULL, 1);
		assert(NULL != malformedDetached);
		assert(RyanJsonTrue == RyanJsonIsDetachedItem(malformedDetached));
		RyanJsonSetPayloadIsLastByFlag(malformedDetached, RyanJsonTrue);
		assert(RyanJsonFalse == RyanJsonIsDetachedItem(malformedDetached));
		// 恢复 flag，避免后续 Delete 走错链表语义
		RyanJsonSetPayloadIsLastByFlag(malformedDetached, RyanJsonFalse);
		RyanJsonDelete(malformedDetached);

		g_fuzzerState.isEnableMemFail = lastIsEnableMemFail;
		detachedFlagBranchCovered = RyanJsonTrue;
	}

	// 覆盖对象重复 key 防御分支（一次性）
	static RyanJsonBool_e duplicateKeyGuardCovered = RyanJsonFalse;
	if (RyanJsonFalse == duplicateKeyGuardCovered)
	{
		RyanJsonBool_e lastIsEnableMemFail = g_fuzzerState.isEnableMemFail;
		g_fuzzerState.isEnableMemFail = false;

		RyanJson_t obj = RyanJsonCreateObject();
		assert(NULL != obj);
		assert(RyanJsonTrue == RyanJsonAddIntToObject(obj, "dup", 1));
#if true == RyanJsonStrictObjectKeyCheck
		assert(RyanJsonFalse == RyanJsonAddIntToObject(obj, "dup", 2));
		assert(RyanJsonFalse == RyanJsonInsert(obj, UINT32_MAX, RyanJsonCreateInt("dup", 3)));
		assert(1 == RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "dup")));
#else
		assert(RyanJsonTrue == RyanJsonAddIntToObject(obj, "dup", 2));
		assert(RyanJsonTrue == RyanJsonInsert(obj, UINT32_MAX, RyanJsonCreateInt("dup", 3)));
		assert(3 == RyanJsonGetSize(obj));
#if true == RyanJsonDefaultAddAtHead
		assert(2 == RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "dup")));
#else
		assert(1 == RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "dup")));
#endif
#endif
		RyanJsonDelete(obj);

		g_fuzzerState.isEnableMemFail = lastIsEnableMemFail;
		duplicateKeyGuardCovered = RyanJsonTrue;
	}

	// RyanJsonInsert 特殊路径：模拟内存分配失败或无效参数输入
	uint32_t index = 6;

	if (RyanJsonFuzzerShouldFail(100))
	{
		assert(RyanJsonFalse == RyanJsonInsert(NULL, UINT32_MAX, RyanJsonCreateString("key", "string")));
		assert(RyanJsonFalse == RyanJsonInsert(pJson, UINT32_MAX, NULL));
		assert(RyanJsonFalse == RyanJsonInsert(NULL, 0, NULL));

		assert(NULL == RyanJsonCreateString(NULL, NULL));
		assert(NULL == RyanJsonCreateString("NULL", NULL));

		assert(RyanJsonFalse == RyanJsonChangeStringValue(NULL, NULL));
		assert(RyanJsonFalse == RyanJsonChangeStringValue(pJson, NULL));
		assert(RyanJsonFalse == RyanJsonChangeStringValue(NULL, "NULL"));
		if (RyanJsonFalse == RyanJsonIsKey(pJson) && RyanJsonFalse == RyanJsonIsString(pJson)) // pJson 类型错误
		{
			assert(RyanJsonFalse == RyanJsonChangeStringValue(pJson, "NULL"));
		}

		assert(RyanJsonFalse == RyanJsonChangeKey(NULL, NULL));
		assert(RyanJsonFalse == RyanJsonChangeKey(pJson, NULL));
		assert(RyanJsonFalse == RyanJsonChangeKey(NULL, "NULL"));
		if (RyanJsonFalse == RyanJsonIsKey(pJson) && RyanJsonFalse == RyanJsonIsString(pJson)) // pJson 类型错误
		{
			g_fuzzerState.isEnableMemFail = false;
			assert(RyanJsonFalse == RyanJsonChangeKey(pJson, "NULL"));
			g_fuzzerState.isEnableMemFail = true;
		}

		// 测试“无 key 但有 strValue”的分支
		if (RyanJsonFalse == RyanJsonIsKey(pJson) && RyanJsonTrue == RyanJsonIsString(pJson))
		{
			g_fuzzerState.isEnableMemFail = false;
			assert(RyanJsonFalse == RyanJsonChangeKey(pJson, "NULL"));
			g_fuzzerState.isEnableMemFail = true;
		}

		assert(RyanJsonFalse == RyanJsonChangeIntValue(NULL, 0));
		assert(RyanJsonFalse == RyanJsonChangeDoubleValue(NULL, 0));
		assert(RyanJsonFalse == RyanJsonChangeBoolValue(NULL, 0));

		// Change 接口类型不匹配分支
		RyanJson_t mismatchNode = RyanJsonCreateDouble(NULL, 1.0);
		assert(RyanJsonFalse == RyanJsonChangeIntValue(mismatchNode, 7));
		if (NULL != mismatchNode) { RyanJsonDelete(mismatchNode); }

		mismatchNode = RyanJsonCreateInt(NULL, 1);
		assert(RyanJsonFalse == RyanJsonChangeDoubleValue(mismatchNode, 7.0));
		if (NULL != mismatchNode) { RyanJsonDelete(mismatchNode); }

		mismatchNode = RyanJsonCreateInt(NULL, 1);
		assert(RyanJsonFalse == RyanJsonChangeBoolValue(mismatchNode, RyanJsonTrue));
		if (NULL != mismatchNode) { RyanJsonDelete(mismatchNode); }

		mismatchNode = RyanJsonCreateString(NULL, "v");
		assert(RyanJsonFalse == RyanJsonChangeKey(mismatchNode, "k2"));
		if (NULL != mismatchNode) { RyanJsonDelete(mismatchNode); }

		mismatchNode = RyanJsonCreateBool(NULL, RyanJsonTrue);
		assert(RyanJsonFalse == RyanJsonChangeStringValue(mismatchNode, "x"));
		if (NULL != mismatchNode) { RyanJsonDelete(mismatchNode); }

		assert(RyanJsonFalse == RyanJsonAddItemToObject(NULL, NULL, NULL));
		assert(RyanJsonFalse == RyanJsonAddItemToObject(pJson, NULL, NULL));

		assert(NULL == RyanJsonCreateIntArray(NULL, 0));
		assert(NULL == RyanJsonCreateDoubleArray(NULL, 0));
		assert(NULL == RyanJsonCreateStringArray(NULL, 0));

		// 仅调用，不判断返回值，因为有可能会成功的
		RyanJsonHasObjectToKey(NULL, "0", "1", "2", "3");
		RyanJsonHasObjectToIndex(NULL, 0, 1, 2, 3);
		RyanJsonHasObjectToKey(pJson, "0", "1", "2", "3");
		RyanJsonHasObjectToIndex(pJson, 0, 1, 2, 3);
		RyanJsonHasObjectToIndex(pJson, 0);

		// 已挂树的 item 不应再次 Insert/AddItem
		{
			g_fuzzerState.isEnableMemFail = false;

			RyanJson_t arr1 = RyanJsonCreateArray();
			RyanJson_t arr2 = RyanJsonCreateArray();
			RyanJson_t attachedArr = RyanJsonCreateArray();
			assert(RyanJsonFalse == RyanJsonIsDetachedItem(NULL));
			assert(NULL != arr1 && NULL != arr2 && NULL != attachedArr);
			assert(RyanJsonTrue == RyanJsonIsDetachedItem(attachedArr));
			assert(RyanJsonTrue == RyanJsonInsert(arr1, 0, attachedArr));
			assert(RyanJsonFalse == RyanJsonIsDetachedItem(attachedArr));
			assert(RyanJsonFalse == RyanJsonInsert(arr2, 0, attachedArr));
			assert(RyanJsonFalse == RyanJsonAddItemToArray(arr2, attachedArr));
			RyanJson_t detachedArr = RyanJsonDetachByIndex(arr1, 0);
			assert(detachedArr == attachedArr);
			assert(RyanJsonTrue == RyanJsonIsDetachedItem(detachedArr));
			RyanJsonDelete(detachedArr);

			RyanJson_t obj1 = RyanJsonCreateObject();
			RyanJson_t obj2 = RyanJsonCreateObject();
			RyanJson_t attachedObj = RyanJsonInternalCreateObjectAndKey("k");
			assert(NULL != obj1 && NULL != obj2 && NULL != attachedObj);
			assert(RyanJsonTrue == RyanJsonIsDetachedItem(attachedObj));
			assert(RyanJsonTrue == RyanJsonInsert(obj1, 0, attachedObj));
			assert(RyanJsonFalse == RyanJsonIsDetachedItem(attachedObj));
			assert(RyanJsonFalse == RyanJsonInsert(obj2, 0, attachedObj));
			assert(RyanJsonFalse == RyanJsonAddItemToObject(obj2, "dup", attachedObj));
			RyanJson_t detachedObj = RyanJsonDetachByIndex(obj1, 0);
			assert(detachedObj == attachedObj);
			assert(RyanJsonTrue == RyanJsonIsDetachedItem(detachedObj));
			RyanJsonDelete(detachedObj);

			RyanJsonDelete(arr2);
			RyanJsonDelete(obj2);
			RyanJsonDelete(arr1);
			RyanJsonDelete(obj1);

			g_fuzzerState.isEnableMemFail = true;
		}
	}

	char *key = "keyaaa";
	RyanJsonAddNullToObject(pJson, key);

	// 如果当前节点是 key 类型，尝试获取其 key 字符串作为后续操作 key
	if (RyanJsonTrue == RyanJsonIsKey(pJson))
	{
		// Change 系列测试已在 modify 用例覆盖
		key = RyanJsonGetKey(pJson);
	}

	// 标量类型追加测试
	if (RyanJsonTrue == RyanJsonIsBool(pJson) && RyanJsonFuzzerShouldFail(index))
	{
		if (RyanJsonTrue == RyanJsonAddBoolToObject(pJson, key, RyanJsonGetBoolValue(pJson)))
		{
			fuzzTestWithMemFail(
				assert(RyanJsonGetBoolValue(RyanJsonGetObjectByKey(pJson, key)) == RyanJsonGetBoolValue(pJson)));
		}
	}

	if (RyanJsonTrue == RyanJsonIsNumber(pJson) && RyanJsonFuzzerShouldFail(index))
	{
		if (RyanJsonTrue == RyanJsonIsInt(pJson))
		{
			if (RyanJsonTrue == RyanJsonAddIntToObject(pJson, key, RyanJsonGetIntValue(pJson)))
			{
				fuzzTestWithMemFail(
					assert(RyanJsonGetIntValue(RyanJsonGetObjectByKey(pJson, key)) == RyanJsonGetIntValue(pJson)));
			}

			// 构造测试用 Int 数组
			int32_t val = RyanJsonGetIntValue(pJson);
			int32_t testIntArray[] = {val, val, val, val, val};
			RyanJsonBool_e jsonAddResult = RyanJsonAddItemToObject(
				pJson, (0 != size % 2) ? key : "arrayString",
				RyanJsonCreateIntArray(testIntArray, sizeof(testIntArray) / sizeof(testIntArray[0])));
			if (RyanJsonTrue == jsonAddResult)
			{
				fuzzTestWithMemFail({
					RyanJson_t itemIntArr = RyanJsonGetObjectByKey(pJson, key);
					assert(RyanJsonIsArray(itemIntArr));
					assert(RyanJsonGetArraySize(itemIntArr) == sizeof(testIntArray) / sizeof(testIntArray[0]));
					RyanJson_t item;
					RyanJsonArrayForEach(itemIntArr, item)
					{
						assert(RyanJsonGetIntValue(item) == val);
					}
				});
			}
		}
		else if (RyanJsonTrue == RyanJsonIsDouble(pJson))
		{
			if (RyanJsonTrue == RyanJsonAddDoubleToObject(pJson, key, RyanJsonGetDoubleValue(pJson)))
			{
				fuzzTestWithMemFail(assert(RyanJsonCompareDouble(RyanJsonGetDoubleValue(RyanJsonGetObjectByKey(pJson, key)),
										 RyanJsonGetDoubleValue(pJson))));
			}

			// 构造测试用 Double 数组

			double val = RyanJsonGetDoubleValue(pJson);
			double testDoubleArray[] = {val, val, val, val, val};
			RyanJsonBool_e jsonAddResult = RyanJsonAddItemToObject(
				pJson, (0 != size % 2) ? key : "arrayString",
				RyanJsonCreateDoubleArray(testDoubleArray, sizeof(testDoubleArray) / sizeof(testDoubleArray[0])));
			if (RyanJsonTrue == jsonAddResult)
			{
				fuzzTestWithMemFail({
					RyanJson_t itemIntArr = RyanJsonGetObjectByKey(pJson, key);
					assert(RyanJsonIsArray(itemIntArr));
					assert(RyanJsonGetArraySize(itemIntArr) == sizeof(testDoubleArray) / sizeof(testDoubleArray[0]));
					RyanJson_t item;
					RyanJsonArrayForEach(itemIntArr, item)
					{
						assert(RyanJsonCompareDouble(RyanJsonGetDoubleValue(item), val));
					}
				});
			}
		}
	}
	if (RyanJsonTrue == RyanJsonIsString(pJson) && RyanJsonFuzzerShouldFail(index))
	{
		if (RyanJsonTrue == RyanJsonAddStringToObject(pJson, key, RyanJsonGetStringValue(pJson)))
		{
			fuzzTestWithMemFail(assert(
				0 == strcmp(RyanJsonGetStringValue(RyanJsonGetObjectByKey(pJson, key)), RyanJsonGetStringValue(pJson))));
		}

		// 构造测试用 String 数组
		const char *val = RyanJsonGetStringValue(pJson);
		const char *testStringArray[] = {val, val, val, val, val};
		RyanJsonBool_e jsonAddResult = RyanJsonAddItemToObject(
			pJson, (0 != size % 2) ? key : "arrayString",
			RyanJsonCreateStringArray(testStringArray, sizeof(testStringArray) / sizeof(testStringArray[0])));
		if (RyanJsonTrue == jsonAddResult)
		{
			fuzzTestWithMemFail({
				RyanJson_t itemIntArr = RyanJsonGetObjectByKey(pJson, key);
				assert(RyanJsonIsArray(itemIntArr));
				assert(RyanJsonGetArraySize(itemIntArr) == sizeof(testStringArray) / sizeof(testStringArray[0]));
				RyanJson_t item;
				RyanJsonArrayForEach(itemIntArr, item)
				{
					assert(0 == strcmp(RyanJsonGetStringValue(item), val));
				}
			});
		}
	}

	// 复合类型递归与插入测试
	if (RyanJsonTrue == RyanJsonIsArray(pJson) || RyanJsonTrue == RyanJsonIsObject(pJson))
	{
		RyanJson_t item;
		// 递归处理子节点
		RyanJsonObjectForEach(pJson, item)
		{
			RyanJsonFuzzerTestCreate(item, size);
		}

		// 添加随机生成的节点 (AddItem 方案A: 仅允许 Array/Object，成功后会消费原 item)
		uint32_t oldSize = RyanJsonGetSize(pJson);
		RyanJson_t newItem = RyanJsonFuzzerCreateRandomNode(pJson);
		RyanJson_t newItemDup = NULL;
		if (newItem && (RyanJsonIsArray(newItem) || RyanJsonIsObject(newItem))) { newItemDup = RyanJsonDuplicate(newItem); }
		RyanJsonBool_e jsonAddResult = RyanJsonAddItemToObject(pJson, key, newItem);
		if (RyanJsonTrue == jsonAddResult)
		{
			fuzzTestWithMemFail({
				assert(RyanJsonGetSize(pJson) == oldSize + 1);
#if true == RyanJsonDefaultAddAtHead
				RyanJson_t itemJson = RyanJsonGetObjectByIndex(pJson, 0);
#else
				RyanJson_t itemJson = RyanJsonGetObjectByIndex(pJson, oldSize);
#endif
				assert(NULL != itemJson);
				assert(RyanJsonIsArray(itemJson) || RyanJsonIsObject(itemJson));
				if (RyanJsonTrue == RyanJsonIsObject(pJson))
				{
					assert(RyanJsonIsKey(itemJson));
					assert(0 == strcmp(RyanJsonGetKey(itemJson), key));
				}
				if (newItemDup) { assert(RyanJsonCompare(itemJson, newItemDup)); }
			});
		}
		if (newItemDup) { RyanJsonDelete(newItemDup); }

		if (RyanJsonTrue == RyanJsonIsArray(pJson))
		{
			if (RyanJsonFuzzerShouldFail(index / 8) && RyanJsonTrue == RyanJsonAddNullToArray(pJson))
			{
				fuzzTestWithMemFail({
					assert(RyanJsonIsArray(pJson));
#if true == RyanJsonDefaultAddAtHead
					assert(RyanJsonIsNull(RyanJsonGetObjectByIndex(pJson, 0)));
#else
					assert(RyanJsonIsNull(RyanJsonGetObjectByIndex(pJson, RyanJsonGetSize(pJson) - 1)));
#endif
				});
			}
			if (RyanJsonFuzzerShouldFail(index / 8) &&
			    RyanJsonTrue == RyanJsonAddBoolToArray(pJson, 0 != size % 2 ? RyanJsonTrue : RyanJsonFalse))
			{
				fuzzTestWithMemFail({
					assert(RyanJsonIsArray(pJson));
#if true == RyanJsonDefaultAddAtHead
					RyanJson_t item = RyanJsonGetObjectByIndex(pJson, 0);
#else
					RyanJson_t item = RyanJsonGetObjectByIndex(pJson, RyanJsonGetSize(pJson) - 1);
#endif
					assert(RyanJsonIsBool(item));
					assert(RyanJsonGetBoolValue(item) == (0 != size % 2 ? RyanJsonTrue : RyanJsonFalse));
				});
			}
			if (RyanJsonFuzzerShouldFail(index / 8) && RyanJsonTrue == RyanJsonAddIntToArray(pJson, size))
			{
				fuzzTestWithMemFail({
					assert(RyanJsonIsArray(pJson));
#if true == RyanJsonDefaultAddAtHead
					RyanJson_t item = RyanJsonGetObjectByIndex(pJson, 0);
#else
					RyanJson_t item = RyanJsonGetObjectByIndex(pJson, RyanJsonGetSize(pJson) - 1);
#endif
					assert(RyanJsonIsInt(item));
					assert(RyanJsonGetIntValue(item) == size);
				});
			}
			if (RyanJsonFuzzerShouldFail(index / 8) && RyanJsonTrue == RyanJsonAddDoubleToArray(pJson, size * 0.123456))
			{
				fuzzTestWithMemFail({
					assert(RyanJsonIsArray(pJson));
#if true == RyanJsonDefaultAddAtHead
					RyanJson_t item = RyanJsonGetObjectByIndex(pJson, 0);
#else
					RyanJson_t item = RyanJsonGetObjectByIndex(pJson, RyanJsonGetSize(pJson) - 1);
#endif
					assert(RyanJsonIsDouble(item));
					assert(RyanJsonCompareDouble(RyanJsonGetDoubleValue(item), size * 0.123456));
				});
			}
			if (RyanJsonFuzzerShouldFail(index / 8) && RyanJsonTrue == RyanJsonAddStringToArray(pJson, "NULL"))
			{
				fuzzTestWithMemFail({
					assert(RyanJsonIsArray(pJson));
#if true == RyanJsonDefaultAddAtHead
					RyanJson_t item = RyanJsonGetObjectByIndex(pJson, 0);
#else
					RyanJson_t item = RyanJsonGetObjectByIndex(pJson, RyanJsonGetSize(pJson) - 1);
#endif
					assert(RyanJsonIsString(item));
					assert(0 == strcmp(RyanJsonGetStringValue(item), "NULL"));
				});
			}
			if (RyanJsonFuzzerShouldFail(index / 8) && RyanJsonTrue == RyanJsonAddItemToArray(pJson, RyanJsonCreateArray()))
			{
				fuzzTestWithMemFail({
					assert(RyanJsonIsArray(pJson));
#if true == RyanJsonDefaultAddAtHead
					RyanJson_t item = RyanJsonGetObjectByIndex(pJson, 0);
#else
					RyanJson_t item = RyanJsonGetObjectByIndex(pJson, RyanJsonGetSize(pJson) - 1);
#endif
					assert(RyanJsonIsArray(item));
				});
			}
			if (RyanJsonFuzzerShouldFail(index / 8) && RyanJsonTrue == RyanJsonAddItemToArray(pJson, RyanJsonCreateObject()))
			{
				fuzzTestWithMemFail({
					assert(RyanJsonIsArray(pJson));
#if true == RyanJsonDefaultAddAtHead
					RyanJson_t item = RyanJsonGetObjectByIndex(pJson, 0);
#else
					RyanJson_t item = RyanJsonGetObjectByIndex(pJson, RyanJsonGetSize(pJson) - 1);
#endif
					assert(RyanJsonIsObject(item));
				});
			}

			if (RyanJsonFuzzerShouldFail(index / 8))
			{
				RyanJson_t randomNode = RyanJsonFuzzerCreateRandomNode(RyanJsonGetArrayValue(pJson));
				RyanJson_t randomNodeDup = NULL;
				if (randomNode && (RyanJsonIsArray(randomNode) || RyanJsonIsObject(randomNode)))
				{
					randomNodeDup = RyanJsonDuplicate(randomNode);
				}
				uint32_t oldLen = RyanJsonGetSize(pJson);
				if (RyanJsonTrue == RyanJsonAddItemToArray(pJson, randomNode))
				{
					fuzzTestWithMemFail({
						assert(RyanJsonIsArray(pJson));
						assert(RyanJsonGetSize(pJson) == oldLen + 1);
#if true == RyanJsonDefaultAddAtHead
						RyanJson_t item = RyanJsonGetObjectByIndex(pJson, 0);
#else
						RyanJson_t item = RyanJsonGetObjectByIndex(pJson, oldLen);
#endif
						assert(NULL != item);
						if (randomNodeDup) { assert(RyanJsonCompare(item, randomNodeDup)); }
					});
				}
				if (randomNodeDup) { RyanJsonDelete(randomNodeDup); }
			}

			// 获取当前大小，避免重复调用 RyanJsonGetSize（其复杂度为 O(N)）
			// 在嵌入式场景下应尽量规避 O(N^2) 路径
			if (RyanJsonFuzzerShouldFail(index / 8))
			{
				uint32_t len = RyanJsonGetSize(pJson);
				uint32_t idx = len / 2;

				// 测试中间位置插入
				// 注意：每次插入后，数组长度增加，idx 相对位置其实在变动，
				// 这里为简化逻辑保持 index 不变，覆盖随机位置插入行为
				RyanJsonInsert(pJson, idx, RyanJsonCreateBool(key, 0 != size % 2 ? RyanJsonTrue : RyanJsonFalse));
				RyanJsonInsert(pJson, idx, RyanJsonCreateString(key, "NULL"));
				RyanJsonInsert(pJson, idx, RyanJsonCreateInt(key, 0));
				RyanJsonInsert(pJson, idx, RyanJsonCreateDouble(key, 0));
				RyanJsonInsert(pJson, idx, RyanJsonCreateArray());
				RyanJsonInsert(pJson, idx, RyanJsonCreateObject());

				// 测试头部插入
				RyanJsonInsert(pJson, 0, RyanJsonCreateBool(key, 0 != size % 2 ? RyanJsonTrue : RyanJsonFalse));
				RyanJsonInsert(pJson, 0, RyanJsonCreateString(key, "NULL"));
				RyanJsonInsert(pJson, 0, RyanJsonCreateInt(key, 0));
				RyanJsonInsert(pJson, 0, RyanJsonCreateDouble(key, 0));
				RyanJsonInsert(pJson, 0, RyanJsonCreateArray());
				RyanJsonInsert(pJson, 0, RyanJsonCreateObject());
			}
		}
	}

	return RyanJsonTrue;
}
