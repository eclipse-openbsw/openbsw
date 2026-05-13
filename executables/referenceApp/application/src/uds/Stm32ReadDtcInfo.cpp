// Copyright 2025 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include "uds/Stm32ReadDtcInfo.h"

#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/DiagSession.h"

namespace uds
{

static uint8_t const SID_READ_DTC_INFO    = 0x19U;
static uint8_t const SF_REPORT_NUM_BY_MASK = 0x01U;
static uint8_t const SF_REPORT_BY_MASK     = 0x02U;
static uint8_t const SF_REPORT_SUPPORTED   = 0x0AU;

// ISO 14229: DTC status availability mask (all bits supported)
static uint8_t const STATUS_AVAILABILITY_MASK = 0xFFU;

Stm32ReadDtcInfo::Stm32ReadDtcInfo(Stm32DtcManager const& dtcManager)
: Service(SID_READ_DTC_INFO, DiagSession::ALL_SESSIONS()), _dtcManager(dtcManager)
{}

DiagReturnCode::Type Stm32ReadDtcInfo::process(
    IncomingDiagConnection& connection,
    uint8_t const* const request,
    uint16_t const requestLength)
{
    if (requestLength < 1U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }

    uint8_t const subFunction = request[0];

    switch (subFunction)
    {
        case SF_REPORT_BY_MASK:
        {
            if (requestLength < 2U)
            {
                return DiagReturnCode::ISO_INVALID_FORMAT;
            }
            return handleReportByStatusMask(connection, request[1]);
        }
        case SF_REPORT_SUPPORTED:
        {
            return handleReportSupportedDtc(connection);
        }
        default:
        {
            return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
        }
    }
}

DiagReturnCode::Type Stm32ReadDtcInfo::handleReportNumberByStatusMask(
    IncomingDiagConnection& connection, uint8_t const statusMask)
{
    uint16_t const count = _dtcManager.getCountByStatusMask(statusMask);

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(SF_REPORT_NUM_BY_MASK);
    (void)response.appendUint8(STATUS_AVAILABILITY_MASK);
    // DTC format identifier (ISO 14229-1 format)
    (void)response.appendUint8(0x01U);
    // DTC count high/low
    (void)response.appendUint8(static_cast<uint8_t>((count >> 8U) & 0xFFU));
    (void)response.appendUint8(static_cast<uint8_t>(count & 0xFFU));
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

DiagReturnCode::Type Stm32ReadDtcInfo::handleReportByStatusMask(
    IncomingDiagConnection& connection, uint8_t const statusMask)
{
    uint8_t dtcBuffer[DTC_BUFFER_SIZE];
    uint8_t const dtcCount
        = _dtcManager.getDtcsByStatusMask(statusMask, dtcBuffer, DTC_BUFFER_SIZE);

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(SF_REPORT_BY_MASK);
    (void)response.appendUint8(STATUS_AVAILABILITY_MASK);

    for (uint16_t i = 0U; i < (static_cast<uint16_t>(dtcCount) * 4U); ++i)
    {
        (void)response.appendUint8(dtcBuffer[i]);
    }

    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

DiagReturnCode::Type Stm32ReadDtcInfo::handleReportSupportedDtc(
    IncomingDiagConnection& connection)
{
    uint8_t dtcBuffer[DTC_BUFFER_SIZE];
    uint8_t const dtcCount = _dtcManager.getSupportedDtcs(dtcBuffer, DTC_BUFFER_SIZE);

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(SF_REPORT_SUPPORTED);
    (void)response.appendUint8(STATUS_AVAILABILITY_MASK);

    for (uint16_t i = 0U; i < (static_cast<uint16_t>(dtcCount) * 4U); ++i)
    {
        (void)response.appendUint8(dtcBuffer[i]);
    }

    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

} // namespace uds
