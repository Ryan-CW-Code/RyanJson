#include "testBase.h"

static void testScenarioCompositeDetachReplaceInsertAndRoundtrip(void)
{
	// 复杂链路：Parse -> Duplicate -> Detach -> Change -> Replace -> Insert -> Print -> Parse。
	// 目标：覆盖“数组与对象之间的协同变更”以及往返一致性。
	const char *source = "{\"jobs\":[{\"id\":\"a\",\"state\":\"new\"},{\"id\":\"b\",\"state\":\"new\"}],"
			     "\"byId\":{\"a\":{\"state\":\"new\"},\"b\":{\"state\":\"new\"}},\"meta\":{\"ver\":1}}";

	RyanJson_t root = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "复杂链路样本解析失败");

	RyanJson_t work = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL_MESSAGE(work, "复杂链路样本 Duplicate 失败");

	RyanJson_t jobs = RyanJsonGetObjectToKey(work, "jobs");
	RyanJson_t byId = RyanJsonGetObjectToKey(work, "byId");
	TEST_ASSERT_NOT_NULL(jobs);
	TEST_ASSERT_NOT_NULL(byId);
	TEST_ASSERT_TRUE(RyanJsonIsArray(jobs));
	TEST_ASSERT_TRUE(RyanJsonIsObject(byId));

	RyanJson_t detached = RyanJsonDetachByIndex(jobs, 1);
	TEST_ASSERT_NOT_NULL_MESSAGE(detached, "从 jobs 分离第二项失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeStringValue(RyanJsonGetObjectToKey(detached, "state"), "running"), "修改分离项状态失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(work, "meta", "ver"), 2), "修改版本号失败");

	RyanJson_t detachedCopy = RyanJsonDuplicate(detached);
	TEST_ASSERT_NOT_NULL(detachedCopy);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(byId, "b", detachedCopy), "按 key 替换 byId.b 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(jobs, 0, detached), "将分离项重新插入 jobs 头部失败");

	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(jobs));
	TEST_ASSERT_EQUAL_STRING("b", RyanJsonGetStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectByIndex(jobs, 0), "id")));
	TEST_ASSERT_EQUAL_STRING("running", RyanJsonGetStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectByIndex(jobs, 0), "state")));
	TEST_ASSERT_EQUAL_STRING("a", RyanJsonGetStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectByIndex(jobs, 1), "id")));
	TEST_ASSERT_EQUAL_STRING("new", RyanJsonGetStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectByIndex(jobs, 1), "state")));
	TEST_ASSERT_EQUAL_STRING("running", RyanJsonGetStringValue(RyanJsonGetObjectToKey(work, "byId", "b", "state")));
	TEST_ASSERT_EQUAL_INT(2, RyanJsonGetIntValue(RyanJsonGetObjectToKey(work, "meta", "ver")));

	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(root, work), "变更后的 work 不应与 root 相等");

	char *printed = RyanJsonPrint(work, 256, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(printed, "复杂链路打印失败");
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL_MESSAGE(roundtrip, "复杂链路往返解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(work, roundtrip), "复杂链路往返 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(work);
	RyanJsonDelete(root);
}

static void testScenarioCompositeMoveAndRenameDetachedKeyNode(void)
{
	// 复杂链路：Parse -> DetachByKey -> ChangeKey/ChangeStringValue -> Insert(Object) -> DeleteByIndex -> ReplaceByKey -> Roundtrip。
	// 目标：覆盖“对象节点分离后重命名并迁移到另一对象”以及多点修改后的一致性检查。
	const char *source = "{\"spec\":{\"owner\":\"ops\",\"retry\":3},\"profile\":{\"team\":\"core\"},\"audit\":[1,2]}";

	RyanJson_t root = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "复杂迁移样本解析失败");
	RyanJson_t snapshot = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL_MESSAGE(snapshot, "复杂迁移样本快照失败");

	RyanJson_t spec = RyanJsonGetObjectToKey(root, "spec");
	RyanJson_t profile = RyanJsonGetObjectToKey(root, "profile");
	RyanJson_t audit = RyanJsonGetObjectToKey(root, "audit");
	TEST_ASSERT_NOT_NULL(spec);
	TEST_ASSERT_NOT_NULL(profile);
	TEST_ASSERT_NOT_NULL(audit);
	TEST_ASSERT_TRUE(RyanJsonIsObject(spec));
	TEST_ASSERT_TRUE(RyanJsonIsObject(profile));
	TEST_ASSERT_TRUE(RyanJsonIsArray(audit));

	RyanJson_t detachedOwner = RyanJsonDetachByKey(spec, "owner");
	TEST_ASSERT_NOT_NULL_MESSAGE(detachedOwner, "分离 spec.owner 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDetachedItem(detachedOwner), "分离后的 owner 节点应为游离状态");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeStringValue(detachedOwner, "platform"), "修改 owner 字符串失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeKey(detachedOwner, "ownerAlias"), "重命名 owner key 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(profile, 0, detachedOwner), "插入 profile.ownerAlias 失败");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonIsDetachedItem(detachedOwner), "插入后节点不应保持游离状态");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonDeleteByIndex(audit, 1), "删除 audit[1] 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(spec, "retry", RyanJsonCreateInt("retry", 5)), "替换 spec.retry 失败");

	TEST_ASSERT_EQUAL_STRING("platform", RyanJsonGetStringValue(RyanJsonGetObjectToKey(root, "profile", "ownerAlias")));
	TEST_ASSERT_NULL_MESSAGE(RyanJsonGetObjectToKey(root, "spec", "owner"), "spec.owner 已迁移后应不存在");
	TEST_ASSERT_EQUAL_INT(5, RyanJsonGetIntValue(RyanJsonGetObjectToKey(root, "spec", "retry")));
	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetArraySize(audit));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(root, snapshot), "迁移修改后 root 不应与 snapshot 相等");

	char *printed = RyanJsonPrint(root, 160, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(printed, "复杂迁移链路打印失败");
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL_MESSAGE(roundtrip, "复杂迁移链路往返解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, roundtrip), "复杂迁移链路往返 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(snapshot);
	RyanJsonDelete(root);
}

static void testScenarioCompositeArrayObjectSyncWithForeachAndExpectedCompare(void)
{
	// 复杂链路：Parse -> DetachByIndex -> Change -> AddItem(Array/Object) -> ReplaceByIndex -> DeleteByIndex -> Insert -> ForEach -> Compare。
	// 目标：验证“数组队列 + 对象索引”双视图在多步变更后仍保持同步语义。
	const char *source =
		"{\"queue\":[{\"id\":\"a\",\"prio\":1},{\"id\":\"b\",\"prio\":2},{\"id\":\"c\",\"prio\":3}],\"done\":[],\"map\":{}}";
	const char *expectText =
		"{\"queue\":[{\"id\":\"a2\",\"prio\":10},{\"id\":\"tail\",\"prio\":7}],\"done\":[{\"id\":\"b\",\"prio\":9}],"
		"\"map\":{\"b\":{\"id\":\"b\",\"prio\":9}}}";

	RyanJson_t root = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "队列同步样本解析失败");

	RyanJson_t queue = RyanJsonGetObjectToKey(root, "queue");
	RyanJson_t done = RyanJsonGetObjectToKey(root, "done");
	RyanJson_t map = RyanJsonGetObjectToKey(root, "map");
	TEST_ASSERT_NOT_NULL(queue);
	TEST_ASSERT_NOT_NULL(done);
	TEST_ASSERT_NOT_NULL(map);

	RyanJson_t taskB = RyanJsonDetachByIndex(queue, 1);
	TEST_ASSERT_NOT_NULL_MESSAGE(taskB, "分离 queue[1] 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeIntValue(RyanJsonGetObjectToKey(taskB, "prio"), 9), "修改 taskB.prio 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToArray(done, taskB), "挂载 taskB 到 done 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToObject(map, "b", RyanJsonDuplicate(RyanJsonGetObjectByIndex(done, 0))),
				 "挂载 map.b 失败");

	RyanJson_t queueHead = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(queueHead);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(queueHead, "id", "a2"));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(queueHead, "prio", 10));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByIndex(queue, 0, queueHead), "替换 queue[0] 失败");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonDeleteByIndex(queue, 1), "删除 queue[1] 失败");

	RyanJson_t queueTail = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(queueTail);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(queueTail, "id", "tail"));
	TEST_ASSERT_TRUE(RyanJsonAddIntToObject(queueTail, "prio", 7));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(queue, 1, queueTail), "插入 queue[1] 失败");

	uint32_t queueCount = 0;
	int32_t queuePrioSum = 0;
	RyanJson_t item = NULL;
	RyanJsonArrayForEach(queue, item)
	{
		queueCount++;
		queuePrioSum += RyanJsonGetIntValue(RyanJsonGetObjectToKey(item, "prio"));
	}
	TEST_ASSERT_EQUAL_UINT32(2U, queueCount);
	TEST_ASSERT_EQUAL_INT(17, queuePrioSum);

	uint32_t doneCount = 0;
	RyanJsonArrayForEach(done, item)
	{
		doneCount++;
		TEST_ASSERT_EQUAL_STRING("b", RyanJsonGetStringValue(RyanJsonGetObjectToKey(item, "id")));
	}
	TEST_ASSERT_EQUAL_UINT32(1U, doneCount);

	RyanJson_t expect = RyanJsonParse(expectText);
	TEST_ASSERT_NOT_NULL_MESSAGE(expect, "期望文档解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, expect), "多步同步后结果与期望文档不一致");

	char *printed = RyanJsonPrint(root, 192, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL_MESSAGE(printed, "队列同步链路打印失败");
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL_MESSAGE(roundtrip, "队列同步链路往返解析失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, roundtrip), "队列同步链路往返 Compare 应相等");

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(expect);
	RyanJsonDelete(root);
}

static void testScenarioCompositeFailedMutationThenReuseDetachedWithoutCorruption(void)
{
	// 复杂链路：
	// Parse -> ReplaceByKey(失败) -> 复用失败 item 插入数组 -> 重复插入失败
	// -> DetachByIndex -> AddItemToObject 迁移 -> Roundtrip。
	// 目标：
	// 1) 验证失败 API 不会错误消费 item；
	// 2) 验证“失败后复用同一节点”不会引发结构连锁损坏；
	// 3) 验证节点在 attached/detached 状态切换后，容器语义与 Compare 仍稳定。
	const char *source = "{\"obj\":{\"x\":1,\"y\":2},\"arr\":[{\"id\":\"a\"},{\"id\":\"b\"}],\"flag\":true}";

	RyanJson_t root = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "失败恢复样本解析失败");
	RyanJson_t snapshot = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL_MESSAGE(snapshot, "失败恢复样本快照失败");

	RyanJson_t candidate = RyanJsonCreateObject();
	TEST_ASSERT_NOT_NULL(candidate);
	TEST_ASSERT_TRUE(RyanJsonAddStringToObject(candidate, "id", "c"));

	// 先触发失败：不存在的 key 替换失败，item 应仍可复用。
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonReplaceByKey(root, "missing", candidate), "ReplaceByKey(不存在 key) 应失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonIsDetachedItem(candidate), "Replace 失败后 candidate 应保持游离状态");

	RyanJson_t arr = RyanJsonGetObjectToKey(root, "arr");
	TEST_ASSERT_NOT_NULL(arr);
	TEST_ASSERT_TRUE(RyanJsonInsert(arr, 1, candidate));
	TEST_ASSERT_FALSE(RyanJsonIsDetachedItem(candidate));

	// 再触发一次失败：已挂树节点不能重复插入，且应保持原树不被破坏。
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonInsert(arr, 0, candidate), "重复插入已挂树节点应失败");
	TEST_ASSERT_EQUAL_UINT32(3U, RyanJsonGetArraySize(arr));
	TEST_ASSERT_EQUAL_STRING("a", RyanJsonGetStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectByIndex(arr, 0), "id")));
	TEST_ASSERT_EQUAL_STRING("c", RyanJsonGetStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectByIndex(arr, 1), "id")));
	TEST_ASSERT_EQUAL_STRING("b", RyanJsonGetStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectByIndex(arr, 2), "id")));

	RyanJson_t moved = RyanJsonDetachByIndex(arr, 1);
	TEST_ASSERT_NOT_NULL(moved);
	TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(moved));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddItemToObject(RyanJsonGetObjectToKey(root, "obj"), "moved", moved),
				 "迁移 candidate 到 obj.moved 失败");

	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(arr));
	TEST_ASSERT_TRUE(RyanJsonHasObjectToKey(root, "obj", "moved", "id"));
	TEST_ASSERT_EQUAL_STRING("c", RyanJsonGetStringValue(RyanJsonGetObjectToKey(root, "obj", "moved", "id")));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(root, snapshot), "链路变更后 root 不应与快照相等");

	char *printed = RyanJsonPrint(root, 192, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(snapshot);
	RyanJsonDelete(root);
}

static void testScenarioCompositeBatchDetachByCollectedIndexes(void)
{
	// 复杂链路：
	// Parse -> ArrayForEach(收集索引/ID) -> 按逆序 DetachByIndex 批量删除 -> AddStringToArray(doneIds)
	// -> ArrayForEach 二次校验 -> Roundtrip。
	// 目标：
	// 1) 验证“先遍历收集、后批处理”可避免索引漂移引发的连锁错误；
	// 2) 验证批量分离后再写入统计容器，结构与语义保持一致；
	// 3) 验证复杂批处理链在不同编译配置下都稳定。
	const char *source = "{\"tasks\":[{\"id\":\"a\",\"state\":\"new\"},{\"id\":\"b\",\"state\":\"done\"},{\"id\":\"c\",\"state\":"
			     "\"done\"}],\"doneIds\":[]}";
	RyanJson_t root = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "批量迁移样本解析失败");

	RyanJson_t tasks = RyanJsonGetObjectToKey(root, "tasks");
	RyanJson_t doneIds = RyanJsonGetObjectToKey(root, "doneIds");
	TEST_ASSERT_NOT_NULL(tasks);
	TEST_ASSERT_NOT_NULL(doneIds);
	TEST_ASSERT_TRUE(RyanJsonIsArray(tasks));
	TEST_ASSERT_TRUE(RyanJsonIsArray(doneIds));

	uint32_t doneIndexes[4] = {0};
	char doneKeyBuf[4][8] = {{0}};
	uint32_t doneCount = 0;
	uint32_t idx = 0;
	RyanJson_t item = NULL;
	RyanJsonArrayForEach(tasks, item)
	{
		const char *state = RyanJsonGetStringValue(RyanJsonGetObjectToKey(item, "state"));
		const char *id = RyanJsonGetStringValue(RyanJsonGetObjectToKey(item, "id"));
		TEST_ASSERT_NOT_NULL(state);
		TEST_ASSERT_NOT_NULL(id);

		if (strcmp(state, "done") == 0)
		{
			TEST_ASSERT_TRUE_MESSAGE(doneCount < 4U, "doneIndexes 缓冲区不足");
			doneIndexes[doneCount] = idx;
			RyanJsonSnprintf(doneKeyBuf[doneCount], sizeof(doneKeyBuf[doneCount]), "%s", id);
			doneCount++;
		}
		else if (strcmp(state, "new") == 0)
		{
			TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeStringValue(RyanJsonGetObjectToKey(item, "state"), "queued"),
						 "将 new 改写为 queued 失败");
		}
		else
		{
			TEST_FAIL_MESSAGE("tasks 中存在未预期状态值");
		}
		idx++;
	}

	TEST_ASSERT_EQUAL_UINT32(2U, doneCount);
	for (uint32_t i = 0; i < doneCount; i++)
	{
		// 逆序删除，避免前项删除导致后续索引偏移。
		uint32_t deleteIdx = doneIndexes[doneCount - 1U - i];
		RyanJson_t detached = RyanJsonDetachByIndex(tasks, deleteIdx);
		TEST_ASSERT_NOT_NULL_MESSAGE(detached, "按收集索引批量分离失败");
		TEST_ASSERT_TRUE(RyanJsonIsDetachedItem(detached));
		RyanJsonDelete(detached);
	}

	for (uint32_t i = 0; i < doneCount; i++)
	{
		TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddStringToArray(doneIds, doneKeyBuf[i]), "写入 doneIds 失败");
	}

	TEST_ASSERT_EQUAL_UINT32(1U, RyanJsonGetArraySize(tasks));
	TEST_ASSERT_EQUAL_UINT32(2U, RyanJsonGetArraySize(doneIds));
	TEST_ASSERT_EQUAL_STRING("a", RyanJsonGetStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectByIndex(tasks, 0), "id")));
	TEST_ASSERT_EQUAL_STRING("queued", RyanJsonGetStringValue(RyanJsonGetObjectToKey(RyanJsonGetObjectByIndex(tasks, 0), "state")));

	uint32_t doneIdHitB = 0;
	uint32_t doneIdHitC = 0;
	RyanJsonArrayForEach(doneIds, item)
	{
		const char *id = RyanJsonGetStringValue(item);
		TEST_ASSERT_NOT_NULL(id);
		if (strcmp(id, "b") == 0) { doneIdHitB++; }
		else if (strcmp(id, "c") == 0) { doneIdHitC++; }
		else
		{
			TEST_FAIL_MESSAGE("doneIds 中存在未预期元素");
		}
	}
	TEST_ASSERT_EQUAL_UINT32(1U, doneIdHitB);
	TEST_ASSERT_EQUAL_UINT32(1U, doneIdHitC);

	char *printed = RyanJsonPrint(root, 160, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(root);
}

void testScenarioCompositeFlowsRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testScenarioCompositeDetachReplaceInsertAndRoundtrip);
	RUN_TEST(testScenarioCompositeMoveAndRenameDetachedKeyNode);
	RUN_TEST(testScenarioCompositeArrayObjectSyncWithForeachAndExpectedCompare);
	RUN_TEST(testScenarioCompositeFailedMutationThenReuseDetachedWithoutCorruption);
	RUN_TEST(testScenarioCompositeBatchDetachByCollectedIndexes);
}
