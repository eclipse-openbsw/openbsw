/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "uds/base/Service.h"

namespace uds
{
/**
 * UDS service ClearDiagnosticInformation (0x14).
 *
 * This service enables a tester to request clearing of diagnostic trouble
 * codes (DTCs). The actual clearing is project-specific and must be provided
 * by the integrator separately. This class provides only a demo implementation
 * which handles the groupOfDTC parameter:
 * - groupOfDTC 0xFFFFFF (all DTCs) returns a positive response
 * - any other group returns NRC 0x31 (requestOutOfRange)
 */
class ClearDiagnosticInformation : public Service
{
public:
    explicit ClearDiagnosticInformation();

private:
    static uint8_t const EXPECTED_REQUEST_LENGTH = 3U;
    static uint8_t const RESPONSE_LENGTH         = 0U;
    static uint32_t const GROUP_ALL_DTCS         = 0xFFFFFFU;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;
};

} // namespace uds
