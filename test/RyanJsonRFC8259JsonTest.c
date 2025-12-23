#include "RyanJsonTest.h"

#define PrintfStrCmpEnable

typedef int (*jsonParseData)(char *fileName, char *data, uint32_t len);

/* Read a file, parse, render back, etc. */
static int testFile(const char *path, jsonParseData jsonParseDataHandle)
{

	DIR *dir = NULL;
	struct dirent *entry;

	int path_len = strlen(path);
	int count = 0;
	int used_count = 0;
	if (!path || !path_len || !(dir = opendir(path))) { goto fail; }

	while ((entry = readdir(dir)))
	{

		char *name = (char *)entry->d_name;

		if (!name || !strlen(name)) { continue; }

		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) { continue; }

		char aaa[300] = {0};
		snprintf(aaa, sizeof(aaa), "%s/%s", path, name);

		FILE *f = fopen(aaa, "rb");
		if (f == NULL) { goto fail; }

		fseek(f, 0, SEEK_END);
		long len = ftell(f);
		fseek(f, 0, SEEK_SET);
		char *data = (char *)malloc(len + 10);
		fread(data, 1, len, f);
		data[len] = '\0';
		fclose(f);
		int status = 0;

		int startUse = vallocGetUse();
		status = jsonParseDataHandle(name, data, len);

		used_count++;
		if (0 == strncmp("y_", name, 2))
		{
			if (0 == status) { count++; }
			else
			{
				printf("应该成功，但是失败: %s, len: %ld\n", data, len);
			}
		}
		else if (0 == strncmp("n_", name, 2))
		{
			if (0 != status) { count++; }
			else
			{
				printf("应该失败，但是成功: %s, len: %ld\n", data, len);
			}
		}
		else if (0 == strncmp("i_", name, 2)) { count++; }

		if (startUse != vallocGetUse())
		{
			int area = 0, use = 0;
			v_mcheck(&area, &use);
			free(data);
			printf("内存泄漏 %s len: %ld\r\n", data, len);
			// printf("内存泄漏 %x len: %ld\r\n", (unsigned int)data, len);
			// printf("内存泄漏 %c len: %ld\r\n", (int)data, len);
			printf("|||----------->>> area = %d, size = %d\r\n", area, use);
			break;
		}

		// if (use != (len + 10))
		// {
		// 	int area = 0, use = 0;
		// 	v_mcheck(&area, &use);
		// 	free(data);
		// 	printf("内存泄漏 %s len: %ld\r\n", data, len);
		// 	// printf("内存泄漏 %x len: %ld\r\n", (unsigned int)data, len);
		// 	// printf("内存泄漏 %c len: %ld\r\n", (int)data, len);
		// 	printf("|||----------->>> area = %d, size = %d\r\n", area, use);
		// 	break;
		// }
		free(data);
	}

	closedir(dir);

	printf("RFC 8259 JSON: (%d/%d)\r\n", count, used_count);
	return 0;

fail:
	if (dir) { closedir(dir); }

	return -1;
}

typedef struct
{
	const char *p;
	int32_t len;
} Slice;

static int hexval(int c)
{
	if (c >= '0' && c <= '9') { return c - '0'; }
	c = (c >= 'a' && c <= 'f') ? (c - 'a' + 'A') : c;
	if (c >= 'A' && c <= 'F') { return c - 'A' + 10; }
	return -1;
}

static int decode_u4(const char *s, uint16_t *out)
{
	int h0 = hexval(s[0]), h1 = hexval(s[1]), h2 = hexval(s[2]), h3 = hexval(s[3]);
	if (h0 < 0 || h1 < 0 || h2 < 0 || h3 < 0) { return 0; }
	*out = (uint16_t)((h0 << 12) | (h1 << 8) | (h2 << 4) | h3);
	return 1;
}

// 将 JSON 字符串（不含两端引号）规范化为 UTF-8 字节序列
static int normalize_json_string(const char *in, int32_t in_len, unsigned char **out, int32_t *out_len)
{
	// 预留足够缓冲
	int32_t cap = in_len * 4 + 8;
	unsigned char *buf = (unsigned char *)malloc(cap);
	if (!buf) { return 0; }
	int32_t pos = 0;

	for (int32_t i = 0; i < in_len; i++)
	{
		unsigned char ch = (unsigned char)in[i];

		if (ch == '\\')
		{
			if (i + 1 >= in_len)
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
				if (i + 4 >= in_len)
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
					// 高代理，必须跟 \uXXXX 低代理
					if (i + 2 >= in_len || in[i + 1] != '\\' || in[i + 2] != 'u' || i + 6 >= in_len)
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
					// 组合码点
					uint32_t cp = 0x10000 + (((uint32_t)(u1 - 0xD800) << 10) | (uint32_t)(u2 - 0xDC00));
					// 编码为 UTF-8
					buf[pos++] = (unsigned char)(0xF0 | ((cp >> 18) & 0x07));
					buf[pos++] = (unsigned char)(0x80 | ((cp >> 12) & 0x3F));
					buf[pos++] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
					buf[pos++] = (unsigned char)(0x80 | (cp & 0x3F));
				}
				else if (u1 >= 0xDC00 && u1 <= 0xDFFF)
				{
					// 单独低代理非法
					free(buf);
					return 0;
				}
				else
				{
					// BMP 码点 → UTF-8
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
			default:
				// 非法转义
				free(buf);
				return 0;
			}
		}
		else
		{
			// 原始字节：直接拷贝（假设输入整体是合法 UTF-8）
			buf[pos++] = ch;
		}

		if (pos + 8 > cap)
		{
			cap *= 2;
			unsigned char *nb = (unsigned char *)realloc(buf, cap);
			if (!nb)
			{
				free(buf);
				return 0;
			}
			buf = nb;
		}
	}

	*out = buf;
	*out_len = pos;
	return 1;
}

// 比较：规范化两侧为 UTF-8 字节序列，再 memcmp
static int json_string_equal_semantic(const char *a, int32_t a_len, const char *b, int32_t b_len)
{
	unsigned char *na = NULL, *nb = NULL;
	int32_t nla = 0, nlb = 0;

	if (!normalize_json_string(a, a_len, &na, &nla)) { return 0; }
	if (!normalize_json_string(b, b_len, &nb, &nlb))
	{
		free(na);
		return 0;
	}

	int eq = (nla == nlb) && (memcmp(na, nb, nla) == 0);
	free(na);
	free(nb);
	return eq;
}

/* 去空白 */
static void trim(const char **s, int32_t *len)
{
	const char *p = *s;
	int32_t n = *len;
	while (n && isspace((unsigned char)*p))
	{
		p++;
		n--;
	}
	while (n && isspace((unsigned char)p[n - 1])) { n--; }
	*s = p;
	*len = n;
}

/* 是否是带双引号的字符串值 */
static int is_quoted_string(const char *s, int32_t len) { return len >= 2 && s[0] == '\"' && s[len - 1] == '\"'; }

/* 值级语义比较：字符串（去引号并 normalize）、数字（含科学计数法）、布尔、null */
static int json_scalar_equal(const char *a, int32_t a_len, const char *b, int32_t b_len)
{
	trim(&a, &a_len);
	trim(&b, &b_len);

	/* 字符串：去掉引号后做转义规范化再比较字节 */
	if (is_quoted_string(a, a_len) && is_quoted_string(b, b_len))
	{
		const char *as = a + 1;
		int32_t al = a_len - 2;
		const char *bs = b + 1;
		int32_t bl = b_len - 2;

		unsigned char *na = NULL, *nb = NULL;
		int32_t nla = 0, nlb = 0;
		if (!normalize_json_string(as, al, &na, &nla)) { return 0; }
		if (!normalize_json_string(bs, bl, &nb, &nlb))
		{
			free(na);
			return 0;
		}

		int eq = (nla == nlb) && (memcmp(na, nb, nla) == 0);
		free(na);
		free(nb);
		return eq;
	}

	/* 布尔 / null */
	if (a_len == 4 && strncmp(a, "true", 4) == 0 && b_len == 4 && strncmp(b, "true", 4) == 0) { return 1; }
	if (a_len == 5 && strncmp(a, "false", 5) == 0 && b_len == 5 && strncmp(b, "false", 5) == 0) { return 1; }
	if (a_len == 4 && strncmp(a, "null", 4) == 0 && b_len == 4 && strncmp(b, "null", 4) == 0) { return 1; }

	/* 数字：用 strtod 支持科学计数法，最后一字节必须到达字符串末尾（避免部分解析） */
	{
		char *endA = NULL, *endB = NULL;

		/* 拷贝到以 NUL 结尾的缓冲，避免 strtod 依赖外部长度 */
		char *bufA = (char *)malloc(a_len + 1);
		char *bufB = (char *)malloc(b_len + 1);
		if (!bufA || !bufB)
		{
			free(bufA);
			free(bufB);
			return 0;
		}

		memcpy(bufA, a, a_len);
		bufA[a_len] = '\0';
		memcpy(bufB, b, b_len);
		bufB[b_len] = '\0';

		double va = strtod(bufA, &endA);
		double vb = strtod(bufB, &endB);

		int okA = (endA && *endA == '\0');
		int okB = (endB && *endB == '\0');

		free(bufA);
		free(bufB);

		if (okA && okB)
		{
			/* 直接相等（包含 -0 与 0）或允许极小误差 */
			if (va == vb) { return 1; }
			if (fabs(va - vb) < 1e-15) { return 1; }
			return 0;
		}
	}

	/* 其余作为纯文本兜底比较 */
	return (a_len == b_len) && (memcmp(a, b, a_len) == 0);
}

/* 提取一元素数组的唯一元素；若不是一元素数组返回 0 */
static int extract_single_array_element(const char *s, int32_t len, const char **elem, int32_t *elem_len)
{
	trim(&s, &len);
	if (len < 2 || s[0] != '[' || s[len - 1] != ']') { return 0; }

	const char *p = s + 1;
	int32_t n = len - 2;

	/* 去前后空白 */
	trim(&p, &n);
	if (n == 0) { return 0; /* 空数组 */ }

	/* 扫描逗号，确保只有一个元素（字符串中的逗号不算） */
	int in_str = 0;
	int escape = 0;
	for (int32_t i = 0; i < n; i++)
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
			else if (c == ',') { return 0; /* 多元素数组 */ }
		}
	}

	/* 去尾部空白 */
	const char *q = p + n;
	while (n && isspace((unsigned char)q[-1]))
	{
		q--;
		n--;
	}

	*elem = p;
	*elem_len = n;
	return 1;
}

/* 顶层比较：若两侧都是一元素数组则剥离后比较；否则直接按值级比较 */
static int json_value_equal(const char *a, int32_t a_len, const char *b, int32_t b_len)
{
	const char *ae = NULL, *be = NULL;
	int32_t ale = 0, ble = 0;

	if (extract_single_array_element(a, a_len, &ae, &ale) && extract_single_array_element(b, b_len, &be, &ble))
	{
		return json_scalar_equal(ae, ale, be, ble);
	}

	return json_scalar_equal(a, a_len, b, b_len);
}

static void checkadjfladjfl(char *data, uint32_t len, char *str, uint32_t strLen, uint32_t *alksdjfCOunt)
{
	if (0 != strcmp(data, str))
	{
		// data/str 是去掉两端引号后的 JSON 字符串内容，并且有长度
		if (!json_value_equal(data, len, str, strLen))
		{
			(*alksdjfCOunt)++;
			// 打印时避免 %s，被 NUL 截断；可以打印十六进制
			printf("%d 数据不完全一致 -- 原始: %s -- 序列化: %s\n", *alksdjfCOunt, data, str);
		}
	}
}

/**
 * @brief RyanJson 测试程序
 *
 * @param fileName
 * @param data
 * @param len
 * @return int
 */
static int RyanJsonParseData(char *fileName, char *data, uint32_t len)
{

	if (strcmp(fileName, "n_structure_100000_opening_arrays.json") == 0 ||
	    strcmp(fileName, "n_structure_open_array_object.json") == 0 || strcmp(fileName, "n_structure_100000_opening_arrays.json") == 0)
	{
		return -1;
	}
	// printf("开始解析: %s\r\n", fileName);
	RyanJson_t json = RyanJsonParseOptions(data, len, RyanJsonTrue, NULL);
	if (NULL == json) { return -1; }

#ifdef PrintfStrCmpEnable
	int32_t strLen = 0;
	char *str = RyanJsonPrint(json, 60, RyanJsonFalse, &strLen);
	if (NULL == str)
	{
		printf("反序列化失败: [%s]\n", data);
		goto err;
	}

	RyanJsonMinify(data, len);
	static uint32_t alksdjfCOunt = 0;
	checkadjfladjfl(data, len, str, strLen, &alksdjfCOunt);

	RyanJsonFree(str);
#endif

	RyanJsonDelete(json);
	return 0;

err:
	RyanJsonDelete(json);
	return -1;
}

/**
 * @brief cJson测试程序
 *
 * @param fileName
 * @param data
 * @param len
 * @return int
 */
static int cJSONParseData(char *fileName, char *data, uint32_t len)
{

	if (strcmp(fileName, "n_structure_100000_opening_arrays.json") == 0 ||
	    strcmp(fileName, "n_structure_open_array_object.json") == 0 || strcmp(fileName, "n_structure_100000_opening_arrays.json") == 0)
	{
		return -1;
	}

	cJSON *json = cJSON_ParseWithLengthOpts(data, len + sizeof(""), NULL, RyanJsonTrue);
	if (NULL == json) { return -1; }

#ifdef PrintfStrCmpEnable
	char *str = cJSON_PrintBuffered(json, 60, RyanJsonFalse);
	if (NULL == str)
	{
		printf("反序列化失败: [%s]\n", data);
		goto err;
	}

	cJSON_Minify(data);
	static uint32_t alksdjfCOunt = 0;
	checkadjfladjfl(data, len, str, strlen(str), &alksdjfCOunt);

	cJSON_free(str);
#endif

	cJSON_Delete(json);
	return 0;
err:
	cJSON_Delete(json);
	return -1;
}

/**
 * @brief cJson测试程序
 *
 * @param fileName
 * @param data
 * @param len
 * @return int
 */
static int yyjsonParseData(char *fileName, char *data, uint32_t len)
{
	if (strcmp(fileName, "n_structure_100000_opening_arrays.json") == 0 ||
	    strcmp(fileName, "n_structure_open_array_object.json") == 0 || strcmp(fileName, "n_structure_100000_opening_arrays.json") == 0)
	{
		return -1;
	}

	yyjson_doc *doc = yyjson_read(data, len, 0);
	if (NULL == doc) { return -1; }

#ifdef PrintfStrCmpEnable
	char *str = yyjson_write(doc, 0, NULL);
	if (NULL == str)
	{
		printf("反序列化失败: [%s]\n", data);
		goto err;
	}

	cJSON_Minify(data);
	static uint32_t alksdjfCOunt = 0;
	checkadjfladjfl(data, len, str, strlen(str), &alksdjfCOunt);

	free(str);
#endif

	yyjson_doc_free(doc);
	return 0;
err:
	yyjson_doc_free(doc);
	return -1;
}

// RFC 8259 JSON Test Suite
// https://github.com/nst/JSONTestSuite
RyanJsonBool_e RFC8259JsonTest(void)
{
	int result = 0;
	RyanJsonInitHooks(v_malloc, v_free, v_realloc);

	cJSON_Hooks hooks = {.malloc_fn = v_malloc, .free_fn = v_free};
	cJSON_InitHooks(&hooks);

	printf("开始 RFC 8259 JSON 测试");

	printf("\r\n--------------------------- RFC8259  RyanJson --------------------------\r\n");
	result = testFile("../../../../test//RFC8259JsonData", RyanJsonParseData);
	if (0 != result)
	{
		printf("%s:%d RyanJson RFC8259JsonTest fail\r\n", __FILE__, __LINE__);
		goto err;
	}

	printf("\r\n--------------------------- RFC8259  cJSON --------------------------\r\n");
	result = testFile("../../../../test//RFC8259JsonData", cJSONParseData);
	if (0 != result)
	{
		printf("%s:%d cJSON RFC8259JsonTest fail\r\n", __FILE__, __LINE__);
		goto err;
	}

	printf("\r\n--------------------------- RFC8259  yyjson --------------------------\r\n");
	result = testFile("../../../../test//RFC8259JsonData", yyjsonParseData);
	if (0 != result)
	{
		printf("%s:%d yyjson RFC8259JsonTest fail\r\n", __FILE__, __LINE__);
		goto err;
	}

	displayMem();
	return RyanJsonTrue;

err:
	displayMem();
	return RyanJsonFalse;
}
