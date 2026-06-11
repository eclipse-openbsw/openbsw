/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "uds/Stm32DtcManager.h"
#include "uds/base/Service.h"

namespace uds
{
/**
 * ControlDTCSetting (SID 0x85).
 *   Subfunction 0x01 = ON  (enable DTC recording)
 *   Subfunction 0x02 = OFF (disable DTC recording)
 */
class Stm32ControlDtcSetting : public Service
{
public:
    explicit Stm32ControlDtcSetting(Stm32DtcManager& dtcManager);

private:
    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    Stm32DtcManager& _dtcManager;
};

} // namespace uds
