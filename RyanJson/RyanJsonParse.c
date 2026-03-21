#include "RyanJsonInternal.h"

typedef struct
{
	const uint8_t *currentPtr; // 待解析字符串地址
	uint32_t remainSize;       // 待解析字符串剩余长度
} RyanJsonParseBuffer;

// 解析缓冲区辅助宏（本文件局部使用）
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
 */
static inline RyanJsonBool_e RyanJsonParseBufTryAdvanceCurrentPtr(RyanJsonParseBuffer *parseBuf, uint32_t bytesToAdvance)
{
	RyanJsonCheckAssert(NULL != parseBuf);

#ifdef isEnableFuzzer
	if (RyanJsonFuzzerShouldFail(800)) { bytesToAdvance = UINT32_MAX; }
#endif

	if (parseBufHasRemainBytes(parseBuf, bytesToAdvance))
	{
		parseBufAdvanceCurrentPrt(parseBuf, bytesToAdvance);
		return RyanJsonTrue;
	}

	return RyanJsonFalse;
}

/**
 * @brief 跳过空白字符
 */
static inline RyanJsonBool_e RyanJsonParseBufSkipWhitespace(RyanJsonParseBuffer *parseBuf)
{
	RyanJsonCheckAssert(NULL != parseBuf);

#ifdef isEnableFuzzer
	RyanJsonCheckReturnFalse(!RyanJsonFuzzerShouldFail(1500));
#endif

	while (parseBufHasRemain(parseBuf))
	{
		uint8_t cursor = *parseBuf->currentPtr;
		if (' ' == cursor || '\n' == cursor || '\r' == cursor || '\t' == cursor)
		{
			RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1));
		}
		else
		{
			break;
		}
	}

	return RyanJsonTrue;
}

/**
 * @brief 解析 4 位十六进制文本（XXXX）
 *
 * @param text 十六进制文本起始地址
 * @param value 解析结果输出
 * @return RyanJsonBool_e 解析是否成功
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
 * @brief 解析 Number（含符号/小数/指数）
 *
 * @param parseBuf 解析缓冲区
 * @param numberValuePtr 输出数值
 * @param isIntPtr 输出是否为 Int
 * @return RyanJsonBool_e 解析是否成功
 */
static RyanJsonBool_e RyanJsonInternalParseDouble(RyanJsonParseBuffer *parseBuf, double *numberValuePtr, RyanJsonBool_e *isIntPtr)
{
	RyanJsonCheckAssert(NULL != parseBuf && NULL != numberValuePtr && NULL != isIntPtr);

	double number = 0;
	int32_t scale = 0;
	int32_t e_sign = 1;
	int32_t e_scale = 0;
	RyanJsonBool_e isNegative = RyanJsonFalse;
	RyanJsonBool_e isInt = RyanJsonTrue;

	// 处理符号
	if ('-' == *parseBuf->currentPtr)
	{
		isNegative = RyanJsonTrue;
		// 正常路径下不会失败（已确保 remain>0），Fuzzer 注入时可能被强制失败
		RyanJsonAssertAlwaysEval(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1));
		RyanJsonCheckReturnFalse(parseBufHasRemain(parseBuf) && *parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9');
	}

	// 前导 0 是非法的
	if ('0' == *parseBuf->currentPtr)
	{
		RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1));
		// 前导 0 后面不允许跟数字，例如 "0123"
		if (parseBufHasRemain(parseBuf)) { RyanJsonCheckReturnFalse(*parseBuf->currentPtr < '0' || *parseBuf->currentPtr > '9'); }
	}

	// Int 部分
	while (parseBufHasRemain(parseBuf) && *parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9')
	{
		number = number * 10.0 + (*parseBuf->currentPtr - '0');
		// 数值过大导致溢出
		RyanJsonCheckReturnFalse(isfinite(number));
		RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1));
	}

	// 小数部分
	if (parseBufHasRemain(parseBuf) && '.' == *parseBuf->currentPtr)
	{
		RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1));
		RyanJsonCheckReturnFalse(parseBufHasRemain(parseBuf) && *parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9');

		while (parseBufHasRemain(parseBuf) && *parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9')
		{
			number = number * 10.0 + (*parseBuf->currentPtr - '0');
			RyanJsonCheckReturnFalse(isfinite(number));
			scale--; // 每读一位小数，scale 减一
			RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1));
		}
		isInt = RyanJsonFalse;
	}

	// 指数部分
	if (parseBufHasRemain(parseBuf) && ('e' == *parseBuf->currentPtr || 'E' == *parseBuf->currentPtr))
	{
		RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1));
		RyanJsonCheckReturnFalse(parseBufHasRemain(parseBuf));

		// 只有遇到 +/- 符号时才跳过
		if ('+' == *parseBuf->currentPtr || '-' == *parseBuf->currentPtr)
		{
			e_sign = ('-' == *parseBuf->currentPtr) ? -1 : 1;
			RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1));
		}

		RyanJsonCheckReturnFalse(parseBufHasRemain(parseBuf) && *parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9');

		while (parseBufHasRemain(parseBuf) && *parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9')
		{
			int32_t digit = (int32_t)(*parseBuf->currentPtr - '0');
			// 防止指数累乘出现有符号溢出
			RyanJsonCheckReturnFalse(e_scale <= (INT32_MAX / 10));
			RyanJsonCheckReturnFalse(e_scale < (INT32_MAX / 10) || digit <= (INT32_MAX % 10));
			e_scale = e_scale * 10 + digit;
			RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1));
		}
		isInt = RyanJsonFalse;
	}

	// 判断符号
	if (RyanJsonTrue == isNegative) { number = -number; }

	// Double 场景还需应用小数与指数缩放
	if (RyanJsonFalse == isInt)
	{
		// 使用更宽位数拼接指数，避免中间表达式溢出
		int64_t exponent = (int64_t)scale + (int64_t)e_sign * (int64_t)e_scale;
		double expFactor = pow(10.0, (double)exponent);
		number *= expFactor;
		RyanJsonCheckReturnFalse(isfinite(number));
	}

	*numberValuePtr = number;
	*isIntPtr = isInt;
	return RyanJsonTrue;
}

/**
 * @brief 解析文本中的 Number 并创建 Json 节点
 */
static RyanJsonBool_e RyanJsonParseNumber(RyanJsonParseBuffer *parseBuf, char *key, RyanJson_t *out)
{
	RyanJsonCheckAssert(NULL != parseBuf && NULL != out);

	double number = 0;
	RyanJsonBool_e isInt = RyanJsonTrue;
	RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonInternalParseDouble(parseBuf, &number, &isInt));

	// 创建 Json 节点
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
 * @brief 预扫描字符串长度并统计是否包含转义字符
 *
 * @param parseBuf 解析缓冲区（当前指向起始双引号）
 * @param lenPtr 输出解码后的最大字节长度（上限，不含 '\0'）
 * @param hasEscapePtr 输出是否包含转义字符
 * @return RyanJsonBool_e 扫描是否成功
 * @note 成功后 parseBuf->currentPtr 已前移到引号后的首字符位置。
 * @note 该函数仅预扫描，不消费结尾引号。
 */
static RyanJsonBool_e RyanJsonParseStringBufferGetLen(RyanJsonParseBuffer *parseBuf, uint32_t *lenPtr, RyanJsonBool_e *hasEscapePtr)
{
	RyanJsonCheckAssert(NULL != parseBuf && NULL != lenPtr && NULL != hasEscapePtr);

	uint32_t len = 0;
	RyanJsonBool_e hasEscape = RyanJsonFalse;

	// 不是字符串
	RyanJsonCheckReturnFalse(parseBufHasRemain(parseBuf) && '\"' == *parseBuf->currentPtr);

	RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1));

	// 估算字符串解码后的最大字节长度
	for (uint32_t i = 0;;)
	{
		RyanJsonCheckReturnFalse(parseBufHasRemainAtIndex(parseBuf, i));

		uint8_t ch = parseBuf->currentPtr[i];

		if ('\"' == ch) { break; }

		// 检查非法控制字符 (ASCII 0–31)
		RyanJsonCheckReturnFalse(ch > 0x1F);

		if ('\\' != ch)
		{
			len++;
			i++;
			continue;
		}

		// 转义字符
		hasEscape = RyanJsonTrue;
		RyanJsonCheckReturnFalse(parseBufHasRemainAtIndex(parseBuf, i + 1));
		uint8_t esc = parseBuf->currentPtr[i + 1];
		switch (esc)
		{
		case '\"':
		case '\\':
		case '/':
		case 'b':
		case 'f':
		case 'n':
		case 'r':
		case 't':
			len++;
			i += 2;
			break;
		case 'u': {
			RyanJsonCheckReturnFalse(parseBufHasRemainAtIndex(parseBuf, i + 5));
			// 十六进制合法性在后续真正解码时校验

			// 简化：每个 \\uXXXX 预估最大 4 字节 UTF-8
			len += 4;
			i += 6;
			break;
		}
		default:
#ifdef isEnableFuzzer
			if (RyanJsonFuzzerShouldFail(2))
			{
				len++;
				i += 2;
				break;
			}
#endif
			return RyanJsonFalse;
		}
	}

	*lenPtr = len;
	*hasEscapePtr = hasEscape;
	return RyanJsonTrue;
}

/**
 * @brief 将 Json 字符串字面量片段（引号内文本）解码到目标缓冲区
 *
 * @note 调用前必须先执行 RyanJsonParseStringBufferGetLen 获取长度与转义信息。
 * @note buffer 至少需要 len + 1 字节，函数会写入 '\0'。
 * @note 成功后 parseBuf->currentPtr 指向结尾引号后的下一个字符。
 */
static RyanJsonBool_e RyanJsonParseStringBuffer(RyanJsonParseBuffer *parseBuf, char *buffer, uint32_t len, RyanJsonBool_e hasEscape)
{
	uint8_t *outCurrentPtr = (uint8_t *)buffer;

	// 获取长度时已确保有结尾引号
	if (RyanJsonFalse == hasEscape)
	{
		RyanJsonMemcpy(outCurrentPtr, parseBuf->currentPtr, len);
		outCurrentPtr[len] = '\0';
		RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, len), { goto error__; });

		// 跳过结尾引号
		RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1), { goto error__; });
		return RyanJsonTrue;
	}

	// 预扫描长度阶段已确保字符串一定存在结束引号
	while ('\"' != *parseBuf->currentPtr)
	{
		// 普通字符
		if ('\\' != *parseBuf->currentPtr)
		{
			*outCurrentPtr++ = *parseBuf->currentPtr;
			RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1), { goto error__; });
			continue;
		}

		// 转义字符
		RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1), { goto error__; });
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
			RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 4), { goto error__; });
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

				RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 6), { goto error__; });
				uint32_t secondCode = 0;
				// 读取代理后半部分
				RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseHex(parseBuf->currentPtr - 3, &secondCode),
						  { goto error__; });
				RyanJsonCheckCode(secondCode >= 0xDC00 && secondCode <= 0xDFFF, {
					goto error__; // 无效的代理后半部分
				});

				codepoint = 0x10000 + (((firstCode & 0x3FF) << 10) | (secondCode & 0x3FF));
			}
			else
			{
				codepoint = firstCode;
			}

			// 将 Unicode 码点编码为 UTF-8
			uint8_t utf8Length;
			uint8_t firstByteMark;
			if (codepoint < 0x80)
			{
				utf8Length = 1; // ASCII：0xxxxxxx
				firstByteMark = 0;
			}
			else if (codepoint < 0x800)
			{
				utf8Length = 2;       // 双字节：110xxxxx 10xxxxxx
				firstByteMark = 0xC0; // 11000000
			}
			else if (codepoint < 0x10000)
			{
				utf8Length = 3;       // 三字节：1110xxxx 10xxxxxx 10xxxxxx
				firstByteMark = 0xE0; // 11100000
			}
			else
			{
				utf8Length = 4;       // 四字节：11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
				firstByteMark = 0xF0; // 11110000
			}

			// 先从末尾写 continuation byte（10xxxxxx）
			for (uint8_t utf8Position = (uint8_t)(utf8Length - 1); utf8Position > 0; utf8Position--)
			{
				outCurrentPtr[utf8Position] = (uint8_t)((codepoint | 0x80) & 0xBF); // 10xxxxxx
				codepoint >>= 6;
			}

			// 再写首字节
			if (utf8Length > 1) { outCurrentPtr[0] = (uint8_t)((codepoint | firstByteMark) & 0xFF); }
			else
			{
				outCurrentPtr[0] = (uint8_t)(codepoint & 0x7F);
			}
			outCurrentPtr += utf8Length;
			break;
		}

		default: goto error__;
		}

		RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1), { goto error__; });
	}
	*outCurrentPtr = '\0';

	RyanJsonCheckAssert('\"' == *parseBuf->currentPtr);

	RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseBufTryAdvanceCurrentPtr(parseBuf, 1), { goto error__; });
	return RyanJsonTrue;

error__:
	return RyanJsonFalse;
}

/**
 * @brief 解析 String 节点并创建 String 节点
 */
static RyanJsonBool_e RyanJsonParseString(RyanJsonParseBuffer *parseBuf, char *key, RyanJson_t *out)
{
	RyanJsonCheckAssert(NULL != parseBuf && NULL != out);

	uint32_t len;
	RyanJsonBool_e hasEscape = RyanJsonFalse;
	RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonParseStringBufferGetLen(parseBuf, &len, &hasEscape));

	if (len + 1 > RyanJsonInlineStringSize)
	{
		char *bufferMalloc = (char *)jsonMalloc((size_t)(len + 1U));
		RyanJsonCheckReturnFalse(NULL != bufferMalloc);
		RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseStringBuffer(parseBuf, bufferMalloc, len, hasEscape), {
			jsonFree(bufferMalloc);
			return RyanJsonFalse;
		});

		RyanJson_t newItem = RyanJsonCreateString(key, bufferMalloc);
		RyanJsonCheckCode(NULL != newItem, {
			jsonFree(bufferMalloc);
			return RyanJsonFalse;
		});

		jsonFree(bufferMalloc);
		*out = newItem;
	}
	else
	{
		char buffer[RyanJsonInlineStringSize] = {0};
		RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonParseStringBuffer(parseBuf, buffer, len, hasEscape));
		RyanJson_t newItem = RyanJsonCreateString(key, buffer);
		RyanJsonCheckReturnFalse(NULL != newItem);
		*out = newItem;
	}

	return RyanJsonTrue;
}

/**
 * @brief 解析单个 Json 值（非递归，仅创建当前层节点）
 *
 * @note 对 Array/Object 仅创建空容器并消费起始符号（'[' 或 '{'），
 *       后续子节点由外层迭代器继续解析。
 */
static RyanJsonBool_e RyanJsonParseValue(RyanJsonParseBuffer *parseBuf, char *key, RyanJson_t *out)
{
	RyanJsonCheckAssert(NULL != parseBuf && NULL != out);

	RyanJsonCheckReturnFalse(parseBufHasRemain(parseBuf));

	*out = NULL;

	if ('\"' == *parseBuf->currentPtr) { return RyanJsonParseString(parseBuf, key, out); }
	if ('{' == *parseBuf->currentPtr)
	{
		*out = RyanJsonInternalCreateObjectAndKey(key);
		RyanJsonCheckReturnFalse(NULL != *out);
		parseBufAdvanceCurrentPrt(parseBuf, 1); // 消费掉 '{'，后续迭代解析器会处理内部
		return RyanJsonTrue;
	}
	if ('-' == *parseBuf->currentPtr || (*parseBuf->currentPtr >= '0' && *parseBuf->currentPtr <= '9'))
	{
		return RyanJsonParseNumber(parseBuf, key, out);
	}
	if ('[' == *parseBuf->currentPtr)
	{
		*out = RyanJsonInternalCreateArrayAndKey(key);
		RyanJsonCheckReturnFalse(NULL != *out);
		parseBufAdvanceCurrentPrt(parseBuf, 1); // 消费掉 '['，后续迭代解析器会处理内部
		return RyanJsonTrue;
	}

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
 * @brief 迭代解析器 (使用线索链表维护父子关系，不使用显式栈)
 */
static RyanJsonBool_e RyanJsonParseIterative(RyanJsonParseBuffer *parseBuf, RyanJson_t *root)
{
	RyanJsonCheckAssert(NULL != parseBuf && NULL != root);

	// 先解析根节点
	RyanJsonCheckReturnFalse(RyanJsonTrue == RyanJsonParseValue(parseBuf, NULL, root));

	// 如果是标量 (String, Number, Bool, Null)，直接返回，无需迭代
	if (!RyanJsonIsArray(*root) && !RyanJsonIsObject(*root)) { return RyanJsonTrue; }

	// 初始化迭代状态
	RyanJson_t scopeParent = *root; // 当前容器 (父节点)
	RyanJson_t lastSibling = NULL;  // 同级上一个节点 (用来链接 sibling->next)

	char shortKey[RyanJsonInlineStringSize];       // 栈上短 key 缓存
	char *key = NULL;                              // 指向当前使用的 key (shortKey 或 堆内存)
	RyanJsonBool_e isKeyAllocated = RyanJsonFalse; // 标记 key 是否需要释放
	RyanJson_t newItem = NULL;                     // 新解析出的节点

	while (1)
	{
		RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseBufSkipWhitespace(parseBuf), { goto error__; });
		RyanJsonCheckCode(parseBufHasRemain(parseBuf), { goto error__; });

		// 阶段：检查当前容器是否结束（']' 或 '}'）
		uint8_t ch = *parseBuf->currentPtr;
		RyanJsonBool_e scopeParentIsArray = RyanJsonIsArray(scopeParent);

		if ((scopeParentIsArray && ']' == ch) || (!scopeParentIsArray && '}' == ch))
		{
			parseBufAdvanceCurrentPrt(parseBuf, 1);

			// 当前容器已经闭合，接下来回溯到父容器。
			// 父容器指针保存在 scopeParent->next（下沉时写入的线索）。

			// 如果回到根节点，说明整个 Json 解析完成
			if (scopeParent == *root) { return RyanJsonTrue; }

			// 读取当前容器的父容器
			RyanJson_t parent = scopeParent->next;

			// 更新回溯后的层级状态
			// 回到父层后，当前容器变成上一层的 lastSibling。
			lastSibling = scopeParent;
			scopeParent = parent;

			// 继续检查父层后续（逗号或结束符）
			continue;
		}

		// 阶段：处理同层分隔符
		if (lastSibling)
		{
			if (',' == ch)
			{
				parseBufAdvanceCurrentPrt(parseBuf, 1);
				RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseBufSkipWhitespace(parseBuf), { goto error__; });
			}
			else
			{
				goto error__; // 缺少逗号
			}
		}

		// 阶段：解析 Object key（仅 Object）
		if (!scopeParentIsArray)
		{
			uint32_t len;
			RyanJsonBool_e hasEscape = RyanJsonFalse;
			RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseStringBufferGetLen(parseBuf, &len, &hasEscape), { goto error__; });

			// 短 key 优化：优先使用栈内存
			if (len + 1 > RyanJsonInlineStringSize)
			{
				key = (char *)jsonMalloc((size_t)(len + 1U));
				RyanJsonCheckCode(NULL != key, { goto error__; });
				isKeyAllocated = RyanJsonTrue;
			}
			else
			{
				key = shortKey;
				isKeyAllocated = RyanJsonFalse;
			}

			// 解析 key 到缓冲区（key 指向堆内存或栈内存）
			RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseStringBuffer(parseBuf, key, len, hasEscape), { goto error__; });

			RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseBufSkipWhitespace(parseBuf), { goto error__; });
			RyanJsonCheckCode(parseBufHasRemain(parseBuf) && ':' == *parseBuf->currentPtr, { goto error__; });
			parseBufAdvanceCurrentPrt(parseBuf, 1);
			RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseBufSkipWhitespace(parseBuf), { goto error__; });

			// 严格模式下：Object 从源头拒绝重复 key，避免后续语义歧义（Get/Replace/Compare）
#if true == RyanJsonStrictObjectKeyCheck
			RyanJsonCheckCode(RyanJsonFalse == RyanJsonHasObjectByKey(scopeParent, key), { goto error__; });
#endif
		}
		else
		{
			key = NULL; // Array 没有 key
			isKeyAllocated = RyanJsonFalse;
		}

		// 阶段：解析 value
		// 解析值 (可能是标量，也可能是新的容器)
		RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseValue(parseBuf, key, &newItem), { goto error__; });

		// 释放动态分配的 key（Node 内部已拷贝一份）
		if (isKeyAllocated)
		{
			jsonFree(key);
			isKeyAllocated = RyanJsonFalse;
		}
		key = NULL; // 重置 key 指针

		// 阶段：挂接到父容器
		RyanJsonInternalListInsertAfter(scopeParent, lastSibling, newItem);

		lastSibling = newItem; // 更新游标

		// 阶段：遇到容器时下沉
		if (_checkType(newItem, RyanJsonTypeArray) || _checkType(newItem, RyanJsonTypeObject))
		{
			// 更新下沉后的层级状态
			scopeParent = newItem;
			lastSibling = NULL; // 新容器初始没有子节点
		}
	}

error__:
	// 失败收敛路径：释放临时资源并清理已构建的树
	if (isKeyAllocated) { jsonFree(key); }

	// 删除根节点（因为已经链接好了，删除根节点会递归删除所有已解析的部分）
	RyanJsonDelete(*root);
	*root = NULL;

	return RyanJsonFalse;
}

/**
 * @brief 校验解析结束位置是否合法
 *
 * @param parseBuf 解析缓冲区
 * @param requireNullTerminator RyanJsonTrue 时要求仅剩空白
 * @return RyanJsonBool_e 校验是否成功
 */
static RyanJsonBool_e RyanJsonParseCheckNullTerminator(RyanJsonParseBuffer *parseBuf, RyanJsonBool_e requireNullTerminator)
{
	RyanJsonCheckAssert(NULL != parseBuf);

	if (requireNullTerminator)
	{
		// 故意不检查，允许空白
		(void)RyanJsonParseBufSkipWhitespace(parseBuf);

		// 上面已经去掉空白，如果后面还有数据，则失败
		RyanJsonCheckReturnFalse(!parseBufHasRemain(parseBuf));
	}

	return RyanJsonTrue;
}

/**
 * @brief 解析 Json 文本（可配置长度与尾部校验）
 *
 * @param text 输入文本
 * @param size 文本长度
 * @param requireNullTerminator 是否要求解析后仅剩空白
 * @param parseEndPtr 输出第一个未消费字符位置，可为 NULL
 * @return RyanJson_t 解析成功返回根节点，失败返回 NULL
 * @note parseEndPtr 仅在解析成功时写入。
 */
RyanJson_t RyanJsonParseOptions(const char *text, uint32_t size, RyanJsonBool_e requireNullTerminator, const char **parseEndPtr)
{
	RyanJsonCheckReturnNull(NULL != text);

	RyanJsonParseBuffer parseBuf = {.currentPtr = (const uint8_t *)text, .remainSize = size};
	RyanJsonCheckReturnNull(RyanJsonTrue == RyanJsonParseBufSkipWhitespace(&parseBuf));

	RyanJson_t pJson;
	RyanJsonCheckReturnNull(RyanJsonTrue == RyanJsonParseIterative(&parseBuf, &pJson));

	// 检查解析后的文本后面是否有无意义的字符
	RyanJsonCheckCode(RyanJsonTrue == RyanJsonParseCheckNullTerminator(&parseBuf, requireNullTerminator), {
		RyanJsonDelete(pJson);
		return NULL;
	});

	if (parseEndPtr) { *parseEndPtr = (const char *)parseBuf.currentPtr; }

	return pJson;
}

/**
 * @brief 解析以 '\\0' 结尾的 Json 文本
 *
 * @param text 输入文本
 * @return RyanJson_t 解析成功返回根节点，失败返回 NULL
 */
RyanJson_t RyanJsonParse(const char *text)
{
	RyanJsonCheckReturnNull(NULL != text);
	return RyanJsonParseOptions(text, (uint32_t)RyanJsonStrlen(text), RyanJsonFalse, NULL);
}

/**
 * @brief 解析原始 Number 文本（打印回读校验辅助）
 *
 * @param currentPtr Number 文本起始地址
 * @param remainSize Number 文本长度
 * @param numberValuePtr 输出数值
 * @return RyanJsonBool_e 解析是否成功
 */
RyanJsonInternalApi RyanJsonBool_e RyanJsonInternalParseDoubleRaw(const uint8_t *currentPtr, uint32_t remainSize, double *numberValuePtr)
{
	RyanJsonCheckAssert(NULL != currentPtr && NULL != numberValuePtr);
	RyanJsonCheckAssert(remainSize > 0);

	RyanJsonBool_e isInt = RyanJsonTrue;
	RyanJsonParseBuffer parseBuf = {.currentPtr = currentPtr, .remainSize = remainSize};
	return RyanJsonInternalParseDouble(&parseBuf, numberValuePtr, &isInt);
}
