/********************************************************************************
 * Copyright (c) 2024 Accenture
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
/**
 * UDS service ReadDataByIdentifier (0x22).
 *
 */
class ReadDataByIdentifier : public Service
{
public:
    ReadDataByIdentifier();

private:
    static uint8_t const EXPECTED_REQUEST_LENGTH = 3U;

    /**
     * \see AbstractDiagJob::verify();
     */
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;
};

} // namespace uds
