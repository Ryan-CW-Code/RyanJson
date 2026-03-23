# API 高频回答模板

## 范围
- 本页用于统一高频 API 问题的回答骨架，减少遗漏宏前提、失败路径和所有权。
- 详细 API 选择仍以 `apiPatterns.md`、`quickstart.md`、`ownershipAndErrors.md` 为准。

## 通用输出骨架
1. 前提：环境、hooks、宏前提。
2. 最小方案：只保留当前问题需要的公开 API。
3. 失败路径：逐个说明返回 false/NULL 时怎么退出。
4. 所有权：明确谁释放、何时转移。
5. 验证点：用户最小可检查项。

## 模板 1：最小初始化 / Parse + Get
- 前提：
  - `RyanJsonInitHooks` 必须先于任何 Json API。
  - `Get*` 前先判空，再 `RyanJsonIsXXX` 判型。
- 最小方案：
  - `InitHooks -> Parse -> GetObjectByKey -> IsXXX -> GetXXXValue -> Delete(root)`
- 失败路径：
  - hooks 失败：立即返回。
  - Parse 失败：立即返回。
  - key 缺失或类型不符：释放 root 后返回。
- 所有权：
  - `root` 由调用方在结束时 `RyanJsonDelete`。
- 验证点：
  - 错 key / 错类型 / 非法 JSON 都不会崩溃。

## 模板 2：Create + Add + PrintPreallocated
- 前提：
  - 输出缓冲由调用方管理。
  - 传输输出优先非格式化打印。
- 最小方案：
  - `InitHooks -> CreateObject -> Add*ToObject -> PrintPreallocated(..., RyanJsonFalse, ...) -> Delete(root)`
- 失败路径：
  - 任一 Add 失败：删除 root 后返回。
  - 预分配打印失败：删除 root 后返回。
- 所有权：
  - `root` 始终由调用方清理。
  - 输出缓冲不需要 `RyanJsonFree`。
- 验证点：
  - 容量不足时返回失败，缓冲不被当作成功输出使用。

## 模板 3：Replace 失败谁释放
- 前提：
  - 只讨论 `ReplaceByKey/ReplaceByIndex`，不要混入 Add/Insert 语义。
- 最小方案：
  - 先构造 `newItem`，再调用 `RyanJsonReplaceBy*`。
- 失败路径：
  - Replace 返回 false：调用方复用或 `RyanJsonDelete(newItem)`。
- 所有权：
  - Replace 成功：`newItem` 转移给父节点。
  - Replace 失败：`newItem` 仍归调用方。
- 验证点：
  - 失败后 `newItem` 仍可访问，且原树未被破坏。

## 模板 4：Add/Insert 失败谁释放
- 前提：
  - 先确认 `item` 是否为游离节点。
  - 不要把 Add/Insert 失败语义套成 Replace。
- 最小方案：
  - `Create item -> IsDetachedItem -> Add/Insert`
- 失败路径：
  - 游离节点失败：当前实现下通常由库侧清理，不要盲目二次释放。
  - 非游离节点失败：返回 false，但库不会替你破坏原树。
- 所有权：
  - 成功后转移给父节点。
  - 失败后是否仍归调用方，取决于节点是否游离和具体 API 路径。
- 验证点：
  - 已挂树节点重复插入不会破坏原树。

## 模板 5：Detach 后怎么重挂
- 前提：
  - `Detach` 成功后节点归调用方。
- 最小方案：
  - `detached = Detach* -> AddItem*/Insert`
- 失败路径：
  - 重挂失败：按目标 API 的失败语义处理 `detached`。
- 所有权：
  - `Detach` 后必须“重挂”或“释放”。
- 验证点：
  - 迁移前后旧父节点和新父节点结构都可验证。

## 模板 6：Print 还是 Minify
- 前提：
  - 先区分“树输出”还是“已有文本清洗”。
- 最小方案：
  - 树输出：`RyanJsonPrint(..., RyanJsonFalse, ...)` 或 `RyanJsonPrintPreallocated(..., RyanJsonFalse, ...)`
  - 文本清洗：`RyanJsonMinify`
- 失败路径：
  - Print 失败：按输出 API 释放或回退。
  - Minify 失败：按输入缓冲与长度约束处理。
- 所有权：
  - `RyanJsonPrint` 的返回缓冲由调用方 `RyanJsonFree`。
  - `Minify` 不负责生成新的树或接管输入缓冲。
- 验证点：
  - 不要把 `Minify` 当成主打印路径。
