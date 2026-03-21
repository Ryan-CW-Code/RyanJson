#include "RyanJsonInternal.h"

/**
 * @brief 全局内存钩子。
 * @note 由 RyanJsonInitHooks 在运行前初始化。
 */
RyanJsonMalloc_t jsonMalloc = NULL;
RyanJsonFree_t jsonFree = NULL;
RyanJsonRealloc_t jsonRealloc = NULL;

/**
 * @brief 初始化内存钩子（malloc/free/realloc）
 *
 * @param userMalloc 用户自定义 malloc
 * @param userFree 用户自定义 free
 * @param userRealloc 用户自定义 realloc，可为 NULL
 * @return RyanJsonBool_e 初始化是否成功
 * @note 除编译期配置外，其他 API 调用前必须先完成初始化。
 */
RyanJsonBool_e RyanJsonInitHooks(RyanJsonMalloc_t userMalloc, RyanJsonFree_t userFree, RyanJsonRealloc_t userRealloc)
{
	RyanJsonCheckReturnFalse(NULL != userMalloc && NULL != userFree);

	jsonMalloc = userMalloc;
	jsonFree = userFree;
	jsonRealloc = userRealloc;
	return RyanJsonTrue;
}

/**
 * @brief 释放 RyanJson 动态分配的内存块
 *
 * @param block 待释放内存
 * @note 需已通过 RyanJsonInitHooks 注册内存钩子。
 */
void RyanJsonFree(void *block)
{
	jsonFree(block);
}

/**
 * @brief 扩容内存块（优先使用 hooks 中的 realloc）
 *
 * @param block 原始内存块
 * @param oldSize 原始大小
 * @param newSize 新大小
 * @return void* 扩容后的内存地址，失败返回 NULL
 */
RyanJsonInternalApi void *RyanJsonInternalExpandRealloc(void *block, uint32_t oldSize, uint32_t newSize)
{
	// 不考虑 block 为空的情况
	RyanJsonCheckAssert(NULL != block);
	if (NULL != jsonRealloc) { return jsonRealloc(block, newSize); }

	void *newBlock = jsonMalloc(newSize);
	RyanJsonCheckReturnNull(NULL != newBlock);

	RyanJsonMemcpy(newBlock, block, oldSize);
	jsonFree(block);
	return newBlock;
}

/**
 * @brief 删除 Json 树并释放所有资源
 *
 * @param pJson 待删除的根节点
 */
void RyanJsonDelete(RyanJson_t pJson)
{
	RyanJsonCheckCode(NULL != pJson, { return; });

	RyanJson_t current = pJson;
	RyanJson_t nextNode;
	while (NULL != current)
	{
		// 容器优先下沉：如果有子节点，先剥离并优先处理子节点
		if (_checkType(current, RyanJsonTypeArray) || _checkType(current, RyanJsonTypeObject))
		{
			nextNode = RyanJsonGetObjectValue(current);
			if (nextNode)
			{
				RyanJsonInternalChangeObjectValue(current, NULL); // 断开子节点连接，防止循环回溯
				current = nextNode;
				continue;
			}
		}

		// 确定后续节点：根节点结束；非根节点通过 next 串联待处理路径
		// 注意：这里不能用 RyanJsonGetNext，因为 Last 节点会返回 NULL，
		// 但删除流程需要利用线索 next 回溯到父节点
		nextNode = (current == pJson) ? NULL : current->next;

		// 释放当前节点资源
		// 如果 strValue 区采用指针模式存储，需先释放外部堆空间
		if (RyanJsonTrue == RyanJsonGetPayloadStrIsPtrByFlag(current)) { jsonFree(RyanJsonInternalGetStrPtrModeBuf(current)); }
		jsonFree(current);
		current = nextNode;
	}
}

/**
 * @brief 获取节点规模（标量为 1，容器为子节点个数）
 *
 * @param pJson 待查询节点
 * @return uint32_t 元素数量；参数非法返回 0
 */
uint32_t RyanJsonGetSize(RyanJson_t pJson)
{
	RyanJsonCheckCode(NULL != pJson, { return 0; });

	if (!_checkType(pJson, RyanJsonTypeArray) && !_checkType(pJson, RyanJsonTypeObject)) { return 1; }

	RyanJson_t nextItem = RyanJsonGetObjectValue(pJson);
	uint32_t size = 0;
	while (NULL != nextItem)
	{
		size++;
		nextItem = RyanJsonGetNext(nextItem);
	}

	return size;
}

/**
 * @brief 复制单个节点（不递归复制子节点）
 *
 * @param pJson 源节点
 * @return RyanJson_t 新节点，失败返回 NULL
 */
static RyanJson_t RyanJsonDuplicateNode(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);

	char *key = RyanJsonIsKey(pJson) ? RyanJsonGetKey(pJson) : NULL;
	RyanJson_t newItem = NULL;

	switch (RyanJsonGetType(pJson))
	{
	case RyanJsonTypeNull: newItem = RyanJsonCreateNull(key); break;
	case RyanJsonTypeBool: // 创建节点时已写入 Bool 值
		newItem = RyanJsonCreateBool(key, RyanJsonGetBoolValue(pJson));
		break;

	case RyanJsonTypeNumber:
		if (RyanJsonIsInt(pJson)) { newItem = RyanJsonCreateInt(key, RyanJsonGetIntValue(pJson)); }
		else
		{
			// Number 节点除 int32_t 外只可能是 Double
			RyanJsonCheckAssert(RyanJsonIsDouble(pJson));
			newItem = RyanJsonCreateDouble(key, RyanJsonGetDoubleValue(pJson));
		}
		break;
	case RyanJsonTypeString: newItem = RyanJsonCreateString(key, RyanJsonGetStringValue(pJson)); break;
	case RyanJsonTypeArray: newItem = RyanJsonInternalCreateArrayAndKey(key); break;
	case RyanJsonTypeObject: newItem = RyanJsonInternalCreateObjectAndKey(key); break;
	}
	return newItem;
}

/**
 * @brief 深拷贝整棵 Json 树（迭代版）
 *
 * @param pJson 源 Json 根节点
 * @return RyanJson_t 拷贝后的根节点，失败返回 NULL
 */
RyanJson_t RyanJsonDuplicate(RyanJson_t pJson)
{
	RyanJsonCheckReturnNull(NULL != pJson);

	// 先复制根节点
	RyanJson_t root = RyanJsonDuplicateNode(pJson);
	RyanJsonCheckReturnNull(NULL != root);

	// 初始化迭代状态
	// sourceNode：当前遍历到的源节点，初始指向根节点的首个子节点
	RyanJson_t sourceNode = NULL;

	if (_checkType(pJson, RyanJsonTypeArray) || _checkType(pJson, RyanJsonTypeObject)) { sourceNode = RyanJsonGetObjectValue(pJson); }

	// 如果根节点不是容器类型（Array 或 Object），或者容器为空（没有子节点），
	// 则不需要进行后续的子节点复制，直接返回根节点副本即可。
	// RyanJsonGetObjectValue 返回容器的子节点起始指针。
	if (NULL == sourceNode) { return root; }

	// targetParent：目标树中当前插入位置的父节点
	RyanJson_t targetParent = root;
	// lastSibling：目标树当前层级里最近插入的兄弟节点
	RyanJson_t lastSibling = NULL;

	while (1)
	{
		// 复制当前节点并插入目标树
		{
			RyanJson_t newItem = RyanJsonDuplicateNode(sourceNode);
			RyanJsonCheckCode(NULL != newItem, { goto error__; });

			// 新节点插入到目标父节点下，位置在 lastSibling 之后
			// RyanJsonInternalListInsertAfter 会维护链表连接与 IsLast 标记
			RyanJsonInternalListInsertAfter(targetParent, lastSibling, newItem);
			lastSibling = newItem; // 更新 lastSibling 为刚插入的节点
		}

		// 当前节点是非空容器时下沉到子层
		// 如果当前源节点是容器且非空，则进入下一层级
		if (_checkType(sourceNode, RyanJsonTypeArray) || _checkType(sourceNode, RyanJsonTypeObject))
		{
			RyanJson_t child = RyanJsonGetObjectValue(sourceNode);
			if (child)
			{
				sourceNode = child;         // 移动到子节点
				targetParent = lastSibling; // 上一步插入的新节点成为下一层的父节点
				lastSibling = NULL;         // 新层级尚未插入子节点
				continue;                   // 继续循环处理子节点
			}
		}

		// 同层前进，必要时回溯到父层
		while (1)
		{
			// 优先尝试移动到下一个兄弟节点
			RyanJson_t next = RyanJsonGetNext(sourceNode);
			if (next)
			{
				sourceNode = next; // 移动到兄弟节点
				break;             // 跳出内层循环，回到外层循环进行复制
			}

			// 无兄弟节点（当前为 Last）时沿线索回溯到父节点
			// 线索化链表中，IsLast=1 的节点其 next 指向父节点
			sourceNode = sourceNode->next;

			// 回溯到起始根节点，说明整棵树遍历完成
			if (sourceNode == pJson) { return root; }

			// 目标树同步回溯到上一层
			// 此时 targetParent 是当前层的父节点，它的所有子节点都已复制完毕。
			// 我们需要回到 targetParent 的父节点层级。
			// 在进入这一层时，targetParent 是作为 lastSibling 被记录的。
			// 也就是：上一层的 lastSibling 就是现在的 targetParent。
			lastSibling = targetParent;

			// 同步遍历下 targetParent 也一定是 Last 节点，
			// 因而它的 next 同样指向父节点。
			targetParent = targetParent->next;
		}
	}

error__:
	RyanJsonDelete(root);
	return NULL;
}

/**
 * @brief 原地压缩 Json 文本（移除空白与注释）
 *
 * @param text 可写缓冲区
 * @param textLen 缓冲区可写长度
 * @return uint32_t 压缩后字符数（不含终止符）
 * @note 支持移除 `//` 与 `/ * ... * /` 形式的注释（非标准 JSON）。
 * @note 仅当返回值小于 textLen 时写入 '\0'。
 * @note 若缓冲区中存在 '\0'，会在该处提前停止处理。
 */
uint32_t RyanJsonMinify(char *text, int32_t textLen)
{
	RyanJsonCheckCode(NULL != text && textLen > 0, { return 0; });

	char *t = (char *)text;           // 写指针
	const char *end = text + textLen; // 边界
	uint32_t count = 0;               // 压缩后字符数

	while (text < end && *text)
	{
		if (' ' == *text || '\t' == *text || '\r' == *text || '\n' == *text) { text++; }
		else if ('/' == *text && (text + 1 < end) && '/' == text[1])
		{
			while (text < end && *text && '\n' != *text)
			{
				text++;
			}
		}
		else if ('/' == *text && (text + 1 < end) && '*' == text[1])
		{
			text += 2;
			while (text < end && *text && !('*' == *text && (text + 1 < end) && '/' == text[1]))
			{
				text++;
			}
			if (text + 1 < end) { text += 2; }
		}
		else if ('\"' == *text)
		{
			*t++ = *text++;
			count++;
			while (text < end && *text && '\"' != *text)
			{
				if ('\\' == *text && text + 1 < end)
				{
					*t++ = *text++;
					count++;
				}
				*t++ = *text++;
				count++;
			}
			if (text < end)
			{
				*t++ = *text++;
				count++;
			}
		}
		else
		{
			*t++ = *text++;
			count++;
		}
	}

	// 仅当缓冲区仍有空间时写入终止符，避免越界写
	if (t < end) { *t = '\0'; }
	return count; // 返回压缩后大小
}

/**
 * @brief Json 内部比较函数（支持全量比较/仅 Key 比较）
 *
 * @param leftJson 左侧节点
 * @param rightJson 右侧节点
 * @param fullCompare RyanJsonTrue 比较值，RyanJsonFalse 仅比较结构与 key
 * @return RyanJsonBool_e 两棵树是否相等
 */
static RyanJsonBool_e RyanJsonInternalCompare(RyanJson_t leftJson, RyanJson_t rightJson, RyanJsonBool_e fullCompare)
{
	RyanJsonCheckReturnFalse(NULL != leftJson && NULL != rightJson);

	if (leftJson == rightJson) { return RyanJsonTrue; }

	RyanJson_t leftCurrent = leftJson;
	RyanJson_t rightCurrent = rightJson;

	while (1)
	{
		// 比较当前节点的类型、值与规模
		RyanJsonType_e leftType = RyanJsonGetType(leftCurrent);
		RyanJsonCheckReturnFalse(leftType == RyanJsonGetType(rightCurrent));

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif
		switch (leftType)
		{
		case RyanJsonTypeNull: break;
		case RyanJsonTypeBool:
			if (fullCompare)
			{
				RyanJsonCheckReturnFalse(RyanJsonGetBoolValue(leftCurrent) == RyanJsonGetBoolValue(rightCurrent));
			}
			break;
		case RyanJsonTypeNumber:
			if (fullCompare)
			{
				if (RyanJsonIsInt(leftCurrent))
				{
					RyanJsonCheckReturnFalse(RyanJsonGetIntValue(leftCurrent) == RyanJsonGetIntValue(rightCurrent));
				}
				else
				{
					RyanJsonCheckReturnFalse(RyanJsonCompareDouble(RyanJsonGetDoubleValue(leftCurrent),
										       RyanJsonGetDoubleValue(rightCurrent)));
				}
			}
			break;
		case RyanJsonTypeString:
			if (fullCompare)
			{
				RyanJsonCheckReturnFalse(
					RyanJsonInternalStrEq(RyanJsonGetStringValue(leftCurrent), RyanJsonGetStringValue(rightCurrent)));
			}
			break;
		case RyanJsonTypeArray:
		case RyanJsonTypeObject: RyanJsonCheckReturnFalse(RyanJsonGetSize(leftCurrent) == RyanJsonGetSize(rightCurrent)); break;
		default: return RyanJsonFalse;
		}
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

		// 容器节点尝试下沉到子节点继续比较
		if (_checkType(leftCurrent, RyanJsonTypeArray) || _checkType(leftCurrent, RyanJsonTypeObject))
		{
			RyanJson_t leftChild = RyanJsonGetObjectValue(leftCurrent);
			if (leftChild)
			{
				RyanJson_t rightChild = NULL;

				if (RyanJsonIsArray(leftCurrent)) { rightChild = RyanJsonGetObjectValue(rightCurrent); }
				else
				{
					RyanJsonCheckAssert(RyanJsonTrue == RyanJsonIsKey(leftChild));
					const char *leftChildKey = RyanJsonGetKey(leftChild);

					// 同序快路径：首子节点 key 一致时直接下沉，避免一次 O(n) 查找
					RyanJson_t rightFirstChild = RyanJsonGetObjectValue(rightCurrent);
					RyanJsonCheckAssert(NULL != rightFirstChild && RyanJsonTrue == RyanJsonIsKey(rightFirstChild));
					if (RyanJsonTrue == RyanJsonInternalStrEq(leftChildKey, RyanJsonGetKey(rightFirstChild)))
					{
						rightChild = rightFirstChild;
					}
					else
					{
						rightChild = RyanJsonGetObjectByKey(rightCurrent, leftChildKey);
					}
				}

				RyanJsonCheckReturnFalse(NULL != rightChild);

				leftCurrent = leftChild;
				rightCurrent = rightChild;
				continue;
			}
		}

		// 走到这里说明当前节点对已经比较完成，且若它们是容器，则其子树也已经比较完毕。
		// 接下来要在“不使用递归栈”的前提下，找到 DFS 意义上的下一个待比较节点对：
		// - 优先尝试同父层的下一个兄弟；
		// - 当前层没有兄弟时，沿内部 parent 回链逐层回溯；
		// - 回溯到入口 root 时，说明整棵树已经完全比较结束。
		while (1)
		{
			// 进入本循环时，leftCurrent/rightCurrent 表示“刚完成比较的节点对”。
			// 如果它已经回到入口 root，且前面也没有可继续下沉的子节点，
			// 则表示从 root 出发的整棵树都已匹配完成，可以直接返回 True。
			if (leftCurrent == leftJson) { return RyanJsonTrue; }

			// 先看左侧当前节点是否还有同父兄弟。
			// 有的话，下一个比较目标就必须切到这个兄弟；右侧则去找“语义上与它对应”的兄弟。
			RyanJson_t leftNext = RyanJsonGetNext(leftCurrent);
			if (leftNext)
			{
				RyanJson_t rightNext = NULL;
				if (RyanJsonFalse == RyanJsonIsKey(leftNext))
				{
					// 无 key 节点只会出现在 Array 路径上，Array 比较是严格按顺序进行的，
					// 因而右侧可以直接取当前节点的 next 兄弟，不需要额外查找。
					rightNext = RyanJsonGetNext(rightCurrent);
				}
				else
				{
					RyanJsonCheckAssert(RyanJsonTrue == RyanJsonIsKey(leftNext));
					const char *leftNextKey = RyanJsonGetKey(leftNext);
					RyanJson_t rightParent = RyanJsonInternalGetParent(rightCurrent);

					// Object 比较是“同层无序、同 key 对齐”语义，不能像 Array 那样直接依赖 rightCurrent->next：
					// - strict 模式下 key 唯一，按 key 找即可；
					// - non-strict 模式下允许重复 key，必须继续保证“同 key 的第 N 次出现”彼此对齐，
					//   否则会把后一个重复 key 错配到右侧第一个同名节点，导致 Compare/CompareOnlyKey 语义漂移。
					RyanJsonCheckAssert(NULL != rightParent && RyanJsonTrue == RyanJsonIsObject(rightParent));
#if true == RyanJsonStrictObjectKeyCheck
					// strict 模式下 key 唯一，直接按 key 查找即可。
					rightNext = RyanJsonGetObjectByKey(rightParent, leftNextKey);
#else
					// 非严格模式下允许重复 key，按“同 key + 相同出现序号”精确匹配。
					RyanJson_t leftParent = RyanJsonInternalGetParent(leftCurrent);
					RyanJsonCheckAssert(NULL != leftParent && RyanJsonTrue == RyanJsonIsObject(leftParent));

					// 先在左父节点里统计 leftNext 是该 key 的第几次出现（0-based）。
					// 例如 leftParent 子节点是 a,b,a,c，则第二个 a 的序号是 1。
					uint32_t leftSameKeyIndex = 0;
					RyanJson_t leftScan = RyanJsonGetObjectValue(leftParent);
					RyanJsonCheckAssert(NULL != leftScan);
					while (leftScan != leftNext)
					{
						RyanJsonCheckAssert(RyanJsonTrue == RyanJsonIsKey(leftScan));
						if (RyanJsonTrue == RyanJsonInternalStrEq(leftNextKey, RyanJsonGetKey(leftScan)))
						{
							leftSameKeyIndex++;
						}
						leftScan = RyanJsonGetNext(leftScan);
						RyanJsonCheckAssert(NULL != leftScan);
					}

					// 再在右父节点里找到“同 key 的同序号节点”。
					// 这样重复 key 的比较就不是“命中任意一个同名节点”，而是稳定地按出现次序对齐。
					uint32_t rightSameKeyIndex = 0;
					RyanJson_t rightScan = RyanJsonGetObjectValue(rightParent);
					while (rightScan)
					{
						RyanJsonCheckAssert(RyanJsonTrue == RyanJsonIsKey(rightScan));
						if (RyanJsonTrue == RyanJsonInternalStrEq(leftNextKey, RyanJsonGetKey(rightScan)))
						{
							if (rightSameKeyIndex == leftSameKeyIndex)
							{
								rightNext = rightScan;
								break;
							}
							rightSameKeyIndex++;
						}
						rightScan = RyanJsonGetNext(rightScan);
					}
#endif
				}

				RyanJsonCheckReturnFalse(NULL != rightNext);

				leftCurrent = leftNext;
				rightCurrent = rightNext;
				break;
			}

			// 当前层已没有可比较的兄弟，开始回溯。
			// 这里故意不用 RyanJsonGetNext(leftCurrent)：
			// - 公开语义下，没有兄弟时它会返回 NULL；
			// - 但内部链表约定中，“最后一个兄弟节点的 next”挂的是父节点，
			//   正是这里做迭代式 DFS 回溯所需要的那条内部回链。
			leftCurrent = leftCurrent->next;

			// leftCurrent 此时已经被抬回父节点，所以这里按“父节点类型”决定右侧如何回溯。
			// - Array：左右两边始终按相同 sibling 顺序推进，rightCurrent->next 正好也挂回父节点；
			// - Object：右侧当前节点可能是按 key/序号匹配得到的任意兄弟，未必处在链表尾部，
			//   这时不能假设 rightCurrent->next 是父节点，只能显式调用 RyanJsonInternalGetParent。
			if (RyanJsonIsArray(leftCurrent)) { rightCurrent = rightCurrent->next; }
			else
			{
				rightCurrent = RyanJsonInternalGetParent(rightCurrent);
			}
		}
	}
}

/**
 * @brief 比较两个 Json 是否完全相等（结构 + key + value）
 *
 * @param leftJson 左侧节点
 * @param rightJson 右侧节点
 * @return RyanJsonBool_e 是否相等
 */
RyanJsonBool_e RyanJsonCompare(RyanJson_t leftJson, RyanJson_t rightJson)
{
	return RyanJsonInternalCompare(leftJson, rightJson, RyanJsonTrue);
}

/**
 * @brief 比较两个 Json 的结构与 key 是否相等（忽略 value）
 *
 * @param leftJson 左侧节点
 * @param rightJson 右侧节点
 * @return RyanJsonBool_e 是否相等
 */
RyanJsonBool_e RyanJsonCompareOnlyKey(RyanJson_t leftJson, RyanJson_t rightJson)
{
	return RyanJsonInternalCompare(leftJson, rightJson, RyanJsonFalse);
}
