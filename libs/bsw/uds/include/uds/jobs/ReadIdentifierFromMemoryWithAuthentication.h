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

#include "uds/DiagReturnCode.h"
#include "uds/authentication/IDiagAuthenticator.h"
#include "uds/base/AbstractDiagJob.h"
#include "uds/jobs/ReadIdentifierFromMemory.h"
#include "uds/session/DiagSession.h"

#include <etl/span.h>

namespace uds
{
/**
 * Generic implementation of a ReadDataByIdentifier which can respond with a fixed
 * value given by a pointer to a memory location.
 */
class ReadIdentifierFromMemoryWithAuthentication : public ReadIdentifierFromMemory
{
public:
    ReadIdentifierFromMemoryWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint16_t identifier,
        uint8_t const* responseData,
        uint16_t responseLength,
        DiagSessionMask sessionMask = DiagSession::ALL_SESSIONS());

    ReadIdentifierFromMemoryWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint16_t identifier,
        ::etl::span<uint8_t const> const& responseData,
        DiagSessionMask sessionMask = DiagSession::ALL_SESSIONS());

protected:
    /**
     * \see AbstractDiagJob::getDiagAuthenticator();
     */
    IDiagAuthenticator const& getDiagAuthenticator() const override;

private:
    IDiagAuthenticator const& _authenticator;
};

} // namespace uds
