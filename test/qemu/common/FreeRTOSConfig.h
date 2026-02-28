#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stddef.h>
#include <stdint.h>

#include "qemuPlatform.h"

#define configASSERT(x)                                                                                                                    \
	do                                                                                                                                 \
	{                                                                                                                                  \
		if ((x) == 0) { qemuAssertFailed(__FILE__, __LINE__); }                                                                    \
	} while (0)

#define configUSE_PREEMPTION                      1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION   0
#define configUSE_TIME_SLICING                    1
#define configUSE_TICK_HOOK                       0
#define configUSE_IDLE_HOOK                       0
#define configCHECK_HANDLER_INSTALLATION          0
#define configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES 1

#define configCPU_CLOCK_HZ              25000000UL
#define configTICK_RATE_HZ              1000UL
#define configMAX_PRIORITIES            16
#define configMINIMAL_STACK_SIZE        256U
#define configMAX_TASK_NAME_LEN         32
#define configIDLE_SHOULD_YIELD         1
#define configRECORD_STACK_HIGH_ADDRESS 1
#define configTICK_TYPE_WIDTH_IN_BITS   TICK_TYPE_WIDTH_32_BITS

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

#define configCHECK_FOR_STACK_OVERFLOW       2
#define configUSE_MALLOC_FAILED_HOOK         1
#define configUSE_SB_COMPLETED_CALLBACK      1
#define configUSE_TRACE_FACILITY             1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configGENERATE_RUN_TIME_STATS        0
#define configSTATS_BUFFER_MAX_LENGTH        0xFFFFU

#define configSUPPORT_STATIC_ALLOCATION  0
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configAPPLICATION_ALLOCATED_HEAP 1
#define configTOTAL_HEAP_SIZE            (2U * 1024U * 1024U)

#define configUSE_CO_ROUTINES        0
#define configUSE_TIMERS             1
#define configTIMER_TASK_PRIORITY    (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH     20
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE * 2U)

#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5
#define configUSE_POSIX_ERRNO                   1

#define configPRIO_BITS                              3
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      7
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY      (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define INCLUDE_vTaskPrioritySet             1
#define INCLUDE_uxTaskPriorityGet            1
#define INCLUDE_vTaskDelete                  1
#define INCLUDE_vTaskSuspend                 1
#define INCLUDE_vTaskDelay                   1
#define INCLUDE_xTaskDelayUntil              1
#define INCLUDE_xTaskGetSchedulerState       1
#define INCLUDE_xTaskGetCurrentTaskHandle    1
#define INCLUDE_xTaskGetIdleTaskHandle       1
#define INCLUDE_eTaskGetState                1
#define INCLUDE_uxTaskGetStackHighWaterMark  1
#define INCLUDE_uxTaskGetStackHighWaterMark2 1
#define INCLUDE_xTaskGetHandle               1
#define INCLUDE_xTaskAbortDelay              1
#define INCLUDE_xTimerPendFunctionCall       1
#define INCLUDE_xTaskResumeFromISR           1
#define INCLUDE_xSemaphoreGetMutexHolder     1

#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif
