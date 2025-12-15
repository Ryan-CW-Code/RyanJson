#ifndef __RyanJsonTest__
#define __RyanJsonTest__

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
#include "valloc.h"
// #define malloc  v_malloc
// #define calloc  v_calloc
// #define free    v_free
// #define realloc v_realloc

#include "RyanJson.h"
#include "RyanJsonUtils.h"
#include "cJSON.h"
// #include "yyjson.h"

#define getArraySize(arr) ((int32_t)(sizeof(arr) / sizeof((arr)[0])))
#define checkMemory                                                                                                                                                                                    \
	do                                                                                                                                                                                             \
	{                                                                                                                                                                                              \
		int area = 0, use = 0;                                                                                                                                                                 \
		v_mcheck(&area, &use);                                                                                                                                                                 \
		if (area != 0 || use != 0)                                                                                                                                                             \
		{                                                                                                                                                                                      \
			RyanMqttLog_e("内存泄漏");                                                                                                                                                     \
			while (1)                                                                                                                                                                      \
			{                                                                                                                                                                              \
				v_mcheck(&area, &use);                                                                                                                                                 \
				RyanMqttLog_e("|||----------->>> area = %d, size = %d", area, use);                                                                                                    \
				delay(3000);                                                                                                                                                           \
			}                                                                                                                                                                              \
		}                                                                                                                                                                                      \
	} while (0)

// 定义枚举类型

// 定义结构体类型

/* extern variables-----------------------------------------------------------*/
extern RyanJsonBool_e RyanJsonExample(void);
extern RyanJsonBool_e RyanJsonBaseTest(void);
extern RyanJsonBool_e RFC8259JsonTest(void);
extern RyanJsonBool_e RyanJsonMemoryFootprintTest(void);
#ifdef __cplusplus
}
#endif

#endif
