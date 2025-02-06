// Copyright 2024 Accenture.

#ifndef GUARD_D56C5D48_A0E2_4EF0_BD21_F4620B6F3AF4
#define GUARD_D56C5D48_A0E2_4EF0_BD21_F4620B6F3AF4

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

#endif /* GUARD_D56C5D48_A0E2_4EF0_BD21_F4620B6F3AF4 */
