#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

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
	// 一次性覆盖对象 ReplaceByIndex 重复 key 防御分支
	static RyanJsonBool_e replaceConflictCovered = RyanJsonFalse;
	if (RyanJsonFalse == replaceConflictCovered)
	{
		RyanJsonBool_e lastIsEnableMemFail = g_fuzzerState.isEnableMemFail;
		g_fuzzerState.isEnableMemFail = false;

		RyanJson_t obj = RyanJsonCreateObject();
		assert(NULL != obj);
		assert(RyanJsonTrue == RyanJsonAddIntToObject(obj, "a", 1));
		assert(RyanJsonTrue == RyanJsonAddIntToObject(obj, "b", 2));

		RyanJson_t newItem = RyanJsonCreateInt("a", 9);
		assert(NULL != newItem);
#if true == RyanJsonDefaultAddAtHead
		uint32_t replaceIndex = 0;
#else
		uint32_t replaceIndex = 1;
#endif

#if true == RyanJsonStrictObjectKeyCheck
		assert(RyanJsonFalse == RyanJsonReplaceByIndex(obj, replaceIndex, newItem));
		RyanJsonDelete(newItem);
		assert(2 == RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "b")));
#else
		assert(RyanJsonTrue == RyanJsonReplaceByIndex(obj, replaceIndex, newItem));
		assert(NULL == RyanJsonGetObjectByKey(obj, "b"));
#if true == RyanJsonDefaultAddAtHead
		assert(9 == RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "a")));
#else
		assert(1 == RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "a")));
#endif
#endif

		// 同 key 替换应成功（覆盖冲突检测的 skipItem 分支）
		newItem = RyanJsonCreateInt("b", 99);
		assert(NULL != newItem);
		assert(RyanJsonTrue == RyanJsonReplaceByIndex(obj, replaceIndex, newItem));
		assert(99 == RyanJsonGetIntValue(RyanJsonGetObjectByKey(obj, "b")));

		RyanJsonDelete(obj);
		g_fuzzerState.isEnableMemFail = lastIsEnableMemFail;
		replaceConflictCovered = RyanJsonTrue;
	}

	// 一次性覆盖“通过 Replace 修改 value 类型”的推荐用法
	static RyanJsonBool_e replaceTypeSwitchCovered = RyanJsonFalse;
	if (RyanJsonFalse == replaceTypeSwitchCovered)
	{
		RyanJsonBool_e lastIsEnableMemFail = g_fuzzerState.isEnableMemFail;
		g_fuzzerState.isEnableMemFail = false;

		RyanJson_t obj = RyanJsonCreateObject();
		assert(NULL != obj);
		assert(RyanJsonTrue == RyanJsonAddIntToObject(obj, "k", 1));

		assert(RyanJsonTrue == RyanJsonReplaceByKey(obj, "k", RyanJsonCreateObject()));
		RyanJson_t item = RyanJsonGetObjectByKey(obj, "k");
		assert(NULL != item && RyanJsonTrue == RyanJsonIsObject(item));
		assert(RyanJsonTrue == RyanJsonAddIntToObject(item, "x", 7));

		assert(RyanJsonTrue == RyanJsonReplaceByKey(obj, "k", RyanJsonCreateArray()));
		item = RyanJsonGetObjectByKey(obj, "k");
		assert(NULL != item && RyanJsonTrue == RyanJsonIsArray(item));

		RyanJson_t arr = RyanJsonCreateArray();
		assert(NULL != arr);
		assert(RyanJsonTrue == RyanJsonAddIntToArray(arr, 1));
		assert(RyanJsonTrue == RyanJsonReplaceByIndex(arr, 0, RyanJsonCreateObject()));
		item = RyanJsonGetObjectByIndex(arr, 0);
		assert(NULL != item && RyanJsonTrue == RyanJsonIsObject(item));

		RyanJsonDelete(arr);
		RyanJsonDelete(obj);

		g_fuzzerState.isEnableMemFail = lastIsEnableMemFail;
		replaceTypeSwitchCovered = RyanJsonTrue;
	}

	// 故障注入与异常参数测试
	if (RyanJsonFuzzerShouldFail(100))
	{
		g_fuzzerState.isEnableMemFail = false; // 临时禁用内存失败模拟，确保测试对象创建成功

		RyanJson_t strItem = RyanJsonCreateString("", "NULL");

		// key 替换异常测试
		assert(RyanJsonFalse == RyanJsonReplaceByKey(NULL, NULL, NULL));
		assert(RyanJsonFalse == RyanJsonReplaceByKey(pJson, NULL, NULL));
		assert(RyanJsonFalse == RyanJsonReplaceByKey(NULL, "NULL", NULL));
		assert(RyanJsonFalse == RyanJsonReplaceByKey(NULL, NULL, strItem));
		assert(RyanJsonFalse == RyanJsonReplaceByKey(pJson, "NULL", NULL));
		assert(RyanJsonFalse == RyanJsonReplaceByKey(NULL, "NULL", strItem));

		// 类型错误测试：非对象调用 ReplaceByKey
		if (RyanJsonFalse == RyanJsonIsObject(RyanJsonGetObjectByIndex(pJson, 0)))
		{
			// 如果意外成功（说明 pJson 其实是对象且碰巧存在该 key），
			// 需要把 strItem 取回，避免后续 RyanJsonDelete(strItem) 触发重复释放。
			if (RyanJsonTrue == RyanJsonReplaceByKey(pJson, "NULL", strItem)) { strItem = RyanJsonDetachByKey(pJson, "NULL"); }
		}

		// index 替换异常测试
		assert(RyanJsonFalse == RyanJsonReplaceByIndex(NULL, 0, NULL));
		assert(RyanJsonFalse == RyanJsonReplaceByIndex(pJson, 0, NULL));
		assert(RyanJsonFalse == RyanJsonReplaceByIndex(NULL, 0, strItem));
		assert(RyanJsonFalse == RyanJsonReplaceByIndex(pJson, 0, NULL));
		assert(RyanJsonFalse == RyanJsonReplaceByIndex(NULL, 0, strItem));

		// 类型错误测试：非容器调用 ReplaceByIndex
		if (RyanJsonFalse == RyanJsonIsArray(pJson) && RyanJsonFalse == RyanJsonIsObject(pJson))
		{
			assert(RyanJsonFalse == RyanJsonReplaceByIndex(pJson, 0, strItem));
		}

		// 构造临时对象，测试不存在 key 和越界 index
		RyanJson_t objItem = RyanJsonCreateObject();
		assert(RyanJsonFalse == RyanJsonReplaceByKey(objItem, "NULL", strItem));
		assert(RyanJsonFalse == RyanJsonReplaceByIndex(objItem, 0, strItem));

		RyanJsonAddItemToObject(objItem, "item", RyanJsonCreateObject());
		assert(RyanJsonFalse == RyanJsonReplaceByKey(objItem, "NULL222", strItem));
		assert(RyanJsonFalse == RyanJsonReplaceByIndex(objItem, INT32_MAX, strItem));

		// Replace 失败后，item 仍应保持游离态（由调用方继续持有）
		{
			RyanJson_t keepObjItem = RyanJsonCreateInt("keep", 1);
			assert(NULL != keepObjItem);
			assert(RyanJsonTrue == RyanJsonIsDetachedItem(keepObjItem));
			assert(RyanJsonFalse == RyanJsonReplaceByKey(objItem, "not_found", keepObjItem));
			assert(RyanJsonTrue == RyanJsonIsDetachedItem(keepObjItem));
			RyanJsonDelete(keepObjItem);

			RyanJson_t arrayItem = RyanJsonCreateArray();
			assert(NULL != arrayItem);
			assert(RyanJsonTrue == RyanJsonAddIntToArray(arrayItem, 9));

			RyanJson_t keepArrItem = RyanJsonCreateString(NULL, "keep");
			assert(NULL != keepArrItem);
			assert(RyanJsonTrue == RyanJsonIsDetachedItem(keepArrItem));
			assert(RyanJsonFalse == RyanJsonReplaceByIndex(arrayItem, 7, keepArrItem));
			assert(RyanJsonTrue == RyanJsonIsDetachedItem(keepArrItem));

			RyanJsonDelete(keepArrItem);
			RyanJsonDelete(arrayItem);
		}

		// 已挂树的 item 不应被 Replace
		{
			RyanJson_t objA = RyanJsonCreateObject();
			RyanJson_t objB = RyanJsonCreateObject();
			RyanJsonAddIntToObject(objA, "a", 1);
			RyanJsonAddIntToObject(objB, "b", 2);

			RyanJson_t attachedObjItem = RyanJsonGetObjectByKey(objA, "a");
			assert(attachedObjItem);
			assert(RyanJsonFalse == RyanJsonIsDetachedItem(attachedObjItem));
			assert(RyanJsonFalse == RyanJsonReplaceByKey(objB, "b", attachedObjItem));
			assert(2 == RyanJsonGetIntValue(RyanJsonGetObjectByKey(objB, "b")));

			RyanJson_t arr1 = RyanJsonCreateArray();
			RyanJson_t arr2 = RyanJsonCreateArray();
			RyanJsonAddIntToArray(arr1, 10);
			RyanJsonAddIntToArray(arr2, 20);

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

		g_fuzzerState.isEnableMemFail = true; // 恢复状态

		RyanJsonDelete(objItem);
		RyanJsonDelete(strItem);
	}

	// 递归遍历与替换
	// 仅处理容器类型（Object/Array）
	if (RyanJsonFalse == RyanJsonIsArray(pJson) && RyanJsonFalse == RyanJsonIsObject(pJson)) { return RyanJsonTrue; }

	RyanJson_t item = NULL;
	RyanJsonObjectForEach(pJson, item)
	{
		// 递归调用
		if (RyanJsonTrue != RyanJsonFuzzerTestReplace(item, size)) { return RyanJsonFalse; }
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
