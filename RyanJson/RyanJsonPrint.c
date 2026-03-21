#include "RyanJsonInternal.h"

typedef struct
{
	uint8_t *bufAddress;      // 打印输出缓冲区地址
	uint32_t cursor;          // 当前写入位置（字节偏移）
	uint32_t size;            // 缓冲区总容量，禁止扩容时写满即返回失败
	RyanJsonBool_e isNoAlloc; // 是否禁止动态扩容（True 表示不扩容）
} RyanJsonPrintBuffer;

static inline void RyanJsonPrintBufPutChar(RyanJsonPrintBuffer *printfBuf, uint8_t ch)
{
	RyanJsonCheckAssert(NULL != printfBuf && NULL != printfBuf->bufAddress);
	printfBuf->bufAddress[printfBuf->cursor++] = ch;
}

static inline void RyanJsonPrintBufPutString(RyanJsonPrintBuffer *printfBuf, const uint8_t *putStr, uint32_t putStrLen)
{
	RyanJsonCheckAssert(NULL != printfBuf && NULL != printfBuf->bufAddress);
	RyanJsonCheckAssert(NULL != putStr);

	for (uint32_t putStrCount = 0; putStrCount < putStrLen; putStrCount++)
	{
		printfBuf->bufAddress[printfBuf->cursor++] = putStr[putStrCount];
	}
}

#define printBufCurrentPtr(printfBuf)  (&((printfBuf)->bufAddress[(printfBuf)->cursor]))
#define printBufRemainBytes(printfBuf) ((printfBuf)->size - (printfBuf)->cursor)

/**
 * @brief 检查并扩展打印缓冲区容量
 */
static RyanJsonBool_e RyanJsonPrintBufAppend(RyanJsonPrintBuffer *printfBuf, uint32_t needed)
{
	RyanJsonCheckAssert(NULL != printfBuf && NULL != printfBuf->bufAddress);

	needed += printfBuf->cursor;

	// 当前缓冲区空间充足
	if (needed <= printfBuf->size) { return RyanJsonTrue; }

	// 禁止动态扩容
	RyanJsonCheckReturnFalse(RyanJsonFalse == printfBuf->isNoAlloc);

	uint32_t size = needed + RyanJsonPrintfPreAlloSize;
	char *address = (char *)RyanJsonInternalExpandRealloc(printfBuf->bufAddress, printfBuf->size, size);
	RyanJsonCheckReturnFalse(NULL != address);

	printfBuf->size = size;
	printfBuf->bufAddress = (uint8_t *)address;
	return RyanJsonTrue;
}

/**
 * @brief 规范化 Double 输出：删除尾部无效的 0（非科学计数法时）
 */
static int32_t RyanJsonTrimDoubleTrailingZeros(RyanJsonPrintBuffer *printfBuf, int32_t len)
{
	// Linux 测试环境：偶尔把 'e' 改为 'E'，覆盖大小写兼容分支
#ifdef RyanJsonLinuxTestEnv
	int32_t eIndex = INT32_MIN;
	if (RyanJsonFuzzerShouldFail(20))
	{
		for (int32_t i = 0; i < len; i++)
		{
			if ('e' == printBufCurrentPtr(printfBuf)[i])
			{
				printBufCurrentPtr(printfBuf)[i] = 'E';
				eIndex = i;
				break;
			}
		}
	}
#endif

	// 检查是否使用科学计数法
	RyanJsonBool_e isScientificNotation = RyanJsonFalse;
	for (int32_t i = 0; i < len; i++)
	{
		// 有些平台会输出'E'
		if ('e' == printBufCurrentPtr(printfBuf)[i] || 'E' == printBufCurrentPtr(printfBuf)[i])
		{
			isScientificNotation = RyanJsonTrue;
			break;
		}
	}

	// 恢复测试环境临时改写的大写 'E'
#ifdef RyanJsonLinuxTestEnv
	if (INT32_MIN != eIndex) { printBufCurrentPtr(printfBuf)[eIndex] = 'e'; }
#endif

	if (RyanJsonFalse == isScientificNotation)
	{
		// 删除小数部分中无效的 0
		// 最小也要为 "0.0"
		while (len > 3)
		{
			if ('0' != printBufCurrentPtr(printfBuf)[len - 1]) { break; }
			if ('.' == printBufCurrentPtr(printfBuf)[len - 2]) { break; }
			len--;
			printBufCurrentPtr(printfBuf)[len] = '\0';
		}
	}

	return len;
}

/**
 * @brief 打印 Number 节点
 */
static RyanJsonBool_e RyanJsonPrintNumber(RyanJson_t pJson, RyanJsonPrintBuffer *printfBuf)
{
	RyanJsonCheckAssert(NULL != pJson && NULL != printfBuf);

	int32_t len;

	// Number 节点按 int32_t 存储
	if (RyanJsonFalse == RyanJsonGetPayloadNumberIsDoubleByFlag(pJson))
	{
		// INT32_MIN = -2147483648 (11 chars) + '\0'
		RyanJsonCheckReturnFalse(RyanJsonPrintBufAppend(printfBuf, 12));

		len = RyanJsonSnprintf((char *)printBufCurrentPtr(printfBuf), printBufRemainBytes(printfBuf), "%" PRId32,
				       RyanJsonGetIntValue(pJson));
		// 这里前面已保证至少 12 字节空间（INT32_MIN + '\0'），正常实现下不会截断
		RyanJsonCheckReturnFalse(len > 0);
		printfBuf->cursor += (uint32_t)len;

		return RyanJsonTrue;
	}

	// Number 节点按 Double 值存储
	RyanJsonCheckReturnFalse(RyanJsonPrintBufAppend(printfBuf, RyanJsonDoubleBufferSize));
	double doubleValue = RyanJsonGetDoubleValue(pJson);

	// 处理特殊值：无穷大和 NaN 输出为 Null（RFC 8259 不支持 Infinity/NaN）
	if (isinf(doubleValue) || isnan(doubleValue))
	{
		RyanJsonPrintBufPutString(printfBuf, (uint8_t *)"null", 4);
		return RyanJsonTrue;
	}

	double absDoubleValue = fabs(doubleValue);

	// 判断是否可按 Int 样式输出，并保留一位小数（例如 5.0 或 0.0）
	// 仅对“真实 0”或 [1e-6, 1e15) 区间内的 Int 样式启用，避免极小非零值被打印成 0.0
	if (((doubleValue == 0.0) || (absDoubleValue < 1.0e15 && absDoubleValue >= 1.0e-6)) &&
	    fabs(floor(doubleValue) - doubleValue) <= DBL_EPSILON)
	{
		len = RyanJsonSnprintf((char *)printBufCurrentPtr(printfBuf), printBufRemainBytes(printfBuf), "%.1lf", doubleValue);
		// 有外层限制 1e-6 ~ 1e15, 所以肯定不会越界
		RyanJsonCheckReturnFalse(len > 0);

		// 嵌入式场景下缓冲区可能偏小，保留额外边界检查
#ifndef RyanJsonLinuxTestEnv
		RyanJsonCheckReturnFalse(len < (int32_t)printBufRemainBytes(printfBuf));
#endif
	}
	else
	{
		// Linux 测试环境：在两种格式间切换，覆盖去零与回读校验分支
#ifdef RyanJsonLinuxTestEnv
#undef RyanJsonSnprintfSupportScientific
		// 基于 Double 值本身选择格式（确定性），保证同一个值总是用相同格式。
		// Linux 测试环境下用简单阈值切换分支，只用于覆盖率，不代表真实阈值策略。
		RyanJsonBool_e RyanJsonSnprintfSupportScientific = doubleValue > 1.0 ? RyanJsonTrue : RyanJsonFalse;

#endif

		// 极大/极小数或普通 Double
		// 若启用科学计数法则用 %.15g，否则使用 %lf。
		// 不使用 %.15g 的固定格式是因为很多嵌入式平台上 %.15g 与 %.17g 效果接近
		// 可能出现 0.2 -> 0.200000003000000，即便去掉尾部 0 仍不美观
		// 使用 %lf 存在缓冲区压力，但在当前嵌入式目标上可接受
		len = RyanJsonSnprintf((char *)printBufCurrentPtr(printfBuf), printBufRemainBytes(printfBuf),
				       RyanJsonSnprintfSupportScientific ? "%.15g" : "%lf", doubleValue);
#ifdef isEnableFuzzer
		// 测试环境：偶尔模拟溢出以触发防御性检查分支
		if (RyanJsonFuzzerShouldFail(1000) && len > 0) { len = (int32_t)printBufRemainBytes(printfBuf) + 1; }
#endif
		RyanJsonCheckReturnFalse(len > 0 && len < (int32_t)printBufRemainBytes(printfBuf));

		// 往返检查：在去 0 之前进行，确保原始精度足够
		// 如果精度不够，改用 %.17g
		double number = 0;
		RyanJsonCheckReturnFalse(RyanJsonTrue ==
					 RyanJsonInternalParseDoubleRaw(printBufCurrentPtr(printfBuf), (uint32_t)len, &number));
		// 容差比较可能把极小非零值与 0 视为相等，这里额外拦截“非零被抹成 0”的情况
		RyanJsonBool_e loseTinyNonZero = RyanJsonMakeBool(number == 0.0);
		if (RyanJsonTrue == loseTinyNonZero || RyanJsonFalse == RyanJsonCompareDouble(number, doubleValue))
		{
			len = RyanJsonSnprintf((char *)printBufCurrentPtr(printfBuf), printBufRemainBytes(printfBuf), "%.17g", doubleValue);
			RyanJsonCheckReturnFalse(len > 0);

#ifndef RyanJsonLinuxTestEnv
			// "%.17g" 也做边界检查，避免平台实现差异导致越界
			RyanJsonCheckReturnFalse(len < (int32_t)printBufRemainBytes(printfBuf));
#endif
		}

		// 删除尾部无效 0。理论上主要作用于 %lf，但统一处理更稳妥
		len = RyanJsonTrimDoubleTrailingZeros(printfBuf, len);
	}

	printfBuf->cursor += (uint32_t)len;
	return RyanJsonTrue;
}

/**
 * @brief 打印字符串并执行转义
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
			// 按最坏情况预留转义空间，保证后续写入安全
			if (*strCurrentPtr < 32) { escapeCharCount += 5; }
			break;
		}
	}

	RyanJsonCheckReturnFalse(
		RyanJsonPrintBufAppend(printfBuf, (uint32_t)(strCurrentPtr - strValue) + escapeCharCount + 2U)); // 最小是\" \"
	RyanJsonPrintBufPutChar(printfBuf, '\"');

	// 没有转义字符
	if (0 == escapeCharCount)
	{
		RyanJsonPrintBufPutString(printfBuf, strValue, (uint32_t)(strCurrentPtr - strValue));
		RyanJsonPrintBufPutChar(printfBuf, '\"');
		return RyanJsonTrue;
	}

	strCurrentPtr = strValue;
	while (*strCurrentPtr)
	{
		if ((*strCurrentPtr) >= ' ' && '\"' != *strCurrentPtr && '\\' != *strCurrentPtr)
		{
			RyanJsonPrintBufPutChar(printfBuf, *strCurrentPtr++);
			continue;
		}

		// 转义和打印
		RyanJsonPrintBufPutChar(printfBuf, '\\');

		switch (*strCurrentPtr)
		{
		case '\\': RyanJsonPrintBufPutChar(printfBuf, '\\'); break;
		case '\"': RyanJsonPrintBufPutChar(printfBuf, '\"'); break;
		case '\b': RyanJsonPrintBufPutChar(printfBuf, 'b'); break;
		case '\f': RyanJsonPrintBufPutChar(printfBuf, 'f'); break;
		case '\n': RyanJsonPrintBufPutChar(printfBuf, 'n'); break;
		case '\r': RyanJsonPrintBufPutChar(printfBuf, 'r'); break;
		case '\t': RyanJsonPrintBufPutChar(printfBuf, 't'); break;

		default: {
			// 这里按字节转义，不做 UTF-8 合法性校验
			RyanJsonCheckReturnFalse(5 == RyanJsonSnprintf((char *)printBufCurrentPtr(printfBuf),
								       printBufRemainBytes(printfBuf), "u%04X", *strCurrentPtr));
			printfBuf->cursor += 5; // uXXXX 四位十六进制编码
			break;
		}
		}
		strCurrentPtr++;
	}

	RyanJsonPrintBufPutChar(printfBuf, '\"');

	return RyanJsonTrue;
}

/**
 * @brief 打印节点中的 strValue
 *
 * @param pJson String 节点
 * @param printfBuf 打印缓冲区
 * @return RyanJsonBool_e 打印是否成功
 */
static RyanJsonBool_e RyanJsonPrintString(RyanJson_t pJson, RyanJsonPrintBuffer *printfBuf)
{
	RyanJsonCheckAssert(NULL != pJson && NULL != printfBuf);
	return RyanJsonPrintStringBuffer((const uint8_t *)RyanJsonGetStringValue(pJson), printfBuf);
}

/**
 * @brief 将 Json 树打印为字符串（迭代实现）
 */
static RyanJsonBool_e RyanJsonPrintValue(RyanJson_t pJson, RyanJsonPrintBuffer *printfBuf, uint32_t depthStart,
					 const RyanJsonPrintStyle *style)
{
	RyanJsonCheckAssert(NULL != pJson && NULL != printfBuf);

	RyanJson_t curr = pJson;
	uint32_t depth = depthStart;

	// 无需显式栈：通过线索化链表与容器状态完成遍历与回溯
	while (1)
	{
		// 打印 key（当前节点包含 key 时）
		if (curr != pJson && RyanJsonIsKey(curr))
		{
			if (style->format)
			{
				uint32_t needed = depth * style->indentLen;
				RyanJsonCheckReturnFalse(RyanJsonPrintBufAppend(printfBuf, needed));
				for (uint32_t i = 0; i < depth; i++)
				{
					RyanJsonPrintBufPutString(printfBuf, (uint8_t *)style->indent, style->indentLen);
				}
			}

			RyanJsonCheckReturnFalse(RyanJsonPrintStringBuffer((const uint8_t *)RyanJsonGetKey(curr), printfBuf));

			// 打印冒号和空格
			uint32_t spaceLen = style->format ? style->spaceAfterColon : 0;
			RyanJsonCheckReturnFalse(RyanJsonPrintBufAppend(printfBuf, 1 + spaceLen));
			RyanJsonPrintBufPutChar(printfBuf, ':');
			if (style->format)
			{
				for (uint32_t i = 0; i < spaceLen; i++)
				{
					RyanJsonPrintBufPutChar(printfBuf, ' ');
				}
			}
		}
		else if (curr != pJson && style->format)
		{
			// Array 元素缩进（没有 key）
			uint32_t needed = depth * style->indentLen;
			RyanJsonCheckReturnFalse(RyanJsonPrintBufAppend(printfBuf, needed));
			for (uint32_t i = 0; i < depth; i++)
			{
				RyanJsonPrintBufPutString(printfBuf, (uint8_t *)style->indent, style->indentLen);
			}
		}

		// 打印 Value（标量值或容器起始符）
		RyanJsonType_e type = RyanJsonGetType(curr);
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif
		switch (type)
		{
		case RyanJsonTypeNull:
			RyanJsonCheckReturnFalse(RyanJsonPrintBufAppend(printfBuf, 4));
			RyanJsonPrintBufPutString(printfBuf, (uint8_t *)"null", 4);
			break;

		case RyanJsonTypeBool: {
			RyanJsonBool_e val = RyanJsonGetBoolValue(curr);
			RyanJsonCheckReturnFalse(RyanJsonPrintBufAppend(printfBuf, val ? 4 : 5));
			RyanJsonPrintBufPutString(printfBuf, val ? (uint8_t *)"true" : (uint8_t *)"false", val ? 4 : 5);
			break;
		}

		case RyanJsonTypeNumber: RyanJsonCheckReturnFalse(RyanJsonPrintNumber(curr, printfBuf)); break;

		case RyanJsonTypeString: RyanJsonCheckReturnFalse(RyanJsonPrintString(curr, printfBuf)); break;

		case RyanJsonTypeArray:
		case RyanJsonTypeObject: {
			RyanJsonBool_e currIsObject = (type == RyanJsonTypeObject);
			RyanJson_t currChild = RyanJsonGetObjectValue(curr);

			// 空容器直接输出 [] 或 {}
			if (NULL == currChild)
			{
				RyanJsonCheckReturnFalse(RyanJsonPrintBufAppend(printfBuf, 2));
				RyanJsonPrintBufPutChar(printfBuf, currIsObject ? '{' : '[');
				RyanJsonPrintBufPutChar(printfBuf, currIsObject ? '}' : ']');
			}
			// 非空容器进入子节点处理
			else
			{
				uint32_t newlineLen = style->format ? style->newlineLen : 0;
				RyanJsonCheckReturnFalse(RyanJsonPrintBufAppend(printfBuf, 1 + newlineLen)); // '[' + newline
				RyanJsonPrintBufPutChar(printfBuf, currIsObject ? '{' : '[');

				// 开启 format 后，非空容器统一走多行输出
				if (style->format) { RyanJsonPrintBufPutString(printfBuf, (uint8_t *)style->newline, newlineLen); }

				curr = currChild;
				depth++;
				continue; // 直接跳转到下一次循环处理 Child
			}
			break;
		}

		default: return RyanJsonFalse;
		}
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

		// 处理逗号、兄弟节点切换与回溯闭合
		if (curr == pJson) { return RyanJsonTrue; }

		do
		{
			// 有兄弟节点时输出逗号并切换到兄弟
			RyanJson_t nextInfo = RyanJsonGetNext(curr); // 能够处理 IsLast
			if (nextInfo)
			{
				uint32_t newlineLen = style->format ? style->newlineLen : 0;
				RyanJsonCheckReturnFalse(RyanJsonPrintBufAppend(printfBuf, 1 + newlineLen)); // ',' + newline
				RyanJsonPrintBufPutChar(printfBuf, ',');

				if (style->format) { RyanJsonPrintBufPutString(printfBuf, (uint8_t *)style->newline, newlineLen); }

				curr = nextInfo;
				break; // 处理新的 curr（兄弟节点）
			}

			// 无兄弟节点时回溯到父节点并闭合容器
			// 利用线索化特性：IsLast 节点的 next 指向父节点
			curr = curr->next;
			depth--;

			// 打印结束括号前的缩进（仅 format 模式）
			if (style->format)
			{
				uint32_t needed = style->newlineLen + depth * style->indentLen;

				RyanJsonCheckReturnFalse(RyanJsonPrintBufAppend(printfBuf, needed));
				RyanJsonPrintBufPutString(printfBuf, (uint8_t *)style->newline, style->newlineLen);
				for (uint32_t i = 0; i < depth; i++)
				{
					RyanJsonPrintBufPutString(printfBuf, (uint8_t *)style->indent, style->indentLen);
				}
			}

			RyanJsonCheckReturnFalse(RyanJsonPrintBufAppend(printfBuf, 1));
			RyanJsonPrintBufPutChar(printfBuf, RyanJsonIsArray(curr) ? ']' : '}');

			// 如果回溯到了起始根节点，结束打印
			if (curr == pJson) { return RyanJsonTrue; }

			// 继续循环，检查父节点是否仍有同层兄弟
		} while (1);
	}
}

/**
 * @brief 按指定风格打印 Json（动态分配输出缓冲）
 *
 * @param pJson 待打印节点
 * @param preset 初始缓冲大小
 * @param style 打印风格
 * @param len 输出长度，可为 NULL
 * @return char* 打印结果，需调用 RyanJsonFree 释放
 */
char *RyanJsonPrintWithStyle(RyanJson_t pJson, uint32_t preset, const RyanJsonPrintStyle *style, uint32_t *len)
{
	RyanJsonCheckReturnNull(NULL != pJson && NULL != style);

	RyanJsonPrintBuffer printfBuf = {
		.isNoAlloc = RyanJsonFalse,
		.size = preset,
		.cursor = 0,
	};

	if (printfBuf.size < RyanJsonPrintfPreAlloSize) { printfBuf.size = RyanJsonPrintfPreAlloSize; }
	printfBuf.bufAddress = (uint8_t *)jsonMalloc(printfBuf.size);
	RyanJsonCheckReturnNull(NULL != printfBuf.bufAddress);

	RyanJsonCheckCode(RyanJsonTrue == RyanJsonPrintValue(pJson, &printfBuf, 0, style), {
		jsonFree(printfBuf.bufAddress);
		return NULL;
	});

	RyanJsonCheckCode(RyanJsonPrintBufAppend(&printfBuf, 1), {
		jsonFree(printfBuf.bufAddress);
		return NULL;
	});

	printfBuf.bufAddress[printfBuf.cursor] = '\0';
	if (len) { *len = printfBuf.cursor; }

	return (char *)printfBuf.bufAddress;
}

/**
 * @brief 使用默认风格打印 Json（动态分配输出缓冲）
 *
 * @param pJson 待打印节点
 * @param preset 初始缓冲大小
 * @param format 是否格式化输出
 * @param len 输出长度，可为 NULL
 * @return char* 打印结果，需调用 RyanJsonFree 释放
 */
char *RyanJsonPrint(RyanJson_t pJson, uint32_t preset, RyanJsonBool_e format, uint32_t *len)
{
	RyanJsonPrintStyle style = {
		.indent = "\t", .newline = "\n", .indentLen = 1, .newlineLen = 1, .spaceAfterColon = 1, .format = format};
	return RyanJsonPrintWithStyle(pJson, preset, &style, len);
}

/**
 * @brief 按指定风格打印 Json（使用外部预分配缓冲）
 *
 * @param pJson 待打印节点
 * @param buffer 外部缓冲区
 * @param length 缓冲区大小
 * @param style 打印风格
 * @param len 输出长度，可为 NULL
 * @return char* 成功返回 buffer，失败返回 NULL
 */
char *RyanJsonPrintPreallocatedWithStyle(RyanJson_t pJson, char *buffer, uint32_t length, const RyanJsonPrintStyle *style, uint32_t *len)
{
	RyanJsonCheckReturnNull(NULL != pJson && NULL != buffer && NULL != style && length > 0);

	RyanJsonPrintBuffer printfBuf = {
		.bufAddress = (uint8_t *)buffer,
		.isNoAlloc = RyanJsonTrue,
		.size = length,
		.cursor = 0,
	};

	RyanJsonCheckReturnNull(RyanJsonTrue == RyanJsonPrintValue(pJson, &printfBuf, 0, style));
	RyanJsonCheckReturnNull(RyanJsonPrintBufAppend(&printfBuf, 1));
	printfBuf.bufAddress[printfBuf.cursor] = '\0';
	if (len) { *len = printfBuf.cursor; }

	return (char *)printfBuf.bufAddress;
}

/**
 * @brief 使用默认风格打印 Json（使用外部预分配缓冲）
 *
 * @param pJson 待打印节点
 * @param buffer 外部缓冲区
 * @param length 缓冲区大小
 * @param format 是否格式化输出
 * @param len 输出长度，可为 NULL
 * @return char* 成功返回 buffer，失败返回 NULL
 */
char *RyanJsonPrintPreallocated(RyanJson_t pJson, char *buffer, uint32_t length, RyanJsonBool_e format, uint32_t *len)
{
	RyanJsonPrintStyle style = {
		.indent = "\t", .newline = "\n", .indentLen = 1, .newlineLen = 1, .spaceAfterColon = 1, .format = format};
	return RyanJsonPrintPreallocatedWithStyle(pJson, buffer, length, &style, len);
}
