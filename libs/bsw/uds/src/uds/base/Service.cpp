// Copyright 2024 Accenture.

#include "uds/base/Service.h"

#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
Service::Service(uint8_t const service, DiagSession::DiagSessionMask const sessionMask)
: AbstractDiagJob(
    fService,
    1U,
    0U,
    AbstractDiagJob::VARIABLE_REQUEST_LENGTH,
    AbstractDiagJob::VARIABLE_RESPONSE_LENGTH,
    sessionMask)
{
    init(service);
}

Service::Service(
    uint8_t const service,
    uint8_t const requestPayloadLength,
    uint8_t const responseLength,
    DiagSession::DiagSessionMask const sessionMask)
: AbstractDiagJob(fService, 1U, 0U, requestPayloadLength, responseLength, sessionMask)
{
    init(service);
}

void Service::init(uint8_t const service)
{
    fService[0] = service;
    setDefaultDiagReturnCode(DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED);
}

DiagReturnCode::Type Service::verify(uint8_t const* const request, uint16_t const requestLength)
{
    if (request[0] != fpImplementedRequest[0])
    {
        return DiagReturnCode::NOT_RESPONSIBLE;
    }
    if (!fAllowedSessions.match(getSession()))
    {
        return DiagReturnCode::ISO_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    return DiagReturnCode::OK;
}

} // namespace uds
