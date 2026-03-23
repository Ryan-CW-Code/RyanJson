---
name: ryanjson-project-guide
description: RyanJson 仓库导航与协作入口。用于回答“项目结构是什么、先看哪里、怎么构建/跑脚本、CI 跑什么、报告/README/skills 在哪、当前应走哪个 skill”这类仓库级问题；API 语义转 ryanjson-api-usage，核心实现转 ryanjson-optimization，测试补强转 ryanjson-test-engineering。
---

# RyanJson 项目导航技能

## 目标
- 用最短路径解释仓库结构、构建验证入口、报告位置与 AI 路由。
- 先帮助用户建立 RyanJson 的项目心智，再分流到正确的 skill 或脚本。
- 优先回答“先看哪里、先跑什么、当前问题该走哪个 skill”。

## 必要输入
- 用户当前关注点：目录结构 / 构建脚本 / CI / 报告文档 / AI 路由。
- 若用户只说“介绍一下项目”，默认给一屏总览 + 下一步入口。

## 核心流程
1. 先读 `../shared/ryanJsonCommon.md`，确认当前仓库事实与脚本基线。
2. 根据问题类型选择参考：
   - 新手入门 / 问题模糊：`../shared/questionRouter.md`、`../shared/routingExamples.md`、`references/entryScenarios.md`
   - 仓库结构与入口：`references/repoMap.md`
   - 构建、脚本与 CI：`references/buildAndValidation.md`
   - 文档、报告与 skills 路由：`references/docsAndReports.md`
3. 输出一屏总结，附精确路径、脚本或命令。
4. 若问题下钻到 API 语义、核心实现或测试归属，明确转到对应 skill。

## 约束（必须遵守）
- 区分“仓库事实”和“建议路径”，不要把个人偏好写成既定规则。
- 用户第一次接触 RyanJson 时，先用一句话解释“这是什么仓库”，再给最短阅读路径。
- 根目录 `run_local_*` 是正式入口；不要建议把它们想当然地下沉到内部目录。
- `reports/` 下的 Markdown 是脚本生成且会被引用的仓库文档，不按临时文件处理。
- 构建与验证入口优先引用当前仓库脚本和 `xmake`，不要退回历史 `Makefile` 流程。
- 避免一次性罗列过多目录；优先给 3 到 5 个第一跳入口。
- 用户只要导航时，不展开内部实现细节；给清晰下一跳即可。

## 输出格式
1. 一句话结论：RyanJson 是什么仓库，当前问题该从哪里开始。
2. 最短入口：3 到 5 个第一跳文件、脚本或文档。
3. 下一步：若需要深入，建议进入哪个 skill、脚本或报告。

## 用户问题覆盖（按用户视角路由）
- 第一次接触 RyanJson、我该先看哪里：`../shared/questionRouter.md`、`references/entryScenarios.md`
- 项目结构、目录职责、先看哪里：`references/repoMap.md`
- 怎么构建、怎么跑本地脚本、CI 在跑什么：`references/buildAndValidation.md`
- README、报告、skills、AGENTS 分别管什么：`references/docsAndReports.md`
- 具体 API 用法与失败语义：转 `../ryanjson-api-usage/SKILL.md`
- 内部实现、性能/内存优化与 crash 风险：转 `../ryanjson-optimization/SKILL.md`
- 单测补强、归属治理与回归策略：转 `../ryanjson-test-engineering/SKILL.md`

## 示例
- 用户问“我是第一次接触 RyanJson，先看哪几份东西？”：走本技能，先解释仓库形态，再给最短阅读路径与下一跳。
- 用户问“这个仓库我应该先看哪几个文件？”：走本技能，给 `README.md`、`AGENTS.md`、`example/`、对应脚本入口。
- 用户问“提交前本地应该跑什么？”：走本技能，按变更类型路由到 `run_local_*`。
- 用户问“内存报告和 RFC8259 报告现在在哪？”：走本技能，给 `reports/` 路径与生成脚本。
- 用户问“我这个问题应该走哪个 skill？”：走本技能，先分类再转到对应 skill。

## 参考导航
- 共享基线：`../shared/ryanJsonCommon.md`
- 快速分流：`../shared/questionRouter.md`
- 路由样例：`../shared/routingExamples.md`
- 仓库结构：`references/repoMap.md`
- 构建与验证：`references/buildAndValidation.md`
- 文档与报告：`references/docsAndReports.md`
- 入门场景：`references/entryScenarios.md`
- AI 总入口：`../../AGENTS.md`
- 本地压缩文档：`context.md`、`sop.md`
