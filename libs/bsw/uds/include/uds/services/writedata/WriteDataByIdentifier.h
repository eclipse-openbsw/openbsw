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
 * UDS service WriteDataByIdentifier (0x2E).
 *
 */
class WriteDataByIdentifier : public Service
{
public:
    WriteDataByIdentifier();

private:
    /**
     * \see AbstractDiagJob::verify()
     */
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;
};

} // namespace uds
