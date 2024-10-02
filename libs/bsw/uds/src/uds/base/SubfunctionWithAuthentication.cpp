// Copyright 2024 Accenture.

#include "uds/base/SubfunctionWithAuthentication.h"

namespace uds
{
SubfunctionWithAuthentication::SubfunctionWithAuthentication(
    IDiagAuthenticator const& authenticator,
    uint8_t const* const implementedRequest,
    DiagSession::DiagSessionMask const sessionMask)
: Subfunction(implementedRequest, sessionMask), fDiagAuthenticator(authenticator)
{}

SubfunctionWithAuthentication::SubfunctionWithAuthentication(
    IDiagAuthenticator const& authenticator,
    uint8_t const* const implementedRequest,
    uint8_t const requestPayloadLength,
    uint8_t const responseLength,
    DiagSession::DiagSessionMask const sessionMask)
: Subfunction(implementedRequest, requestPayloadLength, responseLength, sessionMask)
, fDiagAuthenticator(authenticator)
{}

IDiagAuthenticator const& SubfunctionWithAuthentication::getDiagAuthenticator() const
{
    return fDiagAuthenticator;
}

} // namespace uds
