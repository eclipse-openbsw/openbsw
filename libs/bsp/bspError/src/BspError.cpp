// Copyright 2024 Accenture.

#include "bspError/BspError.h"

#include <interrupts/suspendResumeAllInterrupts.h>

namespace bios
{

uint32_t BspError::sEvent           = 0UL;
uint32_t BspError::sOldEvent        = 0UL;
IEventListener* BspError::fListener = nullptr;

BspError::BspError()
{
    BspError::fListener = nullptr;
    sEvent              = 0UL;
    sOldEvent           = 0UL;
}

void BspError::setListener(IEventListener& l) { fListener = &l; }

void BspError::event(
    uint32_t const event,
    bool const active,
    uint32_t status0,
    uint32_t status1,
    uint32_t status2,
    uint32_t status3)
{
    {
        uint32_t const lock = getMachineStateRegisterValueAndSuspendAllInterrupts();
        if (active)
        {
            sEvent |= event;
        }
        else
        {
            sEvent &= ~event;
        }

        resumeAllInterrupts(lock);
    }
    if (sEvent != sOldEvent)
    {
        if (fListener != nullptr)
        {
            fListener->event(event, active, status0, status1, status2, status3);
        }
        sOldEvent = sEvent;
    }
}

} // namespace bios
