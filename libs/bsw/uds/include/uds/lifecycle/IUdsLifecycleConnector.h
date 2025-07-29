// Copyright 2024 Accenture.

#pragma once

#include "platform/estdint.h"

namespace uds
{
class IUdsLifecycleConnector
{
public:
    enum ShutdownType
    {
        NO_SHUTDOWN,
        POWER_DOWN,
        HARD_RESET,
        SOFT_RESET,
        SOFTWARE_DESTRUCTIVE_RESET,
        GOTO_UPDATER,
        BOOTLOADER_UPDATE
    };

    virtual bool isModeChangePossible() const                         = 0;
    virtual bool requestPowerdown(bool rapid, uint8_t& time)          = 0;
    virtual bool requestShutdown(ShutdownType type, uint32_t timeout) = 0;
};

} // namespace uds
