#ifndef RyanJsonTest
#define RyanJsonTest

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <inttypes.h>
#include "valloc.h"
#include "RyanJson.h"
#include "RyanJsonUtils.h"
#include "cJSON.h"
#include "yyjson.h"

#define getArraySize(arr) ((int32_t)(sizeof(arr) / sizeof((arr)[0])))
#define checkMemory                                                                                                                        \
	do                                                                                                                                 \
	{                                                                                                                                  \
		int area = 0, use = 0;                                                                                                     \
		v_mcheck(&area, &use);                                                                                                     \
		if (area != 0 || use != 0)                                                                                                 \
		{                                                                                                                          \
			RyanMqttLog_e("内存泄漏");                                                                                         \
			while (1)                                                                                                          \
			{                                                                                                                  \
				v_mcheck(&area, &use);                                                                                     \
				RyanMqttLog_e("|||----------->>> area = %d, size = %d", area, use);                                        \
				delay(3000);                                                                                               \
			}                                                                                                                  \
		}                                                                                                                          \
	} while (0)

// 定义枚举类型
extern void *v_malloc_tlsf(size_t size);
extern void v_free_tlsf(void *block);
extern void *v_realloc_tlsf(void *block, size_t size);
extern int32_t vallocGetUseByTlsf(void);

// 定义结构体类型
uint64_t platformUptimeMs(void);

#define runTestWithLogAndTimer(fun)                                                                                                        \
	do                                                                                                                                 \
	{                                                                                                                                  \
		testRunCount++;                                                                                                            \
		printf("┌── [TEST %d] 开始执行: " #fun "()\r\n", testRunCount);                                                            \
		funcStartMs = platformUptimeMs();                                                                                          \
		result = fun();                                                                                                            \
		printf("└── [TEST %" PRIu32 "] 结束执行: 返回值 = %" PRId32 " %s | 耗时: %" PRIu64 " ms\x1b[0m\r\n\r\n", testRunCount,     \
		       result, (result == RyanJsonTrue) ? "✅" : "❌", (platformUptimeMs() - funcStartMs));                                \
		RyanJsonCheckCodeNoReturn(RyanJsonTrue == result, { return RyanJsonFalse; });                                              \
	} while (0)

/* extern variables-----------------------------------------------------------*/
extern RyanJsonBool_e RyanJsonExample(void);
extern RyanJsonBool_e RyanJsonBaseTest(void);
extern RyanJsonBool_e RFC8259JsonTest(void);
extern RyanJsonBool_e RyanJsonMemoryFootprintTest(void);
extern RyanJsonBool_e RyanJsonTestFun(void);
#ifdef __cplusplus
}
#endif

#endif
