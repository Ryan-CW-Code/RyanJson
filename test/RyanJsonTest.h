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
			printf("内存泄漏\r\n");                                                                                            \
			while (1)                                                                                                          \
			{                                                                                                                  \
				v_mcheck(&area, &use);                                                                                     \
				printf("|||----------->>> area = %d, size = %d\r\n", area, use);                                           \
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
		/* 开始执行：绿色高亮，文件名放在 [TEST n] 后面 */                                                                         \
		printf("\x1b[32m┌── [TEST %d | %s:%d] 开始执行: %s()\x1b[0m\r\n", testRunCount, __FILE__, __LINE__, #fun);                 \
                                                                                                                                           \
		funcStartMs = platformUptimeMs();                                                                                          \
		result = fun();                                                                                                            \
                                                                                                                                           \
		/* 结束执行：根据结果显示绿色或红色，文件名放在 [TEST n] 后面 */                                                           \
		printf("%s└── [TEST %" PRIu32 " | %s:%d] 结束执行: 结果 %s | 耗时: %" PRIu64 " ms\x1b[0m\r\n\r\n",                         \
		       (result == RyanJsonTrue) ? "\x1b[32m" : "\x1b[31m", testRunCount, __FILE__, __LINE__,                               \
		       (result == RyanJsonTrue) ? "✅" : "❌", (platformUptimeMs() - funcStartMs));                                        \
                                                                                                                                           \
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
