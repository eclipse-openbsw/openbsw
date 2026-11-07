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

class DiagMgr;

/**
 * UDS service ClearDiagnosticInformation (0x14).
 *
 * Handles the groupOfDTC parameter:
 * - groupOfDTC 0xFFFFFF (all DTCs): delegates to DiagMgr::clearAllDTCs()
 *   which clears the RAM table and storage.
 * - any other group returns NRC 0x31 (requestOutOfRange)
 *
 * Without a wired DiagMgr, stub behaviour is retained for backwards
 * compatibility.
 */
class ClearDiagnosticInformation : public Service
{
public:
    explicit ClearDiagnosticInformation();

    /** Wire DiagMgr backend. Call during UdsSystem::init(). */
    void setDiagMgr(DiagMgr* diagMgr) { _diagMgr = diagMgr; }

private:
    static uint8_t const EXPECTED_REQUEST_LENGTH = 3U;
    static uint8_t const RESPONSE_LENGTH         = 0U;
    static uint32_t const GROUP_ALL_DTCS         = 0xFFFFFFU;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    DiagMgr* _diagMgr;
};

} // namespace uds
