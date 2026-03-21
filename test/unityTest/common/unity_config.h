#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

#include <stdint.h>

/* 输出与格式 */
#define UNITY_OUTPUT_COLOR             // 开启彩色打印
#define UNITY_INCLUDE_PRINT_FORMATTED  // 开启 UnityPrintF 支持
#define UNITY_INCLUDE_EXEC_TIME        // 开启测试执行时间统计
#define UNITY_DIFFERENTIATE_FINAL_FAIL // 最终汇总显示 FAILED

// #define UNITY_OUTPUT_FOR_QT_CREATOR      // 输出 file://path:line，方便点击跳转
// #define UNITY_OUTPUT_FOR_ECLIPSE         // 输出 Eclipse 友好的格式
// #define UNITY_OUTPUT_FOR_IAR_WORKBENCH   // 输出 IAR Workbench 友好的格式

// #define UNITY_OUTPUT_CHAR(a) my_putchar(a)            // 替换输出字符函数
// #define UNITY_OUTPUT_CHAR_HEADER_DECLARATION my_putchar // 声明输出函数（仅声明，不定义）
// #define UNITY_OMIT_OUTPUT_CHAR_HEADER_DECLARATION     // 不在头文件中声明输出函数

// #define UNITY_OUTPUT_FLUSH() my_flush()               // 替换 flush
// #define UNITY_OUTPUT_FLUSH_HEADER_DECLARATION my_flush // 声明 flush 函数
// #define UNITY_USE_FLUSH_STDOUT                        // flush 使用 fflush(stdout)

// #define UNITY_PRINT_EOL() UNITY_OUTPUT_CHAR('\n')     // 自定义换行输出
// #define UNITY_OUTPUT_START()                           // 测试开始前回调
// #define UNITY_OUTPUT_COMPLETE()                        // 测试结束后回调
// #define UNITY_PRINT_TEST_CONTEXT()                     // 失败时输出上下文

/* Int/指针宽度与头文件 */
#define UNITY_SUPPORT_64    // 开启 64 位 Int 支持
#define UNITY_INT_WIDTH  32 // int 位宽（手动指定时使用）
#define UNITY_UINT_WIDTH 32 // uint 位宽（Unity 默认未使用，可保留）
// #define UNITY_LONG_WIDTH 32          // long 位宽
// #define UNITY_POINTER_WIDTH 32       // 指针位宽

// #define UNITY_COUNTER_TYPE uint16_t  // 计数器类型（测试数量较大时可调大）
// #define UNITY_LINE_TYPE uint16_t     // 行号类型（大文件可调大）

// #define UNITY_EXCLUDE_STDINT_H       // 不包含 <stdint.h>
// #define UNITY_EXCLUDE_LIMITS_H       // 不包含 <limits.h>
// #define UNITY_EXCLUDE_STDDEF_H       // 不包含 <stddef.h>
// #define UNITY_EXCLUDE_MATH_H         // 不包含 <math.h>
// #define UNITY_EXCLUDE_SETJMP_H       // 不包含 <setjmp.h>

/* 浮点配置 */
#define UNITY_INCLUDE_DOUBLE // 开启 double 支持
// #define UNITY_EXCLUDE_DOUBLE         // 禁用 double（默认）
// #define UNITY_DOUBLE_TYPE double     // 自定义 double 类型
// #define UNITY_DOUBLE_PRECISION 1e-12 // double 误差

// #define UNITY_EXCLUDE_FLOAT          // 禁用 float
// #define UNITY_FLOAT_TYPE float       // 自定义 float 类型
// #define UNITY_FLOAT_PRECISION 1e-6f  // float 误差
// #define UNITY_EXCLUDE_FLOAT_PRINT    // 禁止浮点打印（减小体积）

// #define UNITY_NAN_NOT_EQUAL_NAN      // NaN 与 NaN 比较为不相等
// #define UNITY_ROUND_TIES_AWAY_FROM_ZERO // 浮点四舍五入策略
// #define UNITY_IS_NAN(x) my_isnan(x)  // 自定义 NaN 判断
// #define UNITY_IS_INF(x) my_isinf(x)  // 自定义 Inf 判断

/* 细节/诊断信息 */
// #define UNITY_EXCLUDE_DETAILS        // 关闭细节栈
// #define UNITY_DETAIL1_NAME "Function" // 细节1名字
// #define UNITY_DETAIL2_NAME "Argument" // 细节2名字
// #define UNITY_DETAIL_STACK_SIZE 8    // 细节栈大小（启用后用栈保存）
// #define UNITY_DETAIL_LABEL_TYPE uint8_t
// #define UNITY_DETAIL_VALUE_TYPE uint32_t
// #define UNITY_DETAIL_LABEL_NAMES {0, "Func", "Arg"} // 自定义标签名

/* 执行时间统计 */
// #define UNITY_TIME_TYPE uint32_t     // 时间类型
// #define UNITY_CLOCK_MS() my_clock_ms() // 返回毫秒
// #define UNITY_EXEC_TIME_START() do { } while (0) // 自定义开始计时
// #define UNITY_EXEC_TIME_STOP()  do { } while (0) // 自定义停止计时
// #define UNITY_PRINT_EXEC_TIME() do { } while (0) // 自定义打印耗时

/* 测试用例/Runner */
// #define UNITY_USE_COMMAND_LINE_ARGS  // 支持命令行参数
// #define UNITY_SKIP_DEFAULT_RUNNER    // 禁用默认 Runner
// #define UNITY_SUPPORT_TEST_CASES     // 启用 TEST_CASE 宏

/* Shorthand 行为 */
// #define UNITY_SHORTHAND_AS_OLD       // 旧版 shorthand
// #define UNITY_SHORTHAND_AS_INT       // 强制按 int 解释
// #define UNITY_SHORTHAND_AS_MEM       // 强制按 memory 解释
// #define UNITY_SHORTHAND_AS_RAW       // 原始不转换
// #define UNITY_SHORTHAND_AS_NONE      // 禁用 shorthand

/* 平台/内存 */
// #define UNITY_PROGMEM                // PROGMEM 支持（如 AVR）
// #define UNITY_PTR_ATTRIBUTE          // 指针属性修饰
// #define UNITY_INTERNAL_PTR           // 内部指针类型

/* 特殊行为 */
// #define UNITY_COMPARE_PTRS_ON_ZERO_ARRAY // 允许对长度为 0 的 Array 做指针比较

#endif /* UNITY_CONFIG_H */
