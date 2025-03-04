// Copyright 2024 Accenture.

#ifndef GUARD_A34A1932_3ECA_481E_BF1D_8DC25E60BCDF
#define GUARD_A34A1932_3ECA_481E_BF1D_8DC25E60BCDF

#include "safeLifecycle/interrupts/suspendResumeAllInterrupts.h"

namespace safety
{
class IsrLock final
{
public:
    IsrLock()
    {
        fOldMachineStateRegisterValue = getMachineStateRegisterValueAndSuspendAllInterrupts();
    }

    ~IsrLock() { resumeAllInterrupts(fOldMachineStateRegisterValue); }

private:
    uint32_t fOldMachineStateRegisterValue;
};

} // end namespace safety

#endif /* GUARD_A34A1932_3ECA_481E_BF1D_8DC25E60BCDF */
