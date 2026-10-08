/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/base/Service.h"

#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
namespace
{
::etl::array<uint8_t, 1U> makeImplementedRequest(uint8_t const service) { return {{service}}; }
} // namespace

Service::Service(uint8_t const service, DiagSession::DiagSessionMask const sessionMask)
: AbstractDiagJob(
    makeImplementedRequest(service),
    0U,
    AbstractDiagJob::VARIABLE_REQUEST_LENGTH,
    AbstractDiagJob::VARIABLE_RESPONSE_LENGTH,
    sessionMask)
{
    init();
}

Service::Service(
    uint8_t const service,
    uint8_t const requestPayloadLength,
    uint8_t const responseLength,
    DiagSession::DiagSessionMask const sessionMask)
: AbstractDiagJob(
    makeImplementedRequest(service), 0U, requestPayloadLength, responseLength, sessionMask)
{
    init();
}

void Service::init() { setDefaultDiagReturnCode(DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED); }

DiagReturnCode::Type
Service::verify(uint8_t const* const request, uint16_t const /* requestLength */)
{
    ::etl::span<uint8_t const> const requestView(request, 1U);
    if (requestView[0U] != getImplementedRequestView()[0U])
    {
        return DiagReturnCode::NOT_RESPONSIBLE;
    }
    if (!fAllowedSessions.match(getSession()))
    {
        return DiagReturnCode::ISO_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    return DiagReturnCode::OK;
}

} // namespace uds
