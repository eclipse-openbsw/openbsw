// Copyright 2024 Accenture.

#ifndef GUARD_A3316EC5_4B46_48EC_B023_A8F3BC468B1D
#define GUARD_A3316EC5_4B46_48EC_B023_A8F3BC468B1D

#include "estd/uncopyable.h"
#include "uds/base/Service.h"

namespace uds
{
/**
 * UDS service RoutineControl (0x31).
 *
 */
class RoutineControl : public Service
{
    UNCOPYABLE(RoutineControl);

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

#endif // GUARD_A3316EC5_4B46_48EC_B023_A8F3BC468B1D
