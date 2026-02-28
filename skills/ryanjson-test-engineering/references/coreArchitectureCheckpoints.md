# 核心架构测试检查点

## 范围
- 把 RyanJson 核心结构映射为“必须覆盖的测试断言点”。
- 本页强调“测什么”，执行与覆盖入口见 `../../shared/ryanJsonCommon.md`。

## 1. `RyanJson_t` / payload 布局相关
检查点：
- 类型读取是否与 flag 一致（Null/Bool/Number/String/Array/Object）。
- number 的 int/double 分支都要有断言。
- object/array 的 children 指针读取应只在容器类型下进行。

建议：
- 使用前必须使用 `RyanJsonIsXXX` 判断类型。

## 2. 字符串 inline/ptr 模式切换
检查点：
- 短字符串走 inline，长字符串走 ptr。
- ChangeString 后模式切换时不泄漏旧堆块。
- key/value 边界长度（刚好临界值）必须有测试。

建议：
- 单测做临界值；fuzzer 做随机长度扰动。

## 3. 线索化链结构（IsLast + next）
检查点：
- 尾节点 `IsLast=1` 时 `RyanJsonGetNext` 返回 NULL。
- 内部操作后链是否保持可遍历（Insert/Replace/Detach/Delete 组合）。
- 错误使用非游离节点插入时能否被防御性拒绝。

建议：
- 多层对象+数组混合结构下验证 Delete 无崩溃无泄漏。

## 4. 所有权与释放语义
检查点：
- Create 后未挂树节点是否可安全释放。
- Add/Insert/Replace 成功后所有权转移是否成立。
- Detach 返回节点是否需要调用者释放。
- Add/Insert 失败时：游离 item 由库侧清理，非游离 item 不释放。
- Replace 失败时：item 仍由调用者持有（可复用/可释放）。

建议：
- 每个接口至少 1 个“失败后内存计数归零”用例。

## 5. hooks 前置条件
检查点：
- hooks 初始化前调用核心 API 的行为（当前实现无默认 hooks，测试侧应视为禁止路径）。
- hooks 初始化失败是否被正确处理。
- OOM 注入下失败路径是否可恢复且无泄漏。

## 6. 语义不明确时的取证顺序
1. `example/`
2. `test/unityTest/`
3. `test/fuzzer/`

若仍不明确，回到头文件注释与核心实现，避免靠经验推断。

## 依据（仓库内）
- `RyanJson/RyanJson.h`：类型判断公开接口 `RyanJsonIsNull/Bool/Number/...`
- `RyanJson/RyanJson.c`：hooks 全局指针初始化与 `RyanJsonInitHooks`
- `test/unityTest/common/testCommon.c`：测试入口 hooks 初始化
- `test/unityTest/cases/core/testCreate.c`、`test/unityTest/cases/core/testReplace.c`：所有权失败语义断言
