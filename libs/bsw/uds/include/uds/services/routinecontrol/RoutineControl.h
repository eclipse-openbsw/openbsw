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
 * UDS service RoutineControl (0x31).
 *
 */
class RoutineControl : public Service
{
public:
    RoutineControl();

    enum class Subfunction : uint8_t
    {
        START_ROUTINE           = 0x01U,
        STOP_ROUTINE            = 0x02U,
        REQUEST_ROUTINE_RESULTS = 0x03U,
    };

private:
    /**
     * \see AbstractDiagJob::verify()
     */
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;
};

} // namespace uds
