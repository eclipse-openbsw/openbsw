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

namespace uds
{
class IncomingDiagConnection;

/**
 * UDS service ReadDTCInformation (0x19).
 *
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

private:
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;

    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;
};

} // namespace uds
