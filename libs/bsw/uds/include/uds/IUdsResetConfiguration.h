// Copyright 2024 Accenture.

#ifndef GUARD_DC0BA923_A62C_454C_A6B7_E4908C117069
#define GUARD_DC0BA923_A62C_454C_A6B7_E4908C117069

#include "platform/estdint.h"

namespace uds
{
class IUdsResetConfiguration
{
public:
    virtual void callApplicationForHardReset()      = 0;
    virtual void callApplicationForSoftReset()      = 0;
    virtual void callApplicationForPowerDown()      = 0;
    virtual void callApplicationForRapidPowerDown() = 0;
    virtual uint8_t getRapidPowerDownTime()         = 0;
};

} // namespace uds

#endif // GUARD_DC0BA923_A62C_454C_A6B7_E4908C117069
