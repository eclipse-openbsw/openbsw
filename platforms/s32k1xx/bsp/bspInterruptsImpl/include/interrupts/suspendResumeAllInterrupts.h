// Copyright 2024 Accenture.

#ifndef SUSPENDRESUMEALLINTERRUPTS_H_
#define SUSPENDRESUMEALLINTERRUPTS_H_

#include "platform/estdint.h"

// TODO: migrate getMachineStateRegisterValueAndSuspendAllInterrupts to the newer
// getOldIntEnabledStatusValueAndSuspendAllInterrupts below and remove this define
typedef int32_t OldIntEnabledStatusValueType;
#define getOldIntEnabledStatusValueAndSuspendAllInterrupts \
    getMachineStateRegisterValueAndSuspendAllInterrupts

// clang-format off
static inline __attribute__((always_inline))
volatile uint32_t getMachineStateRegisterValueAndSuspendAllInterrupts(void)
{
   uint32_t _PRIMASK;
   __asm("    mrs     %0, PRIMASK\n"
         "    cpsid   i\n"
         : "=r" (_PRIMASK));
    return(_PRIMASK);
}
static inline __attribute__((always_inline))
volatile void resumeAllInterrupts(uint32_t oldMachineStateRegisterValue)
{
    __asm("      msr     PRIMASK,%[Input]\n"
          ::[Input] "r" (oldMachineStateRegisterValue)
        );
}

// clang-format on

#endif /* SUSPENDRESUMEALLINTERRUPTS_H_ */
