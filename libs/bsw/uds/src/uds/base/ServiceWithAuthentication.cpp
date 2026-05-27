/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
