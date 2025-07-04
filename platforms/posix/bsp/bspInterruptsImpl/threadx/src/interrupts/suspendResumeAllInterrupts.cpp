// Copyright 2025 Accenture.

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
        status = _tx_thread_interrupt_disable();
    }

    return status;
}

void resumeAllInterrupts(OldIntEnabledStatusValueType const oldIntEnabledStatusValue)
{
    if (threadXInitialized)
    {
        _tx_thread_interrupt_restore(oldIntEnabledStatusValue);
    }
}
