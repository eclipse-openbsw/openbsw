// Copyright 2024 Accenture.

#include "uds/base/ServiceWithAuthenticationAndSessionControl.h"

namespace uds
{
ServiceWithAuthenticationAndSessionControl::ServiceWithAuthenticationAndSessionControl(
    IDiagAuthenticator const& authenticator,
    IDiagSessionManager& sessionManager,
    uint8_t const service,
    DiagSession::DiagSessionMask const sessionMask)
: ServiceWithAuthentication(authenticator, service, sessionMask), fSessionManager(sessionManager)
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE);
}

ServiceWithAuthenticationAndSessionControl::ServiceWithAuthenticationAndSessionControl(
    IDiagAuthenticator const& authenticator,
    IDiagSessionManager& sessionManager,
    uint8_t const service,
    uint8_t const requestPayloadLength,
    uint8_t const responseLength,
    DiagSession::DiagSessionMask const sessionMask)
: ServiceWithAuthentication(
    authenticator, service, requestPayloadLength, responseLength, sessionMask)
, fSessionManager(sessionManager)
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE);
}

IDiagSessionManager& ServiceWithAuthenticationAndSessionControl::getDiagSessionManager()
{
    return fSessionManager;
}

IDiagSessionManager const& ServiceWithAuthenticationAndSessionControl::getDiagSessionManager() const
{
    return fSessionManager;
}

} // namespace uds
