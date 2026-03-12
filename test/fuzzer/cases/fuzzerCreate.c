#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

/**
 * @brief 在运行期 fuzz 中主动覆盖 CreateXArray builder 成功路径
 *
 * 这类 API 不再放进 SelfTestOnce，而是在 create 用例里按当前输入触发。
 * 这里只覆盖成功构建路径；OOM 路径仍交给运行期内存故障注入。
 */
static void RyanJsonFuzzerExerciseCreateArrayBuilderCases(uint32_t size)
{
	static const char *const stringPool[] = {"alpha", "beta", "gamma", "delta"};
	int32_t intValues[] = {(int32_t)size, -((int32_t)size + 1)};
	double doubleValues[] = {(double)size * 0.25 + 1.25, -((double)size * 0.5 + 2.5)};
	const char *stringValues[] = {stringPool[size % (sizeof(stringPool) / sizeof(stringPool[0]))],
				      stringPool[(size + 1U) % (sizeof(stringPool) / sizeof(stringPool[0]))]};

	fuzzTestWithMemFail({
		RyanJson_t emptyIntArray = RyanJsonCreateIntArray(intValues, 0);
		RyanJson_t intArray = RyanJsonCreateIntArray(intValues, sizeof(intValues) / sizeof(intValues[0]));
		assert(NULL != emptyIntArray && NULL != intArray);
		assert(0 == RyanJsonGetArraySize(emptyIntArray));
		assert(2 == RyanJsonGetArraySize(intArray));
		assert(intValues[0] == RyanJsonGetIntValue(RyanJsonGetObjectByIndex(intArray, 0)));
		assert(intValues[1] == RyanJsonGetIntValue(RyanJsonGetObjectByIndex(intArray, 1)));
		RyanJsonDelete(emptyIntArray);
		RyanJsonDelete(intArray);

		RyanJson_t emptyDoubleArray = RyanJsonCreateDoubleArray(doubleValues, 0);
		RyanJson_t doubleArray = RyanJsonCreateDoubleArray(doubleValues, sizeof(doubleValues) / sizeof(doubleValues[0]));
		assert(NULL != emptyDoubleArray && NULL != doubleArray);
		assert(0 == RyanJsonGetArraySize(emptyDoubleArray));
		assert(2 == RyanJsonGetArraySize(doubleArray));
		assert(RyanJsonCompareDouble(doubleValues[0], RyanJsonGetDoubleValue(RyanJsonGetObjectByIndex(doubleArray, 0))));
		assert(RyanJsonCompareDouble(doubleValues[1], RyanJsonGetDoubleValue(RyanJsonGetObjectByIndex(doubleArray, 1))));
		RyanJsonDelete(emptyDoubleArray);
		RyanJsonDelete(doubleArray);

		RyanJson_t emptyStringArray = RyanJsonCreateStringArray(stringValues, 0);
		RyanJson_t stringArray = RyanJsonCreateStringArray(stringValues, sizeof(stringValues) / sizeof(stringValues[0]));
		assert(NULL != emptyStringArray && NULL != stringArray);
		assert(0 == RyanJsonGetArraySize(emptyStringArray));
		assert(2 == RyanJsonGetArraySize(stringArray));
		assert(0 == strcmp(stringValues[0], RyanJsonGetStringValue(RyanJsonGetObjectByIndex(stringArray, 0))));
		assert(0 == strcmp(stringValues[1], RyanJsonGetStringValue(RyanJsonGetObjectByIndex(stringArray, 1))));
		RyanJsonDelete(emptyStringArray);
		RyanJsonDelete(stringArray);
	});
}

/**
 * @brief 补齐“非空容器包装后挂树”路径
 *
 * 运行期 generator 只会生成空 object/array，无法自然到达 RyanJsonCreateItem
 * 中 children!=NULL 的分支。
 */
static void RyanJsonFuzzerSelfTestCreateWrappedContainerCases(void)
{
	RyanJson_t parent = RyanJsonCreateObject();
	RyanJson_t childArray = RyanJsonCreateArray();
	assert(NULL != parent && NULL != childArray);
	assert(RyanJsonTrue == RyanJsonAddIntToArray(childArray, 1));
	assert(RyanJsonTrue == RyanJsonAddIntToArray(childArray, 2));
	assert(RyanJsonTrue == RyanJsonAddItemToObject(parent, "arr", childArray));
	RyanJson_t wrappedArray = RyanJsonGetObjectByKey(parent, "arr");
	assert(NULL != wrappedArray && RyanJsonTrue == RyanJsonIsArray(wrappedArray));
	assert(2 == RyanJsonGetArraySize(wrappedArray));
	assert(1 == RyanJsonGetIntValue(RyanJsonGetObjectByIndex(wrappedArray, 0)));
	assert(2 == RyanJsonGetIntValue(RyanJsonGetObjectByIndex(wrappedArray, 1)));
	RyanJsonDelete(parent);

	parent = RyanJsonCreateObject();
	RyanJson_t childObject = RyanJsonCreateObject();
	assert(NULL != parent && NULL != childObject);
	assert(RyanJsonTrue == RyanJsonAddIntToObject(childObject, "a", 1));
	assert(RyanJsonTrue == RyanJsonAddIntToObject(childObject, "b", 2));
	assert(RyanJsonTrue == RyanJsonAddItemToObject(parent, "obj", childObject));
	RyanJson_t wrappedObject = RyanJsonGetObjectByKey(parent, "obj");
	assert(NULL != wrappedObject && RyanJsonTrue == RyanJsonIsObject(wrappedObject));
	assert(2 == RyanJsonGetSize(wrappedObject));
	assert(1 == RyanJsonGetIntValue(RyanJsonGetObjectByKey(wrappedObject, "a")));
	assert(2 == RyanJsonGetIntValue(RyanJsonGetObjectByKey(wrappedObject, "b")));
	RyanJsonDelete(parent);
}

/**
 * @brief create 模块的一次性确定性自检
 *
 * 这里只保留运行期 fuzz 无法主动构造的非法内部状态和非空容器包装路径。
 */
void RyanJsonFuzzerSelfTestCreateCases(void)
{
	RyanJsonBool_e lastIsEnableMemFail;
	RyanJsonFuzzerMemFailPush(lastIsEnableMemFail, RyanJsonFalse);

	RyanJson_t malformedDetached = RyanJsonCreateInt(NULL, 1);
	assert(NULL != malformedDetached);
	assert(RyanJsonTrue == RyanJsonIsDetachedItem(malformedDetached));
	RyanJsonSetPayloadIsLastByFlag(malformedDetached, RyanJsonTrue);
	assert(RyanJsonFalse == RyanJsonIsDetachedItem(malformedDetached));
	RyanJsonSetPayloadIsLastByFlag(malformedDetached, RyanJsonFalse);
	RyanJsonDelete(malformedDetached);

	RyanJsonFuzzerSelfTestCreateWrappedContainerCases();

	RyanJsonFuzzerMemFailPop(lastIsEnableMemFail);
}

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
	static RyanJsonBool_e duplicateKeyGuardCovered = RyanJsonFalse;

	// duplicate key 守卫与当前输入无关，只需确定性命中一次，避免每轮重复造对象。
	if (RyanJsonFalse == duplicateKeyGuardCovered)
	{
		RyanJsonBool_e lastIsEnableMemFail;
		RyanJsonFuzzerMemFailPush(lastIsEnableMemFail, RyanJsonFalse);

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

		RyanJsonFuzzerMemFailPop(lastIsEnableMemFail);
		duplicateKeyGuardCovered = RyanJsonTrue;
	}

	// RyanJsonInsert 特殊路径：模拟内存分配失败或无效参数输入
	uint32_t index = 6;

	// 仅在根节点触发 builder 成功路径，避免递归遍历整棵树时指数级放大成本。
	if (NULL == RyanJsonInternalGetParent(pJson) && RyanJsonFuzzerShouldFail(100))
	{
		RyanJsonFuzzerExerciseCreateArrayBuilderCases(size);
	}

	// 这批合同检查覆盖面广，但与当前输入关联较弱，因此降频执行，控制热路径成本。
	if (RyanJsonFuzzerShouldFail(100))
	{
		// 第一层：纯参数守卫和类型守卫。
		// 这些断言的共同点是“即使不依赖当前输入的具体值，也能验证接口合同是否稳固”。
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
			RyanJsonBool_e lastIsEnableMemFail;
			RyanJsonFuzzerMemFailPush(lastIsEnableMemFail, RyanJsonFalse);
			assert(RyanJsonFalse == RyanJsonChangeKey(pJson, "NULL"));
			RyanJsonFuzzerMemFailPop(lastIsEnableMemFail);
		}

		// 测试“无 key 但有 strValue”的分支
		if (RyanJsonFalse == RyanJsonIsKey(pJson) && RyanJsonTrue == RyanJsonIsString(pJson))
		{
			RyanJsonBool_e lastIsEnableMemFail;
			RyanJsonFuzzerMemFailPush(lastIsEnableMemFail, RyanJsonFalse);
			assert(RyanJsonFalse == RyanJsonChangeKey(pJson, "NULL"));
			RyanJsonFuzzerMemFailPop(lastIsEnableMemFail);
		}

		assert(RyanJsonFalse == RyanJsonChangeIntValue(NULL, 0));
		assert(RyanJsonFalse == RyanJsonChangeDoubleValue(NULL, 0));
		assert(RyanJsonFalse == RyanJsonChangeBoolValue(NULL, 0));

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

		// 第二层：已挂树 item 的复用/二次插入守卫。
		// 这类路径在随机 generator 下不够稳定，集中在这里以确定性方式补齐。
		{
			RyanJsonBool_e lastIsEnableMemFail;
			RyanJsonFuzzerMemFailPush(lastIsEnableMemFail, RyanJsonFalse);

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

			RyanJsonFuzzerMemFailPop(lastIsEnableMemFail);
		}
	}

	char *key = "keyaaa";
	// 对容器这是正常 append；对标量则是刻意保留的误用合同覆盖。
	RyanJsonAddNullToObject(pJson, key);

	// 如果当前节点是 key 类型，尝试获取其 key 字符串作为后续操作 key
	if (RyanJsonTrue == RyanJsonIsKey(pJson))
	{
		// Change 系列测试已在 modify 用例覆盖
		key = RyanJsonGetKey(pJson);
	}

	if (RyanJsonTrue == RyanJsonIsBool(pJson) && RyanJsonFuzzerShouldFail(index))
	{
		// 标量追加测试仍保留在运行期：这里需要当前节点值参与断言，不能挪到 SelfTestOnce。
		if (RyanJsonTrue == RyanJsonAddBoolToObject(pJson, key, RyanJsonGetBoolValue(pJson)))
		{
			fuzzTestWithMemFail(
				assert(RyanJsonGetBoolValue(RyanJsonGetObjectByKey(pJson, key)) == RyanJsonGetBoolValue(pJson)));
		}
	}

	if (RyanJsonTrue == RyanJsonIsNumber(pJson) && RyanJsonFuzzerShouldFail(index))
	{
		// number 路径既覆盖 AddInt/AddDouble，也顺带覆盖 typed-array builder 的运行期成功路径。
		// 这正是“数学上可由 fuzz 命中，所以不应塞进 self-test”的典型例子。
		if (RyanJsonTrue == RyanJsonIsInt(pJson))
		{
			if (RyanJsonTrue == RyanJsonAddIntToObject(pJson, key, RyanJsonGetIntValue(pJson)))
			{
				fuzzTestWithMemFail(
					assert(RyanJsonGetIntValue(RyanJsonGetObjectByKey(pJson, key)) == RyanJsonGetIntValue(pJson)));
			}

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
		// string 路径与数值路径保持相同结构，方便后续维护时按类型成组审阅覆盖缺口。
		if (RyanJsonTrue == RyanJsonAddStringToObject(pJson, key, RyanJsonGetStringValue(pJson)))
		{
			fuzzTestWithMemFail(assert(
				0 == strcmp(RyanJsonGetStringValue(RyanJsonGetObjectByKey(pJson, key)), RyanJsonGetStringValue(pJson))));
		}

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
		// 先递归已有子节点，再追加新节点，避免本轮新增节点在同一轮里被继续放大递归成本。
		// 递归处理子节点
		if (RyanJsonTrue == RyanJsonIsArray(pJson))
		{
			RyanJsonArrayForEach(pJson, item)
			{
				RyanJsonFuzzerTestCreate(item, size);
			}
		}
		else
		{
			// 对 object 必须保留 key 语义，不能偷懒统一走 ArrayForEach。
			RyanJsonObjectForEach(pJson, item)
			{
				RyanJsonFuzzerTestCreate(item, size);
			}
		}

		// 添加随机生成的节点 (AddItem 方案A: 仅允许 Array/Object，成功后会消费原 item)
		uint32_t oldSize = RyanJsonGetSize(pJson);
		RyanJson_t newItem = RyanJsonFuzzerCreateRandomNode(pJson);
		RyanJson_t newItemDup = NULL;
		// 只给容器节点做 duplicate 快照，避免为标量白白增加复制和删除成本。
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
			// 以下分支只在 array 下执行，集中覆盖数组专属 append/insert 语义。
			// object 的 key 语义更强，相关路径已在前面的 Add*ToObject / AddItemToObject 中单独验证。
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
