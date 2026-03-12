# 配置宏设计规范

## 范围
- 本页只讲“宏语义设计”和“宏变更验收点”。
- 完整优化任务交付骨架见 `optimizationTemplate.md`。

## 宏新增/修改基本规则
- 只为“可明确切换的行为”加宏，不为临时调试加宏。
- 宏默认值优先兼容历史行为。
- 注释必须写清：启用后收益、代价、兼容影响、测试影响。

## 变更流程
1. 定义宏语义和默认值。
2. 在 `RyanJsonConfig.h` 写清行为差异。
3. 同步核心代码分支。
4. 同步 unity + fuzzer 条件断言。
5. 在变更说明中列出风险与回滚条件。

## 典型宏：`RyanJsonStrictObjectKeyCheck`
- `true`：拒绝重复 key，输入更严格，早发现脏数据。
- `false`：允许重复 key，通常首匹配，语义依赖调用方约束。

## 宏级别验收清单
- Parse 是否符合宏期望。
- Add/Insert/Replace 是否符合宏期望。
- Compare/读取行为是否在文档中声明。
- fuzzer 断言是否按宏分支。

## 提交说明模板
- 变更宏：
- 默认行为变化：
- 受影响 API：
- 测试覆盖：
- 兼容风险：
- 回滚策略：

## 依据（仓库内）
- `RyanJson/RyanJsonConfig.h`：严格 key / AddAtHead 宏定义与默认值
- `RyanJson/RyanJson.h`：宏影响的公开 API 语义注释
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`：宏分支断言
