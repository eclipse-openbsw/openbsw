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
 * ClearDiagnosticInformation (SID 0x14).
 * Accepts 3-byte group-of-DTC. 0xFFFFFF = clear all.
 */
class Stm32ClearDtc : public Service
{
public:
    explicit Stm32ClearDtc(Stm32DtcManager& dtcManager);

private:
    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    Stm32DtcManager& _dtcManager;
};

} // namespace uds
