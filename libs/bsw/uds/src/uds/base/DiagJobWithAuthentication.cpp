// Copyright 2024 Accenture.

#include "uds/base/DiagJobWithAuthentication.h"

namespace uds
{
DiagJobWithAuthentication::DiagJobWithAuthentication(
    IDiagAuthenticator const& authenticator,
    uint8_t const* const implementedRequest,
    uint8_t const requestLength,
    uint8_t const prefixLength,
    DiagSessionMask const sessionMask)
: AbstractDiagJob(implementedRequest, requestLength, prefixLength, sessionMask)
, fAuthenticator(authenticator)
{}

DiagJobWithAuthentication::DiagJobWithAuthentication(
    IDiagAuthenticator const& authenticator,
    uint8_t const* const implementedRequest,
    uint8_t const requestLength,
    uint8_t const prefixLength,
    uint8_t const requestPayloadLength,
    uint8_t const responseLength,
    DiagSessionMask const sessionMask)
: AbstractDiagJob(
    implementedRequest,
    requestLength,
    prefixLength,
    requestPayloadLength,
    responseLength,
    sessionMask)
, fAuthenticator(authenticator)
{}

IDiagAuthenticator const& DiagJobWithAuthentication::getDiagAuthenticator() const
{
    return fAuthenticator;
}

} // namespace uds
