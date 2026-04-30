// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file FreeRtosPlatformConfig.h
 * \brief FreeRTOS device-specific configuration for NUCLEO-G474RE (170 MHz)
 *
 * Included by asyncFreeRtos/freeRtosConfiguration/FreeRTOSConfig.h.
 */

#pragma once

#include "bsp/SystemTime.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define configCPU_CLOCK_HZ (170000000UL)

#undef configMINIMAL_STACK_SIZE
#define configMINIMAL_STACK_SIZE  ((unsigned short)512)
#define configMAX_TASK_NAME_LEN   (10)
#define configIDLE_SHOULD_YIELD   1
#define configUSE_MUTEXES         1
#define configQUEUE_REGISTRY_SIZE 30
#undef configCHECK_FOR_STACK_OVERFLOW
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configRECORD_STACK_HIGH_ADDRESS         1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_MALLOC_FAILED_HOOK            0
#define configUSE_APPLICATION_TASK_TAG          0
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_FPU                           0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0

/* Memory allocation */
#define configTOTAL_HEAP_SIZE            512
#define configAPPLICATION_ALLOCATED_HEAP 1

/* Co-routine definitions */
#define configMAX_CO_ROUTINE_PRIORITIES (0)

/* Software timer definitions */
#undef configTIMER_QUEUE_LENGTH
#define configTIMER_QUEUE_LENGTH 30
#undef configTIMER_TASK_STACK_DEPTH
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE)

/* API function includes */
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

/* Cortex-M specific definitions */
#ifdef __NVIC_PRIO_BITS
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      0x0F
// CAN ISR at priority 5 (0x50) must not be masked by BASEPRI.
// BASEPRI = (this value << 4). With 5: BASEPRI=0x50 masks priority 5.
// With 6: BASEPRI=0x60, priority 5 (0x50) is NOT masked.
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 0x06

#ifndef configKERNEL_INTERRUPT_PRIORITY
#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#endif

#ifndef configMAX_SYSCALL_INTERRUPT_PRIORITY
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#endif

/* Assert */
#define DEV_ASSERT(x)
#if defined(__GNUC__) && (!defined(__ASSEMBLER__))
#include "mcu/mcu.h"
#endif
#define configASSERT(x) DEV_ASSERT(x)

/* Tickless idle */
#define configUSE_TICKLESS_IDLE               0
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP 2
#define configUSE_TICKLESS_IDLE_DECISION_HOOK 0

/* Map FreeRTOS port handlers to CMSIS standard names */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* Run-time stats */
#define configGENERATE_RUN_TIME_STATS (1)
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portGET_RUN_TIME_COUNTER_VALUE() getSystemTimeUs32Bit()

#define configTIMER_SERVICE_TASK_NAME "TIMER_OS"

#ifdef __cplusplus
} /* extern "C" */
#endif
