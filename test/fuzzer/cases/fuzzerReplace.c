#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

/**
 * @brief replace 模块的一次性确定性自检
 *
 * 这里只保留运行期 fuzz 不会主动构造的失败合同，
 * 包括 attached item 与非法插入组合。
 */
void RyanJsonFuzzerSelfTestReplaceCases(void)
{
	RyanJsonBool_e lastIsEnableMemFail;
	RyanJsonFuzzerMemFailPush(lastIsEnableMemFail, RyanJsonFalse);

	RyanJson_t scalar = RyanJsonCreateInt(NULL, 1);
	RyanJson_t strItem = RyanJsonCreateString("", "NULL");
	assert(NULL != scalar && NULL != strItem);

	assert(RyanJsonFalse == RyanJsonIsDetachedItem(NULL));
	assert(RyanJsonFalse == RyanJsonReplaceByKey(NULL, NULL, NULL));
	assert(RyanJsonFalse == RyanJsonReplaceByKey(scalar, NULL, NULL));
	assert(RyanJsonFalse == RyanJsonReplaceByKey(NULL, "NULL", NULL));
	assert(RyanJsonFalse == RyanJsonReplaceByKey(NULL, NULL, strItem));
	assert(RyanJsonFalse == RyanJsonReplaceByKey(scalar, "NULL", NULL));
	assert(RyanJsonFalse == RyanJsonReplaceByKey(NULL, "NULL", strItem));
	assert(RyanJsonFalse == RyanJsonReplaceByKey(scalar, "NULL", strItem));

	assert(RyanJsonFalse == RyanJsonReplaceByIndex(NULL, 0, NULL));
	assert(RyanJsonFalse == RyanJsonReplaceByIndex(scalar, 0, NULL));
	assert(RyanJsonFalse == RyanJsonReplaceByIndex(NULL, 0, strItem));
	assert(RyanJsonFalse == RyanJsonReplaceByIndex(scalar, 0, strItem));

	{
		RyanJson_t objItem = RyanJsonCreateObject();
		assert(NULL != objItem);
		assert(RyanJsonFalse == RyanJsonReplaceByKey(objItem, "NULL", strItem));
		assert(RyanJsonFalse == RyanJsonReplaceByIndex(objItem, 0, strItem));

		assert(RyanJsonTrue == RyanJsonAddItemToObject(objItem, "item", RyanJsonCreateObject()));
		assert(RyanJsonFalse == RyanJsonReplaceByKey(objItem, "NULL222", strItem));
		assert(RyanJsonFalse == RyanJsonReplaceByIndex(objItem, INT32_MAX, strItem));

		RyanJson_t keepObjItem = RyanJsonCreateInt("keep", 1);
		assert(NULL != keepObjItem);
		assert(RyanJsonTrue == RyanJsonIsDetachedItem(keepObjItem));
		assert(RyanJsonFalse == RyanJsonReplaceByKey(objItem, "not_found", keepObjItem));
		assert(RyanJsonTrue == RyanJsonIsDetachedItem(keepObjItem));
		RyanJsonDelete(keepObjItem);

		RyanJsonDelete(objItem);
	}

	{
		RyanJson_t arrayItem = RyanJsonCreateArray();
		RyanJson_t keepArrItem = RyanJsonCreateString(NULL, "keep");
		assert(NULL != arrayItem && NULL != keepArrItem);
		assert(RyanJsonTrue == RyanJsonAddIntToArray(arrayItem, 9));
		assert(RyanJsonTrue == RyanJsonIsDetachedItem(keepArrItem));
		assert(RyanJsonFalse == RyanJsonReplaceByIndex(arrayItem, 7, keepArrItem));
		assert(RyanJsonTrue == RyanJsonIsDetachedItem(keepArrItem));
		RyanJsonDelete(keepArrItem);
		RyanJsonDelete(arrayItem);
	}

	{
		RyanJson_t objA = RyanJsonCreateObject();
		RyanJson_t objB = RyanJsonCreateObject();
		assert(NULL != objA && NULL != objB);
		assert(RyanJsonTrue == RyanJsonAddIntToObject(objA, "a", 1));
		assert(RyanJsonTrue == RyanJsonAddIntToObject(objB, "b", 2));

		RyanJson_t attachedObjItem = RyanJsonGetObjectByKey(objA, "a");
		assert(attachedObjItem);
		assert(RyanJsonFalse == RyanJsonIsDetachedItem(attachedObjItem));
		assert(RyanJsonFalse == RyanJsonReplaceByKey(objB, "b", attachedObjItem));
		assert(2 == RyanJsonGetIntValue(RyanJsonGetObjectByKey(objB, "b")));

		RyanJson_t arr1 = RyanJsonCreateArray();
		RyanJson_t arr2 = RyanJsonCreateArray();
		assert(NULL != arr1 && NULL != arr2);
		assert(RyanJsonTrue == RyanJsonAddIntToArray(arr1, 10));
		assert(RyanJsonTrue == RyanJsonAddIntToArray(arr2, 20));

		RyanJson_t attachedArrItem = RyanJsonGetObjectByIndex(arr1, 0);
		assert(attachedArrItem);
		assert(RyanJsonFalse == RyanJsonIsDetachedItem(attachedArrItem));
		assert(RyanJsonFalse == RyanJsonReplaceByIndex(arr2, 0, attachedArrItem));
		assert(20 == RyanJsonGetIntValue(RyanJsonGetObjectByIndex(arr2, 0)));

		RyanJsonDelete(objA);
		RyanJsonDelete(objB);
		RyanJsonDelete(arr1);
		RyanJsonDelete(arr2);
	}

	{
		RyanJson_t ownerObj = RyanJsonCreateObject();
		RyanJson_t dstObj = RyanJsonCreateObject();
		assert(NULL != ownerObj && NULL != dstObj);
		assert(RyanJsonTrue == RyanJsonAddIntToObject(ownerObj, "src", 7));

		RyanJson_t attachedObjItem = RyanJsonGetObjectByKey(ownerObj, "src");
		assert(NULL != attachedObjItem);
		assert(RyanJsonFalse == RyanJsonIsDetachedItem(attachedObjItem));
		assert(RyanJsonFalse == RyanJsonAddItemToObject(dstObj, "dst", attachedObjItem));
		assert(RyanJsonFalse == RyanJsonIsDetachedItem(attachedObjItem));

		RyanJsonDelete(ownerObj);
		RyanJsonDelete(dstObj);
	}

	{
		RyanJson_t ownerArray = RyanJsonCreateArray();
		RyanJson_t dstArray = RyanJsonCreateArray();
		assert(NULL != ownerArray && NULL != dstArray);
		assert(RyanJsonTrue == RyanJsonAddIntToArray(ownerArray, 9));

		RyanJson_t attachedArrayItem = RyanJsonGetObjectByIndex(ownerArray, 0);
		assert(NULL != attachedArrayItem);
		assert(RyanJsonFalse == RyanJsonIsDetachedItem(attachedArrayItem));
		assert(RyanJsonFalse == RyanJsonInsert(dstArray, 0, attachedArrayItem));
		assert(RyanJsonFalse == RyanJsonIsDetachedItem(attachedArrayItem));

		RyanJsonDelete(ownerArray);
		RyanJsonDelete(dstArray);
	}

	{
		RyanJson_t objectParent = RyanJsonCreateObject();
		RyanJson_t invalidScalarItem = RyanJsonCreateString(NULL, "invalid");
		assert(NULL != objectParent && NULL != invalidScalarItem);
		assert(RyanJsonTrue == RyanJsonIsDetachedItem(invalidScalarItem));
		assert(RyanJsonFalse == RyanJsonInsert(objectParent, 0, invalidScalarItem));
		RyanJsonDelete(objectParent);
	}

	RyanJsonDelete(strItem);
	RyanJsonDelete(scalar);
	RyanJsonFuzzerMemFailPop(lastIsEnableMemFail);
}

/**
 * @brief 节点替换测试
 *
 * 测试 RyanJson 的节点替换功能（ReplaceByKey、ReplaceByIndex）。
 * 覆盖场景：
 * 异常替换：测试无效参数、类型不匹配、越界索引等错误。
 * 递归替换：遍历 Json 树，随机替换子节点为新生成的随机节点。
 * 内存管理：确保替换操作后，被替换的节点被正确释放，新节点被正确挂载。
 *
 * @param state Fuzzer 状态上下文
 * @param pJson 当前正在操作的 Json 节点
 * @param size 输入数据大小
 */
RyanJsonBool_e RyanJsonFuzzerTestReplace(RyanJson_t pJson, uint32_t size)
{

	// 递归遍历与替换
	// 仅处理容器类型（Object/Array）
	if (RyanJsonFalse == RyanJsonIsArray(pJson) && RyanJsonFalse == RyanJsonIsObject(pJson)) { return RyanJsonTrue; }

	RyanJson_t item = NULL;
	if (RyanJsonTrue == RyanJsonIsArray(pJson))
	{
		RyanJsonArrayForEach(pJson, item)
		{
			// 递归调用
			if (RyanJsonTrue != RyanJsonFuzzerTestReplace(item, size)) { return RyanJsonFalse; }
		}
	}
	else
	{
		RyanJsonObjectForEach(pJson, item)
		{
			// 递归调用
			if (RyanJsonTrue != RyanJsonFuzzerTestReplace(item, size)) { return RyanJsonFalse; }
		}
	}

	// 按 key 替换（仅 Object）
	do
	{
		if (RyanJsonFalse == RyanJsonIsObject(pJson)) { break; }

		// 随机尝试替换 0 / 中间 / 结尾 子节点
		uint32_t jsonSize = RyanJsonGetSize(pJson);
		if (0 == jsonSize) { break; }

		uint32_t index = RyanJsonFuzzerNextRand() % jsonSize;
		RyanJson_t targetItem = RyanJsonGetObjectByIndex(pJson, index);
		if (NULL == targetItem || RyanJsonTrue != RyanJsonIsKey(targetItem)) { break; }

		RyanJson_t newItem = NULL;
		uint32_t choice = RyanJsonFuzzerNextRand() % 3;

		// 策略一：使用相同 key（覆盖 key 相等分支）
		if (0 == choice) { newItem = RyanJsonFuzzerCreateRandomNodeWithKey(pJson, RyanJsonGetKey(targetItem)); }
		// 策略二：使用不同 key（覆盖 key 不相等分支，触发 ChangeKey）
		else if (1 == choice) { newItem = RyanJsonFuzzerCreateRandomNodeWithKey(pJson, "diff_key_random"); }
		// 策略三：不带 key（覆盖无 key 分支，触发 CreateItem）
		else
		{
			newItem = RyanJsonFuzzerCreateRandomNode(pJson);
		}

		// 尝试替换
		if (RyanJsonFalse == RyanJsonReplaceByKey(pJson, RyanJsonGetKey(targetItem), newItem))
		{
			// 替换失败（可能是内存模拟失败），需手动释放 newItem 防止泄漏
			if (NULL != newItem) { RyanJsonDelete(newItem); }
		}
	} while (0);

	// 按 index 替换（Array/Object）
	{
		uint32_t idx = 0;
		uint32_t jsonSize = RyanJsonGetSize(pJson);
		if (0 != jsonSize) { idx = size % jsonSize; } // 随机选择一个有效 index

		RyanJson_t newItem = RyanJsonFuzzerCreateRandomNode(pJson);

		// 执行替换
		if (RyanJsonFalse == RyanJsonReplaceByIndex(pJson, idx, newItem))
		{
			// 替换失败，手动释放
			if (NULL != newItem) { RyanJsonDelete(newItem); }
		}
	}

	return RyanJsonTrue;
}
