// Copyright 2024 Accenture.

#include "interrupts/suspendResumeAllInterrupts.h"

#include <FreeRTOS.h>
#include <task.h>
#include <thread>
#include <unistd.h>

static std::thread::id MAIN_THREAD_ID;

void main_thread_setup(void) { MAIN_THREAD_ID = std::this_thread::get_id(); }

OldIntEnabledStatusValueType getOldIntEnabledStatusValueAndSuspendAllInterrupts(void)
{
    if (std::this_thread::get_id() == MAIN_THREAD_ID)
    {
        // We will reach this place in two cases:
        // 1. The main thread still haven't entered vTaskStartScheduler() - or already exited it.
        //    No locks needed.
        // 2. Inside vTaskStartScheduler(), with interrupts disabled, asyncEnterTask() is called.
        //    We shall not mess with the interrupt logic there.

        return OldIntEnabledStatusValueType(0);
    }

    taskENTER_CRITICAL();
    return OldIntEnabledStatusValueType(1);
}

void resumeAllInterrupts(OldIntEnabledStatusValueType const oldIntEnabledStatusValue)
{
    if (oldIntEnabledStatusValue != 0)
    {
        taskEXIT_CRITICAL();
    }
}
