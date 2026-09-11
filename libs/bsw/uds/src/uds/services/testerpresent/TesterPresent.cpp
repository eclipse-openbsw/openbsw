/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/testerpresent/TesterPresent.h"

#include "uds/UdsLogger.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"
#include "uds/session/ProgrammingSession.h"

namespace uds
{
using ::util::logger::Logger;
using ::util::logger::UDS;

TesterPresent::TesterPresent()
: Service(
    ServiceId::TESTER_PRESENT,
    EXPECTED_REQUEST_LENGTH,
    RESPONSE_LENGTH,
    DiagSession::ALL_SESSIONS())
{
    enableSuppressPositiveResponse();
}

DiagReturnCode::Type TesterPresent::process(
    IncomingDiagConnection& connection,
    uint8_t const* const request,
    uint16_t const /* requestLength */)
{
    ::etl::span<uint8_t const> const requestView(request, 1U);
    if (requestView[0U] == TESTER_PRESENT_ANSWER)
    {
        PositiveResponse& response = connection.releaseRequestGetResponse();
        (void)response.appendUint8(TESTER_PRESENT_ANSWER);
        (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
        return DiagReturnCode::OK;
    }

    return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
}

} // namespace uds
