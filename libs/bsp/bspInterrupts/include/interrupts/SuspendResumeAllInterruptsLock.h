// Copyright 2024 Accenture.

#ifndef GUARD_D7E38C8A_C22E_47BC_B05A_773E8AA4F30F
#define GUARD_D7E38C8A_C22E_47BC_B05A_773E8AA4F30F

#include "interrupts/suspendResumeAllInterrupts.h"

namespace interrupts
{
class SuspendResumeAllInterruptsLock
{
public:
    // [PUBLICAPI_START]
    /**
     * Suspend all interrupts and store previous state in an class internal variable
     */
    void suspend()
    {
        fOldMachineStateRegisterValue = getMachineStateRegisterValueAndSuspendAllInterrupts();
    }

    /**
     * Resume all interrupts restoring the interrupt state that has been saved during the suspend()
     * call from the class internal variable
     */
    void resume() { resumeAllInterrupts(fOldMachineStateRegisterValue); }

    // [PUBLICAPI_END]

private:
    uint32_t fOldMachineStateRegisterValue{};
};

} /* namespace interrupts */

#endif /* GUARD_D7E38C8A_C22E_47BC_B05A_773E8AA4F30F */
