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

#include "uds/base/Subfunction.h"

namespace uds
{
class IDiagAuthenticator;

/**
 * Base class for UDS subfunctions
 *
 */
class SubfunctionWithAuthentication : public Subfunction
{
public:
    SubfunctionWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const implementedRequest[],
        DiagSession::DiagSessionMask sessionMask);

    SubfunctionWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const implementedRequest[],
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask);

protected:
    /**
     * \see AbstractDiagJob::getDiagAuthenticator()
     */
    IDiagAuthenticator const& getDiagAuthenticator() const override;

private:
    IDiagAuthenticator const& fDiagAuthenticator;
};

} // namespace uds
