# RyanJson 优化入口场景

## 范围
- 用于回答“我这个问题刚进入优化领域，第一跳该看哪里”。
- 不直接展开完整方案，只做首屏分流。

## 场景 1：Parse / Print 变慢了
- 先看：`coreWorkflow.md`
- 再看：`moduleHotspots.md`
- 再看：`regressionGates.md`

## 场景 2：Replace / Add / Compare 路径 crash 了
- 先看：`coreWorkflow.md`
- 再看：`moduleHotspots.md`
- 再看：`../../shared/architecture.md`

## 场景 3：想压低内存峰值，但不确定能不能改宏
- 先看：`configSwitches.md`
- 再看：`optimizationRecipes.md`
- 再看：`regressionGates.md`

## 场景 4：想评估一个实现改动有没有风险
- 先看：`moduleHotspots.md`
- 再看：`regressionGates.md`
- 再看：`optimizationTemplate.md`

## 场景 5：问题其实更像 API 用法或测试归属
- API 用法、失败语义、集成问题：转 `../../ryanjson-api-usage/SKILL.md`
- 补测、去重、回归归属：转 `../../ryanjson-test-engineering/SKILL.md`
