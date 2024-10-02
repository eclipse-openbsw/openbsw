// Copyright 2024 Accenture.

#ifndef GUARD_00A5BD7E_31EC_4CAF_A1B4_C4659B74160B
#define GUARD_00A5BD7E_31EC_4CAF_A1B4_C4659B74160B

#include "bsp/error/IEventListener.h"

#include <platform/estdint.h>

namespace bios
{

class BspError
{
public:
    BspError();

    //   static void event(uint32_t event, bool active, uint32_t status = IEventListener::INVALID);
    static void event(
        uint32_t event,
        bool active,
        uint32_t status0 = IEventListener::INVALID,
        uint32_t status1 = IEventListener::INVALID,
        uint32_t status2 = IEventListener::INVALID,
        uint32_t status3 = IEventListener::INVALID);
    static void setListener(IEventListener& l);

    static inline uint32_t getError() { return sEvent; }

private:
    static uint32_t sEvent;
    static uint32_t sOldEvent;
    static IEventListener* fListener;
};

} // namespace bios

#endif /* GUARD_00A5BD7E_31EC_4CAF_A1B4_C4659B74160B */
