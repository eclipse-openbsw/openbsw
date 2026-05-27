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
class InputOutputControlByIdentifier : public Service
{
public:
    struct IOControlParameter
    {
        enum ID
        {
            RETURN_CONTROL_TO_ECU = 0x00U,
            FREEZE_CURRENT_STATE  = 0x02U,
            SHORT_TERM_ADJUSTMENT = 0x03U,
            NONE                  = 0xFFU
        };
    };

    InputOutputControlByIdentifier();

private:
    /**
     * \see AbstractDiagJob::verify();
     */
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;
};
} // namespace uds
