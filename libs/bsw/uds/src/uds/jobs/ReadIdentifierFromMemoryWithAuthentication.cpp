// Copyright 2024 Accenture.

#include "uds/jobs/ReadIdentifierFromMemoryWithAuthentication.h"

#include "uds/authentication/IDiagAuthenticator.h"
#include "uds/base/AbstractDiagJob.h"
#include "uds/jobs/ReadIdentifierFromMemory.h"

namespace uds
{
ReadIdentifierFromMemoryWithAuthentication::ReadIdentifierFromMemoryWithAuthentication(
    IDiagAuthenticator const& authenticator,
    uint16_t const identifier,
    uint8_t const* const responseData,
    uint16_t const responseLength,
    DiagSessionMask const sessionMask)
: ReadIdentifierFromMemoryWithAuthentication(
    authenticator,
    identifier,
    ::estd::slice<uint8_t const>::from_pointer(responseData, responseLength),
    sessionMask)
{}

ReadIdentifierFromMemoryWithAuthentication::ReadIdentifierFromMemoryWithAuthentication(
    IDiagAuthenticator const& authenticator,
    uint16_t const identifier,
    ::estd::slice<uint8_t const> const& responseData,
    DiagSessionMask const sessionMask)
: ReadIdentifierFromMemory(identifier, responseData, sessionMask), _authenticator{authenticator}
{}

IDiagAuthenticator const& ReadIdentifierFromMemoryWithAuthentication::getDiagAuthenticator() const
{
    return _authenticator;
}

} // namespace uds
