/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
