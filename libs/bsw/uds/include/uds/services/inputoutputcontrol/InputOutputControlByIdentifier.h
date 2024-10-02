// Copyright 2024 Accenture.

#ifndef GUARD_5E99DA0B_F4E2_457F_8E60_6526A61BAA3C
#define GUARD_5E99DA0B_F4E2_457F_8E60_6526A61BAA3C

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

#endif // GUARD_5E99DA0B_F4E2_457F_8E60_6526A61BAA3C
