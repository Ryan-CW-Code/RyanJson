#include "RyanJson.h"
#include "RyanJsonFuzzer.h"

/**
 * @brief 节点分离测试
 *
 * 测试 RyanJson 的节点分离功能（DetachByKey、DetachByIndex）。
 * 与 Delete 不同，Detach 不释放内存，而是将节点从树中摘除并返回。
 * 覆盖场景：
 * 异常分离：测试无效参数、越界索引等错误。
 * 递归分离：随机分离子节点，并立即释放分离出的节点以防止内存泄漏。
 *
 * @param state Fuzzer 状态上下文
 * @param pJson 当前正在操作的 Json 节点
 * @param size 输入数据大小
 */
RyanJsonBool_e RyanJsonFuzzerTestDetach(RyanJson_t pJson, uint32_t size)
{
	// 故障注入与异常参数测试
	if (RyanJsonFuzzerShouldFail(100))
	{
		// 错误的 DetachByKey 调用
		assert(RyanJsonFalse == RyanJsonDetachByKey(NULL, NULL));
		assert(RyanJsonFalse == RyanJsonDetachByKey(pJson, NULL));
		assert(RyanJsonFalse == RyanJsonDetachByKey(NULL, "NULL"));

		if (RyanJsonFalse == RyanJsonIsObject(pJson)) // 类型错误
		{
			assert(NULL == RyanJsonDetachByKey(pJson, "NULL"));
		}

		// 错误的 DetachByIndex 调用
		assert(NULL == RyanJsonDetachByIndex(NULL, 10));
		if (RyanJsonFalse == RyanJsonIsArray(pJson) && RyanJsonFalse == RyanJsonIsObject(pJson)) // 类型错误
		{
			assert(NULL == RyanJsonDetachByIndex(pJson, 0));
		}
	}

	// 仅处理容器类型（Object/Array）
	if (RyanJsonFalse == RyanJsonIsArray(pJson) && RyanJsonFalse == RyanJsonIsObject(pJson)) { return RyanJsonTrue; }

	// 递归遍历
	// 先递归处理子节点
	RyanJson_t item = NULL;
	RyanJson_t lastItem = NULL;
	RyanJsonObjectForEach(pJson, item)
	{
		RyanJsonFuzzerTestDetach(item, size);
		lastItem = item;
	}

	// 节点分离

	// 按 key 分离（仅 Object）
	if (RyanJsonTrue == RyanJsonIsObject(pJson))
	{
		// 尝试分离最后一个子节点
		if (NULL != lastItem && RyanJsonTrue == RyanJsonIsKey(lastItem))
		{
			RyanJson_t detachedItem = RyanJsonDetachByKey(pJson, RyanJsonGetKey(lastItem));

			// 重要：Detach 仅分离节点，不释放内存，必须手动 Delete 防止泄漏
			if (NULL != detachedItem) { RyanJsonDelete(detachedItem); }
		}
	}

	// 按 index 分离（Array/Object）
	{
		uint32_t idx = 0;
		uint32_t currentSize = RyanJsonGetSize(pJson);

		if (0 != currentSize)
		{
			idx = size % currentSize;

			RyanJson_t detachedItem = RyanJsonDetachByIndex(pJson, idx);
			if (NULL != detachedItem) { RyanJsonDelete(detachedItem); }
		}
	}

	return RyanJsonTrue;
}
