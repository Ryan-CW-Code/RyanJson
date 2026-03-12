# RyanJson API 说明（公开接口，使用导向）

## 范围
- 本页是公开 API 的语义速查，不展开内部实现优化细节。
- 场景化路径优先看 `apiPatterns.md`，所有权细节看 `ownershipAndErrors.md`。
- hooks 与平台接入看 `hooksInitPolicy.md`、`rtThreadExamples.md`。

## 0. 初始化与配置
### `RyanJsonInitHooks(malloc, free, realloc)`
- 必须最先调用。
- 成功返回 `RyanJsonTrue`，失败返回 `RyanJsonFalse`。
- 失败后禁止继续调用 Parse/Create/Add/Replace/Print 等 API。
- 依据：当前实现的 `jsonMalloc/jsonFree/jsonRealloc` 全局指针默认是 `NULL`（`RyanJson/RyanJson.c`）。

### `RyanJsonStrictObjectKeyCheck`（`RyanJsonConfig.h`）
- `true`：Object 下拒绝重复 key（Parse/Insert/ReplaceByIndex 等路径更严格）。
- `false`：允许重复 key；按 key 查询/替换/删除通常命中第一个，语义由上层约束。

### `RyanJsonFree(void *block)`
- 用于释放 `RyanJsonPrint` 返回的动态字符串。

## 1. Parse 类
### `RyanJsonParse(const char *text)`
- 输入 `\0` 结尾字符串。
- 成功返回根节点，失败返回 `NULL`。
- 成功返回值由调用方 `RyanJsonDelete`。
- 默认等价于 `RyanJsonParseOptions(text, strlen(text), RyanJsonFalse, NULL)`。
- **默认是非严格尾部模式**（允许尾部存在未消费数据），以当前实现为准。

### `RyanJsonParseOptions(text, size, requireNullTerminator, parseEndPtr)`
- 适合非 `\0` 缓冲区或精确控制解析终点。
- `requireNullTerminator = RyanJsonTrue` 时，解析后仅允许尾部空白。

## 2. Create 类
### 标量创建
- `RyanJsonCreateNull(key)`
- `RyanJsonCreateBool(key, boolean)`
- `RyanJsonCreateInt(key, number)`
- `RyanJsonCreateDouble(key, number)`
- `RyanJsonCreateString(key, string)`

### 容器创建
- `RyanJsonCreateObject()`
- `RyanJsonCreateArray()`
- `RyanJsonCreateIntArray(numbers, count)`
- `RyanJsonCreateDoubleArray(numbers, count)`
- `RyanJsonCreateStringArray(strings, count)`

语义：
- Create 成功后节点归调用者。
- 节点未挂到父树前，异常路径必须由调用者释放。

## 3. Add / Insert 类
### 常用语法糖
- Object：`RyanJsonAddIntToObject` / `RyanJsonAddStringToObject` ...
- Array：`RyanJsonAddIntToArray` / `RyanJsonAddStringToArray` ...

### `RyanJsonAddItemToObject(pJson, key, item)` / `RyanJsonAddItemToArray`
- `AddItem` 仅接受 `Array/Object` 节点。
- 标量请使用 `AddInt/AddString/...`。

### `RyanJsonInsert(pJson, index, item)`
- `index=0` 头插。
- `index=UINT32_MAX` 或越界可视为尾插。
- Object 场景要求 `item` 带 key。

所有权：
- 成功：`item` 转移到父节点。
- 失败：
  - `item` 为游离节点时，`Add/Insert` 失败路径由库侧清理。
  - `item` 非游离节点时，直接返回 false，不接管释放（保护原树）。
  - `AddItemToObject` 传入标量时会直接失败并删除该标量节点（当前实现语义）。

## 4. Change 类（仅同类型改值）
- `RyanJsonChangeKey(pJson, key)`
- `RyanJsonChangeStringValue(pJson, strValue)`
- `RyanJsonChangeIntValue(pJson, number)`
- `RyanJsonChangeDoubleValue(pJson, number)`
- `RyanJsonChangeBoolValue(pJson, boolean)`

规则：
- Change 会做基础入参/类型校验，失败返回 `RyanJsonFalse`。
- Change 不做类型切换；类型切换请用 Replace。

## 5. Replace 类（类型切换主入口）
### `RyanJsonReplaceByIndex(pJson, index, item)`
- 数组/对象都可用（对象场景更推荐 `ReplaceByKey`）。

### `RyanJsonReplaceByKey(pJson, key, item)`
- 仅 Object。
- `item` 无 key 时会按目标 key 包装。
- `item` 有 key 且不同于目标 key 时会尝试改 key。

规则：
- 调用前要求 `item` 为游离节点（`RyanJsonIsDetachedItem`）。
- 成功：旧节点被删除，新节点接管到树中。
- 失败：默认不消费 `item`，调用方负责复用或释放。

## 6. Get / Has / 路径类
- `RyanJsonGetObjectByKey` / `RyanJsonGetObjectByIndex`
- `RyanJsonHasObjectByKey` / `RyanJsonHasObjectByIndex`
- `RyanJsonGetObjectToKey` / `RyanJsonGetObjectToIndex`

关键约束：
- `GetKey/GetString/GetInt/GetDouble/GetBool/GetObjectValue` 这类取值前，必须先判空并用 `RyanJsonIsXXX` 判型。

## 7. Detach / Delete 类
### `RyanJsonDetachByKey/DetachByIndex`
- 从树中摘除节点并返回。
- 返回节点归调用者，必须手动 `RyanJsonDelete`（或再次挂树）。

### `RyanJsonDeleteByKey/DeleteByIndex`
- 直接删除目标节点，不返回。

### `RyanJsonDelete(root)`
- 删除整棵树。

## 8. Print / Minify / Compare
- `RyanJsonPrint`：动态输出，返回值用 `RyanJsonFree`。
- `RyanJsonPrintPreallocated`：预分配输出，适合 RT-Thread 固定缓冲。
- 传输场景优先：`Print(..., RyanJsonFalse, ...)` / `PrintPreallocated(..., RyanJsonFalse, ...)` 直接输出紧凑 Json。
- `RyanJsonMinify`：原地文本清洗（去空白/注释），用于已有 Json 文本处理，不作为首选传输输出路径。
- `RyanJsonMinify` 终止符规则：
  - 返回值 `< textLen`：会写入 `\0`；
  - 返回值 `== textLen`：不会额外写入 `\0`，调用方需自行保证字符串终止空间。
- `RyanJsonCompare` / `RyanJsonCompareOnlyKey` / `RyanJsonCompareDouble`。

## 依据（仓库内）
- `RyanJson/RyanJson.c`（`RyanJsonInitHooks` 全局 hooks 初始化）
- `RyanJson/RyanJsonParse.c`（`RyanJsonParse -> RyanJsonParseOptions(..., RyanJsonFalse, NULL)`）
- `RyanJson/RyanJsonItem.c`（`RyanJsonInsert` 失败清理、`RyanJsonAddItemToObject` 标量失败删除、`RyanJsonReplaceByKey/ByIndex` 失败不消费）
- `test/unityTest/cases/core/testCreate.c`（Insert/Add 失败语义与已挂树拒绝）
- `test/unityTest/cases/core/testReplace.c`（Replace 失败所有权）
- `test/unityTest/cases/utils/testPrint.c`（`format=false` 紧凑输出与 preallocated 行为）
- `RyanJson/RyanJson.c` 中 `RyanJsonMinify` 实现
- `test/unityTest/cases/utils/testUtils.c`、`test/unityTest/cases/utils/testRobust.c`、`test/fuzzer/cases/fuzzerMinify.c`

## 9. 对外/对内边界
- `RyanJsonChangeObjectValue` 属于内部实现接口，不作为公开 API 使用。
- 对外业务修改对象/数组内容，请通过 Add/Insert/Detach/Replace 组合完成。
