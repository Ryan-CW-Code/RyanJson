#ifndef RyanJson
#define RyanJson

#ifdef __cplusplus
extern "C" {
#endif

#include "RyanJsonConfig.h"

/**
 * @brief 内部错误检查宏。
 * @note 条件失败时会打印内部日志并执行调用方传入的恢复代码。
 */
#define RyanJsonCheckCodeNoReturn(EX, code)                                                                                                \
	if (!(EX))                                                                                                                         \
	{                                                                                                                                  \
		jsonLog("\r\n[INTERNAL ERROR] %s:%d: Check failed (%s)\n", __FILE__, __LINE__, #EX);                                       \
		code                                                                                                                       \
	}

#define RyanJsonCheckCode(EX, code) RyanJsonCheckCodeNoReturn(EX, code)

#define RyanJsonCheckReturnFalse(EX) RyanJsonCheckCode(EX, return RyanJsonFalse;)
#define RyanJsonCheckReturnNull(EX)  RyanJsonCheckCode(EX, return NULL;)

/**
 * @brief 断言相关宏。
 * @note 未启用 `RyanJsonEnableAssert` 时，`RyanJsonCheckAssert` 不生效。
 */
#ifdef RyanJsonEnableAssert
#define RyanJsonCheckAssert(EX) RyanJsonCheckCode(EX, RyanJsonAssert(NULL &&#EX);)
#define RyanJsonAssertAlwaysEval(EX)                                                                                                       \
	do                                                                                                                                 \
	{                                                                                                                                  \
		if (!(EX)) RyanJsonAssert(NULL && #EX);                                                                                    \
	} while (0)
#else
#define RyanJsonCheckAssert(EX)
// 无论是否开启断言都会“求值”，但只有在开启断言时才会 assert。保留 EX 的副作用
#define RyanJsonAssertAlwaysEval(EX) ((void)(EX))
#endif

// Json 最基础节点，所有 Json 元素都由该节点表示。
// 结构体仅包含固定的 next 指针，用于单向链表串联。
// 其余数据（如 flag/key/strValue/intValue/doubleValue/objValue 等）均通过动态内存分配管理。
struct RyanJsonNode
{
	// 理论上 next 的低 2 位也可复用
	struct RyanJsonNode *next; // 单链表节点指针

	/**
	 * @brief RyanJson 节点结构与载荷布局说明（面向使用者的实现约定）。
	 * @details
	 * `struct RyanJsonNode` 本体仅保存 `next` 指针，所有元数据与真实载荷都放在结构体后面的
	 * 动态区域（payload）。该 payload 的第一个字节就是 flag，紧跟其后的区域根据 flag 语义切分。
	 * 这种布局能缩小节点本体，并让 key/strValue 的存储策略可切换。
	 *
	 * Layout（逻辑示意）:
	 * [ next | flag(1B) | keyLenField(0/1/2/4B) | inline/ptr payload ... | value ]
	 *
	 * Flag Bits（bit7..bit0）:
	 * - bit0-2: Type（Null/Bool/Number/String/Array/Object）
	 * - bit3  : Bool/Number 扩展位（Bool: true/false；Number: Int/Double）
	 * - bit4-5: keyLenField 编码（0/1/2/4 字节）
	 * - bit6  : strMode（inline/ptr）
	 * - bit7  : IsLast（1 表示 next 指向 Parent 线索）
	 *
	 * keyLenField（key 长度字段）:
	 * - 位于 flag 之后，长度由 bit4-5 编码决定。
	 * - 记录 key 的字节长度（不含 '\\0'），按低字节在前写入。
	 * - 编码值 3 表示字段宽度 4 字节（不是 3 字节）。
	 *   这是为了用 2 bit 表达 0/1/2/4 四种宽度，详见 RyanJsonInternalDecodeKeyLenField。
	 *
	 * Payload（key/strValue 存储策略）:
	 * - 固定字符串区（仅当节点有 key 或类型为 String 时存在）：
	 *   位置：flag 后固定长度 `RyanJsonInlineStringSize`。
	 *   起点：先写 keyLenField（宽度由 flag 编码 0/1/2/4 字节）。
	 *
	 * - inline 模式：
	 *   内容：keyLenField 后顺序写 key\\0 与 strValue\\0。
	 *   变体：String 节点有 strValue；key 为空则仅 strValue\\0；非 String 节点仅 key\\0。
	 *
	 * - ptr 模式：
	 *   指针槽：固定在 flag + RyanJsonKeyFeidLenMaxSize，不随 keyLenField 宽度变化。
	 *   说明：读写指针用 memcpy，规避潜在非对齐访问。
	 *   堆区：有 key 则 [key\\0]；String 节点再追加 [strValue\\0]；无 key 则仅 [strValue\\0]。
	 *
	 * - 内联判定：
	 *   条件：key/strValue 字节总和 + keyLenField 宽度 <= `RyanJsonInlineStringSize`。
	 *   说明：无 key 时 keyLenField 宽度为 0。
	 *
	 * Value 存储位置（与 key 是否存在相关）:
	 * - Number/Array/Object 的 value 位于 payload 中固定偏移处。
	 * - String 的 value 存在于 key/strValue 区域，不使用 value 偏移。
	 * - 如果节点带 key，则 value 放在 flag + RyanJsonInlineStringSize 之后；
	 *   这样无论 inline/ptr 模式，value 偏移都稳定。
	 * - 若节点无 key，则 value 紧跟 flag。
	 * - Null/Bool 仅使用 flag 位表达，无额外 payload。
	 * - 实际偏移以 RyanJsonInternalGetValue 的计算为准。
	 *
	 * inline / ptr 简化示意（payload 仅示意，不含 value 区）:
	 * - inline 示例:
	 *   key + strValue: [ flag | keyLenField | key\\0 | strValue\\0 | ... ]
	 *   key only (非 String): [ flag | keyLenField | key\\0 | ... ]
	 *   strValue only (key 为空): [ flag | keyLenField | strValue\\0 | ... ]
	 * - ptr 示例:
	 *   key + strValue: [ flag | keyLenField | (pad) | ptr | ... ]  ptr -> [ key\\0 | strValue\\0 ]
	 *   key only (非 String): [ flag | keyLenField | (pad) | ptr | ... ]  ptr -> [ key\\0 ]
	 *   strValue only (key 为空): [ flag | keyLenField | (pad) | ptr | ... ]  ptr -> [ strValue\\0 ]
	 *   padding 表示内联区未使用的剩余空间或对齐填充。
	 *
	 * Threaded List（线索化链表）:
	 * - 同层兄弟节点通过 `next` 串联。
	 * - 最后一个兄弟节点的 `next` 指向父节点，并设置 IsLast=1。
	 * - 对外遍历必须使用 `RyanJsonGetNext`，它会屏蔽父节点线索。
	 * - 因此 `next` 不是“永远指向兄弟”，IsLast=1 时它是父节点线索。
	 *
	 * Example（Object 子节点链表示意）:
	 *   root(Object)
	 *     |
	 *     +-- "a":1  -> "b":2  -> (IsLast=1, next=root)
	 *   RyanJsonGetNext("b") == NULL
	 *
	 * Array/Object 子节点与父节点线索示意:
	 *   parent(Object)
	 *     |
	 *     +-- child0 -> child1(Last, next=parent)
	 *            |
	 *            +-- grandChild0 -> grandChild1(Last, next=child1)
	 *
	 * Offset 快速对照（从 payload 起点算起，用于理解访问宏偏移）:
	 * - flag: 0
	 * - keyLenField: 1
	 * - inline/ptr payload: 1 + keyLenField 宽度
	 * - value（无 key）: 1
	 * - value（有 key）: 1 + RyanJsonInlineStringSize
	 *
	 * 修改影响范围提示:
	 * - 改动 flag 位语义或 keyLenField 编码时，需同步 RyanJsonGetKey/RyanJsonGetStringValue。
	 * - 改动 payload 布局或内联阈值时，需同步 RyanJsonInternalGetValue 与相关测试。
	 *
	 * @note 该布局依赖 flag 位语义与 keyLenField 编码规则。
	 * @note 常见误解提示:
	 * - IsLast=1 的节点其 next 不是兄弟，而是父节点线索。
	 * - 遍历同层必须使用 RyanJsonGetNext，不能直接读 next。
	 * - value 偏移与是否有 key 强相关，不能用固定结构体偏移理解。
	 * @note 修改内联阈值或 payload 布局时需同步更新注释与测试。
	 */
};

typedef struct RyanJsonNode *RyanJson_t;

typedef enum
{
	// 类型标志占用 3 bit（共 8 种，1 个保留）
	RyanJsonTypeNull = 1,
	RyanJsonTypeBool = 2,
	RyanJsonTypeNumber = 3,
	RyanJsonTypeString = 4,
	RyanJsonTypeArray = 5,
	RyanJsonTypeObject = 6,
} RyanJsonType_e;

typedef RyanJsonType_e RyanjsonType_e;

#define RyanJsonFalse (false)
#define RyanJsonTrue  (true)

/**
 * @brief 兼容历史版本类型定义。
 */
typedef bool RyanJsonBool_e;
typedef RyanJsonBool_e RyanJsonBool;

/**
 * @brief 内存钩子函数类型定义。
 */
typedef void *(*RyanJsonMalloc_t)(size_t size);
typedef void (*RyanJsonFree_t)(void *block);
typedef void *(*RyanJsonRealloc_t)(void *block, size_t size);

/**
 * @brief 底层访问宏（不建议业务侧直接使用）。
 * @note 仅在明确理解内部内存布局与位字段语义时使用。
 */
#define RyanJsonGetMask(bits)                           (((1U << (bits)) - 1))
#define RyanJsonGetPayloadPtr(pJson)                    ((uint8_t *)(pJson) + sizeof(struct RyanJsonNode))
#define RyanJsonGetPayloadFlagField(pJson, shift, mask) (((*RyanJsonGetPayloadPtr(pJson)) >> (shift)) & (mask))
#define RyanJsonSetPayloadFlagField(pJson, shift, mask, value)                                                                             \
	((*RyanJsonGetPayloadPtr(pJson)) =                                                                                                 \
		 ((*RyanJsonGetPayloadPtr(pJson)) & ~((mask) << (shift))) | ((uint8_t)(((value) & (mask)) << (shift))))

#define RyanJsonGetType(pJson)       ((RyanjsonType_e)RyanJsonGetPayloadFlagField((pJson), 0, RyanJsonGetMask(3)))
#define RyanJsonSetType(pJson, type) (RyanJsonSetPayloadFlagField((pJson), 0, RyanJsonGetMask(3), (RyanjsonType_e)(type)))

// Bool 跟 Number 共用一个字段，因为 Bool 和 Number 类型不会同时存在
#define RyanJsonGetPayloadBoolValueByFlag(pJson)             RyanJsonGetPayloadFlagField((pJson), 3, RyanJsonGetMask(1))
#define RyanJsonSetPayloadBoolValueByFlag(pJson, value)      RyanJsonSetPayloadFlagField((pJson), 3, RyanJsonGetMask(1), (value))
#define RyanJsonGetPayloadNumberIsDoubleByFlag(pJson)        RyanJsonGetPayloadFlagField((pJson), 3, RyanJsonGetMask(1))
#define RyanJsonSetPayloadNumberIsDoubleByFlag(pJson, value) RyanJsonSetPayloadFlagField((pJson), 3, RyanJsonGetMask(1), (value))

/**
 * @brief key 长度字段编码访问宏。
 * @note 该编码当前受 8bit flag 位宽限制。
 */
#define RyanJsonGetPayloadEncodeKeyLenByFlag(pJson)        RyanJsonGetPayloadFlagField((pJson), 4, RyanJsonGetMask(2))
#define RyanJsonSetPayloadEncodeKeyLenByFlag(pJson, value) RyanJsonSetPayloadFlagField((pJson), 4, RyanJsonGetMask(2), (value))

#define RyanJsonGetPayloadStrIsPtrByFlag(pJson)        RyanJsonGetPayloadFlagField((pJson), 6, RyanJsonGetMask(1))
#define RyanJsonSetPayloadStrIsPtrByFlag(pJson, value) RyanJsonSetPayloadFlagField((pJson), 6, RyanJsonGetMask(1), (value))

#define RyanJsonGetPayloadIsLastByFlag(pJson)        RyanJsonGetPayloadFlagField((pJson), 7, RyanJsonGetMask(1))
#define RyanJsonSetPayloadIsLastByFlag(pJson, value) RyanJsonSetPayloadFlagField((pJson), 7, RyanJsonGetMask(1), (value))

extern RyanJsonBool_e RyanJsonInsert(RyanJson_t pJson, uint32_t index, RyanJson_t item);
extern RyanJson_t RyanJsonGetNext(RyanJson_t pJson);

/**
 * @brief 上述底层宏与接口不建议业务侧直接调用。
 */

/**
 * @brief Json 对外接口
 */
extern RyanJsonBool_e RyanJsonInitHooks(RyanJsonMalloc_t userMalloc, RyanJsonFree_t userFree, RyanJsonRealloc_t userRealloc);
extern RyanJson_t RyanJsonParseOptions(const char *text, uint32_t size, RyanJsonBool_e requireNullTerminator,
				       const char **parseEndPtr); // 需用户释放内存
extern RyanJson_t RyanJsonParse(const char *text);                // 需用户释放内存

extern void RyanJsonDelete(RyanJson_t pJson);
extern void RyanJsonFree(void *block);

/**
 * @brief 打印风格配置
 */
typedef struct
{
	char *indent;            // 缩进字符串 (例如 "\t" 或 "  ")
	char *newline;           // 换行字符串 (例如 "\n" 或 "\r\n")
	uint8_t indentLen;       // 缩进字符串长度
	uint8_t newlineLen;      // 换行字符串长度
	uint8_t spaceAfterColon; // 冒号后空格数量
	RyanJsonBool_e format;   // 是否启用格式化逻辑
} RyanJsonPrintStyle;
extern char *RyanJsonPrintWithStyle(RyanJson_t pJson, uint32_t preset, const RyanJsonPrintStyle *style, uint32_t *len);
extern char *RyanJsonPrintPreallocatedWithStyle(RyanJson_t pJson, char *buffer, uint32_t length, const RyanJsonPrintStyle *style,
						uint32_t *len);
extern char *RyanJsonPrint(RyanJson_t pJson, uint32_t preset, RyanJsonBool_e format, uint32_t *len);
extern char *RyanJsonPrintPreallocated(RyanJson_t pJson, char *buffer, uint32_t length, RyanJsonBool_e format, uint32_t *len);

/**
 * @brief Json 杂项函数
 */
extern RyanJson_t RyanJsonDuplicate(RyanJson_t pJson); // 需用户释放内存
extern uint32_t RyanJsonMinify(char *text, int32_t textLen);
extern RyanJsonBool_e RyanJsonCompare(RyanJson_t leftJson, RyanJson_t rightJson);
extern RyanJsonBool_e RyanJsonCompareOnlyKey(RyanJson_t leftJson, RyanJson_t rightJson);
extern RyanJsonBool_e RyanJsonCompareDouble(double a, double b);
extern uint32_t RyanJsonGetSize(RyanJson_t pJson);
#define RyanJsonGetArraySize(pJson) RyanJsonGetSize(pJson)

/**
 * @brief 添加相关函数
 */
extern RyanJson_t RyanJsonCreateObject(void);                                  // 如果没有添加到父 Json，则需释放内存
extern RyanJson_t RyanJsonCreateNull(const char *key);                         // 如果没有添加到父 Json，则需释放内存
extern RyanJson_t RyanJsonCreateBool(const char *key, RyanJsonBool_e boolean); // 如果没有添加到父 Json，则需释放内存
extern RyanJson_t RyanJsonCreateInt(const char *key, int32_t number);          // 如果没有添加到父 Json，则需释放内存
extern RyanJson_t RyanJsonCreateDouble(const char *key, double number);        // 如果没有添加到父 Json，则需释放内存
extern RyanJson_t RyanJsonCreateString(const char *key, const char *string);   // 如果没有添加到父 Json，则需释放内存
extern RyanJson_t RyanJsonCreateArray(void);                                   // 如果没有添加到父 Json，则需释放内存
/**
 * @brief 语法糖
 */
extern RyanJson_t RyanJsonCreateIntArray(const int32_t *numbers, uint32_t count);
extern RyanJson_t RyanJsonCreateDoubleArray(const double *numbers, uint32_t count);
extern RyanJson_t RyanJsonCreateStringArray(const char **strings, uint32_t count);

/**
 * @brief 分离相关函数
 */
extern RyanJson_t RyanJsonDetachByIndex(RyanJson_t pJson, uint32_t index); // 需用户释放内存
extern RyanJson_t RyanJsonDetachByKey(RyanJson_t pJson, const char *key);  // 需用户释放内存

/**
 * @brief 删除相关函数
 */
extern RyanJsonBool_e RyanJsonDeleteByIndex(RyanJson_t pJson, uint32_t index);
extern RyanJsonBool_e RyanJsonDeleteByKey(RyanJson_t pJson, const char *key);

/**
 * @brief 查询函数
 */
extern RyanJson_t RyanJsonGetObjectByKey(RyanJson_t pJson, const char *key);
extern RyanJson_t RyanJsonGetObjectByIndex(RyanJson_t pJson, uint32_t index);

/**
 * @brief 工具宏
 */
#define RyanJsonMakeBool(ex) ((ex) ? RyanJsonTrue : RyanJsonFalse)

/**
 * @brief 查询函数
 */
#define RyanJsonHasObjectByKey(pJson, key)     RyanJsonMakeBool(RyanJsonGetObjectByKey(pJson, key))
#define RyanJsonHasObjectByIndex(pJson, index) RyanJsonMakeBool(RyanJsonGetObjectByIndex(pJson, index))

/**
 * @brief RyanJson 类型判断接口
 */
extern RyanJsonBool_e RyanJsonIsKey(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsNull(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsBool(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsNumber(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsString(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsArray(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsObject(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsInt(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsDouble(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonIsDetachedItem(RyanJson_t item);

/**
 * @brief 节点取值接口。
 * @note 调用前应先判空并使用 `RyanJsonIsXXX` 做类型判断。
 */
extern char *RyanJsonGetKey(RyanJson_t pJson);
extern char *RyanJsonGetStringValue(RyanJson_t pJson);
extern int32_t RyanJsonGetIntValue(RyanJson_t pJson);
extern double RyanJsonGetDoubleValue(RyanJson_t pJson);
extern RyanJson_t RyanJsonGetObjectValue(RyanJson_t pJson);
extern RyanJson_t RyanJsonGetArrayValue(RyanJson_t pJson);
extern RyanJsonBool_e RyanJsonGetBoolValue(RyanJson_t pJson);

/**
 * @brief 变参路径查询底层接口。
 * @note 建议优先使用 `RyanJsonGetObjectToKey/ToIndex` 与 `RyanJsonHasObjectToKey/ToIndex` 宏。
 */
extern RyanJson_t RyanJsonGetObjectByIndexs(RyanJson_t pJson, uint32_t index, ...);
extern RyanJson_t RyanJsonGetObjectByKeys(RyanJson_t pJson, const char *key, ...);
#define RyanJsonGetObjectToKey(pJson, key, ...)     RyanJsonGetObjectByKeys(pJson, (key), ##__VA_ARGS__, NULL)
#define RyanJsonGetObjectToIndex(pJson, index, ...) RyanJsonGetObjectByIndexs(pJson, (index), ##__VA_ARGS__, UINT32_MAX)
#define RyanJsonHasObjectToKey(pJson, key, ...)     RyanJsonMakeBool(RyanJsonGetObjectToKey(pJson, key, ##__VA_ARGS__))
#define RyanJsonHasObjectToIndex(pJson, index, ...) RyanJsonMakeBool(RyanJsonGetObjectToIndex(pJson, index, ##__VA_ARGS__))

/**
 * @brief Add 系列接口与便捷宏。
 * @note 建议调用前先用 `RyanJsonIsObject/RyanJsonIsArray` 做类型校验。
 * @note Add/Insert 在 `item` 为游离节点时失败会自动释放 `item`。
 * @note `item` 非游离节点时失败不会释放 `item`（保护原树）。
 * @note 严格模式下 Object key 必须唯一；非严格模式允许重复 key，但按 key 的 API 通常只命中首个节点。
 * @note `AddItem` 仅接受 Array/Object 节点；标量请使用 `AddInt/AddString` 等接口。
 */
#define RyanJsonAddNullToObject(pJson, key)           RyanJsonInsert(pJson, RyanJsonAddPosition, RyanJsonCreateNull(key))
#define RyanJsonAddBoolToObject(pJson, key, boolean)  RyanJsonInsert(pJson, RyanJsonAddPosition, RyanJsonCreateBool(key, boolean))
#define RyanJsonAddIntToObject(pJson, key, number)    RyanJsonInsert(pJson, RyanJsonAddPosition, RyanJsonCreateInt(key, number))
#define RyanJsonAddDoubleToObject(pJson, key, number) RyanJsonInsert(pJson, RyanJsonAddPosition, RyanJsonCreateDouble(key, number))
#define RyanJsonAddStringToObject(pJson, key, string) RyanJsonInsert(pJson, RyanJsonAddPosition, RyanJsonCreateString(key, string))
extern RyanJsonBool_e RyanJsonAddItemToObject(RyanJson_t pJson, const char *key, RyanJson_t item);

#define RyanJsonAddNullToArray(pJson)           RyanJsonAddNullToObject(pJson, NULL)
#define RyanJsonAddBoolToArray(pJson, boolean)  RyanJsonAddBoolToObject(pJson, NULL, boolean)
#define RyanJsonAddIntToArray(pJson, number)    RyanJsonAddIntToObject(pJson, NULL, number)
#define RyanJsonAddDoubleToArray(pJson, number) RyanJsonAddDoubleToObject(pJson, NULL, number)
#define RyanJsonAddStringToArray(pJson, string) RyanJsonAddStringToObject(pJson, NULL, string)
#define RyanJsonAddItemToArray(pJson, item)     RyanJsonAddItemToObject(pJson, NULL, item)

#define RyanJsonArrayForEach(pJson, item)                                                                                                  \
	for ((item) = RyanJsonIsArray(pJson) ? RyanJsonGetArrayValue(pJson) : NULL; NULL != (item); (item) = RyanJsonGetNext(item))
#define RyanJsonObjectForEach(pJson, item)                                                                                                 \
	for ((item) = RyanJsonIsObject(pJson) ? RyanJsonGetObjectValue(pJson) : NULL; NULL != (item); (item) = RyanJsonGetNext(item))

/**
 * @brief 同类型值修改接口。
 * @note 修改函数会执行基本参数/类型校验，失败返回 false。
 * @note 仍建议调用前使用 `RyanJsonIsXXX` 做前置判断。
 */
extern RyanJsonBool_e RyanJsonChangeKey(RyanJson_t pJson, const char *key);
extern RyanJsonBool_e RyanJsonChangeStringValue(RyanJson_t pJson, const char *strValue);
extern RyanJsonBool_e RyanJsonChangeIntValue(RyanJson_t pJson, int32_t number);
extern RyanJsonBool_e RyanJsonChangeDoubleValue(RyanJson_t pJson, double number);
extern RyanJsonBool_e RyanJsonChangeBoolValue(RyanJson_t pJson, RyanJsonBool_e boolean);

/**
 * @brief 节点替换接口（用于修改 value 类型）
 * @note 需要跨类型替换时使用 `ReplaceByKey/ReplaceByIndex`。
 * @note 示例：`RyanJsonReplaceByKey(root, "k", RyanJsonCreateObject());`
 * @note 示例：`RyanJsonReplaceByIndex(arr, i, RyanJsonCreateString(NULL, "v"));`
 * @note Replace 成功后，`item` 所有权转移到目标树。
 * @note Replace 失败后，调用方仍持有 `item`，需自行释放或复用。
 * @note `ReplaceByIndex` 可用于 Object，但不推荐。
 */
extern RyanJsonBool_e RyanJsonReplaceByKey(RyanJson_t pJson, const char *key, RyanJson_t item);
extern RyanJsonBool_e RyanJsonReplaceByIndex(RyanJson_t pJson, uint32_t index, RyanJson_t item);

#ifdef __cplusplus
}
#endif

#endif
