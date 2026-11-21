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
#include "uds/session/DiagSession.h"
#include "udsDiagMgr/DiagMgr.h"
#include "udsFaultMgr/FaultManagementConfig.h"

#include <etl/array.h>
#include <etl/span.h>
#include <etl/unaligned_type.h>

namespace uds
{
namespace
{
// Scratch buffer large enough for the biggest payload requested from DiagMgr:
//  - DTC list:       MAX_DTC_RECORDS * 4 bytes (3-byte DTC + 1-byte status)
//  - Freeze frame:   1 byte record number + FREEZE_FRAME_SNAPSHOT_SIZE
//  - Extended data:  20 bytes (record 0x01)
constexpr uint16_t DTC_LIST_BUFFER_SIZE
    = static_cast<uint16_t>(FaultManagementConfig::MAX_DTC_RECORDS * 4U);
constexpr uint16_t FREEZE_FRAME_BUFFER_SIZE
    = static_cast<uint16_t>(1U + FaultManagementConfig::FREEZE_FRAME_SNAPSHOT_SIZE);
constexpr uint16_t EXTENDED_DATA_BUFFER_SIZE = 20U;

constexpr uint16_t RESPONSE_BUFFER_SIZE
    = (DTC_LIST_BUFFER_SIZE > FREEZE_FRAME_BUFFER_SIZE)
          ? ((DTC_LIST_BUFFER_SIZE > EXTENDED_DATA_BUFFER_SIZE) ? DTC_LIST_BUFFER_SIZE
                                                                : EXTENDED_DATA_BUFFER_SIZE)
          : ((FREEZE_FRAME_BUFFER_SIZE > EXTENDED_DATA_BUFFER_SIZE) ? FREEZE_FRAME_BUFFER_SIZE
                                                                    : EXTENDED_DATA_BUFFER_SIZE);
} // namespace

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
    if (requestLength < 1U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }
    ::etl::span<uint8_t const> const requestView(request, requestLength);
    uint8_t const subFunction = requestView[0U];
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
// 0x02 — ReportDTCByStatusMask
// ---------------------------------------------------------------------------
DiagReturnCode::Type ReadDTCInformation::handleReportByStatusMask(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    if (requestLength < 2U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }
    ::etl::span<uint8_t const> const requestView(request, requestLength);
    uint8_t const statusMask = requestView[1U];

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(static_cast<uint8_t>(REPORT_DTC_BY_STATUS_MASK));
    (void)response.appendUint8(FaultManagementConfig::DTC_STATUS_AVAILABILITY_MASK);

    if (_diagMgr != nullptr)
    {
        ::etl::array<uint8_t, RESPONSE_BUFFER_SIZE> dtcBuffer{};
        uint16_t dtcLength = 0U;
        (void)_diagMgr->getDTCList(
            statusMask, dtcBuffer.data(), static_cast<uint16_t>(dtcBuffer.size()), dtcLength);
        (void)response.appendData(dtcBuffer.data(), static_cast<size_t>(dtcLength));
    }
    else
    {
        // Stub response (no DiagMgr wired yet)
        static ::etl::array<uint8_t, 4U> const stubDtcData
            = {0x12U, 0x34U, 0x56U, 0x09U}; // DTC 0x123456, status 0x09
        (void)response.appendData(stubDtcData.data(), stubDtcData.size());
    }

    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

// ---------------------------------------------------------------------------
// 0x04 — ReportDTCSnapshotRecordByDTCNumber (Freeze Frame,)
// ---------------------------------------------------------------------------
DiagReturnCode::Type ReadDTCInformation::handleReportSnapshotByDTCNumber(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    // Request: [subFunc(1) | DTC_Hi(1) | DTC_Mid(1) | DTC_Lo(1) | recordNumber(1)]
    if (requestLength < 5U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }
    ::etl::span<uint8_t const> const requestView(request, requestLength);
    ::etl::array<uint8_t, 4U> const dtcBuf
        = {0U, requestView[1U], requestView[2U], requestView[3U]};
    uint32_t const dtcId       = etl::be_uint32_t(dtcBuf.data()).value();
    uint8_t const recordNumber = requestView[4U];

    if (_diagMgr == nullptr)
    {
        return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
    }

    ::etl::array<uint8_t, RESPONSE_BUFFER_SIZE> ffBuffer{};
    uint16_t ffLength = 0U;
    bool const found  = _diagMgr->getFreezeFrame(
        dtcId, recordNumber, ffBuffer.data(), static_cast<uint16_t>(ffBuffer.size()), ffLength);
    if (!found)
    {
        return DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE;
    }

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(static_cast<uint8_t>(REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER));
    (void)response.appendUint24(dtcId);
    (void)response.appendUint8(_diagMgr->getFaultStatus(dtcId));
    (void)response.appendData(ffBuffer.data(), static_cast<size_t>(ffLength));
    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

// ---------------------------------------------------------------------------
// 0x06 — ReportDTCExtendedDataRecordByDTCNumber (Timestamps/Counters, )
// ---------------------------------------------------------------------------
DiagReturnCode::Type ReadDTCInformation::handleReportExtendedDataByDTCNumber(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
{
    // Request: [subFunc(1) | DTC_Hi(1) | DTC_Mid(1) | DTC_Lo(1) | recordNumber(1)]
    if (requestLength < 5U)
    {
        return DiagReturnCode::ISO_INVALID_FORMAT;
    }
    ::etl::span<uint8_t const> const requestView(request, requestLength);
    ::etl::array<uint8_t, 4U> const dtcBuf
        = {0U, requestView[1U], requestView[2U], requestView[3U]};
    uint32_t const dtcId       = etl::be_uint32_t(dtcBuf.data()).value();
    uint8_t const recordNumber = requestView[4U];

    if (_diagMgr == nullptr)
    {
        return DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED;
    }

    ::etl::array<uint8_t, RESPONSE_BUFFER_SIZE> extBuffer{};
    uint16_t extLength = 0U;
    bool const found   = _diagMgr->getExtendedData(
        dtcId, recordNumber, extBuffer.data(), static_cast<uint16_t>(extBuffer.size()), extLength);
    if (!found)
    {
        return DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE;
    }

    PositiveResponse& response = connection.releaseRequestGetResponse();
    (void)response.appendUint8(static_cast<uint8_t>(REPORT_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER));
    (void)response.appendUint24(dtcId);
    (void)response.appendUint8(_diagMgr->getFaultStatus(dtcId));
    (void)response.appendData(extBuffer.data(), static_cast<size_t>(extLength));
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
        ::etl::array<uint8_t, RESPONSE_BUFFER_SIZE> dtcBuffer{};
        uint16_t dtcLength = 0U;
        (void)_diagMgr->getDTCList(
            0xFFU, dtcBuffer.data(), static_cast<uint16_t>(dtcBuffer.size()), dtcLength);
        (void)response.appendData(dtcBuffer.data(), static_cast<size_t>(dtcLength));
    }

    (void)connection.sendPositiveResponseInternal(response.getLength(), *this);
    return DiagReturnCode::OK;
}

} // namespace uds
