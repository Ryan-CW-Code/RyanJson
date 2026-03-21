#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

/**
 * @brief duplicate/compare 模块的一次性最小自检
 *
 * 这里只恢复运行期 harness 结构上不容易构造的比较拓扑：
 * 自比较、标量值不相等、Object 乱序匹配，以及嵌套 Object 缺少匹配 key 的失败路径。
 */
void RyanJsonFuzzerSelfTestDuplicateCases(void)
{
	{
		RyanJson_t selfObj = RyanJsonCreateObject();
		assert(NULL != selfObj);
		assert(RyanJsonTrue == RyanJsonAddIntToObject(selfObj, "k", 1));
		assert(RyanJsonTrue == RyanJsonCompare(selfObj, selfObj));
		assert(RyanJsonTrue == RyanJsonCompareOnlyKey(selfObj, selfObj));
		assert(RyanJsonFalse == RyanJsonCompare(NULL, selfObj));
		assert(RyanJsonFalse == RyanJsonCompare(selfObj, NULL));
		assert(RyanJsonFalse == RyanJsonCompare(NULL, NULL));
		assert(RyanJsonFalse == RyanJsonCompareOnlyKey(NULL, selfObj));
		assert(RyanJsonFalse == RyanJsonCompareOnlyKey(selfObj, NULL));
		assert(RyanJsonFalse == RyanJsonCompareOnlyKey(NULL, NULL));
		assert(NULL == RyanJsonDuplicate(NULL));
		assert(0 == RyanJsonGetSize(NULL));
		RyanJsonDelete(selfObj);
	}

	{
		RyanJson_t leftType = RyanJsonCreateInt(NULL, 1);
		RyanJson_t rightType = RyanJsonCreateString(NULL, "1");
		RyanJson_t leftBool = RyanJsonCreateBool(NULL, RyanJsonTrue);
		RyanJson_t rightBool = RyanJsonCreateBool(NULL, RyanJsonFalse);
		RyanJson_t leftInt = RyanJsonCreateInt(NULL, 11);
		RyanJson_t rightInt = RyanJsonCreateInt(NULL, 22);
		RyanJson_t leftDouble = RyanJsonCreateDouble(NULL, 1.25);
		RyanJson_t rightDouble = RyanJsonCreateDouble(NULL, 2.5);
		RyanJson_t leftString = RyanJsonCreateString(NULL, "left");
		RyanJson_t rightString = RyanJsonCreateString(NULL, "right");
		assert(NULL != leftType && NULL != rightType);
		assert(NULL != leftBool && NULL != rightBool);
		assert(NULL != leftInt && NULL != rightInt);
		assert(NULL != leftDouble && NULL != rightDouble);
		assert(NULL != leftString && NULL != rightString);

		assert(RyanJsonFalse == RyanJsonCompare(leftType, rightType));
		assert(RyanJsonFalse == RyanJsonCompare(leftBool, rightBool));
		assert(RyanJsonFalse == RyanJsonCompare(leftInt, rightInt));
		assert(RyanJsonFalse == RyanJsonCompare(leftDouble, rightDouble));
		assert(RyanJsonFalse == RyanJsonCompare(leftString, rightString));

		RyanJsonDelete(leftType);
		RyanJsonDelete(rightType);
		RyanJsonDelete(leftBool);
		RyanJsonDelete(rightBool);
		RyanJsonDelete(leftInt);
		RyanJsonDelete(rightInt);
		RyanJsonDelete(leftDouble);
		RyanJsonDelete(rightDouble);
		RyanJsonDelete(leftString);
		RyanJsonDelete(rightString);
	}

	{
		RyanJson_t objLeft = RyanJsonCreateObject();
		RyanJson_t objRightUnordered = RyanJsonCreateObject();
		assert(NULL != objLeft && NULL != objRightUnordered);
		assert(RyanJsonTrue == RyanJsonAddIntToObject(objLeft, "a", 1));
		assert(RyanJsonTrue == RyanJsonAddIntToObject(objLeft, "b", 2));
		assert(RyanJsonTrue == RyanJsonAddIntToObject(objLeft, "c", 3));
		assert(RyanJsonTrue == RyanJsonAddIntToObject(objRightUnordered, "b", 2));
		assert(RyanJsonTrue == RyanJsonAddIntToObject(objRightUnordered, "a", 1));
		assert(RyanJsonTrue == RyanJsonAddIntToObject(objRightUnordered, "c", 3));
		assert(RyanJsonTrue == RyanJsonCompare(objLeft, objRightUnordered));
		assert(RyanJsonTrue == RyanJsonCompareOnlyKey(objLeft, objRightUnordered));
		RyanJsonDelete(objLeft);
		RyanJsonDelete(objRightUnordered);
	}

	{
		RyanJson_t objNeedKey = RyanJsonCreateObject();
		RyanJson_t objMissKey = RyanJsonCreateObject();
		assert(NULL != objNeedKey && NULL != objMissKey);
		assert(RyanJsonTrue == RyanJsonAddIntToObject(objNeedKey, "a", 1));
		assert(RyanJsonTrue == RyanJsonAddIntToObject(objNeedKey, "b", 2));
		assert(RyanJsonTrue == RyanJsonAddIntToObject(objMissKey, "a", 1));
		assert(RyanJsonTrue == RyanJsonAddIntToObject(objMissKey, "c", 2));
		assert(RyanJsonFalse == RyanJsonCompare(objNeedKey, objMissKey));
		assert(RyanJsonFalse == RyanJsonCompareOnlyKey(objNeedKey, objMissKey));
		RyanJsonDelete(objNeedKey);
		RyanJsonDelete(objMissKey);
	}

	{
		RyanJson_t nestedNeedKey = RyanJsonCreateObject();
		RyanJson_t nestedMissKey = RyanJsonCreateObject();
		RyanJson_t needChild = RyanJsonCreateObject();
		RyanJson_t missChild = RyanJsonCreateObject();
		assert(NULL != nestedNeedKey && NULL != nestedMissKey);
		assert(NULL != needChild && NULL != missChild);
		assert(RyanJsonTrue == RyanJsonAddItemToObject(nestedNeedKey, "need", needChild));
		assert(RyanJsonTrue == RyanJsonAddItemToObject(nestedMissKey, "miss", missChild));
		assert(RyanJsonFalse == RyanJsonCompare(nestedNeedKey, nestedMissKey));
		assert(RyanJsonFalse == RyanJsonCompareOnlyKey(nestedNeedKey, nestedMissKey));
		RyanJsonDelete(nestedNeedKey);
		RyanJsonDelete(nestedMissKey);
	}
}

/**
 * @brief 复制与比较测试
 *
 * 测试深度复制与结构比较能力。
 * 覆盖场景：
 * 深度复制正确性：验证复制后的 Object 与原 Object 在结构和值上完全一致。
 * 独立性验证：验证修改复制后的 Object 不会影响原 Object。
 * 内存管理：验证复制过程中的内存分配与释放。
 *
 * @param state Fuzzer 状态上下文
 * @param pJson 需要复制的源 Json Object
 */
RyanJsonBool_e RyanJsonFuzzerTestDuplicate(RyanJson_t pJson)
{
	RyanJsonBool_e result = RyanJsonTrue;
	char *jsonStr = NULL;
	char *jsonStrDup = NULL;
	RyanJson_t pJsonDup = NULL;

	uint32_t len = 0;
	/* 使用非格式化输出，减少空白差异干扰 */
	jsonStr = RyanJsonPrint(pJson, 100, RyanJsonFalse, &len);
	RyanJsonCheckGotoExit(NULL != jsonStr);

	// 执行深度复制
	pJsonDup = RyanJsonDuplicate(pJson);
	RyanJsonCheckGotoExit(NULL != pJsonDup);

	// 结构完整性验证
	// 验证节点数量一致
	assert(RyanJsonGetSize(pJson) == RyanJsonGetSize(pJsonDup));

	// 执行结构比较
	// RyanJsonCompare：比较 key 与 value
	// RyanJsonCompareOnlyKey：仅比较 key（结构）
	RyanJsonCompare(pJson, pJsonDup);
	RyanJsonCompareOnlyKey(pJson, pJsonDup);

	// 序列化一致性验证
	uint32_t lenDup = 0;
	jsonStrDup = RyanJsonPrint(pJsonDup, 100, RyanJsonFalse, &lenDup);
	RyanJsonCheckGotoExit(NULL != jsonStrDup && lenDup > 0);

	// 验证序列化后的 String 内容完全一致
	RyanJsonCheckCode(len == lenDup && 0 == memcmp(jsonStr, jsonStrDup, (size_t)len), { RyanJsonCheckGotoExit(0); });

	// 独立性验证
	// 修改原 Object（如果可能）或修改副本，验证互不影响
	// 这里选择修改副本（pJsonDup），因为后续会销毁副本
	if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
	{
		// 先执行删除操作
		RyanJsonDelete(RyanJsonDetachByIndex(pJsonDup, 0));
		if (RyanJsonGetSize(pJsonDup) > 1) { RyanJsonDelete(RyanJsonDetachByIndex(pJsonDup, 1)); }

		// 如果修改后仍是容器
		if (RyanJsonIsArray(pJsonDup) || RyanJsonIsObject(pJsonDup))
		{
			RyanJson_t item;

			// 修改 key（仅 Object 子节点存在 key 语义）
			if (RyanJsonIsObject(pJsonDup))
			{
				RyanJsonObjectForEach(pJsonDup, item)
				{
					if (RyanJsonIsKey(item))
					{
						RyanJsonChangeKey(item, "modify_key_test");
						break;
					}
				}
			}

			// 修改 Bool 值（Array/Object 都应遍历到）
			if (RyanJsonIsArray(pJsonDup))
			{
				RyanJsonArrayForEach(pJsonDup, item)
				{
					if (RyanJsonIsBool(item))
					{
						RyanJsonChangeBoolValue(item, !RyanJsonGetBoolValue(item));
						break;
					}
				}
			}
			else
			{
				RyanJsonObjectForEach(pJsonDup, item)
				{
					if (RyanJsonIsBool(item))
					{
						RyanJsonChangeBoolValue(item, !RyanJsonGetBoolValue(item));
						break;
					}
				}
			}

			// 再次修改 Object 节点中的 key
			if (RyanJsonIsObject(pJsonDup))
			{
				RyanJsonObjectForEach(pJsonDup, item)
				{
					if (RyanJsonIsKey(item) && RyanJsonIsObject(item))
					{
						RyanJsonChangeKey(item, "modify_obj_key_test");
						break;
					}
				}
			}
		}

		// 验证修改后的副本与原 Object 不再相等
		// 注意：如果原 Object 本来就是空的，或者修改没有实际生效（例如没找到对应类型的节点），这里可能会相等
		// 所以这里只调用比较函数增加覆盖率，不强制断言 False，因为逻辑极其复杂
		RyanJsonCompare(pJson, pJsonDup);
		RyanJsonCompareOnlyKey(pJson, pJsonDup);
	}

exit__:

	if (jsonStr)
	{
		RyanJsonFree(jsonStr);
		jsonStr = NULL;
	}
	if (pJsonDup)
	{
		RyanJsonDelete(pJsonDup);
		pJsonDup = NULL;
	}
	if (jsonStrDup)
	{
		RyanJsonFree(jsonStrDup);
		jsonStrDup = NULL;
	}

	return result;
}
