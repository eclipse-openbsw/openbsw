/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "async/Config.h"
#include "async/Hook.h"
#include "mcu/mcu.h"
#include "tx_api.h"

#include <cstdio>

extern "C"
{
extern TX_THREAD _tx_timer_thread;

// Called from tx_initialize_low_level.S. NVIC priorities for peripheral
// interrupts are configured by the owning systems during board bring-up.
void setupApplicationsIsr() {}

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
