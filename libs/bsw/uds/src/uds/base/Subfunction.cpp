/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/base/Subfunction.h"

#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
Subfunction::Subfunction(
    uint8_t const* const implementedRequest, DiagSession::DiagSessionMask const sessionMask)
: AbstractDiagJob(
    implementedRequest,
    2U,
    1U,
    AbstractDiagJob::VARIABLE_REQUEST_LENGTH,
    AbstractDiagJob::VARIABLE_RESPONSE_LENGTH,
    sessionMask)
{}

Subfunction::Subfunction(
    uint8_t const* const implementedRequest,
    uint8_t const requestPayloadLength,
    uint8_t const responseLength,
    DiagSession::DiagSessionMask const sessionMask)
: AbstractDiagJob(implementedRequest, 2U, 1U, requestPayloadLength, responseLength, sessionMask)
{}

DiagReturnCode::Type Subfunction::verify(uint8_t const* const request, uint16_t const requestLength)
{
    if (requestLength < MINIMUM_REQUEST_LENGTH)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }
    ::etl::span<uint8_t const> const requestView(request, requestLength);
    if (requestView[0U] != getImplementedRequestView()[1U])
    {
        return DiagReturnCode::NOT_RESPONSIBLE;
    }
    if (!getAllowedSessions().match(getSession()))
    {
        return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    return DiagReturnCode::OK;
}

} // namespace uds
