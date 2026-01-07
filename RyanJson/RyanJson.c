#include "RyanJson.h"

#ifdef isEnableFuzzer
#undef RyanJsonNestingLimit
#define RyanJsonNestingLimit 350U

#undef RyanJsonSnprintf
#include <stdarg.h>
#include <time.h>

static uint32_t RyanJsonRandRange(uint32_t min, uint32_t max)
{
	// int32_t seedp = (int32_t)time(NULL);
	// return min + rand_r(&seedp) % (max - min + 1);

	static uint64_t state = 0;
	// 初始化一次种子
	if (state == 0) { state = (uint64_t)time(NULL); }

	// Xorshift64* 算法
	state ^= state >> 12;
	state ^= state << 25;
	state ^= state >> 27;
	uint64_t result = state * 2685821657736338717ULL;

	return min + (uint32_t)(result % (max - min + 1));
}

static int32_t RyanJsonSnprintf(char *buf, size_t size, const char *fmt, ...)
{
	static uint32_t jsonsnprintCount = 1;
	jsonsnprintCount++;
	if (jsonsnprintCount % RyanJsonRandRange(10, 500) == 0) { return 0; }

	va_list args;
	va_start(args, fmt); // 每 500 次随机触发一次“失败”
	int32_t ret = vsnprintf(buf, size, fmt, args);
	va_end(args);
	return ret;
}
#endif

typedef struct
{
	const uint8_t *currentPtr; // 待解析字符串地址
	uint32_t remainSize;       // 待解析字符串剩余长度
	uint32_t depth;            // How deeply nested (in arrays/objects) is the input at the current offset.
} RyanJsonParseBuffer;

typedef struct
{
	uint8_t *bufAddress;      // 反序列化后的字符串地址
	uint32_t cursor;          // 解析到那个buf位置上了
	uint32_t size;            // 待解析字符串剩余长度, 不动态申请内存时，到达此size大小将返回失败
	RyanJsonBool_e isNoAlloc; // 是否动态申请内存
} RyanJsonPrintBuffer;

// !这部分跟 struct RyanJsonNode 要保持一致
typedef struct
{
	const char *key;
	const char *strValue;

	RyanjsonType_e type;
	RyanJsonBool_e boolIsTrueFlag;
	RyanJsonBool_e numberIsDoubleFlag;
} RyanJsonNodeInfo_t;

#define RyanJsonFlagSize               sizeof(uint8_t)
#define RyanJsonKeyFeidLenMaxSize      sizeof(uint32_t)
#define RyanJsonAlign(size, align)     (((size) + (align) - 1) & ~((align) - 1))
#define RyanJsonAlignDown(size, align) ((size) & ~((align) - 1))
#define _checkType(info, type)         ((info) == (type))
#define RyanJsonUnused(x)              (void)(x)

/**
 * @brief printBuf相关宏
 *
 */
#define printBufPutChar(printfBuf, char)                                                                                                   \
	do { ((printfBuf)->bufAddress[(printfBuf)->cursor++] = (char)); } while (0)
#define printBufPutString(printfBuf, string, len)                                                                                          \
	do                                                                                                                                 \
	{                                                                                                                                  \
		for (uint32_t i = 0; i < (uint32_t)(len); i++) printBufPutChar(printfBuf, (string)[i]);                                    \
	} while (0)
#define printBufCurrentPtr(printfBuf) (&((printfBuf)->bufAddress[(printfBuf)->cursor]))

/**
 * @brief parseBuf相关宏
 *
 */
#define parseBufAdvanceCurrentPrt(parseBuf, bytesToAdvance)                                                                                \
	do                                                                                                                                 \
	{                                                                                                                                  \
		(parseBuf)->currentPtr += (bytesToAdvance);                                                                                \
		(parseBuf)->remainSize -= (bytesToAdvance);                                                                                \
	} while (0)

// 是否还有可读的待解析文本在指定索引处
#define parseBufHasRemainAtIndex(parseBuf, index) ((index) < (parseBuf)->remainSize)
// 是否还有可读的待解析文本
#define parseBufHasRemainBytes(parseBuf, bytes)   ((parseBuf)->remainSize >= (bytes))
#define parseBufHasRemain(parseBuf)               parseBufHasRemainBytes(parseBuf, 1)

/**
 * @brief 尝试向前移动解析缓冲区指针
 *
 * @param parseBuf
 * @param bytesToAdvance
 * @return RyanJsonBool_e
 */
static inline RyanJsonBool_e parseBufTyrAdvanceCurrentPrt(RyanJsonParseBuffer *parseBuf, uint32_t bytesToAdvance)
{
	RyanJsonCheckAssert(NULL != parseBuf);

#ifdef isEnableFuzzer
	static uint32_t count = 0;
	count++;
	RyanJsonCheckReturnFalse(0 != count % RyanJsonRandRange(10, 2000));
#endif

	if (parseBufHasRemainBytes(parseBuf, bytesToAdvance))
	{
		parseBufAdvanceCurrentPrt(parseBuf, bytesToAdvance);
		return RyanJsonTrue;
	}

	return RyanJsonFalse;
}

/**
 * @brief 跳过无意义的字符
 *
 * @param parseBuf
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e parseBufSkipWhitespace(RyanJsonParseBuffer *parseBuf)
{
	RyanJsonCheckAssert(NULL != parseBuf);

#ifdef isEnableFuzzer
	static uint32_t jsonskipCount = 1;
	jsonskipCount++;
	RyanJsonCheckReturnFalse(0 != jsonskipCount % RyanJsonRandRange(10, 2000));
#endif

	const uint8_t *cursor = parseBuf->currentPtr;
	while (parseBufHasRemain(parseBuf) && *cursor && (' ' == *cursor || '\n' == *cursor || '\r' == *cursor))
	{
		RyanJsonCheckReturnFalse(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1));
		// 更新本地指针以反映 buf->address 的变化（若 parseBufTyrAdvanceCurrentPrt 移动 address）
		cursor = parseBuf->currentPtr;
	}

	return RyanJsonTrue;
}

static RyanJsonMalloc_t jsonMalloc = NULL;
static RyanJsonFree_t jsonFree = NULL;
static RyanJsonRealloc_t jsonRealloc = NULL;

static RyanJsonBool_e RyanJsonParseValue(RyanJsonParseBuffer *parseBuf, char *key, RyanJson_t *out);
static RyanJsonBool_e RyanJsonPrintValue(RyanJson_t pJson, RyanJsonPrintBuffer *printfBuf, uint32_t depth, RyanJsonBool_e format);

static RyanJson_t RyanJsonCreateObjectAndKey(const char *key);
static RyanJson_t RyanJsonCreateArrayAndKey(const char *key);

/**
 * @brief 获取内联字符串的大小
 * 用函数可读性更强一点，编译器会优化的
 * @return uint32_t
 */
#define RyanJsonGetInlineStringSize                                                                                                        \
	(RyanJsonAlign((sizeof(void *) - RyanJsonFlagSize + sizeof(void *) + (RyanJsonMallocHeaderSize / 2) + RyanJsonFlagSize),           \
		       RyanJsonMallocAlign) -                                                                                              \
	 RyanJsonFlagSize)
// static uint32_t RyanJsonGetInlineStringSize(void)
// {
// 	uint32_t baseSize = sizeof(void *) - RyanJsonFlagSize;     // flag的偏移字节量
// 	baseSize += sizeof(void *) + RyanJsonMallocHeaderSize / 2; // 存储指针变量的字节量
// 	return (uint32_t)(RyanJsonAlign(baseSize + RyanJsonFlagSize, RyanJsonMallocAlign) - RyanJsonFlagSize);
// }

static inline uint8_t *RyanJsonGetHiddePrt(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);

	// 用memcpy规避非对其警告
	void *tmpPtr = NULL;
	RyanJsonMemcpy((void *)&tmpPtr, (RyanJsonGetPayloadPtr(pJson) + RyanJsonFlagSize + RyanJsonKeyFeidLenMaxSize), sizeof(void *));
	return (uint8_t *)tmpPtr;
}
static inline void RyanJsonSetHiddePrt(RyanJson_t pJson, uint8_t *hiddePrt)
{
	RyanJsonCheckAssert(NULL != pJson);
	RyanJsonCheckAssert(NULL != hiddePrt);

	// 用memcpy规避非对其警告
	void *tmpPtr = hiddePrt;
	// uint8_t是flag，uint32_t是记录key的长度空间
	RyanJsonMemcpy((RyanJsonGetPayloadPtr(pJson) + RyanJsonFlagSize + RyanJsonKeyFeidLenMaxSize), (const void *)&tmpPtr,
		       sizeof(void *));
}

/**
 * @brief 获取隐藏指针在某个索引处的值
 *
 * @param pJson
 * @param index
 * @return uint8_t*
 */
static inline uint8_t *RyanJsonGetHiddenPtrAt(RyanJson_t pJson, uint32_t index)
{
	RyanJsonCheckAssert(NULL != pJson);
	return (uint8_t *)(RyanJsonGetHiddePrt(pJson) + (index));
}

static inline void RyanJsonSetKeyLen(RyanJson_t pJson, uint32_t value)
{
	RyanJsonCheckAssert(NULL != pJson);
	uint8_t *buf = RyanJsonGetPayloadPtr(pJson) + RyanJsonFlagSize;
	uint8_t keyFieldLen = RyanJsonGetPayloadEncodeKeyLenByFlag(pJson);
	RyanJsonCheckAssert(keyFieldLen <= RyanJsonKeyFeidLenMaxSize);

	// 使用大小端无关的方式写入
	for (uint8_t i = 0; i < keyFieldLen; i++)
	{
		buf[i] = (uint8_t)(value & 0xFF);
		value >>= 8;
	}
}

static inline uint32_t RyanJsonGetKeyLen(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);
	uint8_t *buf = RyanJsonGetPayloadPtr(pJson) + RyanJsonFlagSize;
	uint8_t keyFieldLen = RyanJsonGetPayloadEncodeKeyLenByFlag(pJson);
	RyanJsonCheckAssert(keyFieldLen <= RyanJsonKeyFeidLenMaxSize);

	// 使用大小端无关的方式读取
	uint32_t value = 0;
	for (uint8_t i = 0; i < keyFieldLen; i++) { value |= ((uint32_t)buf[i]) << (i * 8); }
	return value;
}

static inline void *RyanJsonGetValue(RyanJson_t pJson)
{
	RyanJsonCheckAssert(NULL != pJson);

	uint32_t len = RyanJsonFlagSize;
	//? 目前的场景不会发生 RyanJsonIsKey(pJson) 为false的情况
	// if (RyanJsonIsKey(pJson) || RyanJsonIsString(pJson))
	if (RyanJsonIsKey(pJson))
	{
		len += RyanJsonGetInlineStringSize;
		// jsonLog(" keyLen: %d, keyLenField: %d, \r\n", RyanJsonGetKeyLen(pJson),
		// RyanJsonGetPayloadEncodeKeyLenByFlag(pJson));
	}

	return RyanJsonGetPayloadPtr(pJson) + len;
}

/**
 * @brief 用户不要使用，仅考虑realloc增大情况，没有考虑减少
 *
 * @param block
 * @param oldSize
 * @param newSize
 * @return void*
 */
static void *RyanJsonExpandRealloc(void *block, uint32_t oldSize, uint32_t newSize)
{
	// 不考虑 block 为空的情况
	RyanJsonCheckAssert(NULL != block);
	if (NULL != jsonRealloc) { return jsonRealloc(block, newSize); }

	void *newBlock = jsonMalloc(newSize);
	RyanJsonCheckReturnNull(NULL != newBlock);

	RyanJsonMemcpy(newBlock, block, oldSize);
	jsonFree(block);
	return newBlock;
}

/**
 * @brief 计算长度字段所需的字节数
 *
 * @param len
 * @return uint8_t
 */
static inline uint8_t RyanJsonCalcLenBytes(uint32_t len)
{
	if (len < 0xff) { return 0; }
	if (len < 0xffff) { return 1; }
	if (len < 0xffffff) { return 2; }
	return 3;
}

/**
 * @brief 申请buf容量, 决定是否进行扩容
 *
 * @param buf
 * @param needed
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e printBufAppend(RyanJsonPrintBuffer *printfBuf, uint32_t needed)
{
	RyanJsonCheckAssert(NULL != printfBuf && NULL != printfBuf->bufAddress);

	needed += printfBuf->cursor;

	// 当前 buf 中有足够的空间
	if (needed < printfBuf->size) { return RyanJsonTrue; }

	// 不使用动态内存分配
	RyanJsonCheckReturnFalse(RyanJsonFalse == printfBuf->isNoAlloc);

	uint32_t size = needed + RyanJsonPrintfPreAlloSize;
	char *address = (char *)RyanJsonExpandRealloc(printfBuf->bufAddress, printfBuf->size, size);
	RyanJsonCheckReturnFalse(NULL != address);

	printfBuf->size = size;
	printfBuf->bufAddress = (uint8_t *)address;
	return RyanJsonTrue;
}

/**
 * @brief 替换json对象节点
 *
 * @param prev
 * @param oldItem
 * @param newItem
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e RyanJsonReplaceNode(RyanJson_t prev, RyanJson_t oldItem, RyanJson_t newItem)
{
	RyanJsonCheckAssert(NULL != oldItem && NULL != newItem);

	// 链接前驱和新节点
	if (NULL != prev) { prev->next = newItem; }

	// 链接后继和新节点
	if (NULL != oldItem->next) { newItem->next = oldItem->next; }

	oldItem->next = NULL;
	return RyanJsonTrue;
}

static inline RyanJsonBool_e RyanJsonChangeObjectValue(RyanJson_t pJson, RyanJson_t objValue)
{
	RyanJsonMemcpy(RyanJsonGetValue(pJson), (void *)&objValue, sizeof(void *));
	return RyanJsonTrue;
}

static RyanJsonBool_e RyanJsonChangeString(RyanJson_t pJson, RyanJsonBool_e isNew, const char *key, const char *strValue)
{
	RyanJsonCheckAssert(NULL != pJson);

	uint32_t keyLen = 0;      // key的长度
	uint8_t keyLenField = 0;  // 记录key长度需要几个字节
	uint32_t strValueLen = 0; // stringValue的长度

	uint32_t mallocSize = 0;

	// 获取需要 malloc 的设备
	if (NULL != key)
	{
		keyLen = RyanJsonStrlen(key);
		keyLenField = RyanJsonCalcLenBytes(keyLen);
		mallocSize += keyLen + 1;

#ifdef isEnableFuzzer
		{
			RyanJsonAssert(0 == RyanJsonCalcLenBytes(0xff - 1));
			RyanJsonAssert(1 == RyanJsonCalcLenBytes(0xffff - 1));
			RyanJsonAssert(2 == RyanJsonCalcLenBytes(0xffffff - 1));
			RyanJsonAssert(3 == RyanJsonCalcLenBytes(UINT32_MAX - 1));
		}
#endif
	}

	if (NULL != strValue)
	{
		strValueLen = RyanJsonStrlen(strValue);
		mallocSize += strValueLen + 1;
	}
	if (0 == mallocSize) { return RyanJsonTrue; }

	// 释放旧的内存
	uint8_t *oldPrt = NULL;
	if (RyanJsonFalse == isNew)
	{
		if (RyanJsonTrue == RyanJsonGetPayloadStrIsPtrByFlag(pJson))
		{
			RyanJsonCheckAssert(RyanJsonIsKey(pJson) || RyanJsonIsString(pJson));
			oldPrt = RyanJsonGetHiddePrt(pJson);
		}
	}

	char arr[RyanJsonGetInlineStringSize] = {0};
	// keyLenField(0-3) + 1 为key的长度
	if ((mallocSize + keyLenField + 1) <= RyanJsonGetInlineStringSize)
	{
		RyanJsonSetPayloadStrIsPtrByFlag(pJson, RyanJsonFalse);
		RyanJsonMemcpy(arr, key, keyLen);
		RyanJsonMemcpy(arr + keyLen, strValue, strValueLen);
	}
	else
	{
		// 申请新的空间
		uint8_t *newPtr = (uint8_t *)jsonMalloc(mallocSize);
		RyanJsonCheckReturnFalse(NULL != newPtr);

		// 先赋值，因为 RyanJsonSetHiddePrt 可能会覆盖key和string的空间
		if (NULL != key)
		{
			if (0 != keyLen) { RyanJsonMemcpy(newPtr, key, keyLen); }
			newPtr[keyLen] = '\0';
		}

		if (NULL != strValue)
		{
			uint8_t *strValueBuf = newPtr;
			if (NULL != key) { strValueBuf = newPtr + keyLen + 1; }

			if (0 != strValueLen) { RyanJsonMemcpy(strValueBuf, strValue, strValueLen); }
			strValueBuf[strValueLen] = '\0';
		}

		RyanJsonSetHiddePrt(pJson, newPtr);
		RyanJsonSetPayloadStrIsPtrByFlag(pJson, RyanJsonTrue);
	}

	// 设置key
	if (NULL != key)
	{
		RyanJsonSetPayloadWhiteKeyByFlag(pJson, RyanJsonTrue);
		RyanJsonSetPayloadEncodeKeyLenByFlag(pJson, keyLenField);
		RyanJsonSetKeyLen(pJson, keyLen);

		jsonLog(" keyLen: %d, keyLenField: %d, \r\n", RyanJsonGetKeyLen(pJson), RyanJsonGetPayloadEncodeKeyLenByFlag(pJson));

		if (RyanJsonFalse == RyanJsonGetPayloadStrIsPtrByFlag(pJson))
		{
			char *keyBuf = RyanJsonGetKey(pJson);
			if (0 != keyLen) { RyanJsonMemcpy(keyBuf, arr, keyLen); }
			keyBuf[keyLen] = '\0';
		}
	}
	else
	{
		RyanJsonSetPayloadWhiteKeyByFlag(pJson, RyanJsonFalse);
		RyanJsonSetPayloadEncodeKeyLenByFlag(pJson, 0);
	}

	// 设置字符串值
	if (NULL != strValue)
	{
		jsonLog("stLen: %d, strValue: %s \r\n", strValueLen, strValue);
		if (RyanJsonFalse == RyanJsonGetPayloadStrIsPtrByFlag(pJson))
		{
			char *strValueBuf = RyanJsonGetStringValue(pJson);
			if (0 != strValueLen) { RyanJsonMemcpy(strValueBuf, arr + keyLen, strValueLen); }
			strValueBuf[strValueLen] = '\0';
		}
	}

	if (oldPrt) { jsonFree(oldPrt); }
	return RyanJsonTrue;
}

static RyanJson_t RyanJsonNewNode(RyanJsonNodeInfo_t *info)
{
	RyanJsonCheckAssert(NULL != info);

	// 加1是flag的空间
	uint32_t size = sizeof(struct RyanJsonNode) + RyanJsonFlagSize;

	if (_checkType(info->type, RyanJsonTypeNumber))
	{
		if (RyanJsonFalse == info->numberIsDoubleFlag) { size += sizeof(int32_t); }
		else
		{
			size += sizeof(double);
		}
	}
	else if (_checkType(info->type, RyanJsonTypeArray) || _checkType(info->type, RyanJsonTypeObject)) { size += sizeof(RyanJson_t); }

	if (NULL != info->key || _checkType(info->type, RyanJsonTypeString)) { size += RyanJsonGetInlineStringSize; }

	RyanJson_t pJson = (RyanJson_t)jsonMalloc((size_t)size);
	RyanJsonCheckReturnNull(NULL != pJson);

	// 只清空结构体就行了
	RyanJsonMemset(pJson, 0, size); // 这个size很小，没有优化的必要，直接memset吧

	RyanJsonSetType(pJson, info->type);

	RyanJsonCheckCode(RyanJsonTrue == RyanJsonChangeString(pJson, RyanJsonTrue, info->key, info->strValue), {
		jsonFree(pJson);
		return NULL;
	});

	if (_checkType(info->type, RyanJsonTypeBool)) { RyanJsonSetPayloadBoolValueByFlag(pJson, info->boolIsTrueFlag); }
	else if (_checkType(info->type, RyanJsonTypeNumber)) { RyanJsonSetPayloadNumberIsDoubleByFlag(pJson, info->numberIsDoubleFlag); }

	return pJson;
}

/**
 * @brief 创建一个item对象
 * 带有key的对象才可以方便的通过replace替换，
 * !此接口不推荐用户调用
 *
 * @param key
 * @param item
 * @return RyanJson_t
 */
static RyanJson_t RyanJsonCreateItem(const char *key, RyanJson_t item)
{
	RyanJsonCheckReturnNull(NULL != item);

	RyanJsonNodeInfo_t nodeInfo = {
		.type = _checkType(RyanJsonGetType(item), RyanJsonTypeArray) ? RyanJsonTypeArray : RyanJsonTypeObject,
		.key = key,
	};

	RyanJson_t newItem = RyanJsonNewNode(&nodeInfo);

	RyanJsonCheckReturnNull(NULL != newItem);

	if (_checkType(RyanJsonGetType(item), RyanJsonTypeArray) || _checkType(RyanJsonGetType(item), RyanJsonTypeObject))
	{
		RyanJsonChangeObjectValue(newItem, RyanJsonGetObjectValue(item));

		if (RyanJsonTrue == RyanJsonGetPayloadStrIsPtrByFlag(item)) { jsonFree(RyanJsonGetHiddePrt(item)); }
		jsonFree(item);
	}
	else
	{
		RyanJsonChangeObjectValue(newItem, item);
	}

	return newItem;
}

/**
 * @brief 提供内存钩子函数
 *
 * @param userMalloc
 * @param userFree
 * @param userRealloc 可以为NULL
 * @return RyanJsonBool_e
 */
RyanJsonBool_e RyanJsonInitHooks(RyanJsonMalloc_t userMalloc, RyanJsonFree_t userFree, RyanJsonRealloc_t userRealloc)
{
	RyanJsonCheckReturnFalse(NULL != userMalloc && NULL != userFree);

	jsonMalloc = userMalloc;
	jsonFree = userFree;
	jsonRealloc = userRealloc;
	return RyanJsonTrue;
}

char *RyanJsonGetKey(RyanJson_t pJson)
{
	RyanJsonCheckReturnNull(NULL != pJson);
	if (RyanJsonFalse == RyanJsonGetPayloadStrIsPtrByFlag(pJson))
	{
		uint8_t keyFieldLen = RyanJsonGetPayloadEncodeKeyLenByFlag(pJson);
		RyanJsonCheckAssert(keyFieldLen <= RyanJsonKeyFeidLenMaxSize);
		return (char *)(RyanJsonGetPayloadPtr(pJson) + RyanJsonFlagSize + keyFieldLen);
	}

	return (char *)RyanJsonGetHiddenPtrAt(pJson, 0);
}

char *RyanJsonGetStringValue(RyanJson_t pJson)
{
	RyanJsonCheckReturnNull(NULL != pJson);

	uint32_t len = 0;

	if (RyanJsonFalse == RyanJsonGetPayloadStrIsPtrByFlag(pJson))
	{
		uint8_t keyFieldLen = RyanJsonGetPayloadEncodeKeyLenByFlag(pJson);
		RyanJsonCheckAssert(keyFieldLen <= RyanJsonKeyFeidLenMaxSize);

		len += keyFieldLen;
		if (RyanJsonIsKey(pJson)) { len += RyanJsonGetKeyLen(pJson) + 1U; }
		return (char *)(RyanJsonGetPayloadPtr(pJson) + RyanJsonFlagSize + len);
	}

	if (RyanJsonIsKey(pJson)) { len = RyanJsonGetKeyLen(pJson) + 1U; }

	return (char *)RyanJsonGetHiddenPtrAt(pJson, len);
}

int32_t RyanJsonGetIntValue(RyanJson_t pJson)
{
	RyanJsonCheckCodeNoReturn(NULL != pJson, { return 0; });

	int32_t intValue;
	RyanJsonMemcpy(&intValue, RyanJsonGetValue(pJson), sizeof(intValue));
	return intValue;
}

double RyanJsonGetDoubleValue(RyanJson_t pJson)
{
	RyanJsonCheckCodeNoReturn(NULL != pJson, { return 0; });

	double doubleValue;
	RyanJsonMemcpy(&doubleValue, RyanJsonGetValue(pJson), sizeof(doubleValue));
	return doubleValue;
}

RyanJson_t RyanJsonGetObjectValue(RyanJson_t pJson)
{
	RyanJsonCheckCodeNoReturn(NULL != pJson, { return 0; });

	RyanJson_t objValue;
	RyanJsonMemcpy((void *)&objValue, RyanJsonGetValue(pJson), sizeof(void *));
	return objValue;
}

RyanJson_t RyanJsonGetArrayValue(RyanJson_t pJson) { return RyanJsonGetObjectValue(pJson); }

/**
 * @brief 删除json及其子项
 *
 * @param pJson
 */
void RyanJsonDelete(RyanJson_t pJson)
{
	RyanJson_t next;

	while (pJson)
	{
		next = pJson->next;

		// 递归删除
		if (_checkType(RyanJsonGetType(pJson), RyanJsonTypeArray) || _checkType(RyanJsonGetType(pJson), RyanJsonTypeObject))
		{
			RyanJsonDelete(RyanJsonGetObjectValue(pJson)); // 递归删除子对象
		}

		if (RyanJsonTrue == RyanJsonGetPayloadStrIsPtrByFlag(pJson)) { jsonFree(RyanJsonGetHiddePrt(pJson)); }

		jsonFree(pJson);

		pJson = next;
	}
}

/**
 * @brief 释放RyanJson申请的资源
 *
 * @param block
 */
void RyanJsonFree(void *block) { jsonFree(block); }

/**
 * @brief 从字符串中获取十六进制值
 *
 * @param text
 * @return uint32_t 16进制值
 */
static RyanJsonBool_e RyanJsonParseHex(const uint8_t *text, uint32_t *value)
{
	RyanJsonCheckAssert(NULL != text && NULL != value);
	uint32_t valueTemp = 0;

	for (uint8_t i = 0; i < 4; ++i)
	{
		switch (text[i])
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': valueTemp = (valueTemp << 4) + (text[i] - '0'); break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f': valueTemp = (valueTemp << 4) + 10 + (text[i] - 'a'); break;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F': valueTemp = (valueTemp << 4) + 10 + (text[i] - 'A'); break;
		default: return RyanJsonFalse;
		}
	}

	*value = valueTemp;

	return RyanJsonTrue;
}

/**
 * @brief 内部函数：解析数字字符串为 double（用于序列化往返检查）
 * 替代 strtod 以提高嵌入式平台兼容性
 *
 * @param str 数字字符串
 * @return double 解析后的值
 */
static double RyanJsonInternalParseDouble(const char *str)
{
	double number = 0;
	int32_t scale = 0;
	int32_t e_sign = 1;
	int32_t e_scale = 0;
	RyanJsonBool_e isNegative = RyanJsonFalse;

	// 处理符号
	if ('-' == *str)
	{
		isNegative = RyanJsonTrue;
		str++;
	}

	// 整数部分
	while (*str >= '0' && *str <= '9')
	{
		number = number * 10.0 + (*str - '0');
		str++;
	}

	// 小数部分
	if ('.' == *str)
	{
		str++;
		while (*str >= '0' && *str <= '9')
		{
			number = number * 10.0 + (*str - '0');
			scale--;
			str++;
		}
	}

	// 指数部分
	if ('e' == *str || 'E' == *str)
	{
		str++;
		if ('+' == *str || '-' == *str)
		{
			e_sign = ('-' == *str) ? -1 : 1;
			str++;
		}
		while (*str >= '0' && *str <= '9')
		{
			e_scale = e_scale * 10 + (*str - '0');
			str++;
		}
	}

	// 应用符号和指数
	if (isNegative) { number = -number; }
	return number * pow(10.0, scale + e_sign * e_scale);
}

/**
 * @brief 解析文本中的数字，添加到json节点中
 *
 * @param buf 解析缓冲区
 * @param key 对应的key
 * @param out 用于接收解析后的pJson对象的地址
 * @return RyanJsonBool_e 成功或失败
 */
static RyanJsonBool_e RyanJsonParseNumber(RyanJsonParseBuffer *parseBuf, char *key, RyanJson_t *out)
{
	RyanJsonCheckAssert(NULL != parseBuf && NULL != out);

	// 记录数字起始位置
	const char *numberStart = (const char *)parseBuf->currentPtr;
	RyanJsonBool_e isInt = RyanJsonTrue;

	// 处理符号
	if ('-' == *parseBuf->currentPtr)
	{
		// 这个不会失败因为进来前已经判断过 parseBufHasRemain(parseBuf)
		RyanJsonCheckNeverNoAssert(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1));
		RyanJsonCheckReturnFalse(parseBufHasRemain(parseBuf) && *parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9');
	}

	// 跳过前导零
	while ('0' == *parseBuf->currentPtr)
	{
		RyanJsonCheckReturnFalse(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1));
		// 前导0后面不允许跟数组，比如"0123"
		RyanJsonCheckReturnFalse(parseBufHasRemain(parseBuf) && (*parseBuf->currentPtr < '0' || *parseBuf->currentPtr > '9'));
	}

	// 整数部分
	while (parseBufHasRemain(parseBuf) && *parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9')
	{
		RyanJsonCheckReturnFalse(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1));
	}

	// 小数部分
	if (parseBufHasRemain(parseBuf) && '.' == *parseBuf->currentPtr)
	{
		RyanJsonCheckReturnFalse(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1));
		RyanJsonCheckReturnFalse(parseBufHasRemain(parseBuf) && *parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9');

		while (parseBufHasRemain(parseBuf) && *parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9')
		{
			RyanJsonCheckReturnFalse(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1));
		}
		isInt = RyanJsonFalse;
	}

	// 指数部分
	if (parseBufHasRemain(parseBuf) && ('e' == *parseBuf->currentPtr || 'E' == *parseBuf->currentPtr))
	{
		RyanJsonCheckReturnFalse(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1));
		RyanJsonCheckReturnFalse(parseBufHasRemain(parseBuf));

		// 只有遇到 +/- 符号时才跳过
		if ('+' == *parseBuf->currentPtr || '-' == *parseBuf->currentPtr)
		{
			RyanJsonCheckReturnFalse(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1));
		}

		RyanJsonCheckReturnFalse(parseBufHasRemain(parseBuf) && *parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9');

		while (parseBufHasRemain(parseBuf) && *parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9')
		{
			RyanJsonCheckReturnFalse(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1));
		}
		isInt = RyanJsonFalse;
	}

	// 使用内部解析函数计算数值
	double number = RyanJsonInternalParseDouble(numberStart);

	// 创建 JSON 节点
	RyanJson_t newItem = NULL;
	if (RyanJsonTrue == isInt && number >= INT32_MIN && number <= INT32_MAX) { newItem = RyanJsonCreateInt(key, (int32_t)number); }
	else
	{
		newItem = RyanJsonCreateDouble(key, number);
	}

	RyanJsonCheckReturnFalse(NULL != newItem);

	*out = newItem;
	return RyanJsonTrue;
}

/**
 * @brief 解析文本中的字符串，添加到json节点中
 *
 * @param text 带有jsonString的文本
 * @param buf 接收解析后的字符串指针的地址
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e RyanJsonParseStringBuffer(RyanJsonParseBuffer *parseBuf, char **buffer)
{
	RyanJsonCheckAssert(NULL != parseBuf && NULL != buffer);

	uint32_t len = 0;
	*buffer = NULL;

	// 不是字符串
	RyanJsonCheckReturnFalse(parseBufHasRemain(parseBuf) && '\"' == *parseBuf->currentPtr);

	RyanJsonCheckReturnFalse(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1));

	// 获取字符串长度
	for (uint32_t i = 0;; i++)
	{
		RyanJsonCheckReturnFalse(parseBufHasRemainAtIndex(parseBuf, i));

		uint8_t ch = parseBuf->currentPtr[i];

		if (ch == '\"') { break; }

		// 检查非法控制字符 (ASCII 0–31)
		RyanJsonCheckReturnFalse(ch > 0x1F);

		if (ch == '\\') // 跳过转义符号
		{
			RyanJsonCheckReturnFalse(parseBufHasRemainAtIndex(parseBuf, i + 1));
			i++;
		}

		len++;
	}

	uint8_t *outBuffer = (uint8_t *)jsonMalloc((size_t)(len + 1U));
	RyanJsonCheckReturnFalse(NULL != outBuffer);

	uint8_t *outCurrentPtr = outBuffer;
	// ?获取字符串长度的时候已经确保 '\"' != *parseBuf->currentPtr 一定有结尾了
	while ('\"' != *parseBuf->currentPtr)
	{
		// 普通字符
		if ('\\' != *parseBuf->currentPtr)
		{
			*outCurrentPtr++ = *parseBuf->currentPtr;
			RyanJsonCheckCode(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1), { goto error__; });
			continue;
		}

		// 转义字符
		RyanJsonCheckCode(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1), { goto error__; });
		switch (*parseBuf->currentPtr)
		{

		case 'b': *outCurrentPtr++ = '\b'; break;
		case 'f': *outCurrentPtr++ = '\f'; break;
		case 'n': *outCurrentPtr++ = '\n'; break;
		case 'r': *outCurrentPtr++ = '\r'; break;
		case 't': *outCurrentPtr++ = '\t'; break;
		case '\"':
		case '\\':
		case '/': *outCurrentPtr++ = *parseBuf->currentPtr; break;

		case 'u': {
			// 获取 Unicode 字符
			uint64_t codepoint = 0;
			RyanJsonCheckCode(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 4), { goto error__; });
			uint32_t firstCode = 0;
			RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseHex(parseBuf->currentPtr - 3, &firstCode), { goto error__; });
			// 检查是否有效
			RyanJsonCheckCode(firstCode < 0xDC00 || firstCode > 0xDFFF, { goto error__; });

			if (firstCode >= 0xD800 && firstCode <= 0xDBFF) // UTF16 代理对
			{
				RyanJsonCheckCode(parseBufHasRemainAtIndex(parseBuf, 2), { goto error__; });

				RyanJsonCheckCode('\\' == parseBuf->currentPtr[1] && 'u' == parseBuf->currentPtr[2], {
					goto error__; // 缺少代理的后半部分
				});

				RyanJsonCheckCode(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 6), { goto error__; });
				uint32_t secondCode = 0;
				// 上面已经判断过，这个不应该失败
				RyanJsonCheckNeverNoAssert(RyanJsonTrue == RyanJsonParseHex(parseBuf->currentPtr - 3, &secondCode));
				RyanJsonCheckCode(secondCode >= 0xDC00 && secondCode <= 0xDFFF, {
					goto error__; // 无效的代理后半部分
				});

				codepoint = 0x10000 + (((firstCode & 0x3FF) << 10) | (secondCode & 0x3FF));
			}
			else
			{
				codepoint = firstCode;
			}

			/* encode as UTF-8
			 * takes at maximum 4 bytes to encode:
			 * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
			uint8_t utf8Length;
			uint8_t firstByteMark;
			if (codepoint < 0x80)
			{
				utf8Length = 1; // normal ascii, encoding 0xxxxxxx
				firstByteMark = 0;
			}
			else if (codepoint < 0x800)
			{
				utf8Length = 2;       // two bytes, encoding 110xxxxx 10xxxxxx
				firstByteMark = 0xC0; // 11000000
			}
			else if (codepoint < 0x10000)
			{
				utf8Length = 3;       // three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx
				firstByteMark = 0xE0; // 11100000
			}
			else
			{
				utf8Length = 4;       // four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx
				firstByteMark = 0xF0; // 11110000
			}
			// 不太可能发生
			// else
			// {
			// 	goto error__; // 无效的 unicode 代码点
			// }

			// encode as utf8
			for (uint8_t utf8Position = (uint8_t)(utf8Length - 1); utf8Position > 0; utf8Position--)
			{
				outCurrentPtr[utf8Position] = (uint8_t)((codepoint | 0x80) & 0xBF); // 10xxxxxx
				codepoint >>= 6;
			}

			// encode first byte
			if (utf8Length > 1) { outCurrentPtr[0] = (uint8_t)((codepoint | firstByteMark) & 0xFF); }
			else
			{
				outCurrentPtr[0] = (uint8_t)(codepoint & 0x7F);
			}
			outCurrentPtr += utf8Length;
			break;
		}

		default:
			// *outCurrentPtr++ = *buf->currentPtr;
			goto error__;
		}

		RyanJsonCheckCode(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1), { goto error__; });
	}
	*outCurrentPtr = '\0';

	RyanJsonCheckNeverNoAssert('\"' == *parseBuf->currentPtr);

	RyanJsonCheckCode(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1), { goto error__; });
	*buffer = (char *)outBuffer;
	return RyanJsonTrue;

error__:
	jsonFree(outBuffer);
	*buffer = NULL;
	return RyanJsonFalse;
}

/**
 * @brief 解析文本中的string节点，添加到json节点中
 *
 * @param text
 * @param key 对应的key
 * @param out 接收解析后的pJson对象的地址
 * @return const char*
 */
static RyanJsonBool_e RyanJsonParseString(RyanJsonParseBuffer *parseBuf, char *key, RyanJson_t *out)
{
	RyanJsonCheckAssert(NULL != parseBuf && NULL != out);

	char *buffer;
	RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonParseStringBuffer(parseBuf, &buffer));

	RyanJson_t newItem = RyanJsonCreateString(key, buffer);
	RyanJsonCheckCode(NULL != newItem, {
		jsonFree(buffer);
		return RyanJsonFalse;
	});

	jsonFree(buffer);
	*out = newItem;
	return RyanJsonTrue;
}

/**
 * @brief 解析文本中的数组
 *
 * @param text
 * @param key 对应的key
 * @param out 接收解析后的pJson对象的地址
 * @return const char*
 */
static RyanJsonBool_e RyanJsonParseArray(RyanJsonParseBuffer *parseBuf, char *key, RyanJson_t *out)
{
	RyanJsonCheckAssert(NULL != parseBuf && NULL != out);

	RyanJson_t newItem = RyanJsonCreateArrayAndKey(key);
	RyanJsonCheckReturnFalse(NULL != newItem);

	RyanJson_t prev = NULL, item;
	RyanJsonCheckNeverNoAssert(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1));
	RyanJsonCheckCode(RyanJsonTrue == parseBufSkipWhitespace(parseBuf), { goto error__; });

	// 空数组
	RyanJsonCheckCode(parseBufHasRemain(parseBuf), { goto error__; });
	if (*parseBuf->currentPtr == ']') { goto next__; }

	do
	{
		// 跳过 ','
		if (NULL != prev)
		{
			RyanJsonCheckCode(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1), { goto error__; });
		}

		RyanJsonCheckCode(RyanJsonTrue == parseBufSkipWhitespace(parseBuf), { goto error__; });

		RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseValue(parseBuf, NULL, &item), { goto error__; });

		RyanJsonCheckNeverNoAssert(RyanJsonTrue == RyanJsonInsert(newItem, UINT32_MAX, item));

		prev = item;

	} while (parseBufHasRemain(parseBuf) && *parseBuf->currentPtr == ',');

	RyanJsonCheckCode(RyanJsonTrue == parseBufSkipWhitespace(parseBuf), { goto error__; });
	RyanJsonCheckCode(parseBufHasRemain(parseBuf) && *parseBuf->currentPtr == ']', { goto error__; });

next__:
	RyanJsonCheckCode(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1), { goto error__; });
	*out = newItem;

	return RyanJsonTrue;

error__:
	RyanJsonDelete(newItem);
	*out = NULL;
	return RyanJsonFalse;
}

/**
 * @brief 解析文本中的对象
 *
 * @param text
 * @param key
 * @param out
 * @return const char*
 */
static RyanJsonBool_e RyanJsonParseObject(RyanJsonParseBuffer *parseBuf, char *key, RyanJson_t *out)
{
	RyanJsonCheckAssert(NULL != parseBuf && NULL != out);
	char *objKey = NULL;

	RyanJson_t newItem = RyanJsonCreateObjectAndKey(key);
	RyanJsonCheckReturnFalse(NULL != newItem);

	RyanJson_t prev = NULL, item;
	RyanJsonCheckNeverNoAssert(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1));
	RyanJsonCheckCode(RyanJsonTrue == parseBufSkipWhitespace(parseBuf), { goto error__; });

	RyanJsonCheckCode(parseBufHasRemain(parseBuf), { goto error__; });
	if (*parseBuf->currentPtr == '}') { goto next__; }

	do
	{
		if (NULL != prev)
		{
			RyanJsonCheckCode(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1), { goto error__; }); // 跳过 ','
		}

		RyanJsonCheckCode(RyanJsonTrue == parseBufSkipWhitespace(parseBuf), { goto error__; });

		RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseStringBuffer(parseBuf, &objKey), { goto error__; });
		RyanJsonCheckNeverNoAssert(NULL != objKey);

		RyanJsonCheckCode(RyanJsonTrue == parseBufSkipWhitespace(parseBuf), { goto error__; });

		// 解析指示符 ':'
		RyanJsonCheckCode(parseBufHasRemain(parseBuf) && ':' == *parseBuf->currentPtr, { goto error__; });

		RyanJsonCheckCode(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1), { goto error__; });
		RyanJsonCheckCode(RyanJsonTrue == parseBufSkipWhitespace(parseBuf), { goto error__; });

		RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseValue(parseBuf, objKey, &item), { goto error__; });
		jsonFree(objKey);
		objKey = NULL;

		RyanJsonCheckNeverNoAssert(RyanJsonTrue == RyanJsonInsert(newItem, UINT32_MAX, item));

		prev = item;

	} while (parseBufHasRemain(parseBuf) && *parseBuf->currentPtr == ',');

	RyanJsonCheckCode(RyanJsonTrue == parseBufSkipWhitespace(parseBuf), { goto error__; });
	RyanJsonCheckCode(parseBufHasRemain(parseBuf) && *parseBuf->currentPtr == '}', {
		objKey = NULL; // 由上层进行删除
		goto error__;
	});

next__:
	RyanJsonCheckCode(RyanJsonTrue == parseBufTyrAdvanceCurrentPrt(parseBuf, 1), { goto error__; });
	*out = newItem;
	return RyanJsonTrue;

error__:
	if (objKey) { jsonFree(objKey); }
	RyanJsonDelete(newItem);
	*out = NULL;
	return RyanJsonFalse;
}

/**
 * @brief 解析文本
 *
 * @param text
 * @param key
 * @param out
 * @return const char*
 */
static RyanJsonBool_e RyanJsonParseValue(RyanJsonParseBuffer *parseBuf, char *key, RyanJson_t *out)
{
	RyanJsonCheckAssert(NULL != parseBuf && NULL != out);

	parseBuf->depth++;
	RyanJsonCheckReturnFalse(parseBuf->depth < RyanJsonNestingLimit);

	RyanJsonCheckReturnFalse(parseBufHasRemain(parseBuf));

	*out = NULL;

	if (*parseBuf->currentPtr == '\"') { return RyanJsonParseString(parseBuf, key, out); }
	if (*parseBuf->currentPtr == '{') { return RyanJsonParseObject(parseBuf, key, out); }
	if (*parseBuf->currentPtr == '-' || (*parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9'))
	{
		return RyanJsonParseNumber(parseBuf, key, out);
	}
	if (*parseBuf->currentPtr == '[') { return RyanJsonParseArray(parseBuf, key, out); }

	if (parseBufHasRemainBytes(parseBuf, 4) && 0 == strncmp((const char *)parseBuf->currentPtr, "null", 4))
	{
		*out = RyanJsonCreateNull(key);
		RyanJsonCheckReturnFalse(NULL != *out);

		parseBufAdvanceCurrentPrt(parseBuf, 4);
		return RyanJsonTrue;
	}
	if (parseBufHasRemainBytes(parseBuf, 5) && 0 == strncmp((const char *)parseBuf->currentPtr, "false", 5))
	{
		*out = RyanJsonCreateBool(key, RyanJsonFalse);
		RyanJsonCheckReturnFalse(NULL != *out);

		parseBufAdvanceCurrentPrt(parseBuf, 5);
		return RyanJsonTrue;
	}
	if (parseBufHasRemainBytes(parseBuf, 4) && 0 == strncmp((const char *)parseBuf->currentPtr, "true", 4))
	{
		*out = RyanJsonCreateBool(key, RyanJsonTrue);
		RyanJsonCheckReturnFalse(NULL != *out);

		parseBufAdvanceCurrentPrt(parseBuf, 4);
		return RyanJsonTrue;
	}

	return RyanJsonFalse;
}

/**
 * @brief pJson 文本解析器
 *
 * @param text 文本地址
 * @param requireNullTerminator 输入的字符串必须以空字符 \0 结尾，并且不附带无效数据
 * @param parseEndPtr 输出解析终止的字符位置
 * @return RyanJson_t
 */
static RyanJsonBool_e RyanJsonParseCheckNullTerminator(RyanJsonParseBuffer *parseBuf, RyanJsonBool_e requireNullTerminator)
{
	RyanJsonCheckAssert(NULL != parseBuf);

	if (requireNullTerminator)
	{
		// 故意不检查
		RyanJsonCheckCode(RyanJsonTrue == parseBufSkipWhitespace(parseBuf), {});

		// 后面还有数据非空字符
		RyanJsonCheckReturnFalse(!(parseBufHasRemain(parseBuf) && *parseBuf->currentPtr));

		// // 后面还有数据
		// RyanJsonCheckReturnFalse(!parseBufHasRemainBytes(parseBuf, 1));

		// // 非空字符
		// RyanJsonCheckReturnFalse(!(parseBufHasRemain(parseBuf) && *parseBuf->currentPtr));
	}

	return RyanJsonTrue;
}

RyanJson_t RyanJsonParseOptions(const char *text, uint32_t size, RyanJsonBool_e requireNullTerminator, const char **parseEndPtr)
{
	RyanJson_t pJson;
	RyanJsonCheckReturnNull(NULL != text);

	RyanJsonParseBuffer parseBuf = {.currentPtr = (const uint8_t *)text, .remainSize = size};
	RyanJsonCheckReturnNull(RyanJsonTrue == parseBufSkipWhitespace(&parseBuf));

	RyanJsonCheckReturnNull(RyanJsonTrue == RyanJsonParseValue(&parseBuf, NULL, &pJson));

	// 检查解析后的文本后面是否有无意义的字符
	RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseCheckNullTerminator(&parseBuf, requireNullTerminator), {
		RyanJsonDelete(pJson);
		return NULL;
	});

	if (parseEndPtr) { *parseEndPtr = (const char *)parseBuf.currentPtr; }

	return pJson;
}

/**
 * @brief 反序列化数字
 *
 * @param pJson
 * @param buf
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e RyanJsonPrintNumber(RyanJson_t pJson, RyanJsonPrintBuffer *printfBuf)
{
	RyanJsonCheckAssert(NULL != pJson && NULL != printfBuf);

	int32_t len;

	// RyanJsonNumber 类型是一个整数
	if (RyanJsonFalse == RyanJsonGetPayloadNumberIsDoubleByFlag(pJson))
	{
		// RyanJsonCheckReturnFalse(printBufAppend(buf, 21)); // 64 位整数最多包含  20 个数字字符、1 符号
		// INT32_MIN = -2147483648 (11 chars)
		RyanJsonCheckReturnFalse(printBufAppend(printfBuf, 11));

		len = RyanJsonSnprintf((char *)printBufCurrentPtr(printfBuf), printfBuf->size, "%" PRId32, RyanJsonGetIntValue(pJson));
		RyanJsonCheckReturnFalse(len > 0); // snprintf 失败
		printfBuf->cursor += (uint32_t)len;
	}
	else // RyanJsonNumber 的类型是浮点型
	{
		RyanJsonCheckReturnFalse(printBufAppend(printfBuf, 64)); // 浮点数用64可以适应大部分情况
		double doubleValue = RyanJsonGetDoubleValue(pJson);
		double absDoubleValue = fabs(doubleValue);

		// 判断是否为整数
		if (fabs(floor(doubleValue) - doubleValue) <= DBL_EPSILON && absDoubleValue < 1.0e60)
		{
			// 整数情况，保留一位小数 (例如 5.0)
			len = RyanJsonSnprintf((char *)printBufCurrentPtr(printfBuf), printfBuf->size, "%.1lf", doubleValue);
			RyanJsonCheckReturnFalse(len > 0); // snprintf 失败
		}
		else
		{
			// 先用 %.15g 尝试序列化（输出更简洁）
			len = RyanJsonSnprintf((char *)printBufCurrentPtr(printfBuf), printfBuf->size, "%.15g", doubleValue);
			RyanJsonCheckReturnFalse(len > 0); // snprintf 失败

			// 往返检查：如果 %.15g 精度不够，改用 %.17g
			if (RyanJsonFalse ==
			    RyanJsonCompareDouble(RyanJsonInternalParseDouble((char *)printBufCurrentPtr(printfBuf)), doubleValue))
			{
				len = RyanJsonSnprintf((char *)printBufCurrentPtr(printfBuf), printfBuf->size, "%.17g", doubleValue);
				RyanJsonCheckReturnFalse(len > 0); // snprintf 失败
			}
		}

		printfBuf->cursor += (uint32_t)len;
	}

	return RyanJsonTrue;
}

/**
 * @brief 反序列化字符串
 *
 * @param strValue
 * @param buf
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e RyanJsonPrintStringBuffer(const uint8_t *strValue, RyanJsonPrintBuffer *printfBuf)
{
	RyanJsonCheckAssert(NULL != strValue && NULL != printfBuf);
	// 获取长度
	const uint8_t *strCurrentPtr = strValue;
	uint32_t escapeCharCount = 0;
	for (strCurrentPtr = strValue; *strCurrentPtr; strCurrentPtr++)
	{
		switch (*strCurrentPtr)
		{
		case '\"':
		case '\\':
		case '\b':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
		case '/': escapeCharCount++; break;

		default:
			// 每个字节都+5肯定满足printf的需求了
			if (*strCurrentPtr < 32) { escapeCharCount += 5; }
			break;
		}
	}

	RyanJsonCheckReturnFalse(printBufAppend(printfBuf, (uint32_t)(strCurrentPtr - strValue) + escapeCharCount + 2U)); // 最小是\" \"
	printBufPutChar(printfBuf, '\"');

	// 没有转义字符
	if (0 == escapeCharCount)
	{
		printBufPutString(printfBuf, strValue, (strCurrentPtr - strValue));
		printBufPutChar(printfBuf, '\"');
		return RyanJsonTrue;
	}

	strCurrentPtr = strValue;
	while (*strCurrentPtr)
	{
		if ((*strCurrentPtr) >= ' ' && *strCurrentPtr != '\"' && *strCurrentPtr != '\\')
		{
			printBufPutChar(printfBuf, *strCurrentPtr++);
			continue;
		}

		// 转义和打印
		printBufPutChar(printfBuf, '\\');

		switch (*strCurrentPtr)
		{
		case '\\': printBufPutChar(printfBuf, '\\'); break;
		case '\"': printBufPutChar(printfBuf, '\"'); break;
		case '\b': printBufPutChar(printfBuf, 'b'); break;
		case '\f': printBufPutChar(printfBuf, 'f'); break;
		case '\n': printBufPutChar(printfBuf, 'n'); break;
		case '\r': printBufPutChar(printfBuf, 'r'); break;
		case '\t': printBufPutChar(printfBuf, 't'); break;

		default: {
			// 可以不加p有效性的判断是因为，这个RyanJson生成的字符串，RyanJson可以确保p一定是有效的
			// jsonLog("hexasdf:\\u%04X\n", codepoint);
			RyanJsonCheckReturnFalse(
				5 == RyanJsonSnprintf((char *)printBufCurrentPtr(printfBuf), printfBuf->size, "u%04X", *strCurrentPtr));
			printfBuf->cursor += 5; // utf
			break;
		}
		}
		strCurrentPtr++;
	}

	printBufPutChar(printfBuf, '\"');
	// printBufPutChar(printfBuf, '\0');

	return RyanJsonTrue;
}

static RyanJsonBool_e RyanJsonPrintString(RyanJson_t pJson, RyanJsonPrintBuffer *printfBuf)
{
	RyanJsonCheckAssert(NULL != pJson && NULL != printfBuf);
	return RyanJsonPrintStringBuffer((const uint8_t *)RyanJsonGetStringValue(pJson), printfBuf);
}

/**
 * @brief 反序列化数组
 *
 * @param pJson
 * @param buf
 * @param depth
 * @param format
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e RyanJsonPrintArray(RyanJson_t pJson, RyanJsonPrintBuffer *printfBuf, uint32_t depth, RyanJsonBool_e format)
{
	RyanJsonCheckAssert(NULL != pJson && NULL != printfBuf);

	uint32_t count = 0;
	RyanJson_t child = RyanJsonGetObjectValue(pJson);

	if (NULL == child)
	{
		RyanJsonCheckReturnFalse(printBufAppend(printfBuf, 2));

		printBufPutChar(printfBuf, '[');
		printBufPutChar(printfBuf, ']');
		return RyanJsonTrue;
	}

	if (format)
	{
		while (child) // 检查子级中是否有数组或对象
		{
			if ((RyanJsonIsArray(child) || RyanJsonIsObject(child)) && RyanJsonGetObjectValue(child))
			{
				count++;
				break;
			}
			child = child->next;
		}
	}

	RyanJsonCheckReturnFalse(printBufAppend(printfBuf, (format && count) ? 2 : 1));

	printBufPutChar(printfBuf, '[');
	if (format && count) { printBufPutChar(printfBuf, '\n'); }

	child = RyanJsonGetObjectValue(pJson);
	while (child)
	{
		// 打印起始缩进
		if (format && count)
		{
			RyanJsonCheckReturnFalse(printBufAppend(printfBuf, depth + 1U));

			for (uint32_t i = 0; i <= depth; i++) { printBufPutChar(printfBuf, '\t'); }
		}

		RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonPrintValue(child, printfBuf, depth + 1U, format));

		// 打印分隔符 ','
		if (child->next)
		{
			RyanJsonCheckReturnFalse(printBufAppend(printfBuf, format ? 2 : 1));

			printBufPutChar(printfBuf, ',');
			if (format)
			{
				if (count) { printBufPutChar(printfBuf, '\n'); }
				else
				{
					printBufPutChar(printfBuf, ' ');
				}
			}
		}

		child = child->next;
	}

	// 打印结束缩进
	RyanJsonCheckReturnFalse(printBufAppend(printfBuf, (format && count) ? depth + 2 : 1));

	if (format && count)
	{
		printBufPutChar(printfBuf, '\n');
		for (uint32_t i = 0; i < depth; i++) { printBufPutChar(printfBuf, '\t'); }
	}
	printBufPutChar(printfBuf, ']');

	return RyanJsonTrue;
}

/**
 * @brief 反序列化对象
 *
 * @param pJson
 * @param buf
 * @param depth
 * @param format
 * @return RyanJsonBool_e
 */
static RyanJsonBool_e RyanJsonPrintObject(RyanJson_t pJson, RyanJsonPrintBuffer *printfBuf, uint32_t depth, RyanJsonBool_e format)
{
	RyanJsonCheckAssert(NULL != pJson && NULL != printfBuf);

	RyanJson_t child = RyanJsonGetObjectValue(pJson);

	if (NULL == child)
	{
		RyanJsonCheckReturnFalse(printBufAppend(printfBuf, 2));

		printBufPutChar(printfBuf, '{');
		printBufPutChar(printfBuf, '}');
		return RyanJsonTrue;
	}

	RyanJsonCheckReturnFalse(printBufAppend(printfBuf, format ? 2 : 1));
	printBufPutChar(printfBuf, '{');
	if (format) { printBufPutChar(printfBuf, '\n'); }

	while (child)
	{
		// 打印起始缩进
		if (format)
		{
			RyanJsonCheckReturnFalse(printBufAppend(printfBuf, depth + 1));

			for (uint32_t i = 0; i <= depth; i++) { printBufPutChar(printfBuf, '\t'); }
		}

		RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonPrintStringBuffer((const uint8_t *)RyanJsonGetKey(child), printfBuf));

		// 打印指示符 ':'
		RyanJsonCheckReturnFalse(printBufAppend(printfBuf, format ? 2 : 1));

		printBufPutChar(printfBuf, ':');
		if (format) { printBufPutChar(printfBuf, '\t'); }

		RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonPrintValue(child, printfBuf, depth + 1, format));

		// 打印分隔符 ','
		RyanJsonCheckReturnFalse(printBufAppend(printfBuf, (child->next ? 1 : 0) + (format ? 1 : 0)));

		if (child->next) { printBufPutChar(printfBuf, ','); }

		RyanJsonCheckReturnFalse(printBufAppend(printfBuf, 1));
		if (format) { printBufPutChar(printfBuf, '\n'); }

		child = child->next;
	}

	// 打印结束缩进
	RyanJsonCheckReturnFalse(printBufAppend(printfBuf, format ? depth + 1 : 1));

	if (format)
	{
		for (uint32_t i = 0; i < depth; i++) { printBufPutChar(printfBuf, '\t'); }
	}
	printBufPutChar(printfBuf, '}');

	return RyanJsonTrue;
}

static RyanJsonBool_e RyanJsonPrintValue(RyanJson_t pJson, RyanJsonPrintBuffer *printfBuf, uint32_t depth, RyanJsonBool_e format)
{
	RyanJsonCheckAssert(NULL != pJson && NULL != printfBuf);

	switch (RyanJsonGetType(pJson))
	{
	case RyanJsonTypeNull: {
		RyanJsonCheckReturnFalse(printBufAppend(printfBuf, 4));
		printBufPutString(printfBuf, (uint8_t *)"null", 4);
		return RyanJsonTrue;
	}
	case RyanJsonTypeBool: {
		if (RyanJsonGetBoolValue(pJson))
		{
			RyanJsonCheckReturnFalse(printBufAppend(printfBuf, 4));
			printBufPutString(printfBuf, (uint8_t *)"true", 4);
		}
		else
		{
			RyanJsonCheckReturnFalse(printBufAppend(printfBuf, 5));
			printBufPutString(printfBuf, (uint8_t *)"false", 5);
		}
		return RyanJsonTrue;
	}
	case RyanJsonTypeNumber: return RyanJsonPrintNumber(pJson, printfBuf);
	case RyanJsonTypeString: return RyanJsonPrintString(pJson, printfBuf);
	case RyanJsonTypeArray: return RyanJsonPrintArray(pJson, printfBuf, depth, format);
	case RyanJsonTypeObject: return RyanJsonPrintObject(pJson, printfBuf, depth, format);
	}
	return RyanJsonFalse;
}

/**
 * @brief 将json对象转换为字符串
 *
 * @param pJson
 * @param preset 对json对象转为字符串后长度的猜测，如果猜测的接近可以减少内存分配次数，提高转换效率
 * @param format 是否格式化
 * @param len 可以通过指针来获取转换后的长度
 * @return char* NULL失败
 */
char *RyanJsonPrint(RyanJson_t pJson, uint32_t preset, RyanJsonBool_e format, uint32_t *len)
{
	RyanJsonCheckReturnNull(NULL != pJson);

	RyanJsonPrintBuffer printfBuf = {
		.isNoAlloc = RyanJsonFalse,
		.size = preset,
		.cursor = 0,
	};

	if (printfBuf.size < RyanJsonPrintfPreAlloSize) { printfBuf.size = RyanJsonPrintfPreAlloSize; }
	printfBuf.bufAddress = (uint8_t *)jsonMalloc(printfBuf.size);
	RyanJsonCheckReturnNull(NULL != printfBuf.bufAddress);

	RyanJsonCheckCode(RyanJsonTrue == RyanJsonPrintValue(pJson, &printfBuf, 0, format), {
		jsonFree(printfBuf.bufAddress);
		return NULL;
	});

	RyanJsonCheckCode(printBufAppend(&printfBuf, 1), {
		jsonFree(printfBuf.bufAddress);
		return NULL;
	});

	printfBuf.bufAddress[printfBuf.cursor] = '\0';
	if (len) { *len = printfBuf.cursor; }

	return (char *)printfBuf.bufAddress;
}

/**
 * @brief 使用给定缓冲区将json对象转换为字符串
 *
 * @param pJson
 * @param buffer 用户给定缓冲区地址
 * @param length 缓冲区长度
 * @param format
 * @param len
 * @return char*
 */
char *RyanJsonPrintPreallocated(RyanJson_t pJson, char *buffer, uint32_t length, RyanJsonBool_e format, uint32_t *len)
{
	RyanJsonCheckReturnNull(NULL != pJson && NULL != buffer);

	RyanJsonPrintBuffer printfBuf = {
		.bufAddress = (uint8_t *)buffer,
		.isNoAlloc = RyanJsonTrue,
		.size = length,
		.cursor = 0,
	};

	RyanJsonCheckReturnNull(RyanJsonTrue == RyanJsonPrintValue(pJson, &printfBuf, 0, format));

	RyanJsonCheckReturnNull(printBufAppend(&printfBuf, 1));
	printfBuf.bufAddress[printfBuf.cursor] = '\0';
	if (len) { *len = printfBuf.cursor; }

	return (char *)printfBuf.bufAddress;
}

/**
 * @brief 获取 json 的子项个数
 *
 * @param pJson
 * @return uint32_t
 */
uint32_t RyanJsonGetSize(RyanJson_t pJson)
{
	RyanJsonCheckCode(NULL != pJson, { return 0; });

	if (!_checkType(RyanJsonGetType(pJson), RyanJsonTypeArray) && !_checkType(RyanJsonGetType(pJson), RyanJsonTypeObject)) { return 1; }

	RyanJson_t nextItem = RyanJsonGetObjectValue(pJson);
	uint32_t size = 0;
	while (NULL != nextItem)
	{
		size++;
		nextItem = nextItem->next;
	}

	return size;
}

/**
 * @brief 通过 索引 获取json对象的子项
 *
 * @param pJson
 * @param index
 * @return RyanJson_t
 */
RyanJson_t RyanJsonGetObjectByIndex(RyanJson_t pJson, uint32_t index)
{
	RyanJsonCheckReturnNull(NULL != pJson);

	RyanJsonCheckReturnNull(_checkType(RyanJsonGetType(pJson), RyanJsonTypeArray) ||
				_checkType(RyanJsonGetType(pJson), RyanJsonTypeObject));

	RyanJson_t nextItem = RyanJsonGetObjectValue(pJson);
	RyanJsonCheckReturnNull(NULL != nextItem);
	while (index > 0)
	{
		index--;
		nextItem = nextItem->next;
		RyanJsonCheckReturnNull(NULL != nextItem);
	}

	return nextItem;
}

/**
 * @brief 通过 key 获取json对象的子项
 *
 * @param pJson
 * @param key
 * @return RyanJson_t
 */
RyanJson_t RyanJsonGetObjectByKey(RyanJson_t pJson, const char *key)
{
	RyanJsonCheckReturnNull(NULL != pJson && NULL != key);

	RyanJsonCheckReturnNull(_checkType(RyanJsonGetType(pJson), RyanJsonTypeObject));

	RyanJson_t nextItem = RyanJsonGetObjectValue(pJson);
	RyanJsonCheckReturnNull(NULL != nextItem);
	RyanJsonCheckNeverNoAssert(RyanJsonIsKey(nextItem));

	while (0 != RyanJsonStrcmp(RyanJsonGetKey(nextItem), key))
	{
		nextItem = nextItem->next;
		RyanJsonCheckReturnNull(NULL != nextItem);
		RyanJsonCheckNeverNoAssert(RyanJsonIsKey(nextItem));
	}

	return nextItem;
}

/**
 * @brief 通过 索引 分离json对象的子项
 *
 * @param pJson
 * @param index
 * @return RyanJson_t 被分离对象的指针
 */
RyanJson_t RyanJsonDetachByIndex(RyanJson_t pJson, uint32_t index)
{
	RyanJsonCheckReturnNull(NULL != pJson);

	RyanJsonCheckReturnNull(_checkType(RyanJsonGetType(pJson), RyanJsonTypeArray) ||
				_checkType(RyanJsonGetType(pJson), RyanJsonTypeObject));

	RyanJson_t prev = NULL;
	RyanJson_t nextItem = RyanJsonGetObjectValue(pJson);
	RyanJsonCheckReturnNull(NULL != nextItem);

	while (index > 0)
	{
		prev = nextItem;
		nextItem = nextItem->next;
		index--;
		RyanJsonCheckReturnNull(NULL != nextItem);
	}

	if (NULL != prev) { prev->next = nextItem->next; }
	else
	{
		RyanJsonChangeObjectValue(pJson, nextItem->next);
	}

	nextItem->next = NULL;

	return nextItem;
}

/**
 * @brief 通过 key 分离json对象的子项
 *
 * @param pJson
 * @param key
 * @return RyanJson_t
 */
RyanJson_t RyanJsonDetachByKey(RyanJson_t pJson, const char *key)
{
	RyanJsonCheckReturnNull(NULL != pJson && NULL != key);
	RyanJsonCheckReturnNull(_checkType(RyanJsonGetType(pJson), RyanJsonTypeObject));

	RyanJson_t nextItem = RyanJsonGetObjectValue(pJson);
	RyanJsonCheckReturnNull(NULL != nextItem);
	RyanJsonCheckNeverNoAssert(RyanJsonIsKey(nextItem));

	RyanJson_t prev = NULL;
	while (0 != RyanJsonStrcmp(RyanJsonGetKey(nextItem), key))
	{
		prev = nextItem;
		nextItem = nextItem->next;
		RyanJsonCheckReturnNull(NULL != nextItem);
		RyanJsonCheckNeverNoAssert(RyanJsonIsKey(nextItem));
	}

	if (NULL != prev) { prev->next = nextItem->next; }
	else // 更改的可能是第一个节点
	{
		RyanJsonChangeObjectValue(pJson, nextItem->next);
	}

	nextItem->next = NULL;

	return nextItem;
}

/**
 * @brief 通过 索引 删除json对象的子项
 *
 * @param pJson
 * @param index
 * @return RyanJsonBool_e
 */
RyanJsonBool_e RyanJsonDeleteByIndex(RyanJson_t pJson, uint32_t index)
{
	RyanJsonCheckReturnFalse(NULL != pJson);

	RyanJson_t nextItem = RyanJsonDetachByIndex(pJson, index);
	RyanJsonCheckReturnFalse(NULL != nextItem);

	RyanJsonDelete(nextItem);
	return RyanJsonTrue;
}

/**
 * @brief 通过 key 删除json对象的子项
 *
 * @param pJson
 * @param key
 * @return RyanJsonBool_e
 */
RyanJsonBool_e RyanJsonDeleteByKey(RyanJson_t pJson, const char *key)
{
	RyanJsonCheckReturnFalse(NULL != pJson && NULL != key);

	RyanJson_t nextItem = RyanJsonDetachByKey(pJson, key);
	RyanJsonCheckReturnFalse(NULL != nextItem);

	RyanJsonDelete(nextItem);
	return RyanJsonTrue;
}

/**
 * @brief 按 索引 插入json对象
 *
 * @param pJson
 * @param index
 * @param item
 * @return RyanJsonBool_e
 */
RyanJsonBool_e RyanJsonInsert(RyanJson_t pJson, uint32_t index, RyanJson_t item)
{
	RyanJson_t nextItem = NULL;
	RyanJson_t prev = NULL;

	RyanJsonCheckReturnFalse(NULL != item);
	RyanJsonCheckCode(NULL != pJson, { goto error__; });

	RyanJsonCheckCode(_checkType(RyanJsonGetType(pJson), RyanJsonTypeArray) ||
				  (_checkType(RyanJsonGetType(pJson), RyanJsonTypeObject) && RyanJsonIsKey(item)),
			  {
				  jsonLog("error__ 不是正确类型 %d\r\n", index);
				  goto error__;
			  });

	nextItem = RyanJsonGetObjectValue(pJson);
	while (nextItem && index > 0)
	{
		prev = nextItem;
		nextItem = nextItem->next;
		index--;
	}

	if (NULL != prev) { prev->next = item; }
	else
	{
		RyanJsonChangeObjectValue(pJson, item);
	}

	// nextItem为NULL时这样赋值也是可以的
	item->next = nextItem;

	return RyanJsonTrue;

error__:
	RyanJsonDelete(item);
	return RyanJsonFalse;
}

RyanJsonBool_e RyanJsonAddItemToObject(RyanJson_t pJson, const char *key, RyanJson_t item)
{
	RyanJsonCheckReturnFalse(NULL != pJson);

	RyanJson_t pItem = RyanJsonCreateItem(key, item);
	RyanJsonCheckCode(NULL != pItem, {
		RyanJsonDelete(item);
		return RyanJsonFalse;
	});

	return RyanJsonInsert(pJson, UINT32_MAX, pItem);
}

/**
 * @brief 通过 索引 替换json对象的子项
 *
 * @param pJson
 * @param index
 * @param item
 * @return RyanJsonBool_e
 */
RyanJsonBool_e RyanJsonReplaceByIndex(RyanJson_t pJson, uint32_t index, RyanJson_t item)
{
	RyanJsonCheckReturnFalse(NULL != pJson && NULL != item);

	RyanJsonCheckReturnFalse(_checkType(RyanJsonGetType(pJson), RyanJsonTypeArray) ||
				 _checkType(RyanJsonGetType(pJson), RyanJsonTypeObject));

	RyanJson_t prev = NULL;
	RyanJson_t nextItem = RyanJsonGetObjectValue(pJson);
	RyanJsonCheckReturnFalse(NULL != nextItem);

	// 查找子项
	while (index > 0)
	{
		prev = nextItem;
		nextItem = nextItem->next;
		index--;
		RyanJsonCheckReturnFalse(NULL != nextItem);
	}

	RyanJsonReplaceNode(prev, nextItem, item);
	if (NULL == prev) { RyanJsonChangeObjectValue(pJson, item); }

	RyanJsonDelete(nextItem);
	return RyanJsonTrue;
}

/**
 * @brief 通过 key 替换json对象的子项
 *
 * @param pJson
 * @param key
 * @param item
 * @return RyanJsonBool_e
 */
RyanJsonBool_e RyanJsonReplaceByKey(RyanJson_t pJson, const char *key, RyanJson_t item)
{
	RyanJsonCheckReturnFalse(NULL != pJson && NULL != key && NULL != item);

	RyanJsonCheckReturnFalse(_checkType(RyanJsonGetType(pJson), RyanJsonTypeObject));

	RyanJson_t prev = NULL;
	RyanJson_t nextItem = RyanJsonGetObjectValue(pJson);
	RyanJsonCheckReturnFalse(NULL != nextItem);
	RyanJsonCheckNeverNoAssert(RyanJsonIsKey(nextItem));

	// 找到要修改的节点
	while (0 != RyanJsonStrcmp(RyanJsonGetKey(nextItem), key))
	{
		prev = nextItem;
		nextItem = nextItem->next;
		RyanJsonCheckReturnFalse(NULL != nextItem);
		RyanJsonCheckNeverNoAssert(RyanJsonIsKey(nextItem));
	}

	// 没有key的对象 申请一个带key的对象
	if (RyanJsonFalse == RyanJsonIsKey(item))
	{
		item = RyanJsonCreateItem(key, item);
		RyanJsonCheckReturnFalse(NULL != item);
	}
	else
	{
		if (0 != RyanJsonStrcmp(RyanJsonGetKey(item), key)) { RyanJsonChangeKey(item, key); }
	}

	RyanJsonReplaceNode(prev, nextItem, item);
	if (NULL == prev) { RyanJsonChangeObjectValue(pJson, item); }

	RyanJsonDelete(nextItem);

	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonChangeKey(RyanJson_t pJson, const char *key)
{
	RyanJsonCheckReturnFalse(NULL != pJson && NULL != key);
	RyanJsonCheckReturnFalse(RyanJsonIsKey(pJson) || RyanJsonIsString(pJson));
	return RyanJsonChangeString(pJson, RyanJsonFalse, key, RyanJsonIsString(pJson) ? RyanJsonGetStringValue(pJson) : NULL);
}

RyanJsonBool_e RyanJsonChangeStringValue(RyanJson_t pJson, const char *strValue)
{
	RyanJsonCheckReturnFalse(NULL != pJson && NULL != strValue);
	RyanJsonCheckReturnFalse(RyanJsonIsKey(pJson) || RyanJsonIsString(pJson));
	return RyanJsonChangeString(pJson, RyanJsonFalse, RyanJsonIsKey(pJson) ? RyanJsonGetKey(pJson) : NULL, strValue);
}

RyanJsonBool_e RyanJsonChangeIntValue(RyanJson_t pJson, int32_t number)
{
	RyanJsonCheckReturnFalse(NULL != pJson);
	RyanJsonMemcpy(RyanJsonGetValue(pJson), &number, sizeof(number));
	return RyanJsonTrue;
}

RyanJsonBool_e RyanJsonChangeDoubleValue(RyanJson_t pJson, double number)
{
	RyanJsonCheckReturnFalse(NULL != pJson);
	RyanJsonMemcpy(RyanJsonGetValue(pJson), &number, sizeof(number));
	return RyanJsonTrue;
}

/**
 * @brief 创建一个 NULL 类型的json对象
 *
 * @param key
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateNull(const char *key)
{
	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeNull, .key = key};
	return RyanJsonNewNode(&nodeInfo);
}

/**
 * @brief 创建一个 boolean 类型的json对象
 *
 * @param key
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateBool(const char *key, RyanJsonBool_e boolean)
{
	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeBool, .key = key, .boolIsTrueFlag = boolean};
	return RyanJsonNewNode(&nodeInfo);
}

/**
 * @brief 创建一个 number 类型中的 int32_t 类型json对象
 *
 * @param key
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateInt(const char *key, int32_t number)
{
	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeNumber, .key = key, .numberIsDoubleFlag = RyanJsonFalse};

	RyanJson_t item = RyanJsonNewNode(&nodeInfo);
	RyanJsonCheckReturnNull(NULL != item);

	RyanJsonChangeIntValue(item, number);
	return item;
}

/**
 * @brief 创建一个 number 类型中的 double 类型json对象
 * 可以使用double来保存int64类型数据,但是更推荐通过字符串保存
 * @param key
 * @param number
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateDouble(const char *key, double number)
{
	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeNumber, .key = key, .numberIsDoubleFlag = RyanJsonTrue};
	RyanJson_t item = RyanJsonNewNode(&nodeInfo);
	RyanJsonCheckReturnNull(NULL != item);

	RyanJsonChangeDoubleValue(item, number);
	return item;
}

/**
 * @brief 创建一个 string 类型的json对象
 *
 * @param key
 * @param string
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateString(const char *key, const char *string)
{
	RyanJsonCheckReturnNull(NULL != string);

	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeString, .key = key, .strValue = string};
	return RyanJsonNewNode(&nodeInfo);
}

/**
 * @brief 创建一个 obj 类型的json对象
 *
 * @return RyanJson_t
 */
static RyanJson_t RyanJsonCreateObjectAndKey(const char *key)
{
	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeObject, .key = key};
	return RyanJsonNewNode(&nodeInfo);
}

RyanJson_t RyanJsonCreateObject(void) { return RyanJsonCreateObjectAndKey(NULL); }

/**
 * @brief 创建一个 arr 类型的json对象
 *
 * @return RyanJson_t
 */
static RyanJson_t RyanJsonCreateArrayAndKey(const char *key)
{
	RyanJsonNodeInfo_t nodeInfo = {.type = RyanJsonTypeArray, .key = key};
	return RyanJsonNewNode(&nodeInfo);
}
RyanJson_t RyanJsonCreateArray(void) { return RyanJsonCreateArrayAndKey(NULL); }

RyanJsonBool_e RyanJsonIsKey(RyanJson_t pJson) { return RyanJsonMakeBool(NULL != pJson && RyanJsonGetPayloadWhiteKeyByFlag(pJson)); }
RyanJsonBool_e RyanJsonIsNull(RyanJson_t pJson) { return RyanJsonMakeBool(NULL != pJson && RyanJsonTypeNull == RyanJsonGetType(pJson)); }
RyanJsonBool_e RyanJsonIsBool(RyanJson_t pJson) { return RyanJsonMakeBool(NULL != pJson && RyanJsonTypeBool == RyanJsonGetType(pJson)); }
RyanJsonBool_e RyanJsonIsNumber(RyanJson_t pJson)
{
	return RyanJsonMakeBool(NULL != pJson && RyanJsonTypeNumber == RyanJsonGetType(pJson));
}
RyanJsonBool_e RyanJsonIsString(RyanJson_t pJson)
{
	return RyanJsonMakeBool(NULL != pJson && RyanJsonTypeString == RyanJsonGetType(pJson));
}
RyanJsonBool_e RyanJsonIsArray(RyanJson_t pJson) { return RyanJsonMakeBool(NULL != pJson && RyanJsonTypeArray == RyanJsonGetType(pJson)); }
RyanJsonBool_e RyanJsonIsObject(RyanJson_t pJson)
{
	return RyanJsonMakeBool(NULL != pJson && RyanJsonTypeObject == RyanJsonGetType(pJson));
}
RyanJsonBool_e RyanJsonIsInt(RyanJson_t pJson)
{
	return RyanJsonMakeBool(RyanJsonIsNumber(pJson) && (RyanJsonFalse == RyanJsonGetPayloadNumberIsDoubleByFlag(pJson)));
}
RyanJsonBool_e RyanJsonIsDouble(RyanJson_t pJson)
{
	return RyanJsonMakeBool(RyanJsonIsNumber(pJson) && (RyanJsonTrue == RyanJsonGetPayloadNumberIsDoubleByFlag(pJson)));
}

/**
 * @brief 深拷贝一份json对象
 *
 * @param pJson
 * @return RyanJson_t 拷贝的新对象指针
 */
RyanJson_t RyanJsonDuplicate(RyanJson_t pJson)
{
	RyanJsonCheckReturnNull(NULL != pJson);

	char *key = NULL;
	if (RyanJsonIsKey(pJson)) { key = RyanJsonGetKey(pJson); }

	RyanJson_t newItem = NULL;
	switch (RyanJsonGetType(pJson))
	{
	case RyanJsonTypeNull: newItem = RyanJsonCreateNull(key); break;
	case RyanJsonTypeBool: // 创建NewNode的时候已经赋值了
		newItem = RyanJsonCreateBool(key, RyanJsonGetBoolValue(pJson));
		break;

	case RyanJsonTypeNumber:
		if (RyanJsonIsInt(pJson)) { newItem = RyanJsonCreateInt(key, RyanJsonGetIntValue(pJson)); }
		else
		{
			// 不可能出现另外一个条件了
			RyanJsonCheckNeverNoAssert(RyanJsonIsDouble(pJson));
			newItem = RyanJsonCreateDouble(key, RyanJsonGetDoubleValue(pJson));
		}
		break;

	case RyanJsonTypeString: newItem = RyanJsonCreateString(key, RyanJsonGetStringValue(pJson)); break;

	case RyanJsonTypeArray:
	case RyanJsonTypeObject: {
		if (_checkType(RyanJsonGetType(pJson), RyanJsonTypeArray)) { newItem = RyanJsonCreateArrayAndKey(key); }
		else
		{
			newItem = RyanJsonCreateObjectAndKey(key);
		}

		RyanJsonCheckCode(NULL != newItem, { goto error__; });

		RyanJson_t item, prev = NULL;
		RyanJson_t temp = RyanJsonGetObjectValue(pJson);
		while (temp)
		{
			item = RyanJsonDuplicate(temp);
			RyanJsonCheckCode(NULL != item, { goto error__; });

			// RyanJsonCheckNeverNoAssert(RyanJsonTrue == RyanJsonInsert(newItem, UINT32_MAX, item));

			if (NULL != prev)
			{
				prev->next = item;
				prev = item;
			}
			else
			{
				RyanJsonChangeObjectValue(newItem, item);
				prev = item;
			}

			temp = temp->next;
		}
		break;
	}

	default: goto error__;
	}

	return newItem;

error__:
	RyanJsonDelete(newItem);
	return NULL;
}

/**
 * @brief 通过删除无效字符、注释等， 减少json文本大小
 *
 * @param text 文本指针
 * @param textLen 文本长度,不使用uint32_t是方式用户传递的参数隐式转换不容易发现bug
 * @return uint32_t
 */
uint32_t RyanJsonMinify(char *text, int32_t textLen)
{
	RyanJsonCheckCode(NULL != text && textLen > 0, { return 0; });

	char *t = (char *)text;           // 假设 text 指向可写缓冲区
	const char *end = text + textLen; // 边界
	uint32_t count = 0;               // 压缩后字符数

	while (text < end && *text)
	{
		if (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') { text++; }
		else if (*text == '/' && (text + 1 < end) && text[1] == '/')
		{
			while (text < end && *text && *text != '\n') { text++; }
		}
		else if (*text == '/' && (text + 1 < end) && text[1] == '*')
		{
			text += 2;
			while (text < end && *text && !(*text == '*' && (text + 1 < end) && text[1] == '/')) { text++; }
			if (text + 1 < end) { text += 2; }
		}
		else if (*text == '\"')
		{
			*t++ = *text++;
			count++;
			while (text < end && *text && *text != '\"')
			{
				if (*text == '\\' && text + 1 < end)
				{
					*t++ = *text++;
					count++;
				}
				*t++ = *text++;
				count++;
			}
			if (text < end)
			{
				*t++ = *text++;
				count++;
			}
		}
		else
		{
			*t++ = *text++;
			count++;
		}
	}

	*t = '\0';    // 调用者需保证缓冲区有空间
	return count; // 返回压缩后大小
}

/**
 * @brief 安全的浮点数比较
 *
 * @param a
 * @param b
 * @return RyanJsonBool_e
 */
RyanJsonBool_e RyanJsonCompareDouble(double a, double b)
{
	double diff = fabs(a - b);
	double absA = fabs(a);
	double absB = fabs(b);
	double maxVal = (absA > absB ? absA : absB);

	// 允许的容差：相对误差 + 绝对误差
	double epsilon = DBL_EPSILON * maxVal;
	double minTolerance = 1e-12; // 可调的绝对容差

	return diff <= (epsilon > minTolerance ? epsilon : minTolerance);
}

/**
 * @brief 递归比较两个 pJson 对象key和value是否相等。
 * 此接口效率较低, 谨慎使用
 * @param leftJson
 * @param rightJson
 * @return RyanJsonBool_e
 */
RyanJsonBool_e RyanJsonCompare(RyanJson_t leftJson, RyanJson_t rightJson)
{
	RyanJsonCheckReturnFalse(NULL != leftJson && NULL != rightJson);

	// 相同的对象相等
	if (leftJson == rightJson) { return RyanJsonTrue; }

	RyanJsonCheckReturnFalse(RyanJsonGetType(leftJson) == RyanJsonGetType(rightJson));

	switch (RyanJsonGetType(leftJson))
	{
	case RyanJsonTypeNull: return RyanJsonTrue;

	case RyanJsonTypeBool: return (RyanJsonGetBoolValue(leftJson) == RyanJsonGetBoolValue(rightJson)) ? RyanJsonTrue : RyanJsonFalse;

	case RyanJsonTypeNumber: {

		if (RyanJsonTrue == RyanJsonIsInt(leftJson) && RyanJsonTrue == RyanJsonIsInt(rightJson))
		{
			return (RyanJsonGetIntValue(leftJson) == RyanJsonGetIntValue(rightJson)) ? RyanJsonTrue : RyanJsonFalse;
		}

		if (RyanJsonTrue == RyanJsonIsDouble(leftJson) && RyanJsonTrue == RyanJsonIsDouble(rightJson))
		{
			return RyanJsonCompareDouble(RyanJsonGetDoubleValue(leftJson), RyanJsonGetDoubleValue(rightJson));
		}

		return RyanJsonFalse;
	}

	case RyanJsonTypeString:
		return (0 == RyanJsonStrcmp(RyanJsonGetStringValue(leftJson), RyanJsonGetStringValue(rightJson))) ? RyanJsonTrue
														  : RyanJsonFalse;

	case RyanJsonTypeArray: {
		RyanJsonCheckReturnFalse(RyanJsonGetSize(leftJson) == RyanJsonGetSize(rightJson));

		RyanJson_t item;
		uint32_t itemIndex = 0;
		RyanJsonArrayForEach(leftJson, item)
		{
			RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonCompare(item, RyanJsonGetObjectByIndex(rightJson, itemIndex)));
			itemIndex++;
		}

		return RyanJsonTrue;
	}

	case RyanJsonTypeObject: {
		RyanJsonCheckReturnFalse(RyanJsonGetSize(leftJson) == RyanJsonGetSize(rightJson));

		RyanJson_t item;
		RyanJsonObjectForEach(leftJson, item)
		{
			RyanJsonCheckReturnFalse(RyanJsonTrue ==
						 RyanJsonCompare(item, RyanJsonGetObjectByKey(rightJson, RyanJsonGetKey(item))));
		}

		return RyanJsonTrue;
	}
	}

	return RyanJsonFalse;
}
