// Copyright 2024 Accenture.

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
