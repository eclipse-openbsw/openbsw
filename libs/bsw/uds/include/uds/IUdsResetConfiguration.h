// Copyright 2024 Accenture.

#pragma once

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
