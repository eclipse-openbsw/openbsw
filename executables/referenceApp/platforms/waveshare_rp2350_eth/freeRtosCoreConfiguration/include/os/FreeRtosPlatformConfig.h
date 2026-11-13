/********************************************************************************
 * Copyright (c) 2026 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/*
 * FreeRTOS V202111.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#ifndef FREERTOS_CONFIG_EXAMPLES_COMMON_H
#define FREERTOS_CONFIG_EXAMPLES_COMMON_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

/* Scheduler Related */
#define configUSE_TICKLESS_IDLE 0

#define configIDLE_SHOULD_YIELD 1

/* The RP2350 reference configuration runs the system clock at 150 MHz, and the
 * Cortex-M33 SysTick is clocked from it.
 *
 * configSYSTICK_CLOCK_HZ is deliberately NOT defined here. The port only
 * defines it itself (as configCPU_CLOCK_HZ) when it is absent, and in that case
 * also sets SysTick's CLKSOURCE bit so the counter runs off the core clock.
 * Defining it, even to the very same value, selects the opposite: SysTick then
 * counts the external reference clock instead, which makes the tick far too
 * slow, so the watchdog is no longer serviced in time and the board resets. */
#define configCPU_CLOCK_HZ (150000000UL)

/* Synchronization Related */
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_APPLICATION_TASK_TAG          0
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_QUEUE_SETS                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_NEWLIB_REENTRANT              0
// todo need this for lwip FreeRTOS sys_arch to compile
#define configENABLE_BACKWARD_COMPATIBILITY     1
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5

/* System */
#define configSTACK_DEPTH_TYPE           uint32_t
#define configMESSAGE_BUFFER_LENGTH_TYPE size_t

/* Memory allocation related definitions. */
#ifndef configSUPPORT_STATIC_ALLOCATION
#define configSUPPORT_STATIC_ALLOCATION 0
#endif
#ifndef configSUPPORT_DYNAMIC_ALLOCATION
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#endif
#define configTOTAL_HEAP_SIZE            (128 * 1024)
#define configAPPLICATION_ALLOCATED_HEAP 0

/* Hook function related definitions. */
#define configUSE_MALLOC_FAILED_HOOK       0
#define configUSE_DAEMON_TASK_STARTUP_HOOK 0

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS        0
#define configUSE_STATS_FORMATTING_FUNCTIONS 0

/* Interrupt nesting behaviour configuration. */
/*
#define configKERNEL_INTERRUPT_PRIORITY         [dependent of processor]
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    [dependent on processor and application]
#define configMAX_API_CALL_INTERRUPT_PRIORITY   [dependent on processor and application]
*/

#define configPRIO_BITS 4

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      0x0F
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 0x05

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#ifndef configKERNEL_INTERRUPT_PRIORITY
#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#endif

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#if PICO_RP2350
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 16
#endif
#ifndef configMAX_SYSCALL_INTERRUPT_PRIORITY
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#endif

#if FREE_RTOS_KERNEL_SMP // set by the RP2xxx SMP port of FreeRTOS
/* SMP port only */
#ifndef configNUMBER_OF_CORES
#define configNUMBER_OF_CORES 2
#endif
#define configNUM_CORES               configNUMBER_OF_CORES
#define configTICK_CORE               0
#define configRUN_MULTIPLE_PRIORITIES 1
#if configNUMBER_OF_CORES > 1
#define configUSE_CORE_AFFINITY 1
#endif
#define configUSE_PASSIVE_IDLE_HOOK 0
#endif

/* RP2350 specific. The RP2350 uses the generic Cortex-M port rather than the
 * pico-sdk SMP port, so neither the SMP configuration nor the pico_sync /
 * pico_time interop layers apply here. */
#define configSUPPORT_PICO_SYNC_INTEROP 0
#define configSUPPORT_PICO_TIME_INTEROP 0

#include <assert.h>
/* Define to trap errors during development. */
#define configASSERT(x) assert(x)

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet       1
#define INCLUDE_uxTaskPriorityGet      1
#define INCLUDE_vTaskDelete            1
#define INCLUDE_vTaskSuspend           1
#define INCLUDE_vTaskDelayUntil        1
#define INCLUDE_vTaskDelay             1
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_eTaskGetState          1
#define INCLUDE_xTaskAbortDelay        1
#define INCLUDE_xTaskGetHandle         1
#define INCLUDE_xTaskResumeFromISR     1
#define INCLUDE_xQueueGetMutexHolder   1

#if PICO_RP2350
#define configENABLE_MPU               0
#define configENABLE_TRUSTZONE         0
#define configRUN_FREERTOS_SECURE_ONLY 1
#define configENABLE_FPU               1
#endif

/* A header file that defines trace macro can be included here. */

#endif /* FREERTOS_CONFIG_H */
