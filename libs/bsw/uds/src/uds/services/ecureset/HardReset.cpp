/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/ecureset/HardReset.h"

#include "uds/DiagDispatcher.h"
#include "uds/UdsConfig.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
uint8_t const HardReset::sfImplementedRequest[] = {ServiceId::ECU_RESET, 0x01U};

HardReset::HardReset(IUdsLifecycleConnector& udsLifecycleConnector, DiagDispatcher& diagDispatcher)
: Subfunction(
    &sfImplementedRequest[0],
    AbstractDiagJob::EMPTY_REQUEST,
    AbstractDiagJob::EMPTY_RESPONSE,
    DiagSession::ALL_SESSIONS())
, fUdsLifecycleConnector(udsLifecycleConnector)
, fDiagDispatcher(diagDispatcher)
{}

DiagReturnCode::Type HardReset::process(
    IncomingDiagConnection& connection,
    uint8_t const* const /* request */,
    uint16_t const /* requestLength */)
{
    if (fUdsLifecycleConnector.isModeChangePossible())
    {
        // BUG-FIX: Do NOT set fEnabled = false here.
        //
        // sendPositiveResponse() posts an async closure that calls
        // DiagDispatcher::send() on the next task cycle.  DiagDispatcher::send()
        // checks fEnabled at the top and returns TP_SEND_FAIL if it is false.
        // Setting fEnabled = false BEFORE the response is sent therefore causes
        // the 0x51 0x01 positive response to be silently dropped — the tester
        // never sees an acknowledgement even though the ECU resets successfully.
        //
        // fEnabled is cleared in responseSent() instead, which is invoked only
        // after the transport layer has confirmed the frame is on the wire
        // (via asyncTransportMessageProcessed callback).  That is the correct
        // point to block new incoming requests.
        (void)connection.sendPositiveResponse(*this);
        return DiagReturnCode::OK;
    }

    return DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT;
}

void HardReset::responseSent(
    IncomingDiagConnection& connection, ResponseSendResult const /* result */)
{
    // Gate the dispatcher against new requests now that the response is confirmed sent.
    fDiagDispatcher.fEnabled = false;
    connection.terminate();
    if (!fUdsLifecycleConnector.requestShutdown(
            IUdsLifecycleConnector::HARD_RESET, UdsConstants::RESET_TIME_HARD))
    {
        // requestShutdown returned false — shutdown was rejected; re-enable the dispatcher.
        fDiagDispatcher.fEnabled = true;
    }
}

} // namespace uds
