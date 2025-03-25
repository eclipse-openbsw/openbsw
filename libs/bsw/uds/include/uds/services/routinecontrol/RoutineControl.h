// Copyright 2024 Accenture.

#pragma once

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
