// Copyright 2024 Accenture.

#pragma once

/* Device-specific asyncFreeRtos configuration.
 *
 * This file contains the part of the FreeRTOS configuration for a specific device.
 * It is included into libs/bsw/asyncFreeRtos/freeRtosConfiguration/FreeRTOSConfig.h, whose
 * own content provides device-independent configuration of FreeRTOS for use with async API.
 *
 * For the description of the config parameters used below,
 * see https://www.freertos.org/Documentation/02-Kernel/03-Supported-devices/02-Customization
 */

#include "bsp/SystemTime.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define configCPU_CLOCK_HZ (80000000UL)
#define configBUS_CLOCK_HZ 40000000UL

#undef configMINIMAL_STACK_SIZE
#define configMINIMAL_STACK_SIZE  ((unsigned short)512)
#define configMAX_TASK_NAME_LEN   (10)
#define configIDLE_SHOULD_YIELD   1
#define configUSE_MUTEXES         1
#define configQUEUE_REGISTRY_SIZE 30
// #define configCHECK_FOR_STACK_OVERFLOW              1
#undef configCHECK_FOR_STACK_OVERFLOW
#define configCHECK_FOR_STACK_OVERFLOW          2 // change from 1 in order to watermark check boundaries
#define configRECORD_STACK_HIGH_ADDRESS         1 // add to record pxEndOfStack max stack extension
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_MALLOC_FAILED_HOOK            0
#define configUSE_APPLICATION_TASK_TAG          0
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_FPU                           0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0

/* Memory allocation related definitions. */
#define configTOTAL_HEAP_SIZE            512
#define configAPPLICATION_ALLOCATED_HEAP 1

/* Co-routine definitions. */
#define configMAX_CO_ROUTINE_PRIORITIES (0)

/* Software timer definitions. */
#undef configTIMER_QUEUE_LENGTH

// Queue length for the timer task. This needs to account for all timers
// that are used across all tasks.
#define configTIMER_QUEUE_LENGTH 30
#undef configTIMER_TASK_STACK_DEPTH
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE)

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet      1
#define INCLUDE_uxTaskPriorityGet     1
#define INCLUDE_vTaskDelete           1
#define INCLUDE_vTaskCleanUpResources 1
#define INCLUDE_vTaskSuspend          1
#define INCLUDE_vResumeFromISR        1
#define INCLUDE_vTaskDelayUntil       1
#define INCLUDE_vTaskDelay            1
#define INCLUDE_eTaskGetState         1
#undef INCLUDE_uxTaskGetStackHighWaterMark
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetSchedulerState      1
#define INCLUDE_xQueueGetMutexHolder        1
#undef INCLUDE_xTaskGetCurrentTaskHandle
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#undef INCLUDE_xTaskGetIdleTaskHandle
#define INCLUDE_xTaskGetIdleTaskHandle   1
#define INCLUDE_pcTaskGetTaskName        1
#define INCLUDE_xEventGroupSetBitFromISR 1
#undef INCLUDE_xTimerPendFunctionCall
#define INCLUDE_xTimerPendFunctionCall 1

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 4
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 0x0F

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 0x05

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#ifndef configKERNEL_INTERRUPT_PRIORITY
#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#endif

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#ifndef configMAX_SYSCALL_INTERRUPT_PRIORITY
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#endif

/* Definition assert() function. */
#define DEV_ASSERT(x)
#if defined(__GNUC__) && (!defined(__ASSEMBLER__))
#include "mcu/mcu.h"
#endif
#define configASSERT(x) DEV_ASSERT(x)

/* Tickless Idle Mode */
#define configUSE_TICKLESS_IDLE               0
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP 2
#define configUSE_TICKLESS_IDLE_DECISION_HOOK 0

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
// ISR names in the interrupt weak functor table
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#define configGENERATE_RUN_TIME_STATS (1)
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portGET_RUN_TIME_COUNTER_VALUE() getSystemTimeUs32Bit()

#define configTIMER_SERVICE_TASK_NAME "TIMER_OS"

#ifdef __cplusplus
} /* extern "C" */
#endif
