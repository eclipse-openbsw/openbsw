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
#include "uds/connection/PositiveResponse.h"
#include "uds/diagmgr/DiagMgr.h"
#include "uds/faultmgr/FaultManagementConfig.h"
#include "uds/session/DiagSession.h"

namespace uds
{

ReadDTCInformation::ReadDTCInformation()
: Service(ServiceId::READ_DTC_INFORMATION, DiagSession::ALL_SESSIONS()), _diagMgr(nullptr)
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED);
}

DiagReturnCode::Type
ReadDTCInformation::verify(uint8_t const* const request, uint16_t const requestLength)
{
    // Minimum request is [SID=0x19 | subFunction] = 2 bytes total.
    // The dispatcher passes the full raw payload including the SID byte, so
    // requestLength must be >= 2 (SID + at least one sub-function byte).
    if (requestLength < 2U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }
    return Service::verify(request, requestLength);
}

DiagReturnCode::Type ReadDTCInformation::process(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    uint8_t const subFunction = request[0];
    switch (subFunction)
    {
        case REPORT_DTC_BY_STATUS_MASK:
            return handleReportByStatusMask(connection, request, requestLength);

        case REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER:
            return handleReportSnapshotByDTCNumber(connection, request, requestLength);

        case REPORT_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER:
            return handleReportExtendedDataByDTCNumber(connection, request, requestLength);

        case REPORT_SUPPORTED_DTCS:
            return handleReportSupportedDTCs(connection, request, requestLength);

        default: return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
    }
}

// ---------------------------------------------------------------------------
// 0x02 — ReportDTCByStatusMask (DGN-FEL001/009)
// ---------------------------------------------------------------------------
DiagReturnCode::Type ReadDTCInformation::handleReportByStatusMask(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    if (requestLength < 2U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }
    uint8_t const statusMask = request[1];

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(static_cast<uint8_t>(REPORT_DTC_BY_STATUS_MASK));
    (void)response.appendUint8(FaultManagementConfig::DTC_STATUS_AVAILABILITY_MASK);

    if (_diagMgr != nullptr)
    {
        // Real DTC data from FaultManager (DGN-FEL001/009)
        uint8_t dtcBuffer[RESPONSE_BUFFER_SIZE] = {};
        uint16_t dtcLength                      = 0U;
        (void)_diagMgr->getDTCList(statusMask, dtcBuffer, sizeof(dtcBuffer), dtcLength);
        (void)response.appendData(dtcBuffer, static_cast<size_t>(dtcLength));
    }
    else
    {
        // Stub response (no DiagMgr wired yet)
        static uint8_t const stubDtcData[] = {
            0x12U, 0x34U, 0x56U, 0x09U // DTC 0x123456, status 0x09
        };
        (void)response.appendData(stubDtcData, sizeof(stubDtcData));
    }

    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

// ---------------------------------------------------------------------------
// 0x04 — ReportDTCSnapshotRecordByDTCNumber (Freeze Frame, DGN-FEL003/012)
// ---------------------------------------------------------------------------
DiagReturnCode::Type ReadDTCInformation::handleReportSnapshotByDTCNumber(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    // Request: [subFunc(1) | DTC_Hi(1) | DTC_Mid(1) | DTC_Lo(1) | recordNumber(1)]
    if (requestLength < 5U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }
    uint32_t const dtcId = (static_cast<uint32_t>(request[1]) << 16U)
                           | (static_cast<uint32_t>(request[2]) << 8U)
                           | static_cast<uint32_t>(request[3]);
    uint8_t const recordNumber = request[4];

    if (_diagMgr == nullptr)
    {
        return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
    }

    uint8_t ffBuffer[RESPONSE_BUFFER_SIZE] = {};
    uint16_t ffLength                      = 0U;
    bool const found
        = _diagMgr->getFreezeFrame(dtcId, recordNumber, ffBuffer, sizeof(ffBuffer), ffLength);
    if (!found)
    {
        return DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE;
    }

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(static_cast<uint8_t>(REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER));
    (void)response.appendUint24(dtcId);
    (void)response.appendUint8(_diagMgr->getFaultStatus(dtcId));
    (void)response.appendData(ffBuffer, static_cast<size_t>(ffLength));
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

// ---------------------------------------------------------------------------
// 0x06 — ReportDTCExtendedDataRecordByDTCNumber (Timestamps/Counters, DGN-FEL011)
// ---------------------------------------------------------------------------
DiagReturnCode::Type ReadDTCInformation::handleReportExtendedDataByDTCNumber(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    // Request: [subFunc(1) | DTC_Hi(1) | DTC_Mid(1) | DTC_Lo(1) | recordNumber(1)]
    if (requestLength < 5U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }
    uint32_t const dtcId = (static_cast<uint32_t>(request[1]) << 16U)
                           | (static_cast<uint32_t>(request[2]) << 8U)
                           | static_cast<uint32_t>(request[3]);
    uint8_t const recordNumber = request[4];

    if (_diagMgr == nullptr)
    {
        return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
    }

    uint8_t extBuffer[RESPONSE_BUFFER_SIZE] = {};
    uint16_t extLength                      = 0U;
    bool const found
        = _diagMgr->getExtendedData(dtcId, recordNumber, extBuffer, sizeof(extBuffer), extLength);
    if (!found)
    {
        return DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE;
    }

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(static_cast<uint8_t>(REPORT_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER));
    (void)response.appendUint24(dtcId);
    (void)response.appendUint8(_diagMgr->getFaultStatus(dtcId));
    (void)response.appendData(extBuffer, static_cast<size_t>(extLength));
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

// ---------------------------------------------------------------------------
// 0x0A — ReportSupportedDTCs
// ---------------------------------------------------------------------------
DiagReturnCode::Type ReadDTCInformation::handleReportSupportedDTCs(
    IncomingDiagConnection& connection,
    uint8_t const* const /*request*/,
    uint16_t const /*requestLength*/)
{
    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(static_cast<uint8_t>(REPORT_SUPPORTED_DTCS));
    (void)response.appendUint8(FaultManagementConfig::DTC_STATUS_AVAILABILITY_MASK);

    if (_diagMgr != nullptr)
    {
        // Report all DTCs regardless of status (mask=0xFF)
        uint8_t dtcBuffer[RESPONSE_BUFFER_SIZE] = {};
        uint16_t dtcLength                      = 0U;
        (void)_diagMgr->getDTCList(0xFFU, dtcBuffer, sizeof(dtcBuffer), dtcLength);
        (void)response.appendData(dtcBuffer, static_cast<size_t>(dtcLength));
    }

    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

} // namespace uds
