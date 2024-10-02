// Copyright 2024 Accenture.

#include "uds/services/ecureset/PowerDown.h"

#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
uint8_t const PowerDown::sfImplementedRequest[2] = {ServiceId::ECU_RESET, 0x41U};

PowerDown::PowerDown(IUdsLifecycleConnector& udsLifecycleConnector)
: Subfunction(
    &sfImplementedRequest[0],
    AbstractDiagJob::EMPTY_REQUEST,
    AbstractDiagJob::EMPTY_RESPONSE,
    DiagSessionMask::getInstance() << DiagSession::APPLICATION_DEFAULT_SESSION()
                                   << DiagSession::APPLICATION_EXTENDED_SESSION())
, fUdsLifecycleConnector(udsLifecycleConnector)
{}

DiagReturnCode::Type PowerDown::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    (void)connection.sendPositiveResponse(*this);
    return DiagReturnCode::OK;
}

void PowerDown::responseSent(IncomingDiagConnection& connection, ResponseSendResult result)
{
    connection.terminate();
    uint8_t dummy;

    (void)fUdsLifecycleConnector.requestPowerdown(false, dummy);
}
} // namespace uds
