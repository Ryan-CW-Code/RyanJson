#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

/**
 * @brief 节点删除测试
 *
 * 测试 RyanJson 的节点删除功能（DeleteByKey、DeleteByIndex）。
 * 覆盖场景：
 * 异常删除：测试无效 key、越界索引、空指针等错误。
 * 特殊位置删除：测试删除头部与尾部节点。
 * 递归删除：遍历 Json 树，随机删除子节点，验证树结构的完整性。
 *
 * @param state Fuzzer 状态上下文
 * @param pJson 当前正在操作的 Json 节点
 * @param size 输入数据大小
 */
RyanJsonBool_e RyanJsonFuzzerTestDelete(RyanJson_t pJson, uint32_t size)
{
	// 仅处理容器类型（Object/Array）
	if (RyanJsonFalse == RyanJsonIsArray(pJson) && RyanJsonFalse == RyanJsonIsObject(pJson)) { return RyanJsonTrue; }

	// 故障注入与异常参数测试
	if (RyanJsonFuzzerShouldFail(100))
	{
		// key 删除错误用例
		RyanJsonDeleteByKey(pJson, "non_exist_key");
		RyanJsonDeleteByKey(NULL, "some_key");
		RyanJsonDeleteByKey(pJson, NULL);
		RyanJsonDeleteByKey(NULL, NULL);

		// index 删除错误用例
		uint32_t currentSize = RyanJsonGetSize(pJson);
		RyanJsonDeleteByIndex(pJson,
				      currentSize); // 越界：index 使用 0-based，size 位置必越界
		RyanJsonDeleteByIndex(NULL, size % (currentSize + 1));
		RyanJsonDeleteByIndex(pJson, (uint32_t)(-(int32_t)size)); // 负数强转
		RyanJsonDeleteByIndex(NULL, (uint32_t)(-(int32_t)size));
	}

	// 特殊位置删除测试（头部/尾部）
	// 在一定概率下尝试删除头部或尾部节点，测试链表操作的鲁棒性
	uint32_t jsonSize = RyanJsonGetSize(pJson);
	if (RyanJsonFuzzerShouldFail(10) && jsonSize > 2)
	{
		// 尝试按 key 删除尾部节点（仅 Object 有效）
		if (RyanJsonTrue == RyanJsonIsObject(pJson))
		{
			RyanJson_t tailNode = RyanJsonGetObjectByIndex(pJson, jsonSize - 1);
			if (NULL != tailNode) { RyanJsonDeleteByKey(pJson, RyanJsonGetKey(tailNode)); }

			// 重新获取 Size，因为刚删了一个
			if (RyanJsonGetSize(pJson) > 0)
			{
				RyanJson_t headNode = RyanJsonGetObjectByIndex(pJson, 0);
				if (NULL != headNode) { RyanJsonDeleteByKey(pJson, RyanJsonGetKey(headNode)); }
			}
		}
	}

	// 递归遍历
	// 先递归处理子节点，再删除当前层级的节点，避免由上而下的删除导致整个分支消失，减少测试覆盖率
	RyanJson_t item = NULL;
	RyanJson_t lastItem = NULL;

	if (RyanJsonTrue == RyanJsonIsArray(pJson))
	{
		RyanJsonArrayForEach(pJson, item)
		{
			// 递归调用
			RyanJsonFuzzerTestDelete(item, size);
			lastItem = item;
		}
	}
	else
	{
		RyanJsonObjectForEach(pJson, item)
		{
			// 递归调用
			RyanJsonFuzzerTestDelete(item, size);
			lastItem = item;
		}
	}

	// 删除节点

	// 按 key 删除（仅 Object）
	if (RyanJsonTrue == RyanJsonIsObject(pJson))
	{
		// 删除最后一个遍历到的子节点
		// 注意：RyanJsonObjectForEach 遍历结束后，childNode 为 NULL，
		// 所以我们需要在循环中维护 lastChild。
		// 并且要确认 lastChild 仍然有效（递归处理的是它的子节点，不应影响它本身）
		// 不过，如果上面的 Head/Tail 删除逻辑把 lastChild 删了，这里就会出问题。
		// 考虑到 RyanJsonGetNodeBy... 的额外开销，以及 Fuzzer 的随机性，
		// 我们这里再次检查 lastChild 是否还存在于 pJson 中比较耗时。
		// 简化策略：若 lastChild 有 key，则直接尝试删除；找不到也允许返回。

		if (NULL != lastItem && RyanJsonTrue == RyanJsonIsKey(lastItem)) { RyanJsonDeleteByKey(pJson, RyanJsonGetKey(lastItem)); }
	}

	// 按 index 删除（Array/Object）
	{
		uint32_t idx = 0;
		uint32_t currentSize = RyanJsonGetSize(pJson);

		if (0 != currentSize)
		{
			idx = size % currentSize;
			RyanJsonDeleteByIndex(pJson, idx);
		}
	}

	return RyanJsonTrue;
}
