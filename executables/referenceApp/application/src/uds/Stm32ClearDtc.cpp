// Copyright 2025 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include "uds/Stm32ClearDtc.h"

#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/DiagSession.h"

namespace uds
{

static uint8_t const SID_CLEAR_DTC = 0x14U;

Stm32ClearDtc::Stm32ClearDtc(Stm32DtcManager& dtcManager)
: Service(SID_CLEAR_DTC, 3U, 0U, DiagSession::ALL_SESSIONS()), _dtcManager(dtcManager)
{}

DiagReturnCode::Type Stm32ClearDtc::process(
    IncomingDiagConnection& connection,
    uint8_t const* const request,
    uint16_t const requestLength)
{
    if (requestLength < 3U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }

    uint32_t const groupOfDtc = (static_cast<uint32_t>(request[0]) << 16U)
                                | (static_cast<uint32_t>(request[1]) << 8U)
                                | static_cast<uint32_t>(request[2]);

    _dtcManager.clearByGroup(groupOfDtc);

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

} // namespace uds
