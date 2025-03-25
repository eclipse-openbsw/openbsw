// Copyright 2024 Accenture.

#pragma once

#include "estd/uncopyable.h"
#include "uds/base/Service.h"

namespace uds
{
class InputOutputControlByIdentifier : public Service
{
    UNCOPYABLE(InputOutputControlByIdentifier);

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
