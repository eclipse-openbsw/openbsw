/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "uds/Stm32DtcManager.h"
#include "uds/base/Service.h"

namespace uds
{
/**
 * ReadDTCInformation (SID 0x19).
 *
 * Subfunctions:
 *   0x01 = reportNumberOfDTCByStatusMask
 *   0x02 = reportDTCByStatusMask
 *   0x0A = reportSupportedDTC
 */
class Stm32ReadDtcInfo : public Service
{
public:
    explicit Stm32ReadDtcInfo(Stm32DtcManager const& dtcManager);

private:
    static uint16_t const DTC_BUFFER_SIZE = 128U;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

    DiagReturnCode::Type
    handleReportNumberByStatusMask(IncomingDiagConnection& connection, uint8_t statusMask);
    DiagReturnCode::Type
    handleReportByStatusMask(IncomingDiagConnection& connection, uint8_t statusMask);
    DiagReturnCode::Type handleReportSupportedDtc(IncomingDiagConnection& connection);

    Stm32DtcManager const& _dtcManager;
};

} // namespace uds
