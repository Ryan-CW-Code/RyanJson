#include "RyanJsonInternal.h"

#ifdef RyanJsonLinuxTestEnv
#include <stdio.h>
#include <time.h>

#undef RyanJsonSnprintf
/**
 * @brief Linux 测试环境下的 snprintf 包装（支持 fuzzer 注入）
 */
RyanJsonInternalApi int32_t RyanJsonSnprintf(char *buf, size_t size, const char *fmt, ...)
{
#ifdef isEnableFuzzer
	// Fuzzer 模式：随机触发失败，测试错误处理路径
	if (RyanJsonFuzzerShouldFail(500)) { return 0; }
#endif

	va_list args;
	va_start(args, fmt);
	int32_t ret = vsnprintf(buf, size, fmt, args);
	va_end(args);
	return ret;
}
#endif // RyanJsonLinuxTestEnv

/**
 * @brief 比较两个 C 字符串是否相等
 *
 * @param s1 字符串1
 * @param s2 字符串2
 * @return RyanJsonBool_e 是否相等
 */
RyanJsonInternalApi RyanJsonBool_e RyanJsonInternalStrEq(const char *s1, const char *s2)
{
	// NULL 检查
	RyanJsonCheckAssert(NULL != s1 && NULL != s2);

	// 地址相同
	if (s1 == s2) { return RyanJsonTrue; }

	// 首字符不同
	if (*s1 != *s2) { return RyanJsonFalse; }

	return RyanJsonMakeBool(0 == RyanJsonStrcmp(s1, s2));
}

/**
 * @brief 安全的浮点数比较
 */
RyanJsonBool_e RyanJsonCompareDouble(double a, double b)
{
	double diff = fabs(a - b);
	double absA = fabs(a);
	double absB = fabs(b);
	double maxVal = (absA > absB ? absA : absB);

	// 允许的容差：相对误差 + 绝对误差
	double epsilon = DBL_EPSILON * maxVal;
	double absTolerance = RyanJsonAbsTolerance; // 绝对容差

	return diff <= (epsilon > absTolerance ? epsilon : absTolerance);
}

/**
 * @brief 获取字符串指针模式的缓冲区地址
 *
 * @param pJson Json 节点
 * @return uint8_t* 缓冲区首地址
 */
RyanJsonInternalApi uint8_t *RyanJsonInternalGetStrPtrModeBuf(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);

	// 使用 memcpy 规避潜在的非对齐访问警告
	void *tmpPtr = NULL;
	RyanJsonMemcpy((void *)&tmpPtr, (RyanJsonGetPayloadPtr(pJson) + RyanJsonFlagSize + RyanJsonKeyFeidLenMaxSize), sizeof(void *));
	return (uint8_t *)tmpPtr;
}

/**
 * @brief 设置字符串指针模式的缓冲区地址
 *
 * @param pJson Json 节点
 * @param heapPtr 缓冲区首地址
 */
RyanJsonInternalApi void RyanJsonInternalSetStrPtrModeBuf(RyanJson_t pJson, uint8_t *heapPtr)
{
	RyanJsonCheckAssert(NULL != pJson);
	RyanJsonCheckAssert(NULL != heapPtr);

	// 使用 memcpy 规避潜在的非对齐访问警告
	void *tmpPtr = heapPtr;
	// 前面依次是 flag 与 key 长度字段空间，这里只写入堆指针
	RyanJsonMemcpy((RyanJsonGetPayloadPtr(pJson) + RyanJsonFlagSize + RyanJsonKeyFeidLenMaxSize), (const void *)&tmpPtr,
		       sizeof(void *));
}

/**
 * @brief 获取字符串指针模式指定偏移处的地址
 *
 * @param pJson Json 节点
 * @param index 索引
 * @return uint8_t* 偏移后的地址
 */
RyanJsonInternalApi uint8_t *RyanJsonInternalGetStrPtrModeBufAt(RyanJson_t pJson, uint32_t index)
{
	RyanJsonCheckAssert(NULL != pJson);
	return (uint8_t *)(RyanJsonInternalGetStrPtrModeBuf(pJson) + (index));
}

/**
 * @brief 计算 key 长度需要的字节数
 *
 * @param len key 长度
 * @return uint8_t 需要的字节数
 */
RyanJsonInternalApi uint8_t RyanJsonInternalCalcLenBytes(uint32_t len)
{
	if (len <= UINT8_MAX) { return 1; }  // 01: 1 byte
	if (len <= UINT16_MAX) { return 2; } // 10: 2 bytes
	return 3;                            // 11: 4 bytes
}

/**
 * @brief 解码 key 长度字段
 *
 * @param encoded 编码后的 key 长度字段
 * @return uint8_t 解码后的 key 长度
 */
RyanJsonInternalApi uint8_t RyanJsonInternalDecodeKeyLenField(uint8_t encoded)
{
	return 3 == encoded ? 4 : encoded;
}

/**
 * @brief 设置 key 长度
 *
 * @param pJson Json 节点
 * @param value key 长度
 */
static void RyanJsonSetKeyLen(RyanJson_t pJson, uint32_t value)
{
	RyanJsonCheckAssert(NULL != pJson);
	uint8_t *buf = RyanJsonGetPayloadPtr(pJson) + RyanJsonFlagSize;
	uint8_t keyFieldLen = RyanJsonInternalDecodeKeyLenField(RyanJsonGetPayloadEncodeKeyLenByFlag(pJson));
	RyanJsonCheckAssert(keyFieldLen <= RyanJsonKeyFeidLenMaxSize);

	// 使用大小端无关的方式写入
	for (uint8_t i = 0; i < keyFieldLen; i++)
	{
		buf[i] = (uint8_t)(value & 0xFF);
		value >>= 8;
	}
}

/**
 * @brief 获取节点 key 的长度
 *
 * @param pJson Json 节点
 * @return uint32_t key 长度
 */
RyanJsonInternalApi uint32_t RyanJsonInternalGetKeyLen(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);
	uint8_t *buf = RyanJsonGetPayloadPtr(pJson) + RyanJsonFlagSize;
	uint8_t keyFieldLen = RyanJsonInternalDecodeKeyLenField(RyanJsonGetPayloadEncodeKeyLenByFlag(pJson));
	RyanJsonCheckAssert(keyFieldLen <= RyanJsonKeyFeidLenMaxSize);

	// 使用大小端无关的方式读取
	uint32_t value = 0;
	for (uint8_t i = 0; i < keyFieldLen; i++)
	{
		value |= ((uint32_t)buf[i]) << (i * 8);
	}
	return value;
}

/**
 * @brief 获取节点 value 存储区地址
 *
 * @param pJson Json 节点
 * @return void* value 地址
 */
RyanJsonInternalApi void *RyanJsonInternalGetValue(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);

	uint32_t len = RyanJsonFlagSize;
	if (RyanJsonIsKey(pJson)) { len += RyanJsonInlineStringSize; }
	return RyanJsonGetPayloadPtr(pJson) + len;
}

/**
 * @brief 更新 key 与 strValue
 *
 * @param pJson Json 节点
 * @param isNew 是否是新创建的节点
 * @param key key 字符串
 * @param strValue strValue
 * @return RyanJsonBool_e
 */
RyanJsonInternalApi RyanJsonBool_e RyanJsonInternalChangeString(RyanJson_t pJson, RyanJsonBool_e isNew, const char *key,
								const char *strValue)
{
	RyanJsonCheckAssert(NULL != pJson);

	uint32_t keyLen = 0;      // key 字节长度
	uint8_t keyLenField = 0;  // key 长度字段编码所需字节数
	uint32_t strValueLen = 0; // strValue 字节长度

	uint32_t mallocSize = 0;

	// 计算 str 缓冲区所需的总字节数
	if (NULL != key)
	{
		keyLen = RyanJsonStrlen(key);
		keyLenField = RyanJsonInternalCalcLenBytes(keyLen);
		mallocSize += keyLen + 1;
	}

	if (NULL != strValue)
	{
		strValueLen = RyanJsonStrlen(strValue);
		mallocSize += strValueLen + 1;
	}
	if (0 == mallocSize) { return RyanJsonTrue; }

	// 记录旧 str 缓冲区，切换成功后再释放
	uint8_t *oldPrt = NULL;
	if (RyanJsonFalse == isNew)
	{
		if (RyanJsonTrue == RyanJsonGetPayloadStrIsPtrByFlag(pJson)) { oldPrt = RyanJsonInternalGetStrPtrModeBuf(pJson); }
	}

	char arr[RyanJsonInlineStringSize];

	// 若 key + value + keyLenField 编码都能放进内联区，则走内联存储
	if ((mallocSize + RyanJsonInternalDecodeKeyLenField(keyLenField)) <= RyanJsonInlineStringSize)
	{
		RyanJsonSetPayloadStrIsPtrByFlag(pJson, RyanJsonFalse);
		// 先写入临时缓冲区，避免切换内联模式时出现覆盖
		if (keyLen) { RyanJsonMemcpy(arr, key, keyLen); }
		if (strValueLen) { RyanJsonMemcpy(arr + keyLen, strValue, strValueLen); }
	}
	else
	{
		// 申请新的 str 缓冲区
		uint8_t *newPtr = (uint8_t *)jsonMalloc(mallocSize);
		RyanJsonCheckReturnFalse(NULL != newPtr);

		// 先拷贝内容，再写回指针，避免指针写入后覆盖原数据
		if (NULL != key)
		{
			if (0 != keyLen) { RyanJsonMemcpy(newPtr, key, keyLen); }
			newPtr[keyLen] = '\0';
		}

		if (NULL != strValue)
		{
			uint8_t *strValueBuf = newPtr;
			if (NULL != key) { strValueBuf = newPtr + keyLen + 1; }

			if (0 != strValueLen) { RyanJsonMemcpy(strValueBuf, strValue, strValueLen); }
			strValueBuf[strValueLen] = '\0';
		}

		RyanJsonInternalSetStrPtrModeBuf(pJson, newPtr);
		RyanJsonSetPayloadStrIsPtrByFlag(pJson, RyanJsonTrue);
	}

	// 设置 key
	if (NULL != key)
	{
		RyanJsonSetPayloadEncodeKeyLenByFlag(pJson, keyLenField);
		RyanJsonSetKeyLen(pJson, keyLen);
		if (RyanJsonFalse == RyanJsonGetPayloadStrIsPtrByFlag(pJson))
		{
			char *keyBuf = RyanJsonGetKey(pJson);
			if (0 != keyLen) { RyanJsonMemcpy(keyBuf, arr, keyLen); }
			keyBuf[keyLen] = '\0';
		}
	}
	else
	{
		RyanJsonSetPayloadEncodeKeyLenByFlag(pJson, 0);
	}

	// 设置 strValue
	if (NULL != strValue)
	{
		if (RyanJsonFalse == RyanJsonGetPayloadStrIsPtrByFlag(pJson))
		{
			char *strValueBuf = RyanJsonGetStringValue(pJson);
			if (0 != strValueLen) { RyanJsonMemcpy(strValueBuf, arr + keyLen, strValueLen); }
			strValueBuf[strValueLen] = '\0';
		}
	}

	if (oldPrt) { jsonFree(oldPrt); }
	return RyanJsonTrue;
}

/**
 * @brief 创建一个节点
 *
 * @param info 节点信息
 * @return RyanJson_t 节点
 */
RyanJsonInternalApi RyanJson_t RyanJsonInternalNewNode(RyanJsonNodeInfo_t *info)
{
	RyanJsonCheckAssert(NULL != info);

	// 加1是flag的空间
	uint32_t size = sizeof(struct RyanJsonNode) + RyanJsonFlagSize;

	switch (info->type)
	{
	case RyanJsonTypeNumber:
		if (RyanJsonFalse == info->numberIsDoubleFlag) { size += sizeof(int32_t); }
		else
		{
			size += sizeof(double);
		}
		break;
	case RyanJsonTypeArray:
	case RyanJsonTypeObject: size += sizeof(RyanJson_t); break;

	default: break;
	}

	// 是否内联字符串
	if (NULL != info->key || RyanJsonTypeString == info->type) { size += RyanJsonInlineStringSize; }

	RyanJson_t pJson = (RyanJson_t)jsonMalloc((size_t)size);
	RyanJsonCheckReturnNull(NULL != pJson);

	// 节点体积较小，直接整块清零即可
	RyanJsonMemset(pJson, 0, size);

	RyanJsonSetType(pJson, info->type);

	// 设置 key 和 value
	RyanJsonCheckCode(RyanJsonTrue == RyanJsonInternalChangeString(pJson, RyanJsonTrue, info->key, info->strValue), {
		jsonFree(pJson);
		return NULL;
	});

	// 设置 bool / number
	if (RyanJsonTypeBool == info->type) { RyanJsonSetPayloadBoolValueByFlag(pJson, info->boolIsTrueFlag); }
	else if (RyanJsonTypeNumber == info->type) { RyanJsonSetPayloadNumberIsDoubleByFlag(pJson, info->numberIsDoubleFlag); }

	return pJson;
}

/**
 * @brief 在父节点中插入子节点（维护线索化链表）
 *
 * @param parent 父节点（Object 或 Array）
 * @param prev 前驱兄弟节点，为 NULL 表示头插
 * @param item 待插入节点
 */
RyanJsonInternalApi void RyanJsonInternalListInsertAfter(RyanJson_t parent, RyanJson_t prev, RyanJson_t item)
{
	RyanJson_t nextItem;

	if (prev)
	{
		// 若 prev 原本是尾节点，则 prev->next 指向 parent，此时 nextItem 应视为 NULL
		// 通过 IsLast 标志区分“下一个兄弟节点”与“父节点线索”。
		if (RyanJsonGetPayloadIsLastByFlag(prev)) { nextItem = NULL; }
		else
		{
			nextItem = prev->next;
		}
	}
	else
	{
		// 插入到头部。RyanJsonNode 结构保证了 Object 和 Array 的 Value 指针位置一致
		nextItem = RyanJsonGetObjectValue(parent);
	}

	// 先链接 prev -> item
	if (prev)
	{
		prev->next = item;
		// prev 不再是最后一个，取消 IsLast 标记
		RyanJsonSetPayloadIsLastByFlag(prev, 0);
	}
	else
	{
		// 只有 item 一个子节点，或者 item 是新的头部
		RyanJsonInternalChangeObjectValue(parent, item);
	}

	// 再链接 item -> nextItem（或 Parent）
	if (nextItem)
	{
		// 不是最后一个节点，item->next 指向下一个兄弟
		item->next = nextItem;
		RyanJsonSetPayloadIsLastByFlag(item, 0);
	}
	else
	{
		// 是最后一个节点，item->next 指向 Parent (线索化)
		item->next = parent;
		RyanJsonSetPayloadIsLastByFlag(item, 1);
	}
}

/**
 * @brief 获取同层下一个兄弟节点
 *
 * @param pJson 当前节点
 * @return RyanJson_t 下一个兄弟节点；若当前为最后节点返回 NULL
 */
RyanJson_t RyanJsonGetNext(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);

	// 尾节点（Bit7=1）的 next 保存的是父节点线索，对外应返回 NULL
	if (RyanJsonGetPayloadIsLastByFlag(pJson)) { return NULL; }
	return pJson->next;
}

/**
 * @brief 获取当前节点的父节点
 *
 * @param pJson 当前节点
 * @return RyanJson_t 父节点
 */
RyanJsonInternalApi RyanJson_t RyanJsonInternalGetParent(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);

	RyanJson_t curr = pJson;
	RyanJson_t next = RyanJsonGetNext(curr);

	// 沿着兄弟链表一直走到该层级的最后一个节点
	while (NULL != next)
	{
		curr = next;
		next = RyanJsonGetNext(curr);
	}

	// 此时 curr 是当前层级的最后一个节点，其 next 指针存放的就是父节点
	return curr->next;
}

/**
 * @brief 按多级 key 路径获取节点
 *
 * @param pJson 起始节点
 * @param key 第一级 key
 * @return RyanJson_t 目标节点，失败返回 NULL
 */
RyanJson_t RyanJsonGetObjectByKeys(RyanJson_t pJson, const char *key, ...)
{
	RyanJsonCheckReturnNull(NULL != pJson && NULL != key);

	const char *nextKey = key;
	RyanJson_t nextItem = RyanJsonGetObjectByKey(pJson, nextKey);
	RyanJsonCheckReturnNull(NULL != nextItem);
	RyanJsonCheckAssert(RyanJsonIsKey(nextItem));

	va_list args;
	va_start(args, key);
	nextKey = va_arg(args, const char *);
	while (nextItem && NULL != nextKey)
	{
		nextItem = RyanJsonGetObjectByKey(nextItem, nextKey);
		nextKey = va_arg(args, const char *);
	}
	va_end(args);

	return nextItem;
}

/**
 * @brief 按多级 index 路径获取节点
 *
 * @param pJson 起始节点
 * @param index 第一级索引
 * @return RyanJson_t 目标节点，失败返回 NULL
 */
RyanJson_t RyanJsonGetObjectByIndexs(RyanJson_t pJson, uint32_t index, ...)
{
	RyanJsonCheckReturnNull(NULL != pJson);

	uint32_t nextIndex = index;
	RyanJson_t nextItem = RyanJsonGetObjectByIndex(pJson, nextIndex);
	RyanJsonCheckReturnNull(NULL != nextItem);

	va_list args;
	va_start(args, index);
	nextIndex = va_arg(args, uint32_t);
	while (nextItem && UINT32_MAX != nextIndex)
	{
		nextItem = RyanJsonGetObjectByIndex(nextItem, nextIndex);
		nextIndex = va_arg(args, uint32_t);
	}
	va_end(args);

	return nextItem;
}

/**
 * @brief 创建 int32_t 数组节点
 *
 * @param numbers 输入数组
 * @param count 元素个数
 * @return RyanJson_t 新建数组节点，失败返回 NULL
 */
RyanJson_t RyanJsonCreateIntArray(const int32_t *numbers, uint32_t count)
{
	RyanJsonCheckReturnNull(NULL != numbers);

	RyanJson_t pJson = RyanJsonCreateArray();
	RyanJsonCheckReturnNull(NULL != pJson);
	for (uint32_t i = 0; i < count; i++)
	{
		RyanJsonCheckCode(RyanJsonTrue == RyanJsonAddIntToArray(pJson, numbers[i]), {
			RyanJsonDelete(pJson);
			return NULL;
		});
	}
	return pJson;
}

/**
 * @brief 创建 double 数组节点
 *
 * @param numbers 输入数组
 * @param count 元素个数
 * @return RyanJson_t 新建数组节点，失败返回 NULL
 */
RyanJson_t RyanJsonCreateDoubleArray(const double *numbers, uint32_t count)
{
	RyanJsonCheckReturnNull(NULL != numbers);

	RyanJson_t pJson = RyanJsonCreateArray();
	RyanJsonCheckReturnNull(NULL != pJson);
	for (uint32_t i = 0; i < count; i++)
	{
		RyanJsonCheckCode(RyanJsonTrue == RyanJsonAddDoubleToArray(pJson, numbers[i]), {
			RyanJsonDelete(pJson);
			return NULL;
		});
	}
	return pJson;
}

/**
 * @brief 创建字符串数组节点
 *
 * @param strings 输入字符串数组
 * @param count 元素个数
 * @return RyanJson_t 新建数组节点，失败返回 NULL
 */
RyanJson_t RyanJsonCreateStringArray(const char **strings, uint32_t count)
{
	RyanJsonCheckReturnNull(NULL != strings);

	RyanJson_t pJson = RyanJsonCreateArray();
	RyanJsonCheckReturnNull(NULL != pJson);
	for (uint32_t i = 0; i < count; i++)
	{
		RyanJsonCheckCode(RyanJsonTrue == RyanJsonAddStringToArray(pJson, strings[i]), {
			RyanJsonDelete(pJson);
			return NULL;
		});
	}
	return pJson;
}
