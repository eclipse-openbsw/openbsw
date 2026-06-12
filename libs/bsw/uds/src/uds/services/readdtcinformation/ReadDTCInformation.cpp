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
    uint8_t const subFunction = request[0];
    switch (subFunction)
    {
        case REPORT_DTC_BY_STATUS_MASK:
        {
            if (requestLength < 2U)
            {
                return DiagReturnCode::ISO_INVALID_FORMAT;
            }
            uint8_t const dtcData[] = {
                0xFFU, // DTCStatusAvailabilityMask
                0x12U,
                0x34U,
                0x56U,
                0x09U // DTC 0x123456, status 0x09
            };
            PositiveResponse& response = connection.releaseRequestGetResponse();
            (void)response.appendUint8(subFunction);
            (void)response.appendData(dtcData, sizeof(dtcData));
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
