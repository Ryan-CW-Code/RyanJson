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
- 相等性与往返：`test/unityTest/cases/equality/`
- 工具/鲁棒性：`test/unityTest/cases/utils/`
- 压力/资源：`test/unityTest/cases/performance/`

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

## 用例结构建议
- 一个用例验证一个核心语义，避免“多目标混测”。
- 测试名称包含场景 + 期望（例如 `testReplaceByKeyRejectDuplicateKeyStrict`）。
- 失败路径应同时验证返回值与对象结构未损坏。

## 容易漏掉的点
- “刚好够用”边界（长度 == 容量）。
- 错误路径下 item 所有权转移（Add/Insert 与 Replace 失败语义不同）。
- 宏开关导致的双语义（严格/非严格）。
- Detach 后节点再用时的游离状态。

## 依据（仓库内）
- `test/unityTest/runner/main.c`、`test/unityTest/common/testCommon.c`：runner/setup hooks 初始化
- `test/unityTest/cases/core/testCreate.c`：Insert/AddItem 失败与游离态断言
- `test/unityTest/cases/core/testReplace.c`：Replace 失败不消费语义
- `test/unityTest/cases/utils/testPrint.c`：preallocated 刚好够用/不足边界
