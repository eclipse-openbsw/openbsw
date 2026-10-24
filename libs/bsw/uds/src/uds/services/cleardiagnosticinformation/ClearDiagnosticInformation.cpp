/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/cleardiagnosticinformation/ClearDiagnosticInformation.h"

#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/DiagSession.h"
#include "udsDiagMgr/DiagMgr.h"

namespace uds
{

ClearDiagnosticInformation::ClearDiagnosticInformation()
: Service(
    ServiceId::CLEAR_DIAGNOSTIC_INFORMATION,
    EXPECTED_REQUEST_LENGTH,
    RESPONSE_LENGTH,
    DiagSession::ALL_SESSIONS())
, _diagMgr(nullptr)
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE);
}

DiagReturnCode::Type ClearDiagnosticInformation::process(
    IncomingDiagConnection& connection,
    uint8_t const* const request,
    uint16_t const /* requestLength */)
{
    uint32_t const groupOfDTC = (static_cast<uint32_t>(request[0]) << 16U)
                                | (static_cast<uint32_t>(request[1]) << 8U)
                                | static_cast<uint32_t>(request[2]);

    switch (groupOfDTC)
    {
        case GROUP_ALL_DTCS:
        {
            // Delegate to DiagMgr if wired — clears RAM table + storage
            if (_diagMgr != nullptr)
            {
                (void)_diagMgr->clearAllDTCs();
            }
            PositiveResponse& response = connection.releaseRequestGetResponse();
            (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
            return DiagReturnCode::OK;
        }
        // Additional groupOfDTC values (powertrain, chassis, body, network) are
        // reserved for project-specific DiagMgr extension.
        default:
        {
            return DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE;
        }
    }
}

} // namespace uds
