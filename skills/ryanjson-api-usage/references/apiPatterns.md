# API 场景模式（回答优先）

## 1. 用途
- 本文用于“按用户意图快速选 API 路径”，减少回答时漏掉失败语义和释放责任。
- 仅覆盖公开 API；内部实现问题转 `ryanjson-optimization`，测试回归问题转 `ryanjson-test-engineering`。

## 2. 回答前固定动作
1. 先确认 `RyanJsonInitHooks` 在任何 Json API 前执行。
2. 先确认宏前提：`RyanJsonStrictObjectKeyCheck`、`RyanJsonDefaultAddAtHead`。
3. 先判定用户意图属于哪类场景，再给最小 API 组合。

## 3. 意图到路径
| 用户意图 | 推荐路径 | 是否改结构 | 关键风险 |
|---|---|---|---|
| 读取配置字段 | Parse + Get + IsXXX | 否 | 未判型直接取值 |
| 周期上报/打包发送 | Create + Add + PrintPreallocated | 是 | 缓冲不足或清理遗漏 |
| 在线改值（同类型） | Get + Change*Value | 否 | 把跨类型更新误写成 Change |
| 字段类型切换 | Create* + ReplaceBy* | 是 | Replace 失败后 `newItem` 泄漏 |
| 子树迁移/重排 | Detach* + Add/Insert | 是 | detach 后未接管 |
| 重复 key 策略切换 | 宏配置 + 业务校验同步 | 视需求 | 宏与测试预期不一致 |
| 传输压缩输出 | Print(format=false) | 否 | 误把 Minify 当传输主路径 |
| 历史文本清洗 | Minify | 否 | `\0` 终止符假设错误 |

## 4. 场景卡

### A. 读取配置（Parse + Get）
- 触发：启动配置读取、外部报文字段抽取。
- 调用顺序：
  1. `RyanJsonInitHooks`
  2. `RyanJsonParse`
  3. `RyanJsonGetObjectByKey`
  4. `RyanJsonIsXXX` 后再 `RyanJsonGetXXXValue`
  5. `RyanJsonDelete(root)`
- 失败口径：
  - `Parse == NULL`：输入非法或资源不足，可恢复错误。
  - key 缺失/类型不符：可恢复错误，不应走崩溃路径。

### B. 周期上报（Create + Add + PrintPreallocated）
- 触发：周期遥测、固定缓冲发送。
- 调用顺序：
  1. `RyanJsonCreateObject`
  2. `RyanJsonAdd*ToObject`
  3. `RyanJsonPrintPreallocated(..., RyanJsonFalse, ...)`
  4. `RyanJsonDelete(root)`
- 失败口径：
  - 任意 Add 失败：立即清理 root 后返回。
  - PrintPreallocated 失败：走降级路径并清理 root。

### C. 同类型热更新（Get + Change）
- 触发：仅变更 value，不变更字段类型。
- 规则：
  - 同类型更新使用 `Change*Value`。
  - 跨类型更新必须切换到 `ReplaceByKey/ReplaceByIndex`。

### D. 跨类型替换（Replace）
- 触发：例如 `int -> object`、`string -> array`。
- 调用顺序：
  1. 构造 `newItem`
  2. 调用 `RyanJsonReplaceByKey/ByIndex`
  3. 失败时调用方复用或 `RyanJsonDelete(newItem)`
- 关键提醒：
  - `Replace` 失败不消费 `newItem`，不能套用 `Add/Insert` 失败语义。

### E. 子树迁移（Detach + Add/Insert）
- 触发：把既有子树迁移到新父节点。
- 调用顺序：
  1. `detached = RyanJsonDetachByKey/ByIndex`
  2. 判空并确认可重挂载
  3. `RyanJsonAddItem*` / `RyanJsonInsert`
  4. 失败分支立即处理 `detached` 所有权
- 关键提醒：
  - detach 后必须“重挂”或“释放”，不能悬空。

### F. 重复 key 策略（StrictKey 宏）
- `RyanJsonStrictObjectKeyCheck=true`：重复 key 更严格。
- `RyanJsonStrictObjectKeyCheck=false`：兼容性更高，但 key 查询可预测性下降。
- 回答时必须显式写明宏前提，并提示测试预期需同步。

### G. 传输压缩输出（非格式化 Print）
- 推荐路径：
  1. 动态输出：`RyanJsonPrint(..., RyanJsonFalse, ...)`，发送后 `RyanJsonFree(str)`。
  2. 固定缓冲：`RyanJsonPrintPreallocated(..., RyanJsonFalse, ...)`。
- 关键提醒：
  - 传输场景优先 `format=false`，不要默认走 `Minify`。

### H. 文本清洗（Minify）
- 适用：已有 Json 文本去空白/注释后再解析或对比。
- 调用顺序：
  1. 准备可写缓冲
  2. `ret = RyanJsonMinify(buf, textLen)`
  3. `ret < textLen` 才可直接按 C 字符串使用
- 关键提醒：
  - `Minify` 是文本清洗工具，不是传输输出主路径。

## 5. 输出模板（回答时）
1. 前提：宏前提、hooks 前提、输入类型。
2. 路径：推荐 API 顺序（成功路径 + 失败路径）。
3. 所有权：每个失败分支由谁释放。
4. 验证：最小可执行检查点（返回值/日志/内存）。

## 6. 高频误答拦截
1. 未判型直接 `GetXXXValue`。
2. 把 `Replace` 失败当成库自动清理。
3. 传输路径建议先格式化再 Minify。
4. 忽略 `RyanJsonDefaultAddAtHead` 导致索引/遍历顺序误判。
5. 示例只写成功路径，不写失败与释放路径。

## 7. 依据（仓库内）
- `RyanJson/RyanJsonItem.c`：Add/Insert/Replace/Detach 失败与所有权路径
- `RyanJson/RyanJson.c`：`RyanJsonMinify` 行为
- `test/unityTest/cases/core/testCreate.c`：Add/Insert/Detach 相关断言
- `test/unityTest/cases/core/testReplace.c`：Replace 失败不消费 `item`
- `test/unityTest/cases/utils/testPrint.c`：非格式化打印与 preallocated 边界
- `test/unityTest/cases/utils/testUtils.c`、`test/unityTest/cases/utils/testRobust.c`、`test/fuzzer/cases/fuzzerMinify.c`：Minify 边界与稳健性
