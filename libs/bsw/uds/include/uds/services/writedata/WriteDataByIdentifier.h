// Copyright 2024 Accenture.

#pragma once

#include "estd/uncopyable.h"
#include "uds/base/Service.h"

namespace uds
{
/**
 * UDS service WriteDataByIdentifier (0x2E).
 *
 */
class WriteDataByIdentifier : public Service
{
    UNCOPYABLE(WriteDataByIdentifier);

public:
    WriteDataByIdentifier();

private:
    /**
     * \see AbstractDiagJob::verify()
     */
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;
};

} // namespace uds
