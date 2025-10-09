// Copyright 2024 Accenture.

#include "uds/services/ecureset/HardReset.h"

#include "uds/DiagDispatcher.h"
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
        fDiagDispatcher.fEnabled = false;
        (void)connection.sendPositiveResponse(*this);
        return DiagReturnCode::OK;
    }
    else
    {
        return DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT;
    }
}

void HardReset::responseSent(
    IncomingDiagConnection& connection, ResponseSendResult const /* result */)
{
    connection.terminate();
    if (!fUdsLifecycleConnector.requestShutdown(IUdsLifecycleConnector::HARD_RESET, RESET_TIME))
    {
        fDiagDispatcher.fEnabled = true;
    }
}

} // namespace uds
