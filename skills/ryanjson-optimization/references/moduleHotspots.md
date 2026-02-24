# RyanJson 模块热点与审查清单

## 范围
- 本页只列“风险热点”和“审查项”。
- 改动步骤见 `coreWorkflow.md`。
- 输出结构见 `optimizationTemplate.md`。

## RyanJsonParse.c（高风险）
- 数字路径：整数、小数、指数累计时的溢出保护。
- 状态推进：`RyanJsonParseBufTryAdvanceCurrentPtr` 的成功/失败路径是否一致。
- 字符串路径：转义、Unicode 解码、代理对有效性。
- 错误回滚：失败后当前节点与解析状态是否残留脏数据。

## RyanJsonPrint.c（高风险）
- 预分配模式：长度刚好够用时的边界是否判定正确。
- append 失败：是否释放内部缓冲、是否返回 NULL。
- double 打印：`isinf`/`isnan` 的 RFC8259 策略是否一致。
- style/preset：用户自定义样式与默认行为是否冲突。

## RyanJsonItem.c（高风险）
- 所有权语义：分别核对 Add/Insert 与 Replace 的失败路径是否符合既定契约（不可混用）。
- 游离节点保护：`RyanJsonIsDetachedItem` 防御分支是否覆盖。
- 重复 key 语义：严格/非严格模式行为与注释一致。
- 链完整性：next/last 标志在插入、替换、分离后的一致性。

## RyanJson.c Compare（高风险）
- 同序快路径：是否会误判 key 对齐。
- 乱序路径：对象查找是否稳定、数组比较是否出现回溯爆炸。
- 深度与栈：深层嵌套下是否触发栈风险。

## RyanJsonInternal.h / 跨文件内部 API（中高风险）
- 仅内部接口使用 `RyanJsonInternalApi` + `RyanJsonInternalXxx` 命名。
- 禁止把内部函数直接暴露为公开 API。
- 跨模块重命名后要同步 callsite、注释与技能文档。

## RyanJsonConfig.h（中高风险）
- 宏新增必须写“收益/代价/兼容影响”。
- 默认值必须兼容历史行为。
- 任何宏语义变化都要同步 unity/fuzzer 预期。

## 依据（仓库内）
- `RyanJson/RyanJsonItem.c`：`RyanJsonInsert`、`RyanJsonAddItemToObject`、`RyanJsonReplaceByKey/ByIndex`
- `RyanJson/RyanJsonConfig.h`：`RyanJsonStrictObjectKeyCheck`、`RyanJsonDefaultAddAtHead`
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`
