/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "uds/base/Service.h"
#include "udsFaultMgr/FaultManagementConfig.h"

namespace uds
{
class IncomingDiagConnection;
class DiagMgr;

/**
 * UDS service ReadDTCInformation (0x19).
 *
 * Backed by DiagMgr/FaultManager for real DTC data
 * Supported sub-functions:
 *   0x02 — ReportDTCByStatusMask
 *   0x04 — ReportDTCSnapshotRecordByDTCNumber
 *   0x06 — ReportDTCExtendedDataRecordByDTCNumber
 *   0x0A — ReportSupportedDTCs
 */
class ReadDTCInformation : public Service
{
public:
    enum SubFunction
    {
        REPORT_DTC_BY_STATUS_MASK                     = 0x02,
        REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER      = 0x04,
        REPORT_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER = 0x06,
        REPORT_SUPPORTED_DTCS                         = 0x0A
    };

    explicit ReadDTCInformation();

    /**
     * Wire the DiagMgr backend. Call during UdsSystem::init() before addDiagJobs().
     * When set, all sub-functions use real FaultManager data.
     * Without it, a safe stub response is returned.
     */
    void setDiagMgr(DiagMgr* diagMgr) { _diagMgr = diagMgr; }

private:
    static constexpr uint16_t RESPONSE_BUFFER_SIZE = 256U;

    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    DiagReturnCode::Type handleReportByStatusMask(
        IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength);

    DiagReturnCode::Type handleReportSnapshotByDTCNumber(
        IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength);

    DiagReturnCode::Type handleReportExtendedDataByDTCNumber(
        IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength);

    DiagReturnCode::Type handleReportSupportedDTCs(
        IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength);

    DiagMgr* _diagMgr;
};

} // namespace uds
