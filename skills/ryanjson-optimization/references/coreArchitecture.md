# RyanJson 核心架构速图（优化视角）

## 范围
- 本页说明优化决策需要的结构心智，不包含具体改动步骤。
- 执行流程见 `coreWorkflow.md`，门禁见 `regressionGates.md`。

## 1. 数据结构基线
- `RyanJson_t` = `struct RyanJsonNode *`。
- `struct RyanJsonNode` 公开只有 `next` 字段；其余元数据与值在 payload 区。
- payload 起始 1 字节是 flag，随后是类型相关数据区。

## 2. 节点分配模型（`RyanJsonInternalNewNode`）
分配大小 = 基础头 + 类型值区 + 可选 inline 区：
- 基础：`sizeof(node) + 1B flag`
- number：+4 或 +8
- array/object：+`sizeof(RyanJson_t)`（children 指针）
- 有 key 或类型是 string：+`RyanJsonInlineStringSize`

优化含义：
- 只有“可能存 key/string”的节点才有 inline 区。
- object/array 的 children 指针在 payload 区，不在 struct 字段里。

## 3. 字符串存储模式
- inline：短 key/value 放在节点内，减少堆分配。
- ptr：长字符串放外部堆块，payload 存指针。
- `RyanJsonInternalChangeString` 负责模式切换和旧堆块释放。

优化红线：
- 不能破坏模式切换后的所有权与释放顺序。

## 4. 线索化链（优化常踩坑）
- `IsLast=0`：`next` 指向兄弟。
- `IsLast=1`：`next` 指向父节点（不是兄弟）。

影响：
- `RyanJsonGetNext` 会屏蔽 `IsLast` 并返回 NULL。
- 内部遍历/删除经常直接用 `node->next` 做回溯。
- 优化遍历逻辑时，不能把 `next` 一律当兄弟指针。

## 5. Delete/Duplicate 都是非递归
- `RyanJsonDelete`：下沉子树，叶子释放后通过线索回溯。
- `RyanJsonDuplicate`：同步遍历源树与目标树，依赖 `IsLast` 维护层级。

优化建议：
- 若要改遍历路径，优先补深层结构回归和栈风险回归。

## 6. Hooks 是全局运行前置
- `jsonMalloc/jsonFree/jsonRealloc` 为全局函数指针。
- 必须由 `RyanJsonInitHooks` 先初始化。
- 优化结论必须在 hooks 已初始化条件下给出（否则数据不可比）。
- 当前实现中 hooks 默认值为 `NULL`，未初始化状态不可作为有效基线。

## 7. 对外/对内 API 边界
- 对外 API 用 `RyanJsonXxx`（头文件公开声明）。
- 跨文件内部 API 用 `RyanJsonInternalApi` + `RyanJsonInternalXxx`。
- 优化时不要把内部辅助函数误暴露为公开契约。

## 8. 配置宏对行为的影响
- `RyanJsonStrictObjectKeyCheck` 改变 parse/insert/replace 的重复 key 行为。
- `RyanJsonInlineStringSize`、`RyanJsonMallocHeaderSize`、`RyanJsonMallocAlign` 影响节点内存布局边界。

优化动作前，先锁定这些宏值。

## 9. 优化取证最小顺序
1. 头文件语义（`RyanJson.h` / `RyanJsonConfig.h`）
2. 核心实现（`RyanJson*.c`）
3. 样例与测试（`example/` -> `test/unityTest/` -> `test/fuzzer/`）

若三者冲突，先修文档和测试，再收敛实现。

## 10. 关键依据（仓库内）
- `RyanJson/RyanJson.c`：hooks 全局变量与 `RyanJsonInitHooks`
- `RyanJson/RyanJsonUtils.c`、`RyanJson/RyanJsonParse.c`、`RyanJson/RyanJsonPrint.c`：`jsonMalloc/jsonFree` 调用路径
- `test/unityTest/common/testCommon.c`、`test/fuzzer/entry.c`：测试入口 hooks 初始化
