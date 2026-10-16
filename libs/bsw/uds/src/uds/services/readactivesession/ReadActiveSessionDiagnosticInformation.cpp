/********************************************************************************
 * Copyright (c) 2026 Contributors to the Eclipse Foundation
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/readactivesession/ReadActiveSessionDiagnosticInformation.h"

#include "uds/UdsLogger.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/connection/PositiveResponse.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg): Logger API uses C-style varargs.
using ::util::logger::Logger;
using ::util::logger::UDS;

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

ReadActiveSessionDiagnosticInformation::ReadActiveSessionDiagnosticInformation()
: DataIdentifierJob(_implementedRequest, DiagSession::ALL_SESSIONS()), _sessionManager(nullptr)
{
    // Build the 3-byte implemented request: [SID=0x22][DID_H=0xF1][DID_L=0x86]
    // Note: _implementedRequest must be populated BEFORE DataIdentifierJob uses it,
    // but DataIdentifierJob stores only the pointer — it is used during verify().
    // We fill the array here (member initialisation order: members before base init
    // is not guaranteed in this pattern, so we re-assign below as DataIdentifierJob
    // copies the pointer, not the bytes).
    _implementedRequest[0] = 0x22U; // Service ID — ReadDataByIdentifier
    _implementedRequest[1] = DID_HIGH;
    _implementedRequest[2] = DID_LOW;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void ReadActiveSessionDiagnosticInformation::setSessionManager(
    IDiagSessionManager const& sessionManager)
{
    _sessionManager = &sessionManager;
}

// ---------------------------------------------------------------------------
// process()
// ---------------------------------------------------------------------------

DiagReturnCode::Type ReadActiveSessionDiagnosticInformation::process(
    IncomingDiagConnection& connection,
    uint8_t const* const /*request*/,
    uint16_t const /*requestLength*/)
{
    // Guard: session manager must be wired before use
    if (_sessionManager == nullptr)
    {
        Logger::warn(UDS, "ReadActiveSession: session manager not wired — NRC 0x22");
        return DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT;
    }

    // Retrieve the currently active diagnostic session
    DiagSession const& activeSession = _sessionManager->getActiveSession();
    uint8_t const sessionByte        = activeSession.getSessionByte();

    Logger::debug(UDS, "ReadActiveSession: active session = 0x%02X", sessionByte);

    // Build positive response: just [sessionByte] — the parent ReadDataByIdentifier
    // service already prepends [0x62][DID_H=0xF1][DID_L=0x86] to the response.
    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(sessionByte);

    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg)

} // namespace uds
