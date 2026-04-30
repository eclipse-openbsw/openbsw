// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   osHooks.cpp
 * \brief  FreeRTOS hook implementations for NUCLEO-F413ZH.
 *
 * Provides the mandatory FreeRTOS callback hooks for stack overflow
 * detection, heap allocation failure, and the system tick.
 * All hooks execute in ISR or scheduler context.
 */

#include "FreeRTOS.h"
#include "mcu/mcu.h"
#include "task.h"

extern "C"
{
/**
 * \brief  Called by FreeRTOS when a task stack overflow is detected.
 *
 * Enters an infinite loop (fail-closed safe state).
 *
 * \note   Invoked from the context switch when configCHECK_FOR_STACK_OVERFLOW
 *         is enabled. Parameters are intentionally unused.
 */
void vApplicationStackOverflowHook(TaskHandle_t /* xTask */, char* /* pcTaskName */)
{
    while (true) {}
}

/**
 * \brief  Called by FreeRTOS when pvPortMalloc() fails.
 *
 * Enters an infinite loop (fail-closed safe state).
 *
 * \note   Invoked from task context when configUSE_MALLOC_FAILED_HOOK is
 *         enabled and the heap is exhausted.
 */
void vApplicationMallocFailedHook(void)
{
    while (true) {}
}

/**
 * \brief  Called by FreeRTOS on every system tick interrupt (~1 ms).
 *
 * Feeds the Independent Watchdog (IWDG) to prevent a hardware reset.
 *
 * \note   The IWDG may have been enabled by a previous firmware image
 *         and persists across soft resets -- it cannot be disabled, only fed.
 */
void vApplicationTickHook(void) { IWDG->KR = 0xAAAAU; }
}
