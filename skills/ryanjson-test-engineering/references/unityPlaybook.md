# Unity 测试补充策略

## 范围
- 本页只保留 Unity 特有编写规范。
- 通用交付结构见 `testcaseTemplate.md`。
- Fuzzer 差异规则见 `fuzzerPlaybook.md`。
- 执行入口与覆盖目录基线见 `../../shared/ryanJsonCommon.md`。

## 前置初始化（必须）
- 在测试 runner 或每组 setup 中先调用 `RyanJsonInitHooks`。
- hooks 初始化失败时应立即终止该组测试。
- 内存统计、泄漏判定、OOM 注入都应通过 hooks 统一实现。

## 分类归档规则
- 核心 API 行为：`test/unityTest/cases/core/`
  细分到 `core/accessor`、`core/compare`、`core/key`、`core/standard` 时，优先放到子目录，不再混回根目录。
- 边界与格式：`test/unityTest/cases/edge/`
  细分到 `edge/container`、`edge/parseprint` 时，优先按容器语义和 parse/print 语义拆分。
- 相等性与往返基线：`test/unityTest/cases/equality/`
- 场景链路：`test/unityTest/cases/scenario/`
- 稳定性与链表结构：`test/unityTest/cases/stability/`
- 用户级 recipe/用法：`test/unityTest/cases/usage/`
- 工具与打印：`test/unityTest/cases/utils/`
  打印主契约放 `utils/print/`，不要再回到旧的 `utils/testPrint.c` 形态。
- 压力/资源：`test/unityTest/cases/performance/`

## 注释规范
- 非显而易见的测试函数前补 1 到 3 行中文注释，说明该用例证明的契约。
- 注释优先解释“为什么在这里测”与“这个分支容易漏什么”，不要逐条复述断言。
- API 名、类型名、宏名保持英文原文，避免把 `ChangeKey`、`CompareOnlyKey` 之类翻译成新术语。
- 一组断言若共同服务于同一条契约，可在代码块前写一条中文块注释，不必每行都注释。

## 编写顺序
1. 成功路径：确认主语义。
2. 失败路径：参数非法、插入失败、替换失败。
3. 边界路径：空对象、空数组、极短/极长字符串、索引 0/中间/末尾。
4. 资源路径：失败后是否自动释放，是否留下脏链。

## 典型断言模板
- 返回值：`TEST_ASSERT_TRUE/FALSE`。
- 指针语义：`TEST_ASSERT_NULL/NOT_NULL`。
- 结构语义：`RyanJsonCompare` 或字段级验证。
- 泄漏语义：结合测试内存统计钩子。

## 公共助手（优先复用）
- OOM 注入：`UNITY_TEST_OOM_BEGIN/END`、`UNITY_TEST_OOM_BEGIN_NO_REALLOC`。
- 泄漏基线：`unityTestLeakScopeBegin/End`。

## 用例结构建议
- 一个用例验证一个核心语义，避免“多目标混测”。
- 测试名称包含场景 + 期望（例如 `testReplaceByKeyRejectDuplicateKeyStrict`）。
- 失败路径应同时验证返回值与对象结构未损坏。

## 容易漏掉的点
- “刚好够用”边界（长度 == 容量）。
- 错误路径下 item 所有权转移（Add/Insert 与 Replace 失败语义不同）。
- 宏开关导致的双语义（严格/非严格）。
- Detach 后节点再用时的游离状态。

## 补测踩坑记录（2026-03）
- `RyanJsonGetObjectToKey` 只能走 key 路径，混用索引会产生隐性误判；数组索引必须用 `RyanJsonGetObjectToIndex`。
- 数组元素无 key，`ChangeKey` 对数组元素应失败；需要挂回对象时应使用 `AddItemToObject` 绑定新 key。
- C 字符串 `\x..` 转义会吞掉后续十六进制字符，UTF-8 字节后面紧跟数字要拆分为相邻字符串字面量。
- 非严格模式允许重复 key，`Compare/CompareOnlyKey` 必须按“同 key 出现序号”对齐；补测时要覆盖“重复 key 计数失配”场景。
- `AddItemToArray/AddItemToObject` 仅接受容器；失败会释放游离 `item`，调用方不可复用该指针。
- 非严格模式下 `DeleteByKey/DetachByKey` 只移除首个重复 key，其余重复项仍存在（需要循环处理）。
- 非严格模式下重复 key 删除/分离后“剩下哪一个”受插入方向影响，测试不要假设具体值。
- `ParseOptions(requireNullTerminator=false)` 的 `parseEndPtr` 可能指向空白，后续解析可直接传给 `ParseOptions` 让其跳过前导空白。
- 本地 `run_local_base.sh` 在沙箱内可能触发 `Bad system call`，需要使用非沙箱执行。

## 复杂调用链补测经验（2026-02）
- 复杂链路测试不要假设固定插入方向；同一用例要在 `RyanJsonDefaultAddAtHead=true/false` 都稳定。
- 需要稳定索引语义时，优先用 `Parse` 直接构造目标数组初始态，再执行 Detach/Replace/Insert。
- `RyanJsonAddItemToObject` 成功后，如果继续使用之前缓存的子容器指针，可能触发隐性不稳定；应从 root 重新获取。
- Unity 断言失败会提前跳出函数，容易把资源清理留在失败路径之外；复杂用例应先保证关键前置断言，再进入多步链路。

## 依据（仓库内）
- `test/unityTest/runner/main.c`、`test/unityTest/common/testCommon.c`：runner/setup hooks 初始化
- `test/unityTest/cases/core/testCreate.c`：Insert/AddItem 失败与游离态断言
- `test/unityTest/cases/core/testReplace.c`：Replace 失败不消费语义
- `test/unityTest/cases/utils/print/testPrintGeneral.c`：`Print`/`PrintPreallocated` 的 headroom、极小 double、固定点边界
- `test/unityTest/cases/utils/print/testPrintStyle.c`：`PrintWithStyle`/`PrintPreallocatedWithStyle` 的 style 与返回长度契约
