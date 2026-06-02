/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/DemoReadDtcInfo.h"

#include "uds/UdsConstants.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/session/DiagSession.h"

#include <etl/array.h>

namespace uds
{

static uint8_t const SF_REPORT_NUM_BY_MASK = 0x01U;
static uint8_t const SF_REPORT_BY_MASK     = 0x02U;
static uint8_t const SF_REPORT_SUPPORTED   = 0x0AU;

// ISO 14229: DTC status availability mask (all bits supported)
static uint8_t const STATUS_AVAILABILITY_MASK = 0xFFU;

DemoReadDtcInfo::DemoReadDtcInfo(DemoDtcManager const& dtcManager)
: Service(ServiceId::READ_DTC_INFORMATION, DiagSession::ALL_SESSIONS()), _dtcManager(dtcManager)
{}

DiagReturnCode::Type DemoReadDtcInfo::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    if (requestLength < 1U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }

    uint8_t const subFunction = request[0];

    switch (subFunction)
    {
        case SF_REPORT_NUM_BY_MASK:
        {
            if (requestLength < 2U)
            {
                return DiagReturnCode::ISO_INVALID_FORMAT;
            }
            return handleReportNumberByStatusMask(connection, request[1]);
        }
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

DiagReturnCode::Type DemoReadDtcInfo::handleReportNumberByStatusMask(
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

DiagReturnCode::Type DemoReadDtcInfo::handleReportByStatusMask(
    IncomingDiagConnection& connection, uint8_t const statusMask)
{
    ::etl::array<uint8_t, DTC_BUFFER_SIZE> dtcBuffer;
    size_t const dtcCount = _dtcManager.getDtcsByStatusMask(statusMask, dtcBuffer);

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(SF_REPORT_BY_MASK);
    (void)response.appendUint8(STATUS_AVAILABILITY_MASK);

    for (size_t i = 0U; i < (dtcCount * 4U); ++i)
    {
        (void)response.appendUint8(dtcBuffer[i]);
    }

    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

DiagReturnCode::Type DemoReadDtcInfo::handleReportSupportedDtc(IncomingDiagConnection& connection)
{
    ::etl::array<uint8_t, DTC_BUFFER_SIZE> dtcBuffer;
    size_t const dtcCount = _dtcManager.getSupportedDtcs(dtcBuffer);

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(SF_REPORT_SUPPORTED);
    (void)response.appendUint8(STATUS_AVAILABILITY_MASK);

    for (size_t i = 0U; i < (dtcCount * 4U); ++i)
    {
        (void)response.appendUint8(dtcBuffer[i]);
    }

    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

} // namespace uds
