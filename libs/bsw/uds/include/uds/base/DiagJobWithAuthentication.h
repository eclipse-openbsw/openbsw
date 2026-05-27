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

#include "uds/base/AbstractDiagJob.h"

namespace uds
{
class IDiagAuthenticator;

class DiagJobWithAuthentication : public AbstractDiagJob
{
public:
    explicit DiagJobWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

    explicit DiagJobWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

protected:
    IDiagAuthenticator const& getDiagAuthenticator() const override;

private:
    IDiagAuthenticator const& fAuthenticator;
};

} // namespace uds
