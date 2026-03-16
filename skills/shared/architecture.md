# RyanJson 数据结构与内部实现（基于源码事实）

## 1. 范围
- 本文仅记录源码与注释中可直接验证的事实，不做推断。
- 需要更细节时，请直接查看对应源码路径。

## 2. 核心数据结构
- `RyanJson` 的基础节点是 `struct RyanJsonNode`，节点通过 `next` 指针组成单向链表。
- 节点的元数据通过 1 字节 `flag` 位域编码，包含类型、bool/number 扩展位、key 长度字段长度、字符串存储模式、是否为最后一个兄弟节点等信息。
- 对外类型枚举为 `RyanJsonTypeNull/Bool/Number/String/Array/Object`，`RyanJson_t` 为节点指针类型别名。
证据路径：`RyanJson/RyanJson.h`。

## 3. 载荷与字符串存储
- 节点载荷位于结构体之后，`flag` 后按类型写入 key、strValue、number、children 指针等数据。
- key 长度字段采用“长度占用字节数”编码方式（1/2/4 字节），由 `flag` 位域决定。
- `RyanJsonInlineStringSize` 定义 key/短字符串的内联阈值，`RyanJsonInternalChangeString` 依据阈值选择内联或指针模式，并用 `flag` 标记指针模式。
- `RyanJsonInternalNewNode` 依据节点类型计算分配尺寸，Number 会按 int32_t 或 double 追加空间，Array/Object 会追加子节点指针空间，key/字符串会追加内联区。
证据路径：`RyanJson/RyanJson.h`、`RyanJson/RyanJsonInternal.h`、`RyanJson/RyanJsonUtils.c`。

## 4. 线索化单链表与父子关系
- Object/Array 的子节点以单向链表组织，插入由 `RyanJsonInternalListInsertAfter` 统一维护。
- `IsLast` 标志表示“当前节点为本层最后一个兄弟”，此时 `next` 保存父节点指针，形成线索化回链。
- `RyanJsonGetNext` 对外屏蔽父节点线索，若为最后节点则返回 NULL；内部获取父节点通过 `RyanJsonInternalGetParent`，其实现是遍历到最后兄弟再取回链。
证据路径：`RyanJson/RyanJsonUtils.c`、`RyanJson/RyanJsonItem.c`。

## 5. 内存与生命周期
- 内存由 `RyanJsonInitHooks` 注入的 `jsonMalloc/jsonFree/jsonRealloc` 管理。
- 动态扩容优先使用 `realloc`，否则走“malloc + memcpy + free”的扩容路径。
- 删除树时使用迭代遍历，遇到指针模式字符串会先释放外部缓冲区，再释放节点本体。
- 修改字符串时若从指针模式切换，会在成功后释放旧缓冲区。
证据路径：`RyanJson/RyanJson.c`、`RyanJson/RyanJsonUtils.c`。

## 6. 解析实现
- 解析使用 `RyanJsonParseBuffer`（指针 + 剩余长度）与白名单字符跳过函数。
- 字符串解析先预扫长度与转义标记，再实际拷贝内容。
- 数字解析支持符号、整数、小数与指数，结果区分 int32_t 与 double。
- 主流程 `RyanJsonParseIterative` 采用迭代而非递归，通过 `scopeParent` 与 `lastSibling` 维护层级，插入子节点时统一调用 `RyanJsonInternalListInsertAfter`。
- 严格 key 模式下解析阶段即拒绝重复 key。
证据路径：`RyanJson/RyanJsonParse.c`、`RyanJson/RyanJsonConfig.h`。

## 7. 打印实现
- 打印缓冲区结构包含 `bufAddress/cursor/size/isNoAlloc`，追加写入时可动态扩容或在禁止扩容时直接失败返回。
- Number 打印区分 int32_t 与 double；NaN/Inf 输出为 `null`；非科学计数法时会裁剪尾部无效 0。
- 打印过程使用迭代遍历；兄弟节点通过 `RyanJsonGetNext` 访问，回溯到父节点时利用 `IsLast` 线索。
证据路径：`RyanJson/RyanJsonPrint.c`。

## 8. 复制与比较
- `RyanJsonDuplicate` 通过迭代遍历完成深拷贝，插入子节点时同样依赖 `RyanJsonInternalListInsertAfter` 维护链表与 `IsLast`。
- `RyanJsonCompare`/`RyanJsonCompareOnlyKey` 使用非递归 DFS；对象比较在严格模式下按 key 直接匹配，非严格模式下按“同 key 的出现序号”对齐。
证据路径：`RyanJson/RyanJson.c`。

## 9. 关键宏与语义开关
- `RyanJsonStrictObjectKeyCheck` 控制对象是否允许重复 key，并影响 Parse/Compare 逻辑。
- `RyanJsonDefaultAddAtHead` 与 `RyanJsonAddPosition` 控制 Add 系列默认插入方向。
- `RyanJsonInlineStringSize`、`RyanJsonMallocHeaderSize`、`RyanJsonMallocAlign` 影响内联字符串阈值与对齐策略。
- `RyanJsonDoubleBufferSize` 与 `RyanJsonSnprintfSupportScientific` 影响 double 序列化缓冲区和格式策略。
证据路径：`RyanJson/RyanJsonConfig.h`、`RyanJson/RyanJsonInternal.h`。

## 10. 源码索引
- 类型与节点结构：`RyanJson/RyanJson.h`
- 解析实现：`RyanJson/RyanJsonParse.c`
- 打印实现：`RyanJson/RyanJsonPrint.c`
- 节点操作与所有权：`RyanJson/RyanJsonItem.c`
- 复制与比较：`RyanJson/RyanJson.c`
- 内部工具与链表：`RyanJson/RyanJsonUtils.c`
- 宏与配置：`RyanJson/RyanJsonConfig.h`、`RyanJson/RyanJsonInternal.h`

## 11. 维护触发条件
- 修改 `RyanJson/*.h` 或 `RyanJson/*.c` 中的数据结构、内存布局或主路径逻辑时，需同步更新本文档对应段落与证据路径。
