// Copyright 2024 Accenture.

#ifndef GUARD_28DCDBB6_1D3C_469F_9058_411A227947FA
#define GUARD_28DCDBB6_1D3C_469F_9058_411A227947FA

#include "estd/uncopyable.h"
#include "uds/base/Service.h"

namespace uds
{
/**
 * UDS service ReadDataByIdentifier (0x22).
 *
 */
class ReadDataByIdentifier : public Service
{
    UNCOPYABLE(ReadDataByIdentifier);

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

#endif // GUARD_28DCDBB6_1D3C_469F_9058_411A227947FA
