// Copyright 2024 Accenture.

#include "uds/base/SubfunctionWithAuthenticationAndSessionControl.h"

namespace uds
{
SubfunctionWithAuthenticationAndSessionControl::SubfunctionWithAuthenticationAndSessionControl(
    IDiagAuthenticator const& authenticator,
    IDiagSessionManager& sessionManager,
    uint8_t const* const implementedRequest,
    DiagSession::DiagSessionMask const sessionMask)
: SubfunctionWithAuthentication(authenticator, implementedRequest, sessionMask)
, fSessionManager(sessionManager)
{}

SubfunctionWithAuthenticationAndSessionControl::SubfunctionWithAuthenticationAndSessionControl(
    IDiagAuthenticator const& authenticator,
    IDiagSessionManager& sessionManager,
    uint8_t const* const implementedRequest,
    uint8_t const requestPayloadLength,
    uint8_t const responseLength,
    DiagSession::DiagSessionMask const sessionMask)
: SubfunctionWithAuthentication(
    authenticator, implementedRequest, requestPayloadLength, responseLength, sessionMask)
, fSessionManager(sessionManager)
{}

IDiagSessionManager& SubfunctionWithAuthenticationAndSessionControl::getDiagSessionManager()
{
    return fSessionManager;
}

IDiagSessionManager const&
SubfunctionWithAuthenticationAndSessionControl::getDiagSessionManager() const
{
    return fSessionManager;
}

} // namespace uds
