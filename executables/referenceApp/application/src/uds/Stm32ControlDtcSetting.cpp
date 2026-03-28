// Copyright 2025 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include "uds/Stm32ControlDtcSetting.h"

#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/DiagSession.h"

namespace uds
{

static uint8_t const SID_CONTROL_DTC_SETTING = 0x85U;
static uint8_t const SF_ON                   = 0x01U;
static uint8_t const SF_OFF                  = 0x02U;

Stm32ControlDtcSetting::Stm32ControlDtcSetting(Stm32DtcManager& dtcManager)
: Service(SID_CONTROL_DTC_SETTING, 1U, 1U, DiagSession::ALL_SESSIONS()), _dtcManager(dtcManager)
{}

DiagReturnCode::Type Stm32ControlDtcSetting::process(
    IncomingDiagConnection& connection,
    uint8_t const* const request,
    uint16_t const requestLength)
{
    if (requestLength < 1U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }

    uint8_t const subFunction = request[0];

    if (subFunction == SF_ON)
    {
        _dtcManager.setDtcSettingEnabled(true);
    }
    else if (subFunction == SF_OFF)
    {
        _dtcManager.setDtcSettingEnabled(false);
    }
    else
    {
        return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
    }

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(subFunction);
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

} // namespace uds
