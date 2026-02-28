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

extern void testChangeRunner(void);
extern void testCompareRunner(void);
extern void testCreateRunner(void);
extern void testDeleteRunner(void);
extern void testDetachRunner(void);
extern void testDuplicateRunner(void);
extern void testForEachRunner(void);
extern void testLoadSuccessRunner(void);
extern void testLoadFailureRunner(void);
extern void testReplaceRunner(void);
extern void testPrintRunner(void);
extern void testMemoryRunner(void);
extern void testDeepRecursionRunner(void);

extern void testEqualityBoolRunner(void);
extern void testEqualityDoubleRunner(void);
extern void testEqualityIntRunner(void);
extern void testEqualityStringRunner(void);

extern void testUtilsRunner(void);
extern void testRobustRunner(void);
extern void testStressRunner(void);
extern void testRfc8259Runner(void);

#ifdef __cplusplus
}
#endif

#endif
