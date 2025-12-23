#ifndef __RyanJsonBaseTest__
#define __RyanJsonBaseTest__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "RyanJson.h"
#include "RyanJsonUtils.h"
#include "cJSON.h"
#include "valloc.h"
#include "RyanJsonTest.h"
#define jsonLog(fmt, ...) printf("%s:%d " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

// 定义枚举类型

// 定义结构体类型

/* extern variables-----------------------------------------------------------*/

extern RyanJsonBool_e compare_double(double a, double b);
extern void printJsonDebug(RyanJson_t json);
extern RyanJsonBool_e rootNodeCheckTest(RyanJson_t json);
extern RyanJsonBool_e itemNodeCheckTest(RyanJson_t json);
extern RyanJsonBool_e arrayNodeCheckTest(RyanJson_t json);
extern RyanJsonBool_e arrayItemNodeCheckTest(RyanJson_t json);
extern RyanJsonBool_e RyanJsonBaseTestCheckRoot(RyanJson_t pJson);

extern RyanJsonBool_e RyanJsonBaseTestChangeJson(void);
extern RyanJsonBool_e RyanJsonBaseTestCompareJson(void);
extern RyanJsonBool_e RyanJsonBaseTestCreateJson(void);
extern RyanJsonBool_e RyanJsonBaseTestDeleteJson(void);
extern RyanJsonBool_e RyanJsonBaseTestDetachJson(void);
extern RyanJsonBool_e RyanJsonBaseTestDuplicateJson(void);
extern RyanJsonBool_e RyanJsonBaseTestForEachJson(void);
extern RyanJsonBool_e RyanJsonBaseTestLoadJson(void);
extern RyanJsonBool_e RyanJsonBaseTestReplaceJson(void);

#ifdef __cplusplus
}
#endif

#endif
