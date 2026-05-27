/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "interrupts/suspendResumeAllInterrupts.h"

#include <tx_api.h>
#include <unistd.h>

static bool threadXInitialized{false};

void setThreadXInitialized() { threadXInitialized = true; }

void main_thread_setup(void)
{ /* Do nothing */
}

OldIntEnabledStatusValueType getOldIntEnabledStatusValueAndSuspendAllInterrupts(void)
{
    OldIntEnabledStatusValueType status{0};

    if (threadXInitialized)
    {
        status = tx_interrupt_control(TX_INT_DISABLE);
    }

    return status;
}

void resumeAllInterrupts(OldIntEnabledStatusValueType const oldIntEnabledStatusValue)
{
    if (threadXInitialized)
    {
        tx_interrupt_control(oldIntEnabledStatusValue);
    }
}
