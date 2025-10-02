// Copyright 2024 Accenture.

#include "uds/services/ecureset/SoftReset.h"

#include "uds/DiagDispatcher.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"
#include "uds/session/ProgrammingSession.h"

namespace uds
{
uint8_t const SoftReset::sfImplementedRequest[2] = {ServiceId::ECU_RESET, 0x03U};

SoftReset::SoftReset(IUdsLifecycleConnector& udsLifecycleConnector, DiagDispatcher& diagDispatcher)
: Subfunction(
    &sfImplementedRequest[0],
    AbstractDiagJob::EMPTY_REQUEST,
    AbstractDiagJob::EMPTY_RESPONSE,
    DiagSessionMask::getInstance()
        << DiagSession::APPLICATION_DEFAULT_SESSION() << DiagSession::PROGRAMMING_SESSION()
        << DiagSession::APPLICATION_EXTENDED_SESSION())
, fUdsLifecycleConnector(udsLifecycleConnector)
, fDiagDispatcher(diagDispatcher)
{}

DiagReturnCode::Type SoftReset::process(
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

void SoftReset::responseSent(
    IncomingDiagConnection& connection, ResponseSendResult const /* result */)
{
    connection.terminate();
    if (!fUdsLifecycleConnector.requestShutdown(IUdsLifecycleConnector::SOFT_RESET, RESET_TIME))
    {
        fDiagDispatcher.fEnabled = true;
    }
}

} // namespace uds
