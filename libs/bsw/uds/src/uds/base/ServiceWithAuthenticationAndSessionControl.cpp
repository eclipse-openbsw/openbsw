/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
