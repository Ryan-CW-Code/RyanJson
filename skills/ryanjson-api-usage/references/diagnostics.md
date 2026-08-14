# API 诊断与故障注入

## 诊断顺序

1. 核对 hooks 是否在第一次依赖分配的调用前初始化。
2. 记录当前严格 key 与默认 Add 顺序宏值。
3. 先缩小输入并记录返回值，再检查内部实现。
4. 依次核对 Create、Add/Insert、Replace、Detach、Print 与 root Delete 的生命周期分支。
5. 只有区分清楚调用方误用和内部缺陷后，才升级到核心工程。

## 高频错误

- 未判空和 `RyanJsonIsXXX` 就调用类型 getter。
- 用 `Change*Value` 改类型。
- 重复挂载已在树中的 item。
- 误以为 Replace 失败会删除新 item。
- 未用 `ParseOptions` 严格尾部模式却假设 Parse 拒绝全部尾随数据。
- Minify 未预留终止符空间却当作 NUL 结束输出。
- 结构修改后继续使用陈旧容器指针。

## 分配失败练习

重要失败路径可用 hook 包装器让第 N 次分配失败，验证 Parse、Create、Add/Insert、Replace、Print 都返回可恢复结果，且所有权严格符合契约。优先在主机侧带泄漏和 sanitizer 诊断运行；资源受限目标上可选。

## 升级

- crash、树损坏、异常分配布局或明显性能退化进入 [核心工程](../../ryanjson-core-engineering/SKILL.md)。
- 已确认契约但缺少确定性回归进入 [测试工程](../../ryanjson-test-engineering/SKILL.md)。
