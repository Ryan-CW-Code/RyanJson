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
// 无论是否开启断言都会“求值”，但只有在开启断言时才会 assert。 保留EX的副作用
#define RyanJsonAssertAlwaysEval(EX) ((void)(EX))
#endif

// Json 的最基础节点，所有 Json 元素都由该节点表示。
// 结构体中仅包含固定的 next 指针，用于单向链表串联。
// 其余数据（flag、key、stringValue、numberValue、doubleValue 等）均通过动态内存分配管理。
struct RyanJsonNode
{
	// 理论上next的低2位也是可以利用起来的
	struct RyanJsonNode *next; // 单链表节点指针

	/**
	 * @brief RyanJson 节点结构体
	 * 每个节点由链表连接，包含元数据标识 (Flag) 与动态载荷存储区。
	 *
	 * 内存布局：
	 * [ next指针 | flag(1字节) | padding/指针空间 | 动态载荷区 ]
	 *
	 * @brief 节点元数据标识 (Flag)
	 * 紧跟 next 指针后，利用 1 字节位域描述节点类型及存储状态。
	 *
	 * flag 位分布定义：
	 * bit7   bit6   bit5   bit4   bit3   bit2   bit1   bit0
	 * -----------------------------------------------------
	 * strMode KeyLen KeyLen HasKey NumExt Type2  Type1  Type0
	 *
	 * 各位含义：
	 * - bit0-2 : 节点类型
	 *            000=Unknown, 001=Null, 010=Bool, 011=Number,
	 *            100=String, 101=Array, 110=Object, 111=Reserved
	 *
	 * - bit3   : 扩展位
	 *            Bool 类型：0=false, 1=true
	 *            Number 类型：0=int32_t(4字节), 1=double(8字节)
	 *
	 * - bit4-5 : Key 长度字段字节数
	 *            00:无key
	 *            01:keyLen=1字节 (≤UINT8_MAX)
	 *            10:keyLen=2字节 (≤UINT16_MAX)
	 *            11:keyLen=4字节 (≤UINT32_MAX)
	 *
	 * - bit6   : 表示key / strValue 存储模式
	 *            0:inline 模式, 1:ptr 模式
	 *
	 * - bit7   : 表示是否为当前链表的最后一位，是的话nexe指针会指向Parent(线索化链表)
	 *            0:next 指向兄弟节点, 1:next 指向Parent节点
	 *
	 * @brief 动态载荷存储区
     * 目的：
     * - 在保持 API 易用性和稳定性的同时，最大限度减少 malloc 调用次数。
     * - 尤其在嵌入式平台，malloc 代价高昂：不仅有堆头部空间浪费，还会产生内存碎片。
     * - 通过利用结构体内的对齐填充 (Padding) 和指针空间，形成一个灵活的缓冲区。
     *
     * 存储策略：
     * 利用结构体内存对齐产生的 Padding（如 Flag 后的空隙）以及原本用于存储指针的空间，形成一个缓冲区
     * 若节点包含 key / strValue，则可能有两种方案：
     * inline 模式 (小数据优化)
     *    - 当 (KeyLen + Key + Value) 的总长度 ≤ 阈值时，直接存储在结构体内部。
     *    - 阈值计算公式：
     *        阈值 = Padding + sizeof(void*) + (malloc头部空间的一半)，再向上对齐到字节边界。
     *      举例：
     *        - 内存对齐：4字节
     *        - malloc头部空间：8字节
     *        - 可用空间 = 3 (flag后padding) + 4 (指针空间) + 4 (malloc头部一半)
     *        - 向上对齐后得到阈值12字节
     *    - 存储布局：
     *        [ KeyLen | Key | Value ]
     *      起始地址即为 flag 之后，数据紧凑排列，无需额外 malloc。
     *
     * ptr 模式 (大数据)
     *    - 当数据长度 > 阈值时，结构体存储一个指针，指向独立的堆区。
     *    - 存储布局：
     *        [ KeyLen | *ptr | Padding ] -> (ptr指向) [ Key | Value ]
     *    - KeyLen 的大小由 flag 中的长度字段决定 (最多 4 字节)。
     *    - 这样保证大数据不会撑爆结构体，同时保持 API 一致性。

     * 其他类型的存储：
	 * - null / bool : 由 flag 位直接表示，无需额外空间。
	 * - number      : 根据 flag 扩展位决定存储 int32_t(4字节) 或 double(8字节)。
	 * - object      : 动态分配空间存储子节点，采用链表结构。
     *
     * 设计考量：
     * - malloc 在嵌入式平台的开销：
     *    * RTT 最小内存管理算法中，malloc 头部约 12 字节(可以考虑tlsf算法头部空间仅4字节，内存碎片也控制的很好，适合物联网应用)。
     *    * 一个 RyanJson 节点本身可能只有个位数字节，头部空间就让内存占用翻倍。
     * - 因此：
     *    * 小数据尽量 inline 存储，避免二次 malloc。
     *    * 大数据 fallback 到 ptr 模式，保证灵活性。
     * - 修改场景：
     *    * 理想情况：节点结构体后面直接跟 key/strValue，修改时释放并重新申请节点。
     *    * 但这样 changKey/changStrValue 接口改动太大，用户层需要修改指针，代价高。
     *    * 实际策略：提供就地修改接口。
     *        - 若新值长度 ≤ 原有 inline 缓冲区，直接覆盖。
     *        - 若超过阈值，自动切换到 ptr 模式，用户层无需关心。
	 *
     * 链表结构示例：
     *   {
     *       "name": "RyanJson",
     *   next (
     *       "version": "xxx",
     *   next (
     *       "repository": "https://github.com/Ryan-CW-Code/RyanJson",
     *   next (
     *       "keywords": [
     *           "json",
     *       next (
     *           "streamlined",
     *       next (
     *           "parser"
     *       ))
     *       ],
     *   next (
     *       "others": { ... }
     *   }
	 */
};

typedef struct RyanJsonNode *RyanJson_t;

typedef enum
{
	// 类型标志 占用8字节,剩余一个备用
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

// bool跟number用一个字段，因为bool和number类型不会同时存在
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
 * @note Object key 必须唯一，重复 key 会失败（Parse 也拒绝重复 key）。
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
 */
extern RyanJsonBool_e RyanJsonReplaceByKey(RyanJson_t pJson, const char *key, RyanJson_t item);
extern RyanJsonBool_e RyanJsonReplaceByIndex(RyanJson_t pJson, uint32_t index, RyanJson_t item); // object对象也可以使用，但是不推荐

#ifdef __cplusplus
}
#endif

#endif
