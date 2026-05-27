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

#include "uds/base/Service.h"

namespace uds
{
class IDiagAuthenticator;

/**
 * Helper base class for UDS services.
 *
 *
 * \see Service
 */
class ServiceWithAuthentication : public Service
{
public:
    explicit ServiceWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const service,
        DiagSession::DiagSessionMask sessionMask);

    explicit ServiceWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const service,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask);

protected:
    /**
     * \see AbstractDiagJob::getDiagAuthenticator();
     */
    IDiagAuthenticator const& getDiagAuthenticator() const override;

private:
    IDiagAuthenticator const& fDiagAuthenticator;
};

} // namespace uds
