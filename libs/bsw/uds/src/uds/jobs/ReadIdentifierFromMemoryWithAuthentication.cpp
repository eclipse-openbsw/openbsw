/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
    ::etl::span<uint8_t const>(responseData, responseLength),
    sessionMask)
{}

ReadIdentifierFromMemoryWithAuthentication::ReadIdentifierFromMemoryWithAuthentication(
    IDiagAuthenticator const& authenticator,
    uint16_t const identifier,
    ::etl::span<uint8_t const> const& responseData,
    DiagSessionMask const sessionMask)
: ReadIdentifierFromMemory(identifier, responseData, sessionMask), _authenticator{authenticator}
{}

IDiagAuthenticator const& ReadIdentifierFromMemoryWithAuthentication::getDiagAuthenticator() const
{
    return _authenticator;
}

} // namespace uds
