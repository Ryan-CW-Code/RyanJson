# RyanJson 问题分流（快速版）

## 1. 作用
- 用最短路径判断“用户现在该先看哪里、该走哪个 skill”。
- 适用于问题很模糊、第一次接触仓库，或请求同时带有导航/API/优化/测试成分的场景。
- 如果边界仍不清晰，继续看 `routingExamples.md`。

## 2. 先看用户的主目标
- 想先认识仓库、找入口、看脚本/CI/报告：进入 `../ryanjson-project-guide/SKILL.md`
- 想把 RyanJson 用起来、接到工程里、确认 API 失败语义：进入 `../ryanjson-api-usage/SKILL.md`
- 想分析内部实现、性能、内存、崩溃根因或宏边界：进入 `../ryanjson-optimization/SKILL.md`
- 想补单测、做去重治理、确认回归归属：进入 `../ryanjson-test-engineering/SKILL.md`

## 3. 混合问题怎么判主 skill
- 用户主要是在“学会用/接进去”：优先 `ryanjson-api-usage`
- 用户主要是在“改实现/查风险/看 crash 根因”：优先 `ryanjson-optimization`
- 用户主要是在“补证据/补回归/定测试归属”：优先 `ryanjson-test-engineering`
- 用户没有上下文，只是在问“先看哪里/这个仓库怎么理解”：优先 `ryanjson-project-guide`
- 同一请求里如果同时出现“API 用法 + 最小回归”，先解 API 路径，再转测试补强。
- 同一请求里如果同时出现“crash 根因 + 回归补测”，先做根因与语义边界分析，再转测试收敛。

## 4. 新用户最短路径
1. `README.md`
2. `AGENTS.md`
3. `example/`
4. 与当前目标匹配的 skill 或 `run_local_*.sh`

## 5. 输出时要避免
- 一上来把整个仓库目录树全列出来
- 把公开 API 用法、内部实现优化、测试补强混成同一种问题
- 在用户还没建立项目心智时直接丢大量细节文档
