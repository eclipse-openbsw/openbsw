/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/base/DiagJobWithAuthenticationAndSessionControl.h"

namespace uds
{
DiagJobWithAuthenticationAndSessionControl::DiagJobWithAuthenticationAndSessionControl(
    IDiagAuthenticator const& authenticator,
    IDiagSessionManager& sessionManager,
    uint8_t const* const implementedRequest,
    uint8_t const requestLength,
    uint8_t const prefixLength,
    DiagSessionMask const sessionMask)
: DiagJobWithAuthentication(
    authenticator, implementedRequest, requestLength, prefixLength, sessionMask)
, fSessionManager(sessionManager)
{}

DiagJobWithAuthenticationAndSessionControl::DiagJobWithAuthenticationAndSessionControl(
    IDiagAuthenticator const& authenticator,
    IDiagSessionManager& sessionManager,
    uint8_t const* const implementedRequest,
    uint8_t const requestLength,
    uint8_t const prefixLength,
    uint8_t const requestPayloadLength,
    uint8_t const responseLength,
    DiagSessionMask const sessionMask)
: DiagJobWithAuthentication(
    authenticator,
    implementedRequest,
    requestLength,
    prefixLength,
    requestPayloadLength,
    responseLength,
    sessionMask)
, fSessionManager(sessionManager)
{}

IDiagSessionManager& DiagJobWithAuthenticationAndSessionControl::getDiagSessionManager()
{
    return fSessionManager;
}

IDiagSessionManager const& DiagJobWithAuthenticationAndSessionControl::getDiagSessionManager() const
{
    return fSessionManager;
}

} // namespace uds
