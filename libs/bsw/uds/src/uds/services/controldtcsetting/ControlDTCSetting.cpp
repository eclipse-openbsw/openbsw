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
#include "udsDiagMgr/DiagMgr.h"

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
, _diagMgr(nullptr)
{
    enableSuppressPositiveResponse();
}

DiagReturnCode::Type ControlDTCSetting::process(
    IncomingDiagConnection& connection,
    uint8_t const* const request,
    uint16_t const /* requestLength */)
{
    uint8_t const dtcSettingType = request[0];
    Logger::debug(UDS, "ControlDTCSetting %d", dtcSettingType);

    switch (dtcSettingType)
    {
        case CONTROL_DTC_SETTING_ON:
        {
            // Enable DTC confirmation — forward to FaultManager
            if (_diagMgr != nullptr)
            {
                _diagMgr->setDTCSettingEnabled(true);
                Logger::debug(UDS, "DTC setting enabled via DiagMgr");
            }
            break;
        }
        case CONTROL_DTC_SETTING_OFF:
        {
            // Disable DTC confirmation — no new DTCs confirmed while OFF
            if (_diagMgr != nullptr)
            {
                _diagMgr->setDTCSettingEnabled(false);
                Logger::debug(UDS, "DTC setting disabled via DiagMgr");
            }
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
