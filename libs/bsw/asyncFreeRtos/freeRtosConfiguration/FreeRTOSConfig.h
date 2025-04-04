// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#pragma once

#include "async/Config.h"
#include "async/Hook.h"

#define configSUPPORT_STATIC_ALLOCATION  (1)
#define configSUPPORT_DYNAMIC_ALLOCATION (0)
#define configUSE_PREEMPTION             (1)
#define configCHECK_FOR_STACK_OVERFLOW   (0)
#define configUSE_TICK_HOOK              (0)

#ifdef ASYNC_CONFIG_TICK_HOOK
#if ASYNC_CONFIG_TICK_HOOK
#define traceTASK_INCREMENT_TICK(xTickCount) \
    if (asyncTickHook() != 0)                \
    {                                        \
        xSwitchRequired = pdTRUE;            \
    }
#endif
#else
#define ASYNC_CONFIG_TICK_HOOK (0)
#endif
#ifndef ASYNC_CONFIG_TASK_CONFIG
#define ASYNC_CONFIG_TASK_CONFIG (0)
#endif
#define configUSE_IDLE_HOOK      (1)
#define configMINIMAL_STACK_SIZE (0)

#define configUSE_16_BIT_TICKS (0)

#define configMAX_PRIORITIES (ASYNC_CONFIG_TASK_COUNT + 1)

#define configUSE_CO_ROUTINES (0)

#define configUSE_TIMERS             (1)
#define configTIMER_TASK_PRIORITY    (ASYNC_CONFIG_TASK_COUNT)
#define configTIMER_QUEUE_LENGTH     (ASYNC_CONFIG_TASK_COUNT)
#define configTIMER_TASK_STACK_DEPTH (0)

#define configTICK_RATE_HZ (1000000U / ASYNC_CONFIG_TICK_IN_US)

#define configUSE_TRACE_FACILITY (1)

#define INCLUDE_xTaskGetCurrentTaskHandle   (1)
#define INCLUDE_xTaskGetIdleTaskHandle      (1)
#define INCLUDE_uxTaskGetStackHighWaterMark (1)
#define INCLUDE_xTimerPendFunctionCall      (1)

#define configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H (1)
#define FREERTOS_TASKS_C_ADDITIONS_INIT           asyncInitialized

#define traceTASK_CREATE(pxCurrentTCB) \
    vTaskSetTaskNumber(pxCurrentTCB, pxCurrentTCB->uxPriority) ASYNC_CONFIGURE_TASK(pxCurrentTCB)
#define traceTASK_SWITCHED_IN()  asyncEnterTask((uint8_t)pxCurrentTCB->uxTaskNumber);
#define traceTASK_SWITCHED_OUT() asyncLeaveTask((uint8_t)pxCurrentTCB->uxTaskNumber);

#ifndef ASYNC_CONFIG_NESTED_INTERRUPTS
#define ASYNC_CONFIG_NESTED_INTERRUPTS (1)
#endif

#if ASYNC_CONFIG_TASK_CONFIG
#define ASYNC_CONFIGURE_TASK(pxCurrentTCB) \
    ;                                      \
    portCONFIGURE_TASK(                    \
        pxCurrentTCB,                      \
        (portTASK_CONFIG_TYPE const*)asyncGetTaskConfig((size_t)pxCurrentTCB->uxPriority))
#define ASYNC_TASK_CONFIG_TYPE portTASK_CONFIG_TYPE
#else
#define ASYNC_CONFIGURE_TASK(pxCurrentTCB)
#define ASYNC_TASK_CONFIG_TYPE void
#endif

#ifndef USE_TRACE_RECORDER
#define USE_TRACE_RECORDER 0
#endif

#include "os/FreeRtosPlatformConfig.h"

#if (configUSE_TRACE_FACILITY == 1) && (USE_TRACE_RECORDER == 1)
#include "trcRecorder.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

void const* asyncGetTaskConfig(size_t taskIdx);

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus

namespace async
{
struct Config
{
    static size_t const TASK_COUNT = static_cast<size_t>(ASYNC_CONFIG_TASK_COUNT);
    static size_t const TICK_IN_US = static_cast<size_t>(ASYNC_CONFIG_TICK_IN_US);
};

} // namespace async

#endif // __cplusplus
