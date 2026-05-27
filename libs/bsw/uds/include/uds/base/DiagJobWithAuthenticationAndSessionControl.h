/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "uds/base/DiagJobWithAuthentication.h"

namespace uds
{
class IDiagSessionManager;

class DiagJobWithAuthenticationAndSessionControl : public DiagJobWithAuthentication
{
public:
    explicit DiagJobWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

    explicit DiagJobWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

protected:
    /**
     * \see AbstractDiagJob::getDiagSessionManager()
     */
    IDiagSessionManager& getDiagSessionManager() override;

    /**
     * \see AbstractDiagJob::getDiagSessionManager()
     */
    IDiagSessionManager const& getDiagSessionManager() const override;

private:
    IDiagSessionManager& fSessionManager;
};

} // namespace uds
