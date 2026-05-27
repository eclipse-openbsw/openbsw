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
