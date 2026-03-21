#include "testBase.h"

static void testScenarioSnapshotRollbackByDuplicateSnapshot(void)
{
	// 复杂链路：Parse -> Duplicate(快照) -> 多次跨类型修改 -> 按快照回滚。
	// 目标：验证复杂变更后可通过 Duplicate 快照恢复到原始语义。
	const char *source = "{\"cfg\":{\"mode\":\"a\",\"retry\":3},\"list\":[1,2,3],\"flag\":true,\"name\":\"x\"}";

	RyanJson_t root = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL(root);
	RyanJson_t snapshot = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL(snapshot);

	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "cfg", RyanJsonCreateString("cfg", "flat")));
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "list", RyanJsonCreateArray()));
	TEST_ASSERT_TRUE(RyanJsonAddIntToArray(RyanJsonGetObjectToKey(root, "list"), 9));
	TEST_ASSERT_TRUE(RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(root, "flag"), RyanJsonFalse));
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(RyanJsonGetObjectToKey(root, "name"), "y"));
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(root, snapshot), "修改后 root 不应与快照相等");

	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "cfg", RyanJsonDuplicate(RyanJsonGetObjectToKey(snapshot, "cfg"))));
	TEST_ASSERT_TRUE(RyanJsonReplaceByKey(root, "list", RyanJsonDuplicate(RyanJsonGetObjectToKey(snapshot, "list"))));
	TEST_ASSERT_TRUE(RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(root, "flag"),
						 RyanJsonGetBoolValue(RyanJsonGetObjectToKey(snapshot, "flag"))));
	TEST_ASSERT_TRUE(RyanJsonChangeStringValue(RyanJsonGetObjectToKey(root, "name"),
						   RyanJsonGetStringValue(RyanJsonGetObjectToKey(snapshot, "name"))));
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, snapshot), "回滚后 root 应与快照相等");

	char *printed = RyanJsonPrint(root, 128, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printed);
	RyanJson_t roundtrip = RyanJsonParse(printed);
	TEST_ASSERT_NOT_NULL(roundtrip);
	TEST_ASSERT_TRUE(RyanJsonCompare(root, roundtrip));

	RyanJsonDelete(roundtrip);
	RyanJsonFree(printed);
	RyanJsonDelete(snapshot);
	RyanJsonDelete(root);
}

static void testScenarioSnapshotRollbackThenBranchMutationKeepsIsolation(void)
{
	// 复杂链路：
	// Parse -> Duplicate(snapshot) -> 多次修改 -> 按快照回滚
	// -> Detach/Change/Insert + ReplaceByKey 再分叉 -> 双文档 Roundtrip。
	// 目标：
	// - 验证回滚后的 live 文档还能继续安全变更；
	// - 验证后续分叉变更不会反向污染 snapshot；
	// - 验证 rollback 后再 branch 的文档与 snapshot 仍各自可稳定 roundtrip。
	const char *source = "{\"cfg\":{\"mode\":\"a\",\"retry\":3},\"list\":[{\"id\":\"a\",\"tags\":[\"x\"]},{\"id\":\"b\",\"tags\":[]}],"
			     "\"meta\":{\"ver\":1},\"flag\":true}";
	const char *branchText =
		"{\"cfg\":{\"mode\":\"a\",\"retry\":3},\"list\":[{\"id\":\"b\",\"tags\":[]},{\"id\":\"aa\",\"tags\":[\"x\"]}],"
		"\"meta\":{\"ver\":2},\"flag\":\"branch\"}";

	RyanJson_t root = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL_MESSAGE(root, "snapshot 分叉样本解析失败");
	RyanJson_t snapshot = RyanJsonDuplicate(root);
	TEST_ASSERT_NOT_NULL_MESSAGE(snapshot, "snapshot 分叉样本快照失败");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(root, "cfg", RyanJsonCreateString("cfg", "flat")), "将 cfg 替换为标量失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(root, "list", RyanJsonCreateArray()), "将 list 替换为空 Array 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonAddIntToArray(RyanJsonGetObjectToKey(root, "list"), 9), "向临时 list 写入 9 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(RyanJsonGetObjectToKey(root, "meta"), "ver", RyanJsonCreateInt("ver", 9)),
				 "修改 meta.ver=9 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(root, "flag"), RyanJsonFalse), "修改 flag=false 失败");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(root, snapshot), "分叉前的破坏性修改后 root 不应与 snapshot 相等");

	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(root, "cfg", RyanJsonDuplicate(RyanJsonGetObjectToKey(snapshot, "cfg"))),
				 "按 snapshot 回滚 cfg 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(root, "list", RyanJsonDuplicate(RyanJsonGetObjectToKey(snapshot, "list"))),
				 "按 snapshot 回滚 list 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(RyanJsonGetObjectToKey(root, "meta"), "ver", RyanJsonCreateInt("ver", 1)),
				 "按 snapshot 回滚 meta.ver 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeBoolValue(RyanJsonGetObjectToKey(root, "flag"), RyanJsonTrue), "按 snapshot 回滚 flag 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, snapshot), "回滚后 root 应与 snapshot 完全一致");

	RyanJson_t list = RyanJsonGetObjectToKey(root, "list");
	TEST_ASSERT_NOT_NULL(list);
	RyanJson_t moved = RyanJsonDetachByIndex(list, 0);
	TEST_ASSERT_NOT_NULL_MESSAGE(moved, "回滚后再次分离 list[0] 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonChangeStringValue(RyanJsonGetObjectToKey(moved, "id"), "aa"), "修改分离节点 id 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonInsert(list, UINT32_MAX, moved), "回滚后重新回插分离节点失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(RyanJsonGetObjectToKey(root, "meta"), "ver", RyanJsonCreateInt("ver", 2)),
				 "branch 阶段修改 meta.ver=2 失败");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonReplaceByKey(root, "flag", RyanJsonCreateString("flag", "branch")), "branch 阶段替换 flag 失败");

	RyanJson_t expectBranch = RyanJsonParse(branchText);
	TEST_ASSERT_NOT_NULL(expectBranch);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, expectBranch), "branch 文档结果与期望结构不一致");
	TEST_ASSERT_FALSE_MESSAGE(RyanJsonCompare(root, snapshot), "branch 后的 root 不应再与 snapshot 相等");

	RyanJson_t expectSnapshot = RyanJsonParse(source);
	TEST_ASSERT_NOT_NULL(expectSnapshot);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(snapshot, expectSnapshot), "snapshot 应保持原始语义，不应被后续 branch 污染");

	char *printedRoot = RyanJsonPrint(root, 192, RyanJsonFalse, NULL);
	char *printedSnapshot = RyanJsonPrint(snapshot, 192, RyanJsonFalse, NULL);
	TEST_ASSERT_NOT_NULL(printedRoot);
	TEST_ASSERT_NOT_NULL(printedSnapshot);

	RyanJson_t roundtripRoot = RyanJsonParse(printedRoot);
	RyanJson_t roundtripSnapshot = RyanJsonParse(printedSnapshot);
	TEST_ASSERT_NOT_NULL(roundtripRoot);
	TEST_ASSERT_NOT_NULL(roundtripSnapshot);
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(root, roundtripRoot), "branch 文档往返 Compare 应相等");
	TEST_ASSERT_TRUE_MESSAGE(RyanJsonCompare(snapshot, roundtripSnapshot), "snapshot 文档往返 Compare 应相等");

	RyanJsonDelete(roundtripSnapshot);
	RyanJsonDelete(roundtripRoot);
	RyanJsonFree(printedSnapshot);
	RyanJsonFree(printedRoot);
	RyanJsonDelete(expectSnapshot);
	RyanJsonDelete(expectBranch);
	RyanJsonDelete(snapshot);
	RyanJsonDelete(root);
}

void testScenarioSnapshotsRunner(void)
{
	UnitySetTestFile(__FILE__);
	RUN_TEST(testScenarioSnapshotRollbackByDuplicateSnapshot);
	RUN_TEST(testScenarioSnapshotRollbackThenBranchMutationKeepsIsolation);
}
