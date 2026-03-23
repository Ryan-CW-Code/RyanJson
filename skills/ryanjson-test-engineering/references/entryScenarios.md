# RyanJson 测试入口场景

## 范围
- 用于回答“刚进入测试问题时，该先看归属、补测方式还是回归入口”。
- 只做第一跳导航，不替代详细测试规则。

## 场景 1：我知道缺测试，但不知道该补在哪
- 先看：`test-map.md`
- 再看：`unityPlaybook.md`
- 再看：`testcaseTemplate.md`

## 场景 2：我怀疑现有测试重复了
- 先看：`test-map.md`
- 再看：`coverageTriage.md`
- 再看：`assertPolicy.md`

## 场景 3：我想把 fuzz 崩溃收敛成 unit 回归
- 先看：`fuzzerPlaybook.md`
- 再看：`unityPlaybook.md`
- 再看：`regressionMatrix.md`

## 场景 4：我改了实现，想知道最小该跑什么
- 先看：`regressionMatrix.md`
- 再看：`coverageTriage.md`
- 再看：`../../shared/ryanJsonCommon.md`

## 场景 5：问题其实更像 API 用法或内部优化
- API 用法、释放责任、集成问题：转 `../../ryanjson-api-usage/SKILL.md`
- 性能、内存、crash 根因与实现审查：转 `../../ryanjson-optimization/SKILL.md`
