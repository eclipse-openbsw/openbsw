/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
