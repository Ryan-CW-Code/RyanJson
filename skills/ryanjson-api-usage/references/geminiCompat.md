# Gemini 兼容说明（API 使用类）

## 范围
- 本页只定义 Gemini 在 API 使用类任务的输入/输出结构。
- 公开 API 语义以 `apiReference.md`、`apiPatterns.md` 为准。

## 推荐输入结构
- 目标：要实现的 Json 业务操作。
- 平台：RT-Thread（线程模型、内存约束）。
- 约束：是否允许额外内存、是否启用严格 key。
- 验收：返回值、日志输出、内存统计。

## 推荐输出结构
1. API 选型说明（仅公开 API）。
2. RT-Thread 可运行示例代码（含 hooks 初始化）。
3. 失败路径与所有权说明。
4. 上板验证步骤（日志/返回值/内存统计）。

## 对齐原则
- 必须把 `RyanJsonInitHooks` 作为前置条件写清楚。
- 不默认要求用户在 RT-Thread 板端运行 unity/fuzzer。
- API 语义不确定时，AI 可按 `example -> test/unityTest -> test/fuzzer` 取证。
- API 技能默认不展开 RyanJson 内部实现细节。

## 术语字典（统一）
- 本文术语统一以 `terminology.md` 为准。

## 依据（仓库内）
- `apiReference.md`：公开 API 语义基线
- `ownershipAndErrors.md`：失败所有权口径
- `apiPatterns.md`：场景化调用顺序
