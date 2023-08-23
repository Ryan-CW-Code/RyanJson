
#include "RyanJson.h"

#define _checkType(info, type) ((info) & (type))

// 跳过无意义的字符
#define parseSkipInvalidChar(buf, errCode)                                                     \
    {                                                                                          \
        while (*(buf)->address &&                                                              \
               (*(buf)->address == ' ' || *(buf)->address == '\n' || *(buf)->address == '\r')) \
        {                                                                                      \
            parseCanRedBufAndOffset(buf, 1, errCode);                                          \
        }                                                                                      \
    }

// 是否还有可读的待解析文本
#define parseCanRead(buf, num) ((buf)->size >= (uint32_t)(num))

// 偏移几个字节
#define parseBufAddOffset(buf, num) \
    {                               \
        (buf)->address += (num);    \
        (buf)->size -= (num);       \
    }

// 上面两个的语法糖
#define parseCanRedBufAndOffset(buf, num, errCode) \
    {                                              \
        if (parseCanRead(buf, num))                \
        {                                          \
            parseBufAddOffset(buf, num)            \
        }                                          \
        else                                       \
        {                                          \
            errCode                                \
        };                                         \
    }

#define printBufAppend(buf, n) expansion(buf, (n))
#define printBufPutc(buf, c) ((buf)->address[(buf)->end++] = (c))
#define printBufPuts(buf, s, len)           \
    {                                       \
        for (int32_t i = 0; i < (len); i++) \
            printBufPutc(buf, (s)[i]);      \
    }
#define printBufEnd(buf) (&((buf)->address[(buf)->end]))

typedef struct
{
    uint32_t size;  // 待序列化字符串长度
    uint32_t depth; /* How deeply nested (in arrays/objects) is the input at the current offset. */
    char *address;  // 待序列化字符串地址
} parseBuffer;

typedef struct
{
    RyanJsonBool noalloc; // 是否动态申请内存
    uint32_t size;        // 不动态申请内存时，到达此size大小将返回失败
    uint32_t end;         // 反序列化后的字符串长度
    char *address;        // 反序列化后的字符串地址
} printBuffer;

typedef union
{
    int32_t int_;
    double double_;
} jsonNumber;

static malloc_t jsonMalloc = malloc;
static free_t jsonFree = free;
static realloc_t jsonRealloc = realloc;

static RyanJsonBool RyanJsonParseValue(parseBuffer *buf, char *key, RyanJson_t *out);
static RyanJsonBool RyanJsonPrintValue(RyanJson_t pJson, printBuffer *buf, uint32_t depth, RyanJsonBool format);

static void *temp_realloc(void *block, size_t size)
{

    void *b = jsonMalloc(size);
    if (NULL == b)
        return NULL;

    if (NULL != block)
    {
        memcpy(b, block, size);
        jsonFree(block);
    }

    return b;
}

/**
 * @brief 提供内存钩子函数
 *
 * @param _malloc
 * @param _free
 * @param _realloc 可以为NULL
 * @return RyanJsonBool
 */
RyanJsonBool RyanJsonInitHooks(malloc_t _malloc, free_t _free, realloc_t _realloc)
{
    if (NULL == _malloc || NULL == _free)
        return RyanJsonFalse;

    jsonMalloc = _malloc;
    jsonFree = _free;
    jsonRealloc = NULL != _realloc ? _realloc : temp_realloc;

    return RyanJsonTrue;
}

/**
 * @brief 安全的浮点数比较
 *
 * @param a
 * @param b
 * @return RyanJsonBool
 */
static RyanJsonBool compare_double(double a, double b)
{
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

/**
 * @brief 得到大于 x 的最小 2 次方。
 *
 * @param x
 * @return int32_t 大于x的最小2次方
 */
static int32_t pow2gt(int32_t x)
{
    int32_t b = sizeof(int32_t) * 8;
    int32_t i = 1;
    --x;
    while (i < b)
    {
        x |= (x >> i);
        i <<= 1;
    }
    return x + 1;
}

/**
 * @brief 申请buf容量, 决定是否进行扩容
 *
 * @param buf
 * @param needed
 * @return RyanJsonBool
 */
static RyanJsonBool expansion(printBuffer *buf, int32_t needed)
{
    char *address = NULL;
    int32_t size = 0;

    if (NULL == buf || NULL == buf->address)
        return RyanJsonFalse;

    needed += buf->end;

    // 当前 buf 中有足够的空间
    if (needed <= buf->size)
        return RyanJsonTrue;

    // 不使用动态内存分配
    if (RyanJsonTrue == buf->noalloc)
        return RyanJsonFalse;

    size = pow2gt(needed);
    address = (char *)jsonRealloc(buf->address, size);
    if (NULL == address)
        return RyanJsonFalse;

    buf->size = size;
    buf->address = address;
    return RyanJsonTrue;
}

/**
 * @brief strdup本地实现
 * 主要为了使用自定义malloc
 * @param str
 * @return char*
 */
static char *RyanJsonStrdup(const char *str)
{
    int32_t size = strlen(str) + 1;
    char *s = (char *)jsonMalloc(size);
    if (NULL == s)
        return NULL;

    memcpy(s, str, size - 1);
    s[size - 1] = '\0';
    return s;
}

/**
 * @brief 创建一个新的json节点
 *
 * @param info 节点信息
 * @param key 带key的对象将动态添加key子项, 没有的传null，由调用者保证key的时间有效性
 * @return RyanJson_t 新json节点句柄
 */
RyanJson_t RyanJsonNewNode(int32_t info, char *key)
{

    RyanJson_t pJson = NULL;
    int32_t size = sizeof(struct RyanJsonNode);

    if (_checkType(info, RyanJsonTypeNumber))
        size += sizeof(jsonNumber);
    else if (_checkType(info, RyanJsonTypeString))
        size += sizeof(char *);
    else if (_checkType(info, RyanJsonTypeArray) || _checkType(info, RyanJsonTypeObject))
        size += sizeof(RyanJson_t);

    if (NULL != key)
    {
        info |= RyanJsonWithKeyFlag;
        size += sizeof(char *);
    }
    else
        info &= (~RyanJsonWithKeyFlag);

    pJson = (RyanJson_t)jsonMalloc(size);
    if (NULL != pJson)
    {
        memset(pJson, 0, size);
        pJson->info = info;
        if (NULL != key)
            RyanJsonGetKey(pJson) = key;
    }

    return pJson;
}

/**
 * @brief 获取json节点value隐藏地址
 *
 * @param pJson
 * @return void* null为失败
 */
void *RyanJsonGetValue(RyanJson_t pJson)
{
    if (NULL == pJson)
        return NULL;

    if (_checkType(pJson->info, RyanJsonTypeNull) || _checkType(pJson->info, RyanJsonTypeBool))
        return NULL;

    char *base = (char *)pJson + sizeof(struct RyanJsonNode);

    if (_checkType(pJson->info, RyanJsonWithKeyFlag))
        base += sizeof(char *);

    return (void *)base;
}

/**
 * @brief 删除json及其子项
 *
 * @param pJson
 */
void RyanJsonDelete(RyanJson_t pJson)
{
    RyanJson_t next = NULL;

    while (pJson)
    {
        next = pJson->next;

        // 递归删除
        if (_checkType(pJson->info, RyanJsonTypeArray) || _checkType(pJson->info, RyanJsonTypeObject))
            RyanJsonDelete(RyanJsonGetObjectValue(pJson)); // 递归删除子对象

        if (_checkType(pJson->info, RyanJsonTypeString) && RyanJsonGetStringValue(pJson))
            jsonFree(RyanJsonGetStringValue(pJson));

        if (_checkType(pJson->info, RyanJsonWithKeyFlag) && RyanJsonGetKey(pJson))
            jsonFree(RyanJsonGetKey(pJson));

        jsonFree(pJson);

        pJson = next;
    }
}

/**
 * @brief 释放RyanJson申请的资源
 *
 * @param block
 */
inline void RyanJsonFree(void *block)
{
    jsonFree(block);
}

/**
 * @brief 从字符串中获取十六进制值
 *
 * @param text
 * @return uint32_t 16进制值
 */
static uint32_t RyanJsonParseHex(const char *text)
{
    uint32_t value = 0;

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
        case '9':
            value = (value << 4) + (text[i] - '0');
            break;
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            value = (value << 4) + 10 + (text[i] - 'a');
            break;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            value = (value << 4) + 10 + (text[i] - 'A');
            break;
        default:
            return RyanJsonFalse;
        }
    }

    return value;
}

/**
 * @brief 解析文本中的数字，添加到json节点中
 *
 * @param text 带有jsonNumber的文本
 * @param key 对应的key
 * @param out 用于接收解析后的pJson对象的地址
 * @return const char* 转换后文本的新地址
 */
static RyanJsonBool RyanJsonParseNumber(parseBuffer *buf, char *key, RyanJson_t *out)
{
    RyanJson_t newItem = NULL;
    double number = 0;
    int32_t sign = 1, scale = 0, e_sign = 1, e_scale = 0;
    RyanJsonBool isint = RyanJsonTrue;

    if (*buf->address == '-') // 符号部分
    {
        sign = -1;
        parseCanRedBufAndOffset(buf, 1, { return RyanJsonFalse; });
        if (!(*buf->address >= '0' && *buf->address <= '9'))
            return RyanJsonFalse;
    }

    // 跳过 0 无效字符
    while (*buf->address == '0')
        parseCanRedBufAndOffset(buf, 1, { return RyanJsonFalse; });

    if (*buf->address >= '1' && *buf->address <= '9') // 整数部分
    {
        do
        {
            number = (number * 10.0) + (*buf->address - '0'); // 进位加法
            parseCanRedBufAndOffset(buf, 1, { return RyanJsonFalse; });
        } while (*buf->address >= '0' && *buf->address <= '9');
    }

    if (*buf->address == '.') // 小数部分
    {
        parseCanRedBufAndOffset(buf, 1, { return RyanJsonFalse; });
        if (!(*buf->address >= '0' && *buf->address <= '9'))
            return RyanJsonFalse;

        do
        {
            number = (number * 10.0) + (*buf->address - '0');
            scale--;
            parseCanRedBufAndOffset(buf, 1, { return RyanJsonFalse; });
        } while (*buf->address >= '0' && *buf->address <= '9');
        isint = RyanJsonFalse;
    }

    if (*buf->address == 'e' || *buf->address == 'E') // 指数部分
    {
        parseCanRedBufAndOffset(buf, 1, { return RyanJsonFalse; });
        if (*buf->address == '+')
        {
            parseCanRedBufAndOffset(buf, 1, { return RyanJsonFalse; });
        }
        else if (*buf->address == '-') // 带符号
        {
            e_sign = -1;
            parseCanRedBufAndOffset(buf, 1, { return RyanJsonFalse; });
        }
        if (!(*buf->address >= '0' && *buf->address <= '9'))
            return RyanJsonFalse;

        while (*buf->address >= '0' && *buf->address <= '9')
        {
            e_scale = (e_scale * 10) + (*buf->address - '0');
            parseCanRedBufAndOffset(buf, 1, { return RyanJsonFalse; });
        }

        isint = RyanJsonFalse;
    }

    newItem = RyanJsonNewNode(RyanJsonTypeNumber, key);
    if (NULL == newItem)
        return RyanJsonFalse;
    *out = newItem;

    number = (double)sign * number * pow(10.0, (scale + e_scale * e_sign));

    if (RyanJsonTrue == isint && INT_MIN <= number && number <= INT_MAX)
    {
        newItem->info |= RyanJsonValueNumberIntFlag;
        RyanJsonGetIntValue(newItem) = (int32_t)number;
    }
    else
        RyanJsonGetDoubleValue(newItem) = number;

    return RyanJsonTrue;
}

/**
 * @brief 解析文本中的字符串，添加到json节点中
 *
 * @param text 带有jsonString的文本
 * @param buf 接收解析后的字符串指针的地址
 * @return RyanJsonBool
 */
static RyanJsonBool parse_string_buffer(parseBuffer *buf, char **buffer)
{
    char *p2 = NULL;
    char *out = NULL;
    int32_t len = 0;

    *buffer = NULL;

    // 不是字符串
    if (*buf->address != '\"')
        return RyanJsonFalse;

    parseCanRedBufAndOffset(buf, 1, { return RyanJsonFalse; });

    // 获取字符串长度
    for (int32_t i = 0;; i++)
    {
        if (!parseCanRead(buf, i))
            return RyanJsonFalse;

        if (!*(buf->address + i))
            return RyanJsonFalse;

        if (*(buf->address + i) == '\"')
            break;

        if (*(buf->address + i) == '\\') // 跳过转义符号
        {
            if (!parseCanRead(buf, i + 1))
                return RyanJsonFalse;
            i++;
        }

        len++;
    }

    out = (char *)jsonMalloc(len + 1);
    if (NULL == out)
        return RyanJsonFalse;

    p2 = out;
    while (*buf->address && *buf->address != '\"')
    {
        // 普通字符
        if (*buf->address != '\\')
        {
            *p2++ = *buf->address;
            parseCanRedBufAndOffset(buf, 1, { goto __error; });
            continue;
        }

        // 转义字符
        parseCanRedBufAndOffset(buf, 1, { goto __error; });
        switch (*buf->address)
        {
        case 'b':
        case 'f':
        case 'n':
        case 'r':
        case 't':
        case '\\':
        case '\"':
        case '/': //! 此函数不会对json注释做任何转化！如果需要清除注释请使用 RyanJsonMinify 函数
            *p2++ = *buf->address;
            break;

        case 'u':
        {
            int32_t utf8_length = 0;
            uint32_t uc, uc2;
            uint8_t first_byte_mark = 0;

            // 获取 Unicode 字符
            parseCanRedBufAndOffset(buf, 4, { goto __error; });
            uc = RyanJsonParseHex(buf->address - 3);

            // 检查是否有效
            if (uc >= 0xDC00 && uc <= 0xDFFF)
                goto __error;

            if (uc >= 0xD800 && uc <= 0xDBFF) // UTF16 代理对
            {
                if (!parseCanRead(buf, 2))
                    goto __error;

                if (buf->address[1] != '\\' || buf->address[2] != 'u')
                    goto __error; // 缺少代理的后半部分

                parseCanRedBufAndOffset(buf, 6, { goto __error; });
                uc2 = RyanJsonParseHex(buf->address - 3);
                if (0 == uc || uc2 < 0xDC00 || uc2 > 0xDFFF)
                    goto __error; // 无效的代理后半部分

                uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
            }

            /* encode as UTF-8
             * takes at maximum 4 bytes to encode:
             * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
            if (uc < 0x80)
            {
                utf8_length = 1; // normal ascii, encoding 0xxxxxxx
            }
            else if (uc < 0x800)
            {
                utf8_length = 2;        // two bytes, encoding 110xxxxx 10xxxxxx
                first_byte_mark = 0xC0; // 11000000
            }
            else if (uc < 0x10000)
            {
                utf8_length = 3;        // three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx
                first_byte_mark = 0xE0; // 11100000
            }
            else if (uc <= 0x10FFFF)
            {

                utf8_length = 4;        // four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx
                first_byte_mark = 0xF0; // 11110000
            }
            else
            {
                goto __error; // 无效的 unicode 代码点
            }

            // encode as utf8
            for (uint8_t utf8_position = (uint8_t)(utf8_length - 1); utf8_position > 0; utf8_position--)
            {
                p2[utf8_position] = (uint8_t)((uc | 0x80) & 0xBF); // 10xxxxxx
                uc >>= 6;
            }

            // encode first byte
            if (utf8_length > 1)
                p2[0] = (uint8_t)((uc | first_byte_mark) & 0xFF);
            else
                p2[0] = (uint8_t)(uc & 0x7F);

            p2 += utf8_length;
            // printf("uc: %04x, d: %d, utf8_length: %d, p2: %04x\r\n", uc, uc, utf8_length, *p2);
        }
        break;

        default:
            // *p2++ = *buf->address;
            goto __error;
        }

        parseCanRedBufAndOffset(buf, 1, { goto __error; });
    }

    *p2 = '\0';
    if (*buf->address == '\"')
    {
        parseCanRedBufAndOffset(buf, 1, { goto __error; });
    }

    *buffer = out;

    return RyanJsonTrue;

__error:
    jsonFree(out);
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
static RyanJsonBool RyanJsonParseString(parseBuffer *buf, char *key, RyanJson_t *out)
{
    RyanJson_t newItem = NULL;
    char *buffer = NULL;

    if (RyanJsonFalse == parse_string_buffer(buf, &buffer))
        return RyanJsonFalse;

    newItem = RyanJsonNewNode(RyanJsonTypeString, key);
    if (NULL == newItem)
    {
        jsonFree(buffer);
        return RyanJsonFalse;
    }

    *out = newItem;
    RyanJsonGetStringValue(newItem) = buffer;

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
static RyanJsonBool RyanJsonParseArray(parseBuffer *buf, char *key, RyanJson_t *out)
{
    RyanJson_t newItem = NULL, prev = NULL, item = NULL;

    if (*buf->address != '[') // 不是数组
        return RyanJsonFalse;

    buf->depth++;
    if (buf->depth > RyanJsonNestingLimit)
        return RyanJsonFalse;

    newItem = RyanJsonNewNode(RyanJsonTypeArray, key);
    if (NULL == newItem)
        return RyanJsonFalse;

    // 防止解析出错时，删除上层传递下来的key
    if (NULL != key)
        RyanJsonGetKey(newItem) = NULL;

    parseCanRedBufAndOffset(buf, 1, { goto __error; });
    parseSkipInvalidChar(buf, { goto __error; });
    if (*buf->address == ']') // 空数组
        goto __next;

    do
    {
        if (NULL != prev)
            parseCanRedBufAndOffset(buf, 1, { goto __error; }); // 跳过 ','

        // 解析值
        parseSkipInvalidChar(buf, { goto __error; });
        if (RyanJsonFalse == RyanJsonParseValue(buf, NULL, &item))
            goto __error;

        parseSkipInvalidChar(buf, { goto __error; });

        if (NULL != prev)
            prev->next = item;
        else
            RyanJsonGetObjectValue(newItem) = item;
        prev = item;

    } while (*buf->address == ',');

    if (*buf->address != ']')
        goto __error;

__next:
    parseCanRedBufAndOffset(buf, 1, { goto __error; });

    if (NULL != key)
        RyanJsonGetKey(newItem) = key;

    *out = newItem;

    return RyanJsonTrue;

__error:
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
static RyanJsonBool RyanJsonParseObject(parseBuffer *buf, char *key, RyanJson_t *out)
{
    RyanJson_t newItem = NULL, prev = NULL, item = NULL;
    char *k = NULL;

    if (*buf->address != '{')
        return RyanJsonFalse;

    buf->depth++;
    if (buf->depth > RyanJsonNestingLimit)
        return RyanJsonFalse;

    newItem = RyanJsonNewNode(RyanJsonTypeObject, key);
    if (NULL == newItem)
        return RyanJsonFalse;

    // 防止解析出错时，删除上层传递下来的key
    if (NULL != key)
        RyanJsonGetKey(newItem) = NULL;

    parseCanRedBufAndOffset(buf, 1, { goto __error; });
    parseSkipInvalidChar(buf, { goto __error; });
    if (*buf->address == '}')
        goto __next;

    do
    {

        if (NULL != prev)
            parseCanRedBufAndOffset(buf, 1, { goto __error; }); // 跳过 ','

        parseSkipInvalidChar(buf, { goto __error; });

        if (RyanJsonFalse == parse_string_buffer(buf, &k))
            goto __error;

        parseSkipInvalidChar(buf, { goto __error; });

        // 解析指示符 ':'
        if (*buf->address != ':')
            goto __error;

        parseCanRedBufAndOffset(buf, 1, { goto __error; });
        parseSkipInvalidChar(buf, { goto __error; });

        if (RyanJsonFalse == RyanJsonParseValue(buf, k, &item))
            goto __error;

        parseSkipInvalidChar(buf, { goto __error; });

        if (NULL != prev)
            prev->next = item;
        else
            RyanJsonGetObjectValue(newItem) = item;
        prev = item;

    } while (*buf->address == ',');

    if (*buf->address != '}')
    {
        k = NULL; // 由上层进行删除
        goto __error;
    }

__next:
    parseCanRedBufAndOffset(buf, 1, { goto __error; });
    if (NULL != key)
        RyanJsonGetKey(newItem) = key;
    *out = newItem;

    return RyanJsonTrue;

__error:
    if (NULL != k)
        jsonFree(k);
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
static RyanJsonBool RyanJsonParseValue(parseBuffer *buf, char *key, RyanJson_t *out)
{
    *out = NULL;

    if (*buf->address == '-' || (*buf->address >= '0' && *buf->address <= '9'))
        return RyanJsonParseNumber(buf, key, out);
    if (*buf->address == '\"')
        return RyanJsonParseString(buf, key, out);
    if (*buf->address == '[')
        return RyanJsonParseArray(buf, key, out);
    if (*buf->address == '{')
        return RyanJsonParseObject(buf, key, out);

    if (parseCanRead(buf, 4) && 0 == strncmp(buf->address, "null", 4))
    {
        *out = RyanJsonNewNode(RyanJsonTypeNull, key);
        if (NULL == *out)
            return RyanJsonFalse;

        parseBufAddOffset(buf, 4);
        return RyanJsonTrue;
    }
    if (parseCanRead(buf, 5) && 0 == strncmp(buf->address, "false", 5))
    {
        *out = RyanJsonNewNode(RyanJsonTypeBool, key);
        if (NULL == *out)
            return RyanJsonFalse;

        parseBufAddOffset(buf, 5);
        return RyanJsonTrue;
    }
    if (parseCanRead(buf, 4) && 0 == strncmp(buf->address, "true", 4))
    {
        *out = RyanJsonNewNode(RyanJsonTypeBool | RyanJsonValueBoolTrueFlag, key);
        if (NULL == *out)
            return RyanJsonFalse;

        parseBufAddOffset(buf, 4);
        return RyanJsonTrue;
    }

    return RyanJsonFalse;
}

/**
 * @brief 反序列化数字
 *
 * @param pJson
 * @param buf
 * @return RyanJsonBool
 */
static RyanJsonBool RyanJsonPrintNumber(RyanJson_t pJson, printBuffer *buf)
{
    double f = 0;
    int32_t len = 0;

    // jsonNumber 类型是一个整数
    if (pJson->info & RyanJsonValueNumberIntFlag)
    {
        // if (!printBufAppend(buf,buf, 11))     // 32 位整数最多包含 10 个数字字符、符号
        if (!printBufAppend(buf, 20)) // 64 位整数最多包含 19 个数字字符、符号
            return RyanJsonFalse;

        len = sprintf(printBufEnd(buf), "%d", RyanJsonGetIntValue(pJson));
        buf->end += len;
    }
    else // jsonNumber 的类型是浮点型
    {
        // if (!printBufAppend(buf,25))
        if (!printBufAppend(buf, 64))
            return RyanJsonFalse;

        f = RyanJsonGetDoubleValue(pJson);

        if (fabs(floor(f) - f) <= DBL_EPSILON && fabs(f) < 1.0e60)
            len = sprintf(printBufEnd(buf), "%.1lf", f);

        else if (fabs(f) < 1.0e-6 || fabs(f) > 1.0e9)
            len = sprintf(printBufEnd(buf), "%e", f);
        else
        {
            len = sprintf(printBufEnd(buf), "%lf", f);

            while (len > 0 && printBufEnd(buf)[len - 1] == '0' && printBufEnd(buf)[len - 2] != '.') // 删除小数部分中无效的 0
                len--;
        }
        buf->end += len;
    }

    return RyanJsonTrue;
}

/**
 * @brief 反序列化字符串
 *
 * @param str
 * @param buf
 * @return RyanJsonBool
 */
static RyanJsonBool print_string_buffer(const char *str, printBuffer *buf)
{
    const char *p = NULL;
    int32_t len = 0;
    RyanJsonBool escape = RyanJsonFalse;

    if (NULL == str)
    {
        if (!printBufAppend(buf, 2))
            return RyanJsonFalse;
        printBufPutc(buf, '\"');
        printBufPutc(buf, '\"');
        return RyanJsonTrue;
    }

    // 获取长度
    p = str;
    while (*p)
    {
        len++;
        switch (*p)
        {
        case '\"':
        case '\\':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '/':
            escape = RyanJsonTrue;
            len++;
            break;

        default:
            if (*p < 32)
            {
                escape = RyanJsonTrue;
                len += 5; // utf
            }
            break;
        }
        p++;
    }

    if (!printBufAppend(buf, len + 2))
        return RyanJsonFalse; // 最小是\" \"

    printBufPutc(buf, '\"');

    // 没有转义字符
    if (RyanJsonFalse == escape)
    {
        printBufPuts(buf, str, len);
        printBufPutc(buf, '\"');
        return RyanJsonTrue;
    }

    p = str;
    while (*p)
    {
        if ((*p) >= ' ' && *p != '\"' && *p != '\\')
        {
            printBufPutc(buf, *p++);
            continue;
        }

        // 转义和打印
        printBufPutc(buf, '\\');
        switch (*p)
        {
        case '\\':
        case '\"':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '/':
            printBufPutc(buf, *p);
            break;

        default:
            sprintf(printBufEnd(buf), "u%04x", (uint8_t)(*p));
            buf->end += 5; // utf
            break;
        }
        p++;
    }

    printBufPutc(buf, '\"');

    return RyanJsonTrue;
}

static inline RyanJsonBool RyanJsonPrintString(RyanJson_t pJson, printBuffer *buf)
{
    return print_string_buffer(RyanJsonGetStringValue(pJson), buf);
}

/**
 * @brief 反序列化数组
 *
 * @param pJson
 * @param buf
 * @param depth
 * @param format
 * @return RyanJsonBool
 */
static RyanJsonBool RyanJsonPrintArray(RyanJson_t pJson, printBuffer *buf, uint32_t depth, RyanJsonBool format)
{
    int32_t count = 0;
    RyanJson_t child = RyanJsonGetObjectValue(pJson);

    if (NULL == child)
    {
        if (!printBufAppend(buf, 2))
            return RyanJsonFalse;
        printBufPutc(buf, '[');
        printBufPutc(buf, ']');
        return RyanJsonTrue;
    }

    if (format)
    {
        while (child) // 检查子级中是否有数组或对象
        {
            if ((_checkType(child->info, RyanJsonTypeArray) || _checkType(child->info, RyanJsonTypeObject)) &&
                RyanJsonGetObjectValue(child))
            {
                count++;
                break;
            }
            child = child->next;
        }
    }

    if (!printBufAppend(buf, (format && count) ? 2 : 1))
        return RyanJsonFalse;

    printBufPutc(buf, '[');
    if (format && count)
        printBufPutc(buf, '\n');

    child = RyanJsonGetObjectValue(pJson);
    while (child)
    {
        // 打印起始缩进
        if (format && count)
        {
            if (!printBufAppend(buf, depth + 1))
                return RyanJsonFalse;
            for (uint32_t i = 0; i < depth + 1; i++)
                printBufPutc(buf, '\t');
        }

        if (RyanJsonFalse == RyanJsonPrintValue(child, buf, depth + 1, format))
            return RyanJsonFalse;

        // 打印分隔符 ','
        if (child->next)
        {
            if (!printBufAppend(buf, format ? 2 : 1))
                return RyanJsonFalse;
            printBufPutc(buf, ',');
            if (format)
            {
                if (count)
                    printBufPutc(buf, '\n');
                else
                    printBufPutc(buf, ' ');
            }
        }

        child = child->next;
    }

    // 打印结束缩进
    if (!printBufAppend(buf, (format && count) ? depth + 2 : 1))
        return RyanJsonFalse;

    if (format && count)
    {
        printBufPutc(buf, '\n');
        for (uint32_t i = 0; i < depth; i++)
            printBufPutc(buf, '\t');
    }
    printBufPutc(buf, ']');

    return RyanJsonTrue;
}

/**
 * @brief 反序列化对象
 *
 * @param pJson
 * @param buf
 * @param depth
 * @param format
 * @return RyanJsonBool
 */
static RyanJsonBool RyanJsonPrintObject(RyanJson_t pJson, printBuffer *buf, uint32_t depth, RyanJsonBool format)
{
    RyanJson_t child = RyanJsonGetObjectValue(pJson);

    if (NULL == child)
    {
        if (!printBufAppend(buf, 2))
            return RyanJsonFalse;
        printBufPutc(buf, '{');
        printBufPutc(buf, '}');
        return RyanJsonTrue;
    }

    if (!printBufAppend(buf, format ? 2 : 1))
        return RyanJsonFalse;
    printBufPutc(buf, '{');
    if (format)
        printBufPutc(buf, '\n');

    while (child)
    {
        // 打印起始缩进
        if (format)
        {
            if (!printBufAppend(buf, depth + 1))
                return RyanJsonFalse;
            for (uint32_t i = 0; i < depth + 1; i++)
                printBufPutc(buf, '\t');
        }

        if (RyanJsonFalse == print_string_buffer(RyanJsonGetKey(child), buf))
            return RyanJsonFalse;

        // 打印指示符 ':'
        if (!printBufAppend(buf, format ? 2 : 1))
            return RyanJsonFalse;
        printBufPutc(buf, ':');
        if (format)
            printBufPutc(buf, '\t');

        if (RyanJsonFalse == RyanJsonPrintValue(child, buf, depth + 1, format))
            return RyanJsonFalse;

        // 打印分隔符 ','
        if (!printBufAppend(buf, (child->next ? 1 : 0) + (format ? 1 : 0)))
            return RyanJsonFalse;
        if (child->next)
            printBufPutc(buf, ',');
        if (format)
            printBufPutc(buf, '\n');

        child = child->next;
    }

    // 打印结束缩进
    if (!printBufAppend(buf, format ? depth + 1 : 1))
        return RyanJsonFalse;
    if (format)
    {
        for (uint32_t i = 0; i < depth; i++)
            printBufPutc(buf, '\t');
    }
    printBufPutc(buf, '}');

    return RyanJsonTrue;
}

static RyanJsonBool RyanJsonPrintValue(RyanJson_t pJson, printBuffer *buf, uint32_t depth, RyanJsonBool format)
{
    if (NULL == pJson)
        return RyanJsonFalse;

    switch (RyanJsonGetType(pJson))
    {
    case RyanJsonTypeUnknow:
    case RyanJsonTypeNull:
    {
        if (RyanJsonFalse == printBufAppend(buf, 4))
            return RyanJsonFalse;

        printBufPuts(buf, "null", 4);
        return RyanJsonTrue;
    }
    case RyanJsonTypeBool:
    {
        if (pJson->info & RyanJsonValueBoolTrueFlag)
        {
            if (RyanJsonFalse == printBufAppend(buf, 4))
                return RyanJsonFalse;

            printBufPuts(buf, "true", 4);
        }
        else
        {
            if (RyanJsonFalse == printBufAppend(buf, 5))
                return RyanJsonFalse;

            printBufPuts(buf, "false", 5);
        }
        return RyanJsonTrue;
    }
    case RyanJsonTypeNumber:
        return RyanJsonPrintNumber(pJson, buf);
    case RyanJsonTypeString:
        return RyanJsonPrintString(pJson, buf);
    case RyanJsonTypeArray:
        return RyanJsonPrintArray(pJson, buf, depth, format);
    case RyanJsonTypeObject:
        return RyanJsonPrintObject(pJson, buf, depth, format);
    }

    return RyanJsonTrue;
}
/**
 * @brief pJson 文本解析器
 *
 * @param text 文本地址
 * @param require_null_terminated 是否允许解析的文本后面有无意义的字符
 * @param return_parse_end 输出解析终止的字符位置
 * @return RyanJson_t
 */
RyanJson_t RyanJsonParseOptions(const char *text, uint32_t size, RyanJsonBool require_null_terminated, const char **return_parse_end)
{
    RyanJson_t pJson = NULL;
    parseBuffer buf = {0};

    if (NULL == text)
        return NULL;

    buf.address = (char *)text;
    buf.size = size;

    parseSkipInvalidChar(&buf, { return NULL; });
    if (RyanJsonFalse == RyanJsonParseValue(&buf, NULL, &pJson))
        return NULL;

    // 检查解析后的文本后面是否有无意义的字符
    if (require_null_terminated)
    {
        parseSkipInvalidChar(&buf, {});
        if (*buf.address)
        {
            RyanJsonDelete(pJson);
            return NULL;
        }
    }

    if (return_parse_end)
        *return_parse_end = buf.address;

    return pJson;

__exit:
    if (return_parse_end)
        *return_parse_end = buf.address;
    return NULL;
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
char *RyanJsonPrint(RyanJson_t pJson, uint32_t preset, RyanJsonBool format, uint32_t *len)
{
    printBuffer buf = {0};

    if (NULL == pJson)
        return NULL;

    if (preset < 24)
        preset = 24;

    buf.address = (char *)jsonMalloc(preset);
    if (NULL == buf.address)
        return NULL;
    buf.noalloc = RyanJsonFalse;
    buf.size = preset;
    buf.end = 0;

    if (RyanJsonFalse == RyanJsonPrintValue(pJson, &buf, 0, format))
    {
        free(buf.address);
        return NULL;
    }

    buf.address[buf.end] = '\0';
    if (len)
        *len = buf.end;

    return buf.address;
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
char *RyanJsonPrintPreallocated(RyanJson_t pJson, char *buffer, uint32_t length, RyanJsonBool format, uint32_t *len)
{

    printBuffer buf = {0};

    if (NULL == pJson || NULL == buffer || length <= 0)
        return NULL;

    buf.address = (char *)buffer;
    if (NULL == buf.address)
        return NULL;
    buf.noalloc = RyanJsonTrue;
    buf.size = length;
    buf.end = 0;

    if (RyanJsonFalse == RyanJsonPrintValue(pJson, &buf, 0, format))
    {
        free(buf.address);
        return NULL;
    }

    buf.address[buf.end] = '\0';
    if (len)
        *len = buf.end;

    return buf.address;
}

/**
 * @brief 获取 json 的子项个数
 *
 * @param pJson
 * @return uint32_t
 */
uint32_t RyanJsonGetSize(RyanJson_t pJson)
{
    RyanJson_t nextItem = NULL;
    uint32_t i = 0;

    if (NULL == pJson)
        return RyanJsonFalse;

    if (!_checkType(pJson->info, RyanJsonTypeArray) && !_checkType(pJson->info, RyanJsonTypeObject))
        return 1;

    nextItem = RyanJsonGetObjectValue(pJson);

    while (nextItem)
    {
        i++;
        nextItem = nextItem->next;
    }

    return i;
}

/**
 * @brief 重新修改字符数据
 *
 * @param dst
 * @param src
 * @return RyanJsonBool
 */
RyanJsonBool RyanJsonReapplyString(char **dst, const char *src)
{
    char *k = NULL;

    if (NULL == dst || NULL == src)
        return RyanJsonFalse;

    // if (0 == strcmp(*dst, src))
    //     return RyanJsonTrue;

    k = RyanJsonStrdup(src);
    if (NULL == k)
        return RyanJsonFalse;

    jsonFree(*dst);
    *dst = k;
    return RyanJsonTrue;
}

/**
 * @brief 通过 索引 获取json对象的子项
 *
 * @param pJson
 * @param index
 * @return RyanJson_t
 */
RyanJson_t RyanJsonGetObjectByIndex(RyanJson_t pJson, int32_t index)
{
    RyanJson_t nextItem = NULL;

    if (NULL == pJson || index < 0)
        return NULL;

    if (!_checkType(pJson->info, RyanJsonTypeArray) && !_checkType(pJson->info, RyanJsonTypeObject))
        return NULL;

    nextItem = RyanJsonGetObjectValue(pJson);

    while (nextItem && index > 0)
    {
        index--;
        nextItem = nextItem->next;
    }

    return nextItem;
}

/**
 * @brief 连续通过 索引 获取json对象的子项
 *
 * @param pJson
 * @param index
 * @param ... 可变参，连续输入索引，直到INT_MIN结束
 * @return RyanJson_t
 */
RyanJson_t RyanJsonGetObjectByIndexs(RyanJson_t pJson, int32_t index, ...)
{
    RyanJson_t nextItem = NULL;
    va_list args = {0};
    int32_t i = index;

    if (NULL == pJson || index < 0)
        return NULL;

    nextItem = RyanJsonGetObjectByIndex(pJson, i);
    if (NULL == nextItem)
        return NULL;

    va_start(args, index);
    i = va_arg(args, int32_t);
    while (nextItem && INT_MIN != i)
    {
        nextItem = RyanJsonGetObjectByIndex(nextItem, i);
        i = va_arg(args, int32_t);
    }

    va_end(args);

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
    RyanJson_t nextItem = NULL;
    if (NULL == pJson || NULL == key)
        return NULL;

    if (!_checkType(pJson->info, RyanJsonTypeObject))
        return NULL;

    nextItem = RyanJsonGetObjectValue(pJson);
    if (RyanJsonFalse == RyanJsonIsKey(nextItem))
        return NULL;

    while (nextItem && strcmp(RyanJsonGetKey(nextItem), key))
        nextItem = nextItem->next;

    return nextItem;
}

/**
 * @brief 连续通过 key 获取json对象的子项
 *
 * @param pJson
 * @param key
 * @param ... 可变参，连续输入key，直到NULL结束
 * @return RyanJson_t
 */
RyanJson_t RyanJsonGetObjectByKeys(RyanJson_t pJson, char *key, ...)
{
    RyanJson_t nextItem = NULL;
    va_list args = {0};
    char *s = key;

    if (NULL == pJson || NULL == key)
        return NULL;

    nextItem = RyanJsonGetObjectByKey(pJson, s);
    if (NULL == nextItem)
        return NULL;

    va_start(args, key);
    s = va_arg(args, char *);
    while (nextItem && NULL != s)
    {
        nextItem = RyanJsonGetObjectByKey(nextItem, s);
        s = va_arg(args, char *);
    }

    va_end(args);

    return nextItem;
}

/**
 * @brief 通过 索引 分离json对象的子项
 *
 * @param pJson
 * @param index
 * @return RyanJson_t 被分离对象的指针
 */
RyanJson_t RyanJsonDetachByIndex(RyanJson_t pJson, int32_t index)
{
    RyanJson_t nextItem = NULL;
    RyanJson_t prev = NULL;

    if (NULL == pJson || index < 0)
        return NULL;

    if (!_checkType(pJson->info, RyanJsonTypeArray) && !_checkType(pJson->info, RyanJsonTypeObject))
        return NULL;

    nextItem = RyanJsonGetObjectValue(pJson);

    while (nextItem && index > 0)
    {
        prev = nextItem;
        nextItem = nextItem->next;
        index--;
    }

    if (NULL == nextItem)
        return NULL;

    if (NULL != prev)
        prev->next = nextItem->next;

    if (nextItem == RyanJsonGetObjectValue(pJson))
        RyanJsonGetObjectValue(pJson) = nextItem->next;

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
    RyanJson_t nextItem = NULL;
    RyanJson_t prev = NULL;

    if (NULL == pJson || NULL == key)
        return NULL;

    if (!_checkType(pJson->info, RyanJsonTypeObject))
        return NULL;

    nextItem = RyanJsonGetObjectValue(pJson);
    if (RyanJsonFalse == RyanJsonIsKey(nextItem))
        return NULL;

    while (nextItem && strcmp(RyanJsonGetKey(nextItem), key))
    {
        prev = nextItem;
        nextItem = nextItem->next;
    }

    if (NULL == nextItem)
        return NULL;

    if (NULL != prev)
        prev->next = nextItem->next;

    if (nextItem == RyanJsonGetObjectValue(pJson))
        RyanJsonGetObjectValue(pJson) = nextItem->next;

    nextItem->next = NULL;

    return nextItem;
}

/**
 * @brief 通过 索引 删除json对象的子项
 *
 * @param pJson
 * @param index
 * @return RyanJsonBool
 */
RyanJsonBool RyanJsonDeleteByIndex(RyanJson_t pJson, int32_t index)
{
    RyanJson_t nextItem = NULL;

    if (NULL == pJson || index < 0)
        return RyanJsonFalse;

    nextItem = RyanJsonDetachByIndex(pJson, index);

    if (NULL == nextItem)
        return RyanJsonFalse;

    RyanJsonDelete(nextItem);

    return RyanJsonTrue;
}

/**
 * @brief 通过 key 删除json对象的子项
 *
 * @param pJson
 * @param key
 * @return RyanJsonBool
 */
RyanJsonBool RyanJsonDeleteByKey(RyanJson_t pJson, const char *key)
{
    RyanJson_t nextItem = NULL;

    if (NULL == pJson || NULL == key)
        return RyanJsonFalse;

    nextItem = RyanJsonDetachByKey(pJson, key);

    if (NULL == nextItem)
        return RyanJsonTrue;

    RyanJsonDelete(nextItem);

    return RyanJsonTrue;
}

/**
 * @brief 按 索引 插入json对象
 *
 * @param pJson
 * @param index
 * @param item
 * @return RyanJsonBool
 */
RyanJsonBool RyanJsonInsert(RyanJson_t pJson, int32_t index, RyanJson_t item)
{
    RyanJson_t nextItem = NULL;
    RyanJson_t prev = NULL;

    if (NULL == pJson || index < 0 || NULL == item)
        return RyanJsonFalse;

    if (!(_checkType(pJson->info, RyanJsonTypeArray)) &&
        !(_checkType(pJson->info, RyanJsonTypeObject) && (item->info & RyanJsonWithKeyFlag)))
    {
        // printf("__error 不是正确类型");
        return RyanJsonFalse;
    }

    nextItem = RyanJsonGetObjectValue(pJson);

    while (nextItem && index > 0)
    {
        prev = nextItem;
        nextItem = nextItem->next;
        index--;
    }

    if (NULL == nextItem && NULL == prev)
    {
        RyanJsonGetObjectValue(pJson) = item;
        return RyanJsonTrue;
    }

    if (NULL != prev)
        prev->next = item;
    item->next = nextItem;

    if (nextItem == RyanJsonGetObjectValue(pJson))
        RyanJsonGetObjectValue(pJson) = item;

    return RyanJsonTrue;
}

/**
 * @brief 通过 索引 替换json对象的子项
 *
 * @param pJson
 * @param index
 * @param item
 * @return RyanJsonBool
 */
RyanJsonBool RyanJsonReplaceByIndex(RyanJson_t pJson, int32_t index, RyanJson_t item)
{
    RyanJson_t nextItem = NULL;
    RyanJson_t prev = NULL;

    if (NULL == pJson || index < 0 || NULL == item)
        return RyanJsonFalse;

    if (!_checkType(pJson->info, RyanJsonTypeArray) && !_checkType(pJson->info, RyanJsonTypeObject))
        return RyanJsonFalse;

    nextItem = RyanJsonGetObjectValue(pJson);

    // 查找子项
    while (nextItem && index > 0)
    {
        prev = nextItem;
        nextItem = nextItem->next;
        index--;
    }

    if (NULL == nextItem)
        return RyanJsonFalse;

    if (NULL != prev)
        prev->next = item;

    item->next = nextItem->next;

    if (nextItem == RyanJsonGetObjectValue(pJson))
        RyanJsonGetObjectValue(pJson) = item;

    nextItem->next = NULL;
    RyanJsonDelete(nextItem);

    return RyanJsonTrue;
}

/**
 * @brief 通过 key 替换json对象的子项
 *
 * @param pJson
 * @param key
 * @param item
 * @return RyanJsonBool
 */
RyanJsonBool RyanJsonReplaceByKey(RyanJson_t pJson, const char *key, RyanJson_t item)
{
    RyanJson_t nextItem = NULL;
    RyanJson_t prev = NULL;

    if (NULL == pJson || NULL == key || NULL == item)
        return RyanJsonFalse;

    if (!_checkType(pJson->info, RyanJsonTypeObject))
        return RyanJsonFalse;

    nextItem = RyanJsonGetObjectValue(pJson);

    // 找到要修改的节点
    while (nextItem && strcmp(RyanJsonGetKey(nextItem), key))
    {
        prev = nextItem;
        nextItem = nextItem->next;
    }

    // 没找到要修改的节点。
    if (NULL == nextItem)
        return RyanJsonFalse;

    // 没有key的对象 申请一个带key的对象
    if (RyanJsonFalse == RyanJsonIsKey(item))
        item = RyanJsonCreateItem(key, item);
    else if (0 != strcmp(RyanJsonGetKey(item), key))
    {
        // 修改key
        jsonFree(RyanJsonGetKey(item));
        RyanJsonGetKey(item) = RyanJsonStrdup(key);
        if (NULL == RyanJsonGetKey(item))
            return RyanJsonFalse;
    }

    // 关联节点
    if (NULL != prev)
        prev->next = item;

    item->next = nextItem->next;

    if (nextItem == RyanJsonGetObjectValue(pJson))
        RyanJsonGetObjectValue(pJson) = item;

    nextItem->next = NULL;
    RyanJsonDelete(nextItem);
    return RyanJsonTrue;
}

/**
 * @brief 创建一个 NULL 类型的json对象
 *
 * @param key
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateNull(char *key)
{
    RyanJson_t item = NULL;
    char *k = NULL;

    if (NULL != key)
    {
        k = RyanJsonStrdup(key);
        if (NULL == k)
            return NULL;
    }

    item = RyanJsonNewNode(RyanJsonTypeNull, k);
    if (NULL == item)
    {
        jsonFree(k);
        return NULL;
    }

    return item;
}

/**
 * @brief 创建一个 boolean 类型的json对象
 *
 * @param key
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateBool(char *key, RyanJsonBool boolean)
{
    RyanJson_t item = NULL;
    int32_t info = RyanJsonTypeBool;
    char *k = NULL;

    if (NULL != key)
    {
        k = RyanJsonStrdup(key);
        if (NULL == k)
            return NULL;
    }

    if (RyanJsonTrue == boolean)
        info |= RyanJsonValueBoolTrueFlag;

    item = RyanJsonNewNode(info, k);
    if (NULL == item)
    {
        jsonFree(k);
        return NULL;
    }

    return item;
}

/**
 * @brief 创建一个 number 类型中的 int32_t 类型json对象
 *
 * @param key
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateInt(char *key, int32_t number)
{
    RyanJson_t item = NULL;
    char *k = NULL;

    if (NULL != key)
    {
        k = RyanJsonStrdup(key);
        if (NULL == k)
            return NULL;
    }

    item = RyanJsonNewNode(RyanJsonTypeNumber | RyanJsonValueNumberIntFlag, k);

    if (NULL == item)
    {
        jsonFree(k);
        return NULL;
    }
    RyanJsonGetIntValue(item) = number;

    return item;
}

/**
 * @brief 创建一个 number 类型中的 double 类型json对象
 * 可以使用double来保存int64类型数据,但是更推荐通过字符串保存
 * @param key
 * @param number
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateDouble(char *key, double number)
{
    RyanJson_t item = NULL;
    char *k = NULL;

    if (NULL != key)
    {
        k = RyanJsonStrdup(key);
        if (NULL == k)
            return NULL;
    }

    item = RyanJsonNewNode(RyanJsonTypeNumber, k);
    if (NULL == item)
    {
        jsonFree(k);
        return NULL;
    }

    RyanJsonGetDoubleValue(item) = number;

    return item;
}

/**
 * @brief 创建一个 string 类型的json对象
 *
 * @param key
 * @param string
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateString(char *key, const char *string)
{
    RyanJson_t item = NULL;
    char *k = NULL;
    char *s = NULL;

    if (NULL != key)
    {
        k = RyanJsonStrdup(key);
        if (NULL == k)
            return NULL;
    }

    item = RyanJsonNewNode(RyanJsonTypeString, k);
    if (NULL == item)
    {
        jsonFree(k);
        return NULL;
    }

    s = RyanJsonStrdup(string);
    if (NULL == s)
    {
        jsonFree(k);
        jsonFree(item);
        return NULL;
    }

    RyanJsonGetStringValue(item) = s;

    return item;
}

/**
 * @brief 创建一个 obj 类型的json对象
 *
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateObject()
{
    RyanJson_t item = NULL;
    item = RyanJsonNewNode(RyanJsonTypeObject, NULL);
    if (NULL == item)
        return NULL;

    return item;
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
RyanJson_t RyanJsonCreateItem(const char *key, RyanJson_t item)
{
    RyanJson_t newItem = NULL;
    char *k = NULL;

    if (NULL == item)
        return NULL;

    if (NULL != key)
    {
        k = RyanJsonStrdup(key);
        if (NULL == k)
            return RyanJsonFalse;
    }

    newItem = RyanJsonNewNode(_checkType(item->info, RyanJsonTypeArray) ? RyanJsonTypeArray : RyanJsonTypeObject, k);
    if (NULL == newItem)
    {
        jsonFree(k);
        return NULL;
    }

    if (_checkType(item->info, RyanJsonTypeArray) || _checkType(item->info, RyanJsonTypeObject))
    {
        RyanJsonGetObjectValue(newItem) = RyanJsonGetObjectValue(item);

        if (RyanJsonIsKey(item))
            jsonFree(RyanJsonGetKey(item));

        jsonFree(item);
    }
    else
    {
        RyanJsonGetObjectValue(newItem) = item;
    }

    return newItem;
}

/**
 * @brief 创建一个 arr 类型的json对象
 *
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateArray()
{
    return RyanJsonNewNode(RyanJsonTypeArray, NULL);
}

/**
 * @brief 创建一个int类型的数组json对象
 *
 * @param numbers 数组的地址必须为int类型
 * @param count 数组的长度
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateIntArray(const int32_t *numbers, int32_t count)
{
    RyanJson_t n = NULL, p = NULL, a = NULL;

    if (NULL == numbers || count < 0)
        return NULL;

    a = RyanJsonCreateArray();

    for (int32_t i = 0; a && i < count; i++)
    {
        n = RyanJsonCreateInt(NULL, numbers[i]);
        if (0 == i)
            RyanJsonGetObjectValue(a) = n;
        else
            p->next = n;

        p = n;
    }

    return a;
}

/**
 * @brief 创建一个double类型的数组json对象
 *
 * @param numbers
 * @param count
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateDoubleArray(const double *numbers, int32_t count)
{
    RyanJson_t n = NULL, p = NULL, a = NULL;

    if (NULL == numbers || count < 0)
        return NULL;

    a = RyanJsonCreateArray();

    for (int32_t i = 0; a && i < count; i++)
    {
        n = RyanJsonCreateDouble(NULL, numbers[i]);
        if (0 == i)
            RyanJsonGetObjectValue(a) = n;
        else
            p->next = n;
        p = n;
    }

    return a;
}

/**
 * @brief 创建一个string类型的数组json对象
 *
 * @param strings
 * @param count
 * @return RyanJson_t
 */
RyanJson_t RyanJsonCreateStringArray(const char **strings, int32_t count)
{
    RyanJson_t n = NULL, p = NULL, a = NULL;

    if (NULL == strings || count < 0)
        return NULL;

    a = RyanJsonCreateArray();

    for (int32_t i = 0; a && i < count; i++)
    {
        n = RyanJsonCreateString(NULL, strings[i]);
        if (0 == i)
            RyanJsonGetObjectValue(a) = n;
        else
            p->next = n;
        p = n;
    }

    return a;
}

/**
 * @brief 深拷贝一份json对象
 *
 * @param pJson
 * @return RyanJson_t 拷贝的新对象指针
 */
RyanJson_t RyanJsonDuplicate(RyanJson_t pJson)
{

    RyanJson_t newitem = NULL, temp = NULL, item = NULL, prev = NULL;
    char *key = NULL;

    if (NULL == pJson)
        return NULL;

    // copy key
    if (_checkType(pJson->info, RyanJsonWithKeyFlag))
    {
        key = RyanJsonStrdup(RyanJsonGetKey(pJson));
        if (NULL == key)
            return NULL;
    }

    // 创建新的 Json
    newitem = RyanJsonNewNode(pJson->info, key);
    if (NULL == newitem)
    {
        jsonFree(key);
        return NULL;
    }

    switch ((char)pJson->info)
    {
    case RyanJsonTypeUnknow:
    case RyanJsonTypeNull:
        break;

    // 创建NewNode的时候已经赋值了
    case RyanJsonTypeBool:
        break;

    case RyanJsonTypeNumber:
        if (RyanJsonIsInt(pJson))
            RyanJsonChangeIntValue(newitem, RyanJsonGetIntValue(pJson));
        else
            RyanJsonChangeDoubleValue(newitem, RyanJsonGetDoubleValue(pJson));
        break;

    case RyanJsonTypeString:
        if (RyanJsonTrue != RyanJsonChangeStringValue(newitem, RyanJsonGetStringValue(pJson)))
        {
            RyanJsonDelete(newitem);
            return NULL;
        }
        break;

    case RyanJsonTypeArray:
    case RyanJsonTypeObject:
        temp = RyanJsonGetObjectValue(pJson);
        while (temp)
        {
            item = RyanJsonDuplicate(temp);
            if (NULL == item)
            {
                RyanJsonDelete(newitem);
                return NULL;
            }

            if (NULL != prev)
            {
                prev->next = item;
                prev = item;
            }
            else
            {
                RyanJsonGetObjectValue(newitem) = item;
                prev = item;
            }

            temp = temp->next;
        }

        break;

    default:
        RyanJsonDelete(newitem);
        return NULL;
    }

    return newitem;
}

/**
 * @brief 通过删除无效字符、注释等， 减少json文本大小
 *
 * @param text 文本指针
 */
void RyanJsonMinify(char *text)
{
    char *t = text;

    while (*text)
    {
        if (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') // 空格字符
            text++;
        else if (*text == '/' && text[1] == '/') // 双斜杠注释，到行尾
        {
            while (*text && *text != '\n')
                text++;
        }
        else if (*text == '/' && text[1] == '*') // 多行注释
        {
            while (*text && !(*text == '*' && text[1] == '/'))
                text++;

            text += 2;
        }
        else if (*text == '\"') // 字符串文本
        {
            *t++ = *text++;
            while (*text && *text != '\"')
            {
                if (*text == '\\')
                    *t++ = *text++;

                *t++ = *text++;
            }
            *t++ = *text++;
        }
        else // 所有其他字符
            *t++ = *text++;
    }

    *t = '\0';
}

/**
 * @brief 递归比较两个 pJson 对象key和value是否相等。
 * 此接口效率较低, 谨慎使用
 * @param a
 * @param b
 * @return RyanJsonBool
 */
RyanJsonBool RyanJsonCompare(RyanJson_t a, RyanJson_t b)
{

    if (NULL == a || NULL == b)
        return RyanJsonFalse;

    // 相同的对象相等
    if (a == b)
        return RyanJsonTrue;

    if (_checkType(a->info, RyanJsonTypeUnknow) || _checkType(b->info, RyanJsonTypeUnknow))
        return RyanJsonFalse;

    if (RyanJsonGetType(a) != RyanJsonGetType(b))
        return RyanJsonFalse;

    switch (RyanJsonGetType(a))
    {

    case RyanJsonTypeNull:
        return RyanJsonTrue;

    case RyanJsonTypeBool:
        return RyanJsonGetBoolValue(a) == RyanJsonGetBoolValue(b) ? RyanJsonTrue : RyanJsonFalse;

    case RyanJsonTypeNumber:
    {
        if (RyanJsonIsInt(a) == RyanJsonIsInt(b))
            return RyanJsonGetIntValue(a) == RyanJsonGetIntValue(b) ? RyanJsonTrue : RyanJsonFalse;

        else if (RyanJsonIsDouble(a) == RyanJsonIsDouble(b))
            return compare_double(RyanJsonGetDoubleValue(a), RyanJsonGetDoubleValue(b));

        return RyanJsonFalse;
    }

    case RyanJsonTypeString:
        return 0 == strcmp(RyanJsonGetStringValue(a), RyanJsonGetStringValue(b)) ? RyanJsonTrue : RyanJsonFalse;

    case RyanJsonTypeArray:
    {
        if (RyanJsonGetSize(a) != RyanJsonGetSize(b))
            return RyanJsonFalse;

        for (uint32_t count = 0; count < RyanJsonGetSize(a); count++)
        {
            if (RyanJsonTrue != RyanJsonCompare(RyanJsonGetObjectToIndex(a, count), RyanJsonGetObjectToIndex(b, count)))
                return RyanJsonFalse;
        }
        return RyanJsonTrue;
    }

    case RyanJsonTypeObject:
    {
        if (RyanJsonGetSize(a) != RyanJsonGetSize(b))
            return RyanJsonFalse;

        RyanJson_t a_element = NULL;
        RyanJson_t b_element = NULL;

        RyanJsonObjectForEach(a, a_element)
        {
            // TODO This has O(n^2) runtime, which is horrible!
            b_element = RyanJsonGetObjectByKey(b, RyanJsonGetKey(a_element));
            if (NULL == b_element)
                return RyanJsonFalse;

            if (RyanJsonTrue != RyanJsonCompare(a_element, b_element))
                return RyanJsonFalse;
        }

        RyanJsonObjectForEach(b, b_element)
        {
            // TODO This has O(n^2) runtime, which is horrible!
            a_element = RyanJsonGetObjectByKey(a, RyanJsonGetKey(b_element));
            if (NULL == a_element)
                return RyanJsonFalse;

            if (RyanJsonTrue != RyanJsonCompare(b_element, a_element))
                return RyanJsonFalse;
        }

        return RyanJsonTrue;
    }

    case RyanJsonTypeUnknow:
    default:
        return RyanJsonFalse;
    }

    return RyanJsonTrue;
}

/**
 * @brief 递归比较两个 pJson 对象key是否相等。
 * 此接口效率较低, 谨慎使用
 * @param a
 * @param b
 * @return RyanJsonBool
 */
RyanJsonBool RyanJsonCompareOnlyKey(RyanJson_t a, RyanJson_t b)
{
    if (NULL == a || NULL == b)
        return RyanJsonFalse;

    // 相同的对象相等
    if (a == b)
        return RyanJsonTrue;

    if (_checkType(a->info, RyanJsonTypeUnknow) || _checkType(b->info, RyanJsonTypeUnknow))
        return RyanJsonFalse;

    if (RyanJsonGetType(a) != RyanJsonGetType(b))
        return RyanJsonFalse;

    switch (RyanJsonGetType(a))
    {
    case RyanJsonTypeBool:
    case RyanJsonTypeNull:
    case RyanJsonTypeNumber:
    case RyanJsonTypeString:
        return RyanJsonTrue;

    case RyanJsonTypeArray:
    {
        if (RyanJsonGetSize(a) != RyanJsonGetSize(b))
            return RyanJsonFalse;

        for (uint32_t count = 0; count < RyanJsonGetSize(a); count++)
        {
            if (RyanJsonTrue != RyanJsonCompareOnlyKey(RyanJsonGetObjectToIndex(a, count), RyanJsonGetObjectToIndex(b, count)))
                return RyanJsonFalse;
        }
        return RyanJsonTrue;
    }

    case RyanJsonTypeObject:
    {
        RyanJson_t a_element = NULL;
        RyanJson_t b_element = NULL;

        if (RyanJsonGetSize(a) != RyanJsonGetSize(b))
            return RyanJsonFalse;

        RyanJsonObjectForEach(a, a_element)
        {
            // TODO This has O(n^2) runtime, which is horrible!
            b_element = RyanJsonGetObjectByKey(b, RyanJsonGetKey(a_element));
            if (NULL == b_element)
                return RyanJsonFalse;

            if (RyanJsonTrue != RyanJsonCompareOnlyKey(a_element, b_element))
                return RyanJsonFalse;
        }

        RyanJsonObjectForEach(b, b_element)
        {
            // TODO This has O(n^2) runtime, which is horrible!
            a_element = RyanJsonGetObjectByKey(a, RyanJsonGetKey(b_element));
            if (NULL == a_element)
                return RyanJsonFalse;

            if (RyanJsonTrue != RyanJsonCompareOnlyKey(b_element, a_element))
                return RyanJsonFalse;
        }
        return RyanJsonTrue;
    }

    case RyanJsonTypeUnknow:
    default:
        return RyanJsonFalse;
    }

    return RyanJsonTrue;
}
