// Copyright 2024 Accenture.

#include "uds/base/ServiceWithAuthentication.h"

namespace uds
{
ServiceWithAuthentication::ServiceWithAuthentication(
    IDiagAuthenticator const& authenticator,
    uint8_t const service,
    DiagSession::DiagSessionMask const sessionMask)
: Service(service, sessionMask), fDiagAuthenticator(authenticator)
{}

ServiceWithAuthentication::ServiceWithAuthentication(
    IDiagAuthenticator const& authenticator,
    uint8_t const service,
    uint8_t const requestPayloadLength,
    uint8_t const responseLength,
    DiagSession::DiagSessionMask const sessionMask)
: Service(service, requestPayloadLength, responseLength, sessionMask)
, fDiagAuthenticator(authenticator)
{}

IDiagAuthenticator const& ServiceWithAuthentication::getDiagAuthenticator() const
{
    return fDiagAuthenticator;
}

} // namespace uds
