// Copyright 2024 Accenture.

#include "uds/base/Subfunction.h"

#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
Subfunction::Subfunction(
    uint8_t const* const implementedRequest, DiagSession::DiagSessionMask const sessionMask)
: AbstractDiagJob(
    implementedRequest,
    2U,
    1U,
    AbstractDiagJob::VARIABLE_REQUEST_LENGTH,
    AbstractDiagJob::VARIABLE_RESPONSE_LENGTH,
    sessionMask)
{}

Subfunction::Subfunction(
    uint8_t const* const implementedRequest,
    uint8_t const requestPayloadLength,
    uint8_t const responseLength,
    DiagSession::DiagSessionMask const sessionMask)
: AbstractDiagJob(implementedRequest, 2U, 1U, requestPayloadLength, responseLength, sessionMask)
{}

DiagReturnCode::Type Subfunction::verify(uint8_t const* const request, uint16_t const requestLength)
{
    if (requestLength < MINIMUM_REQUEST_LENGTH)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }
    if (request[0] != getImplementedRequest()[1])
    {
        return DiagReturnCode::NOT_RESPONSIBLE;
    }
    if (!getAllowedSessions().match(getSession()))
    {
        return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    return DiagReturnCode::OK;
}

} // namespace uds
