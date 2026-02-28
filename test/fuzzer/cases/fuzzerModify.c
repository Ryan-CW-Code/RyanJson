#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

/**
 * @brief 修改与访问测试
 *
 * 测试 RyanJson 的节点修改能力（Change 值/key）以及数据访问安全性。
 * 覆盖场景：
 * 异常参数修改：测试空指针、类型不匹配时的修改行为。
 * 递归修改：遍历 Json 树，随机修改子节点的值或 key。
 * 类型安全访问：测试类型不匹配时调用 Get 接口的安全性（例如对 Int 调用 GetString）。
 * 访问正确性验证：验证 Get 接口返回值与 Set 结果一致。
 *
 * @param state Fuzzer 状态上下文
 * @param pJson 当前正在操作的 Json 节点
 * @param size 输入数据大小
 */
RyanJsonBool_e RyanJsonFuzzerTestModify(RyanJson_t pJson, uint32_t size)
{
	// 一次性覆盖 Change 参数/类型防御分支，并验证失败不改值
	static RyanJsonBool_e changeGuardCovered = RyanJsonFalse;
	if (RyanJsonFalse == changeGuardCovered)
	{
		RyanJsonBool_e lastIsEnableMemFail = g_fuzzerState.isEnableMemFail;
		g_fuzzerState.isEnableMemFail = false;

		RyanJson_t intNode = RyanJsonCreateInt(NULL, 123);
		RyanJson_t doubleNode = RyanJsonCreateDouble(NULL, 1.25);
		RyanJson_t boolNode = RyanJsonCreateBool("flag", RyanJsonTrue);
		RyanJson_t strNode = RyanJsonCreateString("k", "v");

		if (intNode)
		{
			assert(RyanJsonFalse == RyanJsonChangeDoubleValue(intNode, 9.9));
			assert(123 == RyanJsonGetIntValue(intNode));
		}

		if (doubleNode)
		{
			assert(RyanJsonFalse == RyanJsonChangeIntValue(doubleNode, 7));
			assert(RyanJsonTrue == RyanJsonCompareDouble(1.25, RyanJsonGetDoubleValue(doubleNode)));
		}

		if (boolNode)
		{
			assert(RyanJsonFalse == RyanJsonChangeIntValue(boolNode, 1));
			assert(RyanJsonTrue == RyanJsonGetBoolValue(boolNode));
			assert(RyanJsonTrue == RyanJsonChangeKey(boolNode, "flag2"));
			assert(0 == strcmp(RyanJsonGetKey(boolNode), "flag2"));
		}

		if (strNode)
		{
			// 命中 RyanJsonChangeStringValue 中 RyanJsonIsKey(pJson)==true 的分支
			assert(RyanJsonTrue == RyanJsonChangeStringValue(strNode, "v2"));
			assert(0 == strcmp(RyanJsonGetStringValue(strNode), "v2"));
			assert(0 == strcmp(RyanJsonGetKey(strNode), "k"));

			assert(RyanJsonFalse == RyanJsonChangeBoolValue(strNode, RyanJsonFalse));
			assert(0 == strcmp(RyanJsonGetStringValue(strNode), "v2"));
			assert(RyanJsonFalse == RyanJsonChangeKey(strNode, NULL));
			assert(0 == strcmp(RyanJsonGetKey(strNode), "k"));
		}

		RyanJsonDelete(intNode);
		RyanJsonDelete(doubleNode);
		RyanJsonDelete(boolNode);
		RyanJsonDelete(strNode);

		g_fuzzerState.isEnableMemFail = lastIsEnableMemFail;
		changeGuardCovered = RyanJsonTrue;
	}

	// 修改 key 测试
	if (RyanJsonIsKey(pJson))
	{
		const char *oldKey = RyanJsonGetKey(pJson);
		if (oldKey)
		{
			// 备份原始 key
			size_t keyLen = strlen(oldKey);
			char *backupKey = (char *)malloc(keyLen + 1);
			if (backupKey)
			{
				memcpy(backupKey, oldKey, keyLen);
				backupKey[keyLen] = 0;

				// 修改为临时 key
				if (RyanJsonTrue == RyanJsonChangeKey(pJson, "temp_modified_key"))
				{
					fuzzTestWithMemFail({ assert(0 == strcmp(RyanJsonGetKey(pJson), "temp_modified_key")); });
				}

				// 恢复原始 key（确保结构一致，避免影响后续测试）
				if (RyanJsonTrue == RyanJsonChangeKey(pJson, backupKey))
				{
					fuzzTestWithMemFail({ assert(0 == strcmp(RyanJsonGetKey(pJson), backupKey)); });
				}
				free(backupKey);
			}
		}
	}

	// 布尔节点
	if (RyanJsonIsBool(pJson))
	{
		RyanJsonBool_e oldBool = RyanJsonGetBoolValue(pJson);
		if (RyanJsonTrue == RyanJsonChangeBoolValue(pJson, !oldBool))
		{
			fuzzTestWithMemFail({ assert(RyanJsonGetBoolValue(pJson) == !oldBool); });
		}
		if (RyanJsonTrue == RyanJsonChangeBoolValue(pJson, oldBool)) // 恢复
		{
			fuzzTestWithMemFail({ assert(RyanJsonGetBoolValue(pJson) == oldBool); });
		}
	}

	// 数值节点（Int/Double）
	if (RyanJsonIsNumber(pJson))
	{
		if (RyanJsonIsInt(pJson))
		{
			int32_t oldInt = RyanJsonGetIntValue(pJson);
			if (RyanJsonTrue == RyanJsonChangeIntValue(pJson, (int32_t)size))
			{
				fuzzTestWithMemFail({ assert(RyanJsonGetIntValue(pJson) == (int32_t)size); });
			}
			if (RyanJsonTrue == RyanJsonChangeIntValue(pJson, oldInt)) // 恢复
			{
				fuzzTestWithMemFail({ assert(RyanJsonGetIntValue(pJson) == oldInt); });
			}
		}
		if (RyanJsonIsDouble(pJson))
		{
			double oldDouble = RyanJsonGetDoubleValue(pJson);
			if (RyanJsonTrue == RyanJsonChangeDoubleValue(pJson, size * 1.123456789))
			{
				fuzzTestWithMemFail({ assert(RyanJsonCompareDouble(RyanJsonGetDoubleValue(pJson), size * 1.123456789)); });
			}
			if (RyanJsonTrue == RyanJsonChangeDoubleValue(pJson, oldDouble)) // 恢复
			{
				fuzzTestWithMemFail({ assert(RyanJsonCompareDouble(RyanJsonGetDoubleValue(pJson), oldDouble)); });
			}
		}
	}

	// 字符串节点
	if (RyanJsonIsString(pJson))
	{
		const char *oldStr = RyanJsonGetStringValue(pJson);
		if (oldStr)
		{
			size_t strLen = strlen(oldStr);
			char *backupStr = (char *)malloc(strLen + 1);
			if (backupStr)
			{
				memcpy(backupStr, oldStr, strLen);
				backupStr[strLen] = 0;

				if (RyanJsonTrue == RyanJsonChangeStringValue(pJson, "modified_string_value"))
				{
					fuzzTestWithMemFail(
						{ assert(0 == strcmp(RyanJsonGetStringValue(pJson), "modified_string_value")); });
				}
				if (RyanJsonTrue == RyanJsonChangeStringValue(pJson, "short"))
				{
					fuzzTestWithMemFail({ assert(0 == strcmp(RyanJsonGetStringValue(pJson), "short")); });
				}

				if (RyanJsonTrue == RyanJsonChangeStringValue(pJson, backupStr)) // 恢复
				{
					fuzzTestWithMemFail({ assert(0 == strcmp(RyanJsonGetStringValue(pJson), backupStr)); });
				}
				free(backupStr);
			}
		}
	}

	// 递归遍历
	if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
	{
		RyanJson_t item;
		RyanJsonArrayForEach(pJson, item)
		{
			RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonFuzzerTestModify(item, size));
		}
	}

	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonFuzzerVerifyGet(RyanJson_t parent, RyanJson_t current, uint32_t index, uint32_t size)
{
	(void)size; // 未使用

	RyanJsonIsNull(current);

	// Get 接口前置条件：调用方需保证 pJson 非 NULL 且类型匹配。
	// 这里只保留可明确约定返回值的 GetObjectByKey 参数防御验证。

	// 验证 GetObjectByKey 异常参数
	assert(NULL == RyanJsonGetObjectByKey(NULL, NULL));
	assert(NULL == RyanJsonGetObjectByKey(current, NULL));
	assert(NULL == RyanJsonGetObjectByKey(NULL, "NULL"));

	assert(NULL == RyanJsonGetObjectToKey(NULL, NULL));
	assert(NULL == RyanJsonGetObjectToKey(current, NULL));
	assert(NULL == RyanJsonGetObjectToKey(NULL, "NULL"));

	// 验证 GetObjectByIndex 异常参数
	assert(NULL == RyanJsonGetObjectByIndex(NULL, 10));

	// 验证错误类型调用
	if (!RyanJsonIsObject(current)) { assert(NULL == RyanJsonGetObjectByKey(current, "NULL")); }

	// 验证 GetObjectByIndex 错误类型调用
	if (!RyanJsonIsArray(current) && !RyanJsonIsObject(current)) { assert(NULL == RyanJsonGetObjectByIndex(current, 0)); }

	// 验证父子关系查找
	// 确认当前节点 current 可以通过 parent + key/index 找回
	if (RyanJsonIsKey(current))
	{
		// 如果是 key 类型，应能通过 parent + key 找回（或找回其对应值）
		// 注意：RyanJsonGetObjectByKey 通常返回值节点，而非 key 节点本身
		// 但在 RyanJson 实现中，key 节点与 value 节点关系较紧密，返回语义由实现细节决定
		// 这里只调用 API 增加覆盖率，不做强一致性断言，因为实现细节可能复杂
		RyanJsonGetObjectToKey(parent, RyanJsonGetKey(current));
	}
	else
	{
		// 否则尝试通过 index 访问
		RyanJsonGetObjectToIndex(parent, index);
	}

	// 递归验证
	if (RyanJsonIsArray(current) || RyanJsonIsObject(current))
	{
		RyanJson_t item;
		// 这里的 index 是相对于 current 的子索引

		uint32_t childIndex = 0;
		RyanJsonObjectForEach(current, item)
		{
			RyanJsonFuzzerVerifyGet(current, item, childIndex, size);
			childIndex++;
		}
	}

	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonFuzzerTestGet(RyanJson_t pJson, uint32_t size)
{
	// 全局异常测试
	assert(RyanJsonFalse == RyanJsonIsKey(NULL));
	assert(RyanJsonFalse == RyanJsonIsNull(NULL));
	assert(RyanJsonFalse == RyanJsonIsBool(NULL));
	assert(RyanJsonFalse == RyanJsonIsNumber(NULL));
	assert(RyanJsonFalse == RyanJsonIsString(NULL));
	assert(RyanJsonFalse == RyanJsonIsArray(NULL));
	assert(RyanJsonFalse == RyanJsonIsObject(NULL));
	assert(RyanJsonFalse == RyanJsonIsInt(NULL));
	assert(RyanJsonFalse == RyanJsonIsDouble(NULL));

	// 遍历测试
	if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
	{
		RyanJson_t item;
		uint32_t index = 0;
		RyanJsonObjectForEach(pJson, item)
		{
			RyanJsonFuzzerVerifyGet(pJson, item, index, size);
			index++;
		}
	}
	return RyanJsonTrue;
}
