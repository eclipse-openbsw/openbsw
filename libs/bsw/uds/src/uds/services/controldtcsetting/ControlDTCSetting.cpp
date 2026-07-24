/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/controldtcsetting/ControlDTCSetting.h"

#include "uds/UdsLogger.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg): Logger API uses C-style varargs.
using ::util::logger::Logger;
using ::util::logger::UDS;

ControlDTCSetting::ControlDTCSetting()
: Service(
    ServiceId::CONTROL_DTC_SETTING,
    EXPECTED_REQUEST_LENGTH,
    RESPONSE_LENGTH,
    DiagSessionMask::getInstance() << DiagSession::APPLICATION_EXTENDED_SESSION())
{
    enableSuppressPositiveResponse();
}

DiagReturnCode::Type ControlDTCSetting::process(
    IncomingDiagConnection& connection,
    uint8_t const* const request,
    uint16_t const /* requestLength */)
{
    ::etl::span<uint8_t const> const requestView(request, 1U);
    uint8_t const dtcSettingType = requestView[0U];
    Logger::debug(UDS, "ControlDTCSetting %d", dtcSettingType);
    switch (dtcSettingType)
    {
        // NOLINTNEXTLINE(bugprone-branch-clone): Remove suppression when cases are implemented.
        case CONTROL_DTC_SETTING_ON:
        {
            // Allow creation of new DTCs. Intentionally not implemented.
            break;
        }
        case CONTROL_DTC_SETTING_OFF:
        {
            // Disallow creation of new DTCs. Intentionally not implemented.
            break;
        }
        default:
        {
            return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
        }
    }
    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(dtcSettingType);
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
} // namespace uds
