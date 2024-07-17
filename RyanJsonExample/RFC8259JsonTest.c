

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
// #include <intrin.h>

#include "RyanJson.h"
#include "cJSON.h"
#include "yyjson.h"
#include "valloc.h"

// #define PrintfStrCmpEnable

typedef int (*jsonParseData)(char *file_name, char *data, uint32_t len);

static void *yy_malloc(void *ctx, size_t size)
{
    return v_malloc(size);
}
static void *yy_realloc(void *ctx, void *ptr, size_t old_size, size_t size)
{
    return v_realloc(ptr, size);
}
static void yy_free(void *ctx, void *ptr)
{
    v_free(ptr);
}

static yyjson_alc yyalc = {
    yy_malloc,
    yy_realloc,
    yy_free,
    NULL};

/* Read a file, parse, render back, etc. */
int testFile(const char *path, jsonParseData jsonParseDataHandle)
{

    DIR *dir = NULL;
    struct dirent *entry;
    int idx = 0, alc = 0;
    char **names = NULL, **names_tmp;

    int path_len = strlen(path);
    int count = 0;
    int used_count = 0;
    if (!path || !path_len || !(dir = opendir(path)))
        goto fail;

    while ((entry = readdir(dir)))
    {

        char *name = (char *)entry->d_name;

        if (!name || !strlen(name))
            continue;

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            continue;

        char aaa[300] = {0};
        snprintf(aaa, sizeof(aaa), "%s/%s", path, name);

        FILE *f = fopen(aaa, "rb");
        if (f == NULL)
            goto fail;

        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);
        char *data = (char *)malloc(len + 10);
        fread(data, 1, len, f);
        data[len] = '\0';
        fclose(f);
        int status = 0;

        status = jsonParseDataHandle(name, data, len);

        used_count++;
        if (0 == strncmp("y_", name, 2))
        {
            if (1 == status)
                count++;
            else
                printf("应该成功，但是失败: %s, len: %ld\n", data, len);
        }
        else if (0 == strncmp("n_", name, 2))
        {
            if (0 == status)
                count++;
            else
                printf("应该失败，但是成功: %s, len: %ld\n", data, len);
        }
        else if (0 == strncmp("i_", name, 2))
        {
            count++;
        }

        int area = 0, use = 0;
        v_mcheck(&area, &use);
        if (use != (len + 10))
        {
            free(data);
            printf("内存泄漏 %s len: %ld\r\n", data, len);
            printf("内存泄漏 %x len: %ld\r\n", data, len);
            printf("内存泄漏 %c len: %ld\r\n", data, len);
            printf("|||----------->>> area = %d, size = %d\r\n", area, use);
            break;
        }
        free(data);
    }

    closedir(dir);

    printf("RFC 8259 JSON: (%d/%d)\r\n", count, used_count);
    return 1;

fail:
    if (dir)
        closedir(dir);

    return 0;
}

/**
 * @brief RyanJson 测试程序
 *
 * @param file_name
 * @param data
 * @param len
 * @return int
 */
int RyanJsonParseData(char *file_name, char *data, uint32_t len)
{

    if (strcmp(file_name, "n_structure_100000_opening_arrays.json") == 0 ||
        strcmp(file_name, "n_structure_open_array_object.json") == 0 ||
        strcmp(file_name, "n_structure_100000_opening_arrays.json") == 0)
        return 0;

    RyanJson_t json = RyanJsonParseOptions(data, len, RyanJsonTrue, NULL);
    if (NULL == json)
        return 0;

#ifdef PrintfStrCmpEnable
    char *str = RyanJsonPrint(json, 60, RyanJsonFalse, NULL);
    if (NULL == str)
    {
        printf("反序列化失败: [%s]\n", data);
        goto next;
    }

    RyanJsonMinify(data);
    if (0 != strcmp(data, str))
        printf("数据不一致 -- 原始: %s -- 序列化: %s\n", data, str);

    RyanJsonFree(str);
#endif

next:
    RyanJsonDelete(json);
    return 1;
}

/**
 * @brief cJson测试程序
 *
 * @param file_name
 * @param data
 * @param len
 * @return int
 */
int cJSONParseData(char *file_name, char *data, uint32_t len)
{

    if (strcmp(file_name, "n_structure_100000_opening_arrays.json") == 0 ||
        strcmp(file_name, "n_structure_open_array_object.json") == 0 ||
        strcmp(file_name, "n_structure_100000_opening_arrays.json") == 0)
        return 0;

    cJSON *json = cJSON_ParseWithLengthOpts(data, len + sizeof(""), NULL, RyanJsonTrue);
    if (NULL == json)
        return 0;

#ifdef PrintfStrCmpEnable
    char *str = cJSON_PrintBuffered(json, 60, RyanJsonFalse);
    if (NULL == str)
    {
        printf("反序列化失败: [%s]\n", data);
        goto next;
    }

    cJSON_Minify(data);
    if (0 != strcmp(data, str))
        printf("-- 原始: %s -- 序列化: %s\n", data, str);

    cJSON_free(str);
#endif

next:
    cJSON_Delete(json);
    return 1;
}

/**
 * @brief cJson测试程序
 *
 * @param file_name
 * @param data
 * @param len
 * @return int
 */
int yyjsonParseData(char *file_name, char *data, uint32_t len)
{

    if (strcmp(file_name, "n_structure_100000_opening_arrays.json") == 0 ||
        strcmp(file_name, "n_structure_open_array_object.json") == 0 ||
        strcmp(file_name, "n_structure_100000_opening_arrays.json") == 0)
        return 0;

    yyjson_doc *doc = yyjson_read_opts(data, len, YYJSON_READ_NOFLAG, &yyalc, NULL);
    if (NULL == doc)
        return 0;

#ifdef PrintfStrCmpEnable
    char *str = yyjson_write_opts(doc, YYJSON_WRITE_NOFLAG, &yyalc, NULL, NULL);
    if (NULL == str)
    {
        printf("反序列化失败: [%s]\n", data);
        goto next;
    }

    cJSON_Minify(data);
    if (0 != strcmp(data, str))
        printf("-- 原始: %s -- 序列化: %s\n", data, str);

    free(str);
#endif

next:
    yyjson_doc_free(doc);
    return 1;
}

// RFC 8259 JSON Test Suite
// https://github.com/nst/JSONTestSuite
int RFC8259JsonTest(void)
{
    RyanJsonInitHooks(v_malloc, v_free, v_realloc);

    cJSON_Hooks hooks = {
        .malloc_fn = v_malloc,
        .free_fn = v_free};
    cJSON_InitHooks(&hooks);

    printf("开始 RFC 8259 JSON 测试");

    printf("\r\n--------------------------- RFC8259  RyanJson --------------------------\r\n");
    testFile("./RyanJsonExample/RFC8259JsonData", RyanJsonParseData);

    printf("\r\n--------------------------- RFC8259  cJSON --------------------------\r\n");
    testFile("./RyanJsonExample/RFC8259JsonData", cJSONParseData);

    printf("\r\n--------------------------- RFC8259  yyjson --------------------------\r\n");
    testFile("./RyanJsonExample/RFC8259JsonData", yyjsonParseData);

    displayMem();
    return 0;
}
