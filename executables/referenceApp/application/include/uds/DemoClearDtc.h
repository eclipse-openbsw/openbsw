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

#include "uds/DemoDtcManager.h"
#include "uds/base/Service.h"

namespace uds
{
/**
 * ClearDiagnosticInformation (SID 0x14).
 *
 * Accepts a 3-byte groupOfDTC value: 0xFFFFFF clears all DTCs; any other
 * value clears only the single DTC whose number matches exactly. ISO 14229
 * group ranges (e.g. powertrain group) are not implemented.
 */
class DemoClearDtc : public Service
{
public:
    explicit DemoClearDtc(DemoDtcManager& dtcManager);

private:
    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    DemoDtcManager& _dtcManager;
};

} // namespace uds
