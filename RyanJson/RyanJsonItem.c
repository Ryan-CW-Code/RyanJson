#include "RyanJsonInternal.h"

/**
 * @brief 在 Object 节点中按 key 查找子节点
 *
 * @param pJson Object 节点
 * @param key 目标 key
 * @param prevOut 输出前驱节点，可为 NULL
 * @return RyanJson_t 命中节点，未命中返回 NULL
 */
static RyanJson_t RyanJsonFindNodeByKey(RyanJson_t pJson, const char *key, RyanJson_t *prevOut)
{
	RyanJsonCheckAssert(NULL != pJson && NULL != key);
	RyanJsonCheckReturnNull(_checkType(pJson, RyanJsonTypeObject));

	RyanJson_t prev = NULL;
	RyanJson_t nextItem = RyanJsonGetObjectValue(pJson);

	while (nextItem)
	{
		// Object 子节点按约定必须带 key，异常场景下直接返回，避免继续访问无效数据
		RyanJsonCheckAssert(RyanJsonIsKey(nextItem));
		if (RyanJsonTrue == RyanJsonInternalStrEq(RyanJsonGetKey(nextItem), key))
		{
			if (prevOut) { *prevOut = prev; }
			return nextItem;
		}
		prev = nextItem;
		nextItem = RyanJsonGetNext(nextItem);
	}

	return NULL;
}

/**
 * @brief 用新节点替换旧节点并维护链关系
 *
 * @param prev 旧节点前驱，可为 NULL
 * @param oldItem 旧节点
 * @param newItem 新节点
 * @return RyanJsonBool_e 替换是否成功
 */
static RyanJsonBool_e RyanJsonReplaceNode(RyanJson_t prev, RyanJson_t oldItem, RyanJson_t newItem)
{
	RyanJsonCheckAssert(NULL != oldItem && NULL != newItem);

	// 复制 IsLast 标志位
	if (RyanJsonGetPayloadIsLastByFlag(oldItem)) { RyanJsonSetPayloadIsLastByFlag(newItem, 1); }
	else
	{
		RyanJsonSetPayloadIsLastByFlag(newItem, 0);
	}

	// 链接前驱节点
	if (NULL != prev) { prev->next = newItem; }

	// 链接后继节点
	// 即使指向的是父节点(IsLast=1)，我们也把指针复制过来，保持线索化结构
	newItem->next = oldItem->next;

	oldItem->next = NULL;
	RyanJsonSetPayloadIsLastByFlag(oldItem, 0);
	return RyanJsonTrue;
}

#if true == RyanJsonStrictObjectKeyCheck
/**
 * @brief 检查 Object 中是否存在重复 key（可忽略指定节点）
 *
 * @param pJson Object 节点
 * @param key 目标 key
 * @param skipItem 需跳过的节点
 * @return RyanJsonBool_e 是否存在冲突
 */
static RyanJsonBool_e RyanJsonObjectHasKeyConflict(RyanJson_t pJson, const char *key, RyanJson_t skipItem)
{
	RyanJson_t item = RyanJsonGetObjectValue(pJson);
	while (NULL != item)
	{
		if (item != skipItem)
		{
			// Object 节点理论上必须带 key，容错处理避免 Release 模式下异常访问
			RyanJsonCheckAssert(RyanJsonTrue == RyanJsonIsKey(item));
			if (RyanJsonTrue == RyanJsonInternalStrEq(RyanJsonGetKey(item), key)) { return RyanJsonTrue; }
		}
		item = RyanJsonGetNext(item);
	}

	return RyanJsonFalse;
}
#endif

/**
 * @brief 为容器插入场景创建包装节点
 */
static RyanJson_t RyanJsonCreateItem(const char *key, RyanJson_t item)
{
	RyanJsonCheckAssert(NULL != item);

	RyanjsonType_e type = RyanJsonGetType(item);
	RyanJsonNodeInfo_t nodeInfo = {
		.type = (RyanJsonTypeArray == type) ? RyanJsonTypeArray : RyanJsonTypeObject,
		.key = key,
	};

	RyanJson_t newItem = RyanJsonInternalNewNode(&nodeInfo);
	RyanJsonCheckReturnNull(NULL != newItem);

	if (RyanJsonTypeArray == type || RyanJsonTypeObject == type)
	{
		// 转移子节点所有权
		RyanJson_t children = RyanJsonGetObjectValue(item);
		RyanJsonInternalChangeObjectValue(newItem, children);
		RyanJsonInternalChangeObjectValue(item, NULL);

		// 更新线索化链表：最后一个子节点的 next 指向新父节点 (newItem)
		if (children)
		{
			RyanJson_t last = children;
			while (RyanJsonGetNext(last))
			{
				last = RyanJsonGetNext(last);
			}
			last->next = newItem;
		}

		// 销毁旧节点（已剥离子节点）
		// 必须切断 next 指针，防止 RyanJsonDelete 继续遍历
		item->next = NULL;
		RyanJsonDelete(item);
	}
	else
	{
		// 原始类型包装：将 item 作为 newItem 的唯一子节点
		RyanJsonInternalChangeObjectValue(newItem, item);
		item->next = newItem; // 最后一个节点指向父节点
		RyanJsonSetPayloadIsLastByFlag(item, 1);
	}

	return newItem;
}

/**
 * @brief 基础创建接口（语义直观，统一说明）
 *
 * 约定：
 * - key 可为 NULL，表示无 key 节点（如 Array 元素）
 * - 返回值为新建节点，失败返回 NULL
 */
RyanJson_t RyanJsonCreateNull(const char *key)
{
	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeNull, .key = key};
	return RyanJsonInternalNewNode(&nodeInfo);
}
RyanJson_t RyanJsonCreateBool(const char *key, RyanJsonBool_e boolean)
{
	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeBool, .key = key, .boolIsTrueFlag = boolean};
	return RyanJsonInternalNewNode(&nodeInfo);
}
RyanJson_t RyanJsonCreateInt(const char *key, int32_t number)
{
	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeNumber, .key = key, .numberIsDoubleFlag = RyanJsonFalse};

	RyanJson_t item = RyanJsonInternalNewNode(&nodeInfo);
	RyanJsonCheckReturnNull(NULL != item);

	RyanJsonChangeIntValue(item, number);
	return item;
}
RyanJson_t RyanJsonCreateDouble(const char *key, double number)
{
	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeNumber, .key = key, .numberIsDoubleFlag = RyanJsonTrue};
	RyanJson_t item = RyanJsonInternalNewNode(&nodeInfo);
	RyanJsonCheckReturnNull(NULL != item);

	RyanJsonChangeDoubleValue(item, number);
	return item;
}
RyanJson_t RyanJsonCreateString(const char *key, const char *string)
{
	RyanJsonCheckReturnNull(NULL != string);

	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeString, .key = key, .strValue = string};
	return RyanJsonInternalNewNode(&nodeInfo);
}
RyanJsonInternalApi RyanJson_t RyanJsonInternalCreateObjectAndKey(const char *key)
{
	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeObject, .key = key};
	return RyanJsonInternalNewNode(&nodeInfo);
}
RyanJson_t RyanJsonCreateObject(void)
{
	return RyanJsonInternalCreateObjectAndKey(NULL);
}
RyanJsonInternalApi RyanJson_t RyanJsonInternalCreateArrayAndKey(const char *key)
{
	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeArray, .key = key};
	return RyanJsonInternalNewNode(&nodeInfo);
}
RyanJson_t RyanJsonCreateArray(void)
{
	return RyanJsonInternalCreateArrayAndKey(NULL);
}

/**
 * @brief 类型/属性判断接口（语义直观，统一说明）
 *
 * 约定：
 * - 参数为 NULL 时统一返回 RyanJsonFalse
 */
RyanJsonBool_e RyanJsonIsKey(RyanJson_t pJson)
{
	return RyanJsonMakeBool(NULL != pJson && RyanJsonGetPayloadEncodeKeyLenByFlag(pJson));
}
RyanJsonBool_e RyanJsonIsNull(RyanJson_t pJson)
{
	return RyanJsonMakeBool(NULL != pJson && RyanJsonTypeNull == RyanJsonGetType(pJson));
}
RyanJsonBool_e RyanJsonIsBool(RyanJson_t pJson)
{
	return RyanJsonMakeBool(NULL != pJson && RyanJsonTypeBool == RyanJsonGetType(pJson));
}
RyanJsonBool_e RyanJsonIsNumber(RyanJson_t pJson)
{
	return RyanJsonMakeBool(NULL != pJson && RyanJsonTypeNumber == RyanJsonGetType(pJson));
}
RyanJsonBool_e RyanJsonIsString(RyanJson_t pJson)
{
	return RyanJsonMakeBool(NULL != pJson && RyanJsonTypeString == RyanJsonGetType(pJson));
}
RyanJsonBool_e RyanJsonIsArray(RyanJson_t pJson)
{
	return RyanJsonMakeBool(NULL != pJson && RyanJsonTypeArray == RyanJsonGetType(pJson));
}
RyanJsonBool_e RyanJsonIsObject(RyanJson_t pJson)
{
	return RyanJsonMakeBool(NULL != pJson && RyanJsonTypeObject == RyanJsonGetType(pJson));
}
RyanJsonBool_e RyanJsonIsInt(RyanJson_t pJson)
{
	return RyanJsonMakeBool(RyanJsonIsNumber(pJson) && (RyanJsonFalse == RyanJsonGetPayloadNumberIsDoubleByFlag(pJson)));
}
RyanJsonBool_e RyanJsonIsDouble(RyanJson_t pJson)
{
	return RyanJsonMakeBool(RyanJsonIsNumber(pJson) && (RyanJsonTrue == RyanJsonGetPayloadNumberIsDoubleByFlag(pJson)));
}

/**
 * @brief 检查 item 是否为游离节点（未挂到任何树）
 *
 * @param item 待检查节点
 * @return RyanJsonBool_e 是否为游离节点
 */
RyanJsonBool_e RyanJsonIsDetachedItem(RyanJson_t item)
{
	RyanJsonCheckReturnFalse(NULL != item);
	RyanJsonCheckReturnFalse(NULL == item->next);
	RyanJsonCheckReturnFalse(!RyanJsonGetPayloadIsLastByFlag(item));
	return RyanJsonTrue;
}

/**
 * @brief 获取节点 key 字符串指针
 *
 * @param pJson 目标节点
 * @return char* key 字符串，失败返回 NULL
 * @note 返回内部存储指针，节点修改/释放后即失效。
 */
char *RyanJsonGetKey(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);
	if (RyanJsonFalse == RyanJsonGetPayloadStrIsPtrByFlag(pJson))
	{
		uint8_t keyFieldLen = RyanJsonInternalDecodeKeyLenField(RyanJsonGetPayloadEncodeKeyLenByFlag(pJson));
		RyanJsonCheckAssert(keyFieldLen <= RyanJsonKeyFeidLenMaxSize);
		return (char *)(RyanJsonGetPayloadPtr(pJson) + RyanJsonFlagSize + keyFieldLen);
	}

	return (char *)RyanJsonInternalGetStrPtrModeBufAt(pJson, 0);
}

/**
 * @brief 值读取接口（语义直观，统一说明）
 *
 * 约定：
 * - 调用前需保证 pJson 非 NULL 且类型匹配（先用 RyanJsonIsXXXX 判断）
 * - 非匹配类型场景不保证返回值语义
 * - String/key 返回内部存储指针，节点修改/释放后即失效
 */
char *RyanJsonGetStringValue(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);

	uint32_t len = 0;

	if (RyanJsonFalse == RyanJsonGetPayloadStrIsPtrByFlag(pJson))
	{
		uint8_t keyFieldLen = RyanJsonInternalDecodeKeyLenField(RyanJsonGetPayloadEncodeKeyLenByFlag(pJson));
		RyanJsonCheckAssert(keyFieldLen <= RyanJsonKeyFeidLenMaxSize);

		len += keyFieldLen;
		if (RyanJsonIsKey(pJson)) { len += RyanJsonInternalGetKeyLen(pJson) + 1U; }
		return (char *)(RyanJsonGetPayloadPtr(pJson) + RyanJsonFlagSize + len);
	}

	if (RyanJsonIsKey(pJson)) { len = RyanJsonInternalGetKeyLen(pJson) + 1U; }

	return (char *)RyanJsonInternalGetStrPtrModeBufAt(pJson, len);
}
RyanJsonBool_e RyanJsonGetBoolValue(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);
	return RyanJsonGetPayloadBoolValueByFlag(pJson);
}
int32_t RyanJsonGetIntValue(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);

	int32_t intValue;
	RyanJsonMemcpy(&intValue, RyanJsonInternalGetValue(pJson), sizeof(intValue));
	return intValue;
}
double RyanJsonGetDoubleValue(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);

	double doubleValue;
	RyanJsonMemcpy(&doubleValue, RyanJsonInternalGetValue(pJson), sizeof(doubleValue));
	return doubleValue;
}
RyanJson_t RyanJsonGetObjectValue(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);

	RyanJson_t objValue;
	RyanJsonMemcpy((void *)&objValue, RyanJsonInternalGetValue(pJson), sizeof(void *));
	return objValue;
}
RyanJson_t RyanJsonGetArrayValue(RyanJson_t pJson)
{
	return RyanJsonGetObjectValue(pJson);
}

/**
 * @brief 公共值修改接口（语义直观，统一说明）
 *
 * 约定：
 * - 公共 Change 接口会做基础参数/类型校验，失败返回 RyanJsonFalse
 * - Number/Bool 修改为原位写入
 * - key/String 修改会触发字符串存储布局更新
 */
RyanJsonBool_e RyanJsonChangeKey(RyanJson_t pJson, const char *key)
{
	RyanJsonCheckReturnFalse(NULL != pJson && NULL != key);
	RyanJsonCheckReturnFalse(RyanJsonIsKey(pJson));

#if true == RyanJsonStrictObjectKeyCheck
	// 严格模式下，若节点挂在 Object 下，改 key 不能制造重复 key
	if (RyanJsonTrue != RyanJsonInternalStrEq(RyanJsonGetKey(pJson), key))
	{
		RyanJson_t parent = RyanJsonInternalGetParent(pJson);
		if (NULL != parent && RyanJsonTrue == RyanJsonIsObject(parent))
		{
			RyanJsonCheckReturnFalse(RyanJsonFalse == RyanJsonObjectHasKeyConflict(parent, key, pJson));
		}
	}
#endif

	return RyanJsonInternalChangeString(pJson, RyanJsonFalse, key, RyanJsonIsString(pJson) ? RyanJsonGetStringValue(pJson) : NULL);
}

/**
 * @brief 修改 String 节点的 value
 *
 * @param pJson String 节点
 * @param strValue 新 strValue
 * @return RyanJsonBool_e 修改是否成功
 */
RyanJsonBool_e RyanJsonChangeStringValue(RyanJson_t pJson, const char *strValue)
{
	RyanJsonCheckReturnFalse(NULL != pJson && NULL != strValue);
	RyanJsonCheckReturnFalse(RyanJsonIsString(pJson));
	return RyanJsonInternalChangeString(pJson, RyanJsonFalse, RyanJsonIsKey(pJson) ? RyanJsonGetKey(pJson) : NULL, strValue);
}
RyanJsonBool_e RyanJsonChangeIntValue(RyanJson_t pJson, int32_t number)
{
	RyanJsonCheckReturnFalse(NULL != pJson);
	RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonIsInt(pJson));
	RyanJsonMemcpy(RyanJsonInternalGetValue(pJson), &number, sizeof(number));
	return RyanJsonTrue;
}
RyanJsonBool_e RyanJsonChangeDoubleValue(RyanJson_t pJson, double number)
{
	RyanJsonCheckReturnFalse(NULL != pJson);
	RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonIsDouble(pJson));
	RyanJsonMemcpy(RyanJsonInternalGetValue(pJson), &number, sizeof(number));
	return RyanJsonTrue;
}
RyanJsonBool_e RyanJsonChangeBoolValue(RyanJson_t pJson, RyanJsonBool_e boolean)
{
	RyanJsonCheckReturnFalse(NULL != pJson);
	RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonIsBool(pJson));
	RyanJsonSetPayloadBoolValueByFlag(pJson, boolean);
	return RyanJsonTrue;
}

/**
 * @brief 设置容器节点的首子节点指针（内部接口）
 *
 * @param pJson 容器节点（Object 或 Array）
 * @param objValue 新的首子节点，可为 NULL
 * @return RyanJsonBool_e 设置是否成功
 */
RyanJsonInternalApi RyanJsonBool_e RyanJsonInternalChangeObjectValue(RyanJson_t pJson, RyanJson_t objValue)
{
	RyanJsonCheckAssert(NULL != pJson);
	RyanJsonCheckAssert(RyanJsonIsObject(pJson) || RyanJsonIsArray(pJson));
	RyanJsonMemcpy(RyanJsonInternalGetValue(pJson), (void *)&objValue, sizeof(void *));
	return RyanJsonTrue;
}

/**
 * @brief 向 Object 添加容器节点（Array/Object）
 *
 * @param pJson Object 节点
 * @param key 新子节点 key
 * @param item 待添加节点（要求游离）
 * @return RyanJsonBool_e 添加是否成功
 * @note 插入位置遵循 RyanJsonAddPosition（头插/尾插取决于配置）。
 * @note 当 item 为容器时，会创建包装节点并释放原 item 指针，调用方不可继续使用原 item。
 * @note 若 item 不是容器类型，将释放 item 并返回失败。
 */
RyanJsonBool_e RyanJsonAddItemToObject(RyanJson_t pJson, const char *key, RyanJson_t item)
{
	RyanJsonCheckReturnFalse(NULL != item);
	RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonIsDetachedItem(item));

	// AddItem 仅支持容器类型（Array/Object），标量请使用 AddInt/AddString 等接口
	RyanjsonType_e type = RyanJsonGetType(item);
	if (RyanJsonTypeArray != type && RyanJsonTypeObject != type)
	{
		RyanJsonDelete(item);
		return RyanJsonFalse;
	}

	RyanJson_t pItem = RyanJsonCreateItem(key, item);
	RyanJsonCheckCode(NULL != pItem, {
		RyanJsonDelete(item);
		return RyanJsonFalse;
	});
	return RyanJsonInsert(pJson, RyanJsonAddPosition, pItem);
}

/**
 * @brief 按索引替换子节点
 *
 * @param pJson 父节点（Array 或 Object）
 * @param index 目标索引（Object 场景为插入顺序）
 * @param item 新节点（要求游离）
 * @return RyanJsonBool_e 替换是否成功
 * @note Object 场景要求 item 携带 key，严格模式下会拒绝重复 key。
 * @note 成功后旧节点会被释放；失败不会释放 item。
 */
RyanJsonBool_e RyanJsonReplaceByIndex(RyanJson_t pJson, uint32_t index, RyanJson_t item)
{
	RyanJsonCheckReturnFalse(NULL != pJson && NULL != item);
	RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonIsDetachedItem(item));

	RyanJsonCheckReturnFalse(_checkType(pJson, RyanJsonTypeArray) || _checkType(pJson, RyanJsonTypeObject));
	if (_checkType(pJson, RyanJsonTypeObject)) { RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonIsKey(item)); }

	RyanJson_t prev = NULL;
	RyanJson_t nextItem = RyanJsonGetObjectValue(pJson);
	RyanJsonCheckReturnFalse(NULL != nextItem);

	// 定位目标索引节点
	while (index > 0)
	{
		prev = nextItem;
		nextItem = RyanJsonGetNext(nextItem);
		index--;
		RyanJsonCheckReturnFalse(NULL != nextItem);
	}

	// 严格模式下：Object 不允许替换成重复 key
#if true == RyanJsonStrictObjectKeyCheck
	if (_checkType(pJson, RyanJsonTypeObject))
	{
		RyanJsonCheckReturnFalse(RyanJsonFalse == RyanJsonObjectHasKeyConflict(pJson, RyanJsonGetKey(item), nextItem));
	}
#endif

	RyanJsonReplaceNode(prev, nextItem, item);
	if (NULL == prev) { RyanJsonInternalChangeObjectValue(pJson, item); }

	RyanJsonDelete(nextItem);
	return RyanJsonTrue;
}

/**
 * @brief 按 key 替换 Object 子节点
 *
 * @param pJson Object 节点
 * @param key 目标 key
 * @param item 新节点（要求游离）
 * @return RyanJsonBool_e 替换是否成功
 * @note 成功后旧节点会被释放；失败不会释放 item。
 * @note 若 item 无 key，会创建包装节点；此时若 item 为容器会释放原 item 指针。
 */
RyanJsonBool_e RyanJsonReplaceByKey(RyanJson_t pJson, const char *key, RyanJson_t item)
{
	RyanJsonCheckReturnFalse(NULL != pJson && NULL != key && NULL != item);
	RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonIsDetachedItem(item));

	RyanJson_t prev = NULL;
	RyanJson_t nextItem = RyanJsonFindNodeByKey(pJson, key, &prev);
	RyanJsonCheckReturnFalse(NULL != nextItem);

	// 若传入节点没有 key，则构造一个带 key 的包装节点
	if (RyanJsonFalse == RyanJsonIsKey(item))
	{
		item = RyanJsonCreateItem(key, item);
		RyanJsonCheckReturnFalse(NULL != item);
	}
	else
	{
		if (RyanJsonTrue != RyanJsonInternalStrEq(RyanJsonGetKey(item), key))
		{
			RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonChangeKey(item, key));
		}
	}

	RyanJsonReplaceNode(prev, nextItem, item);
	if (NULL == prev) { RyanJsonInternalChangeObjectValue(pJson, item); }

	RyanJsonDelete(nextItem);

	return RyanJsonTrue;
}

/**
 * @brief 按索引获取子节点
 *
 * @param pJson 父节点（Array 或 Object）
 * @param index 子节点索引（Object 场景为插入顺序）
 * @return RyanJson_t 命中节点，失败返回 NULL
 * @note 越界或空容器返回 NULL。
 */
RyanJson_t RyanJsonGetObjectByIndex(RyanJson_t pJson, uint32_t index)
{
	RyanJsonCheckReturnNull(NULL != pJson);

	RyanJsonCheckReturnNull(_checkType(pJson, RyanJsonTypeArray) || _checkType(pJson, RyanJsonTypeObject));

	RyanJson_t nextItem = RyanJsonGetObjectValue(pJson);
	RyanJsonCheckReturnNull(NULL != nextItem);
	while (index > 0)
	{
		index--;
		nextItem = RyanJsonGetNext(nextItem);
		RyanJsonCheckReturnNull(NULL != nextItem);
	}

	return nextItem;
}

/**
 * @brief 按 key 获取 Object 子节点
 *
 * @param pJson Object 节点
 * @param key 目标 key
 * @return RyanJson_t 命中节点，失败返回 NULL
 */
RyanJson_t RyanJsonGetObjectByKey(RyanJson_t pJson, const char *key)
{
	RyanJsonCheckReturnNull(NULL != pJson && NULL != key);
	return RyanJsonFindNodeByKey(pJson, key, NULL);
}

/**
 * @brief 按索引分离子节点（不释放）
 *
 * @param pJson 父节点（Array 或 Object）
 * @param index 子节点索引（Object 场景为插入顺序）
 * @return RyanJson_t 被分离节点，失败返回 NULL
 * @note 返回的节点已成为游离节点，需由调用方负责释放。
 */
RyanJson_t RyanJsonDetachByIndex(RyanJson_t pJson, uint32_t index)
{
	RyanJsonCheckReturnNull(NULL != pJson);

	RyanJsonCheckReturnNull(_checkType(pJson, RyanJsonTypeArray) || _checkType(pJson, RyanJsonTypeObject));

	RyanJson_t prev = NULL;
	RyanJson_t nextItem = RyanJsonGetObjectValue(pJson);
	RyanJsonCheckReturnNull(NULL != nextItem);

	while (index > 0)
	{
		prev = nextItem;
		nextItem = RyanJsonGetNext(nextItem);
		index--;
		RyanJsonCheckReturnNull(NULL != nextItem);
	}

	// 维护线索化链表关系
	RyanJson_t trueNext = RyanJsonGetNext(nextItem);

	if (NULL != prev)
	{
		prev->next = trueNext;
		// trueNext 为 NULL 表示 nextItem 原本是尾节点。
		// 分离后 prev 成为新尾节点，需要回指父节点并设置 IsLast。
		if (NULL == trueNext)
		{
			RyanJsonSetPayloadIsLastByFlag(prev, 1);
			prev->next = pJson; // 指向父节点
		}
	}
	else
	{
		RyanJsonInternalChangeObjectValue(pJson, trueNext);
	}

	nextItem->next = NULL;
	RyanJsonSetPayloadIsLastByFlag(nextItem, 0);

	return nextItem;
}

/**
 * @brief 按 key 分离 Object 子节点（不释放）
 *
 * @param pJson Object 节点
 * @param key 目标 key
 * @return RyanJson_t 被分离节点，失败返回 NULL
 * @note 返回的节点已成为游离节点，需由调用方负责释放。
 */
RyanJson_t RyanJsonDetachByKey(RyanJson_t pJson, const char *key)
{
	RyanJsonCheckReturnNull(NULL != pJson && NULL != key);

	RyanJson_t prev = NULL;
	RyanJson_t nextItem = RyanJsonFindNodeByKey(pJson, key, &prev);
	RyanJsonCheckReturnNull(NULL != nextItem);

	// 维护线索化链表关系
	RyanJson_t trueNext = RyanJsonGetNext(nextItem);

	if (NULL != prev)
	{
		prev->next = trueNext;
		// trueNext 为 NULL 表示 nextItem 原本是尾节点。
		// 分离后 prev 成为新尾节点，需要回指父节点并设置 IsLast。
		if (NULL == trueNext)
		{
			RyanJsonSetPayloadIsLastByFlag(prev, 1);
			prev->next = pJson; // 指向父节点
		}
	}
	else // 分离的是首节点
	{
		RyanJsonInternalChangeObjectValue(pJson, trueNext);
	}

	nextItem->next = NULL;
	RyanJsonSetPayloadIsLastByFlag(nextItem, 0);

	return nextItem;
}

/**
 * @brief 按索引删除子节点
 *
 * @param pJson 父节点（Array 或 Object）
 * @param index 子节点索引（Object 场景为插入顺序）
 * @return RyanJsonBool_e 删除是否成功
 */
RyanJsonBool_e RyanJsonDeleteByIndex(RyanJson_t pJson, uint32_t index)
{
	RyanJsonCheckReturnFalse(NULL != pJson);

	RyanJson_t nextItem = RyanJsonDetachByIndex(pJson, index);
	RyanJsonCheckReturnFalse(NULL != nextItem);

	RyanJsonDelete(nextItem);
	return RyanJsonTrue;
}

/**
 * @brief 按 key 删除 Object 子节点
 *
 * @param pJson Object 节点
 * @param key 目标 key
 * @return RyanJsonBool_e 删除是否成功
 */
RyanJsonBool_e RyanJsonDeleteByKey(RyanJson_t pJson, const char *key)
{
	RyanJsonCheckReturnFalse(NULL != pJson && NULL != key);

	RyanJson_t nextItem = RyanJsonDetachByKey(pJson, key);
	RyanJsonCheckReturnFalse(NULL != nextItem);

	RyanJsonDelete(nextItem);
	return RyanJsonTrue;
}

/**
 * @brief 向容器按索引插入子节点
 *
 * @param pJson 父节点（Array 或 Object）
 * @param index 插入位置，超范围等价尾插
 * @param item 待插入节点（要求游离）
 * @return RyanJsonBool_e 插入是否成功
 * @note Object 场景要求 item 携带 key，严格模式下会拒绝重复 key。
 * @note 进入 error__ 分支时会释放 item；若 item 为 NULL 或非游离节点，将直接失败且不释放 item。
 */
RyanJsonBool_e RyanJsonInsert(RyanJson_t pJson, uint32_t index, RyanJson_t item)
{
	RyanJson_t nextItem = NULL;
	RyanJson_t prev = NULL;

	RyanJsonCheckReturnFalse(NULL != item);
	RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonIsDetachedItem(item));
	RyanJsonCheckCode(NULL != pJson, { goto error__; });

	RyanJsonCheckCode(_checkType(pJson, RyanJsonTypeArray) || (_checkType(pJson, RyanJsonTypeObject) && RyanJsonIsKey(item)), {
		jsonLog("error__ 不是正确类型 %d\r\n", index);
		goto error__;
	});

	// 严格模式下：Object 从插入入口拒绝重复 key
#if true == RyanJsonStrictObjectKeyCheck
	if (_checkType(pJson, RyanJsonTypeObject))
	{
		RyanJsonCheckCode(RyanJsonFalse == RyanJsonObjectHasKeyConflict(pJson, RyanJsonGetKey(item), NULL), { goto error__; });
	}
#endif

	nextItem = RyanJsonGetObjectValue(pJson);
	while (nextItem && index > 0)
	{
		prev = nextItem;
		nextItem = RyanJsonGetNext(nextItem);
		index--;
	}

	if (NULL != prev) { RyanJsonInternalListInsertAfter(pJson, prev, item); }
	else
	{
		// prev 为 NULL 表示头插，交给统一链表插入函数处理
		RyanJsonInternalListInsertAfter(pJson, NULL, item);
	}

	return RyanJsonTrue;

error__:
	RyanJsonDelete(item);
	return RyanJsonFalse;
}
