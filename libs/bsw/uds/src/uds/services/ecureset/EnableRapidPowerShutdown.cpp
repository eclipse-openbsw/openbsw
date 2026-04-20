// Copyright 2024 Accenture.
// Copyright 2026 BMW AG

#include "uds/services/ecureset/EnableRapidPowerShutdown.h"

#include "uds/UdsConfig.h"
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
    IncomingDiagConnection& connection,
    uint8_t const* const /* request */,
    uint16_t const /* requestLength */)
{
    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(UdsConstants::SHUTDOWN_TIME);
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

void EnableRapidPowerShutdown::responseSent(
    IncomingDiagConnection& connection, ResponseSendResult /* result */)
{
    connection.terminate();
    uint8_t localShutdowntime = UdsConstants::SHUTDOWN_TIME;
    (void)fUdsLifecycleConnector.requestPowerdown(true, localShutdowntime);
}
} // namespace uds
