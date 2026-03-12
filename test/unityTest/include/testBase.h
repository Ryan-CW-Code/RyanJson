#ifndef RYANJSON_TEST_BASE_H
#define RYANJSON_TEST_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "RyanJson.h"
#include "RyanJsonInternal.h"
#include "cJSON.h"
#include "valloc.h"
#include "testCommon.h"

#undef jsonLog
#define jsonLog(fmt, ...) testLog("%s:%d " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

// 定义枚举类型

// 定义结构体类型

/* extern variables-----------------------------------------------------------*/

extern void printJsonDebug(RyanJson_t json);
extern void rootNodeCheckTest(RyanJson_t json);
extern void itemNodeCheckTest(RyanJson_t json);
extern void arrayNodeCheckTest(RyanJson_t json, RyanJsonBool_e isReversed);
extern void arrayItemNodeCheckTest(RyanJson_t json);
extern void testCheckRoot(RyanJson_t pJson);
extern void testCheckRootEx(RyanJson_t pJson, RyanJsonBool_e isReversed);
#define UNITY_TEST_LIST_ENTRY(name) extern void name(void);
#include "../runner/test_list.inc"
#undef UNITY_TEST_LIST_ENTRY

#ifdef __cplusplus
}
#endif

#endif
