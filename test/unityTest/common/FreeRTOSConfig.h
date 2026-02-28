#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

/**
 * @brief RyanJson 单元测试的 FreeRTOS 内核配置入口（固定 Linux POSIX 端口）。
 * @note 该文件不是业务代码接口，而是 FreeRTOS 内核编译期配置表。
 * @details xmake 会编译 FreeRTOS Kernel + POSIX port，并由这里的宏控制调度、
 *          内存、断言和 API 裁剪行为。
 */

/**
 * @brief 统一断言出口。
 * @note 具体实现由 `test/unityTest/runner/main.c` 提供。
 * @details 内核触发 `configASSERT()` 时会回调 `vAssertCalled()`，便于在单元测试日志里
 *          精确定位断言文件和行号。
 */
extern void vAssertCalled(const char *file, int32_t line);
#define configASSERT(x)                                                                                                                    \
	do                                                                                                                                 \
	{                                                                                                                                  \
		if ((x) == 0) { vAssertCalled(__FILE__, (int32_t)__LINE__); }                                                              \
	} while (0)

/**
 * @brief 调度与时基配置。
 * @note 这里采用抢占式调度和 1ms Tick，优先覆盖并发场景下的测试行为。
 * @details `configCHECK_HANDLER_INSTALLATION` 在支持的移植层上可增加中断处理器安装检查。
 */
#define configUSE_PREEMPTION                      1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION   0
#define configUSE_TIME_SLICING                    1
#define configUSE_16_BIT_TICKS                    0
#define configUSE_TICK_HOOK                       0
#define configUSE_IDLE_HOOK                       0
#define configCHECK_HANDLER_INSTALLATION          1
#define configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES 1

#define configCPU_CLOCK_HZ              1000000UL
#define configTICK_RATE_HZ              1000UL
#define configMAX_PRIORITIES            16
#define configMINIMAL_STACK_SIZE        256U
#define configMAX_TASK_NAME_LEN         32
#define configIDLE_SHOULD_YIELD         1
#define configRECORD_STACK_HIGH_ADDRESS 1

/**
 * @brief 内核对象能力开关。
 * @note 测试模式下尽量打开常用同步原语，覆盖更多内核交互路径。
 */
#define configUSE_TASK_NOTIFICATIONS          1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 1
#define configUSE_MUTEXES                     1
#define configUSE_RECURSIVE_MUTEXES           1
#define configUSE_COUNTING_SEMAPHORES         1
#define configUSE_QUEUE_SETS                  1
#define configUSE_APPLICATION_TASK_TAG        1
#define configQUEUE_REGISTRY_SIZE             16
#define configENABLE_BACKWARD_COMPATIBILITY   0
#define configUSE_MINI_LIST_ITEM              0
#define configSTACK_DEPTH_TYPE                size_t
#define configMESSAGE_BUFFER_LENGTH_TYPE      size_t

/**
 * @brief 诊断与观测能力。
 * @note 栈溢出与分配失败 hook 已在 runner 中实现，会在失败时直接终止并打日志。
 */
#define configCHECK_FOR_STACK_OVERFLOW  2
#define configUSE_MALLOC_FAILED_HOOK    1
#define configUSE_SB_COMPLETED_CALLBACK 1

#define configUSE_TRACE_FACILITY             1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configGENERATE_RUN_TIME_STATS        0
#define configSTATS_BUFFER_MAX_LENGTH        0xFFFFU

/**
 * @brief 内存分配模型配置（固定 `heap_4`）。
 * @details
 * - 单测链路统一使用 FreeRTOS `heap_4`（可释放且支持空闲块合并）。
 * - `heap_4` 依赖 `configTOTAL_HEAP_SIZE` 定义的静态堆区。
 * - `configHEAP_CLEAR_MEMORY_ON_FREE` 与 `configENABLE_HEAP_PROTECTOR`
 *   可增强 UAF/越界类问题暴露能力。
 */
#define configSUPPORT_STATIC_ALLOCATION  0
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configAPPLICATION_ALLOCATED_HEAP 0
#define configTOTAL_HEAP_SIZE            (64U * 1024U * 1024U)
#define configHEAP_CLEAR_MEMORY_ON_FREE  1
#define configENABLE_HEAP_PROTECTOR      1

#define configUSE_CO_ROUTINES 0

/**
 * @brief 软件定时器配置。
 * @note 定时器任务使用较高优先级，避免测试中的计时行为被长时间饿死。
 */
#define configUSE_TIMERS             1
#define configTIMER_TASK_PRIORITY    (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH     20
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE * 2U)

/**
 * @brief POSIX 端辅助能力。
 * @note 开启 `configUSE_POSIX_ERRNO` 后，每个任务可维护自己的 errno 语义。
 */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5
#define configUSE_POSIX_ERRNO                   1

/**
 * @brief API 暴露开关。
 * @note 测试目标倾向开启更多 API，便于未来补充内核交互类单测。
 */
#define INCLUDE_vTaskPrioritySet             1
#define INCLUDE_uxTaskPriorityGet            1
#define INCLUDE_vTaskDelete                  1
#define INCLUDE_vTaskSuspend                 1
#define INCLUDE_xTaskDelayUntil              1
#define INCLUDE_vTaskDelay                   1
#define INCLUDE_xTaskGetSchedulerState       1
#define INCLUDE_xTaskGetCurrentTaskHandle    1
#define INCLUDE_xTaskGetIdleTaskHandle       1
#define INCLUDE_eTaskGetState                1
#define INCLUDE_xTaskGetHandle               1
#define INCLUDE_xTaskAbortDelay              1
#define INCLUDE_uxTaskGetStackHighWaterMark  1
#define INCLUDE_uxTaskGetStackHighWaterMark2 1
#define INCLUDE_xTimerPendFunctionCall       1
#define INCLUDE_xTaskResumeFromISR           1
#define INCLUDE_xSemaphoreGetMutexHolder     1

#endif
