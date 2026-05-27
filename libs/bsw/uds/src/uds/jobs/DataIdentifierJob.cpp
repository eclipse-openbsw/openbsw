/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
