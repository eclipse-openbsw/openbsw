// Copyright 2024 Accenture.

#include "uds/jobs/DataIdentifierJob.h"

#include "uds/services/readdata/ReadDataByIdentifier.h"

namespace uds
{
DiagReturnCode::Type
DataIdentifierJob::verify(uint8_t const* const request, uint16_t const /* requestLength */)
{
    if (!compare(request, getImplementedRequest() + 1U, 2U))
    {
        return DiagReturnCode::NOT_RESPONSIBLE;
    }
    return DiagReturnCode::OK;
}

} // namespace uds
