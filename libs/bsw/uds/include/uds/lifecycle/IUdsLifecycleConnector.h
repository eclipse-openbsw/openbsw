/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
