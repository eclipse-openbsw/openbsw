/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/readdtcinformation/ReadDTCInformation.h"

#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/DiagSession.h"

#include <etl/array.h>

namespace uds
{
ReadDTCInformation::ReadDTCInformation()
: Service(ServiceId::READ_DTC_INFORMATION, DiagSession::ALL_SESSIONS())
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED);
}

DiagReturnCode::Type
ReadDTCInformation::verify(uint8_t const* const request, uint16_t const requestLength)
{
    return Service::verify(request, requestLength);
}

DiagReturnCode::Type ReadDTCInformation::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    if (requestLength < 1U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }
    ::etl::span<uint8_t const> const requestView(request, requestLength);
    uint8_t const subFunction = requestView[0U];
    switch (subFunction)
    {
        case REPORT_DTC_BY_STATUS_MASK:
        {
            if (requestLength < 2U)
            {
                return DiagReturnCode::ISO_INVALID_FORMAT;
            }
            ::etl::array<uint8_t, 5U> const dtcData = {
                0xFFU, // DTCStatusAvailabilityMask
                0x12U,
                0x34U,
                0x56U,
                0x09U // DTC 0x123456, status 0x09
            };
            PositiveResponse& response = connection.releaseRequestGetResponse();
            (void)response.appendUint8(subFunction);
            (void)response.appendData(dtcData.data(), static_cast<uint16_t>(dtcData.size()));
            (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
            return DiagReturnCode::OK;
        }
        // NOLINTNEXTLINE(bugprone-branch-clone): Remove suppression when cases are implemented.
        case REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER:
        case REPORT_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER:
        case REPORT_SUPPORTED_DTCS:
        {
            return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
        }
        default:
        {
            return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
        }
    }
}

} // namespace uds
