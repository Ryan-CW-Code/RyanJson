# RyanJson 项目语境（压缩版）

## 1. 作用
- 帮助用户和 AI 在陌生上下文下快速建立 RyanJson 的项目心智。
- 优先回答“这是什么仓库、第一跳看哪里、当前问题下一步去哪”。

## 2. 仓库一句话定位
- RyanJson 是一个以 `xmake` 为主入口的 C 语言 JSON 库仓库，包含 host unit、fuzz、QEMU 验证、报告与配套 AI skills。

## 3. 首屏必须覆盖
1. 先用一句话解释仓库形态。
2. 再给 3 到 5 个第一跳入口。
3. 最后给当前问题对应的 skill、脚本或报告。

## 4. 新人最短入口
1. `README.md`
2. `AGENTS.md`
3. `example/`
4. `xmake.lua`
5. `scripts/README.md`

## 5. 常见目标到下一跳
- 想先看仓库整体：`references/repoMap.md`
- 想知道怎么构建/跑脚本/看 CI：`references/buildAndValidation.md`
- 想知道文档、报告、skills 怎么分工：`references/docsAndReports.md`
- 想知道自己当前问题该走哪个 skill：`../shared/questionRouter.md`、`../shared/routingExamples.md`、`references/entryScenarios.md`

## 6. 输出检查单
1. 不要一次性列太多目录。
2. 先解释“是什么”，再解释“去哪看”。
3. 用户问题模糊时，默认给最短阅读路径和下一跳。

## 7. 依据（仓库内）
- `README.md`
- `AGENTS.md`
- `xmake.lua`
- `scripts/README.md`
- `../shared/questionRouter.md`
