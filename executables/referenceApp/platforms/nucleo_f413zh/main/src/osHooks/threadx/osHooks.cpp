// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   osHooks.cpp
 * \brief  ThreadX hook implementations for NUCLEO-F413ZH.
 *
 * Provides the execution change notification hooks required when
 * TX_ENABLE_EXECUTION_CHANGE_NOTIFY is defined, plus low-power stubs.
 */

#include "async/Config.h"
#include "async/Hook.h"
#include "mcu/mcu.h"
#include "tx_api.h"

#include <cstdio>

extern "C"
{
extern TX_THREAD _tx_timer_thread;

void _tx_execution_thread_enter()
{
    TX_THREAD* thread = tx_thread_identify();
    if (thread->tx_thread_priority == _tx_timer_thread.tx_thread_priority)
    {
        asyncEnterTask(static_cast<size_t>(ASYNC_CONFIG_TASK_COUNT));
    }
    else
    {
        asyncEnterTask(static_cast<size_t>(thread->tx_thread_entry_parameter));
    }
}

void _tx_execution_thread_exit()
{
    TX_THREAD* thread = tx_thread_identify();
    if (thread->tx_thread_priority == _tx_timer_thread.tx_thread_priority)
    {
        asyncLeaveTask(static_cast<size_t>(ASYNC_CONFIG_TASK_COUNT));
    }
    else
    {
        asyncLeaveTask(static_cast<size_t>(thread->tx_thread_entry_parameter));
    }
}

void tx_low_power_enter()
{
    // Feed IWDG before entering low-power mode to prevent watchdog reset.
    IWDG->KR = 0xAAAAU;
}

void tx_low_power_exit() {}

void vIllegalISR()
{
    printf("vIllegalISR\r\n");
    for (;;)
        ;
}
}
