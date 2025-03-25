// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace safety
{
static inline __attribute__((always_inline)) uint32_t
getMachineStateRegisterValueAndSuspendAllInterrupts(void)
{
#ifdef PLATFORM_SUPPORT_MPU
    uint32_t _PRIMASK;
    __asm("mrs %0, PRIMASK\n"
          "cpsid i\n"
          : "=r"(_PRIMASK));
    return (_PRIMASK);
#endif
    return 0;
}

static inline __attribute__((always_inline)) void
resumeAllInterrupts(uint32_t oldMachineStateRegisterValue)
{
#ifdef PLATFORM_SUPPORT_MPU
    __asm("msr PRIMASK, %[Input]\n" ::[Input] "r"(oldMachineStateRegisterValue));
#endif
}

} // namespace safety
