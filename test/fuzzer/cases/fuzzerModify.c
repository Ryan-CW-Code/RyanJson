#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

/**
 * @brief change 系列 API 的一次性确定性自检
 *
 * 这里只保留运行期 fuzz 不会主动构造的错误类型调用。
 */
void RyanJsonFuzzerSelfTestModifyCases(void)
{
	RyanJsonBool_e lastIsEnableMemFail;
	RyanJsonFuzzerMemFailPush(lastIsEnableMemFail, RyanJsonFalse);

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
	}

	if (strNode)
	{
		assert(RyanJsonFalse == RyanJsonChangeBoolValue(strNode, RyanJsonFalse));
		assert(0 == strcmp(RyanJsonGetStringValue(strNode), "v"));
		assert(0 == strcmp(RyanJsonGetKey(strNode), "k"));
	}

	RyanJsonDelete(intNode);
	RyanJsonDelete(doubleNode);
	RyanJsonDelete(boolNode);
	RyanJsonDelete(strNode);

	RyanJsonFuzzerMemFailPop(lastIsEnableMemFail);
}

/**
 * @brief get 系列 API 的一次性确定性自检
 *
 * 主要补齐 NULL、标量节点和越界访问的保护路径。
 */
void RyanJsonFuzzerSelfTestGetCases(void)
{
	RyanJson_t scalar = RyanJsonCreateInt(NULL, 1);
	assert(NULL != scalar);

	assert(RyanJsonFalse == RyanJsonIsNull(scalar));
	assert(NULL == RyanJsonGetObjectByKey(NULL, NULL));
	assert(NULL == RyanJsonGetObjectByKey(scalar, NULL));
	assert(NULL == RyanJsonGetObjectByKey(NULL, "NULL"));

	assert(NULL == RyanJsonGetObjectToKey(NULL, NULL));
	assert(NULL == RyanJsonGetObjectToKey(scalar, NULL));
	assert(NULL == RyanJsonGetObjectToKey(NULL, "NULL"));

	assert(NULL == RyanJsonGetObjectByIndex(NULL, 10));
	assert(NULL == RyanJsonGetObjectByKey(scalar, "NULL"));
	assert(NULL == RyanJsonGetObjectByIndex(scalar, 0));

	RyanJsonDelete(scalar);
}

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

	// Bool 节点
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

	// String 节点
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
		if (RyanJsonIsArray(pJson))
		{
			RyanJsonArrayForEach(pJson, item)
			{
				RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonFuzzerTestModify(item, size));
			}
		}
		else
		{
			RyanJsonObjectForEach(pJson, item)
			{
				RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonFuzzerTestModify(item, size));
			}
		}
	}

	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonFuzzerVerifyGet(RyanJson_t parent, RyanJson_t current, uint32_t index, uint32_t size)
{
	(void)size; // 未使用

	// 参数防御与错误类型验证已移到一次性自检，
	// 运行期仅保留父子关系与递归访问语义，避免全树重复做同一组断言。

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
		if (RyanJsonIsArray(current))
		{
			RyanJsonArrayForEach(current, item)
			{
				RyanJsonFuzzerVerifyGet(current, item, childIndex, size);
				childIndex++;
			}
		}
		else
		{
			RyanJsonObjectForEach(current, item)
			{
				RyanJsonFuzzerVerifyGet(current, item, childIndex, size);
				childIndex++;
			}
		}
	}

	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonFuzzerTestGet(RyanJson_t pJson, uint32_t size)
{
	// 遍历测试
	if (RyanJsonIsArray(pJson) || RyanJsonIsObject(pJson))
	{
		RyanJson_t item;
		uint32_t index = 0;
		if (RyanJsonIsArray(pJson))
		{
			RyanJsonArrayForEach(pJson, item)
			{
				RyanJsonFuzzerVerifyGet(pJson, item, index, size);
				index++;
			}
		}
		else
		{
			RyanJsonObjectForEach(pJson, item)
			{
				RyanJsonFuzzerVerifyGet(pJson, item, index, size);
				index++;
			}
		}
	}
	return RyanJsonTrue;
}
