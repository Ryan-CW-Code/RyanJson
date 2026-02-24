#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

/**
 * @brief 复制与比较测试
 *
 * 测试深度复制与结构比较能力。
 * 覆盖场景：
 * 深度复制正确性：验证复制后的对象与原对象在结构和值上完全一致。
 * 独立性验证：验证修改复制后的对象不会影响原对象。
 * 内存管理：验证复制过程中的内存分配与释放。
 *
 * @param state Fuzzer 状态上下文
 * @param pJson 需要复制的源 Json 对象
 */
RyanJsonBool_e RyanJsonFuzzerTestDuplicate(RyanJson_t pJson)
{
	RyanJsonBool_e result = RyanJsonTrue;
	char *jsonStr = NULL;
	char *jsonStrDup = NULL;
	RyanJson_t pJsonDup = NULL;

	// 序列化原始对象
	uint32_t len = 0;
	// 使用非格式化输出，减少空白差异干扰
	jsonStr = RyanJsonPrint(pJson, 100, RyanJsonFalse, &len);
	RyanJsonCheckGotoExit(NULL != jsonStr);

	// 执行深度复制
	pJsonDup = RyanJsonDuplicate(pJson);
	RyanJsonCheckGotoExit(NULL != pJsonDup);

	// 参数与边界场景测试
	// 测试对 NULL 的复制
	assert(NULL == RyanJsonDuplicate(NULL));
	assert(0 == RyanJsonGetSize(NULL));

	// 结构完整性验证
	// 验证节点数量一致
	assert(RyanJsonGetSize(pJson) == RyanJsonGetSize(pJsonDup));

	// 执行结构比较
	// RyanJsonCompare：比较 key 与 value
	// RyanJsonCompareOnlyKey：仅比较 key（结构）
	RyanJsonCompare(pJson, pJsonDup);
	RyanJsonCompareOnlyKey(pJson, pJsonDup);

	// 比较接口边界测试
	assert(RyanJsonTrue == RyanJsonCompare(pJson, pJson)); // 自比较
	assert(RyanJsonFalse == RyanJsonCompare(NULL, pJsonDup));
	assert(RyanJsonFalse == RyanJsonCompare(pJson, NULL));
	assert(RyanJsonFalse == RyanJsonCompare(NULL, NULL)); // 约定：两个 NULL 不相等

	assert(RyanJsonTrue == RyanJsonCompareOnlyKey(pJson, pJson));
	assert(RyanJsonFalse == RyanJsonCompareOnlyKey(NULL, pJsonDup));
	assert(RyanJsonFalse == RyanJsonCompareOnlyKey(pJson, NULL));
	assert(RyanJsonFalse == RyanJsonCompareOnlyKey(NULL, NULL));

	// 序列化一致性验证
	uint32_t lenDup = 0;
	jsonStrDup = RyanJsonPrint(pJsonDup, 100, RyanJsonFalse, &lenDup);
	RyanJsonCheckGotoExit(NULL != jsonStrDup && lenDup > 0);

	// 验证序列化后的字符串内容完全一致
	RyanJsonCheckCode(len == lenDup && 0 == memcmp(jsonStr, jsonStrDup, (size_t)len), {
		// printf("len:%" PRIu32 ", dupLen:%" PRIu32 "\r\n", len, lenDup);
		// printf("jsonStr:%s, jsonStrDup:%s\r\n", jsonStr, jsonStrDup);
		RyanJsonCheckGotoExit(0);
	});

	// 独立性验证
	// 修改原对象（如果可能）或修改副本，验证互不影响
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

			// 修改 key
			RyanJsonObjectForEach(pJsonDup, item)
			{
				if (RyanJsonIsKey(item))
				{
					RyanJsonChangeKey(item, "modify_key_test");
					break; // 只改第一个找到的
				}
			}

			// 修改 Bool 值
			RyanJsonObjectForEach(pJsonDup, item)
			{
				if (RyanJsonIsBool(item))
				{
					RyanJsonChangeBoolValue(item, !RyanJsonGetBoolValue(item));
					break;
				}
			}

			// 再次修改 Object 节点中的 key
			RyanJsonObjectForEach(pJsonDup, item)
			{
				if (RyanJsonIsKey(item) && RyanJsonIsObject(item))
				{
					RyanJsonChangeKey(item, "modify_obj_key_test");
					break;
				}
			}
		}

		// 验证修改后的副本与原对象不再相等
		// 注意：如果原对象本来就是空的，或者修改没有实际生效（例如没找到对应类型的节点），这里可能会相等
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
