// Copyright 2024 Accenture.

#include "uds/services/ecureset/EnableRapidPowerShutdown.h"

#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
uint8_t const EnableRapidPowerShutdown::sfImplementedRequest[2] = {ServiceId::ECU_RESET, 0x04U};

EnableRapidPowerShutdown::EnableRapidPowerShutdown(IUdsLifecycleConnector& udsLifecycleConnector)
: Subfunction(
    &sfImplementedRequest[0],
    AbstractDiagJob::EMPTY_REQUEST,
    RESPONSE_LENGTH,
    DiagSessionMask::getInstance() << DiagSession::APPLICATION_DEFAULT_SESSION()
                                   << DiagSession::APPLICATION_EXTENDED_SESSION())
, fUdsLifecycleConnector(udsLifecycleConnector)
{}

DiagReturnCode::Type EnableRapidPowerShutdown::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(ShutDownTime);
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

void EnableRapidPowerShutdown::responseSent(
    IncomingDiagConnection& connection, ResponseSendResult result)
{
    connection.terminate();
    uint8_t localShutdowntime = ShutDownTime;
    (void)fUdsLifecycleConnector.requestPowerdown(true, localShutdowntime);
}
} // namespace uds
