// Copyright 2025 Accenture.

#include "interrupts/suspendResumeAllInterrupts.h"

uint32_t lockIsrCounter = 0U;

uint32_t getMachineStateRegisterValueAndSuspendAllInterrupts(void) { return ++lockIsrCounter; }

void resumeAllInterrupts(uint32_t oldMachineStateRegisterValue)
{
    lockIsrCounter = oldMachineStateRegisterValue;
}
