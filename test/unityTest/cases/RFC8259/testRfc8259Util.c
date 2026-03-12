#include "RyanJson.h"
#include "testRfc8259Util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

static int32_t hexval(int32_t c)
{
	if (c >= '0' && c <= '9') { return c - '0'; }
	c = (c >= 'a' && c <= 'f') ? (c - 'a' + 'A') : c;
	if (c >= 'A' && c <= 'F') { return c - 'A' + 10; }
	return -1;
}

static int32_t decode_u4(const char *s, uint16_t *out)
{
	int32_t h0 = hexval(s[0]), h1 = hexval(s[1]), h2 = hexval(s[2]), h3 = hexval(s[3]);
	if (h0 < 0 || h1 < 0 || h2 < 0 || h3 < 0) { return 0; }
	*out = (uint16_t)((h0 << 12) | (h1 << 8) | (h2 << 4) | h3);
	return 1;
}

int32_t RyanJsonNormalizeString(const char *in, uint32_t inLen, unsigned char **out, uint32_t *outLen)
{
	uint32_t cap = inLen * 4 + 8;
	unsigned char *buf = (unsigned char *)malloc(cap);
	if (NULL == buf) { return 0; }
	uint32_t pos = 0;

	for (uint32_t i = 0; i < inLen; i++)
	{
		unsigned char ch = (unsigned char)in[i];
		if (ch == '\\')
		{
			if (i + 1 >= inLen)
			{
				free(buf);
				return 0;
			}
			unsigned char esc = (unsigned char)in[++i];
			switch (esc)
			{
			case '\"': buf[pos++] = '\"'; break;
			case '\\': buf[pos++] = '\\'; break;
			case '/': buf[pos++] = '/'; break;
			case 'b': buf[pos++] = '\b'; break;
			case 'f': buf[pos++] = '\f'; break;
			case 'n': buf[pos++] = '\n'; break;
			case 'r': buf[pos++] = '\r'; break;
			case 't': buf[pos++] = '\t'; break;
			case 'u': {
				if (i + 4 >= inLen)
				{
					free(buf);
					return 0;
				}
				uint16_t u1;
				if (!decode_u4(&in[i + 1], &u1))
				{
					free(buf);
					return 0;
				}
				i += 4;
				if (u1 >= 0xD800 && u1 <= 0xDBFF)
				{
					if (i + 2 >= inLen || in[i + 1] != '\\' || in[i + 2] != 'u' || i + 6 >= inLen)
					{
						free(buf);
						return 0;
					}
					i += 2;
					uint16_t u2;
					if (!decode_u4(&in[i + 1], &u2))
					{
						free(buf);
						return 0;
					}
					i += 4;
					if (!(u2 >= 0xDC00 && u2 <= 0xDFFF))
					{
						free(buf);
						return 0;
					}
					uint32_t cp = 0x10000 + (((uint32_t)(u1 - 0xD800) << 10) | (uint32_t)(u2 - 0xDC00));
					buf[pos++] = (unsigned char)(0xF0 | ((cp >> 18) & 0x07));
					buf[pos++] = (unsigned char)(0x80 | ((cp >> 12) & 0x3F));
					buf[pos++] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
					buf[pos++] = (unsigned char)(0x80 | (cp & 0x3F));
				}
				else if (u1 >= 0xDC00 && u1 <= 0xDFFF)
				{
					free(buf);
					return 0;
				}
				else
				{
					if (u1 <= 0x007F) { buf[pos++] = (unsigned char)u1; }
					else if (u1 <= 0x07FF)
					{
						buf[pos++] = (unsigned char)(0xC0 | ((u1 >> 6) & 0x1F));
						buf[pos++] = (unsigned char)(0x80 | (u1 & 0x3F));
					}
					else
					{
						buf[pos++] = (unsigned char)(0xE0 | ((u1 >> 12) & 0x0F));
						buf[pos++] = (unsigned char)(0x80 | ((u1 >> 6) & 0x3F));
						buf[pos++] = (unsigned char)(0x80 | (u1 & 0x3F));
					}
				}
				break;
			}
			default: free(buf); return 0;
			}
		}
		else
		{
			buf[pos++] = ch;
		}

		if (pos + 8 > cap)
		{
			cap *= 2;
			unsigned char *nb = (unsigned char *)realloc(buf, cap);
			if (NULL == nb)
			{
				free(buf);
				return 0;
			}
			buf = nb;
		}
	}
	*out = buf;
	*outLen = pos;
	return 1;
}

static void trim(const char **s, uint32_t *len)
{
	const char *p = *s;
	uint32_t n = *len;
	while (n && isspace((unsigned char)*p))
	{
		p++;
		n--;
	}
	while (n && isspace((unsigned char)p[n - 1]))
	{
		n--;
	}
	*s = p;
	*len = n;
}

static int32_t is_quoted_string(const char *s, uint32_t len)
{
	return len >= 2 && s[0] == '\"' && s[len - 1] == '\"';
}

int32_t RyanJsonScalarSemanticEqual(const char *a, uint32_t aLen, const char *b, uint32_t bLen)
{
	trim(&a, &aLen);
	trim(&b, &bLen);

	if (is_quoted_string(a, aLen) && is_quoted_string(b, bLen))
	{
		unsigned char *na = NULL, *nb = NULL;
		uint32_t nla = 0, nlb = 0;
		if (!RyanJsonNormalizeString(a + 1, aLen - 2, &na, &nla)) { return 0; }
		if (!RyanJsonNormalizeString(b + 1, bLen - 2, &nb, &nlb))
		{
			free(na);
			return 0;
		}
		int32_t eq = (nla == nlb) && (0 == memcmp(na, nb, nla));
		free(na);
		free(nb);
		return eq;
	}

	if (aLen == 4 && 0 == strncmp(a, "true", 4) && bLen == 4 && 0 == strncmp(b, "true", 4)) { return 1; }
	if (aLen == 5 && 0 == strncmp(a, "false", 5) && bLen == 5 && 0 == strncmp(b, "false", 5)) { return 1; }
	if (aLen == 4 && 0 == strncmp(a, "null", 4) && bLen == 4 && 0 == strncmp(b, "null", 4)) { return 1; }

	char *endA = NULL, *endB = NULL;
	char *bufA = (char *)malloc(aLen + 1);
	char *bufB = (char *)malloc(bLen + 1);
	if (NULL != bufA && NULL != bufB)
	{
		memcpy(bufA, a, aLen);
		bufA[aLen] = '\0';
		memcpy(bufB, b, bLen);
		bufB[bLen] = '\0';
		double va = strtod(bufA, &endA);
		double vb = strtod(bufB, &endB);
		int32_t okA = (endA && *endA == '\0');
		int32_t okB = (endB && *endB == '\0');
		if (okA && okB)
		{
			free(bufA);
			free(bufB);

			// 使用相对误差比较，处理极大极小值
			if (RyanJsonCompareDouble(va, vb)) { return 1; }
			return 0;

			// if (va == vb) { return 1; }
			// double absA = fabs(va), absB = fabs(vb);
			// double maxAbs = (absA > absB) ? absA : absB;
			// if (maxAbs < 1e-300) { return 1; } // 两者都接近零
			// return (fabs(va - vb) / maxAbs) < 1e-14;
		}
	}
	free(bufA);
	free(bufB);
	return (aLen == bLen) && (0 == memcmp(a, b, aLen));
}

int32_t RyanJsonExtractSingleArrayElement(const char *s, uint32_t len, const char **elem, uint32_t *elemLen)
{
	trim(&s, &len);
	if (len < 2 || s[0] != '[' || s[len - 1] != ']') { return 0; }
	const char *p = s + 1;
	uint32_t n = len - 2;
	trim(&p, &n);
	if (0 == n) { return 0; }

	int32_t in_str = 0, escape = 0;
	for (uint32_t i = 0; i < n; i++)
	{
		char c = p[i];
		if (in_str)
		{
			if (escape) { escape = 0; }
			else if (c == '\\') { escape = 1; }
			else if (c == '\"') { in_str = 0; }
		}
		else
		{
			if (c == '\"') { in_str = 1; }
			else if (c == ',') { return 0; }
		}
	}
	*elem = p;
	*elemLen = n;
	return 1;
}

int32_t RyanJsonValueSemanticEqual(const char *a, uint32_t aLen, const char *b, uint32_t bLen)
{
	// 尝试将两个字符串解析为 Json 对象进行语义比较
	// 这可以处理对象和数组的递归比较
	RyanJson_t jsonA = RyanJsonParseOptions(a, aLen, RyanJsonFalse, NULL);
	RyanJson_t jsonB = RyanJsonParseOptions(b, bLen, RyanJsonFalse, NULL);

	if (NULL != jsonA && NULL != jsonB)
	{
		// 使用 RyanJsonCompare 进行完整的语义比较
		int32_t result = RyanJsonCompare(jsonA, jsonB);
		(void)RyanJsonDelete(jsonA);
		(void)RyanJsonDelete(jsonB);
		return result;
	}

	// 解析失败时清理并回退到原有逻辑
	if (NULL != jsonA) { (void)RyanJsonDelete(jsonA); }
	if (NULL != jsonB) { (void)RyanJsonDelete(jsonB); }

	// 回退：单元素数组提取比较
	const char *ae = NULL, *be = NULL;
	uint32_t ale = 0, ble = 0;
	if (RyanJsonExtractSingleArrayElement(a, aLen, &ae, &ale) && RyanJsonExtractSingleArrayElement(b, bLen, &be, &ble))
	{
		return RyanJsonScalarSemanticEqual(ae, ale, be, ble);
	}
	return RyanJsonScalarSemanticEqual(a, aLen, b, bLen);
}
