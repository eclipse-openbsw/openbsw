/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/readdata/ReadDataByIdentifier.h"

#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
ReadDataByIdentifier::ReadDataByIdentifier()
: Service(ServiceId::READ_DATA_BY_IDENTIFIER, DiagSession::ALL_SESSIONS())
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE);
}

DiagReturnCode::Type
ReadDataByIdentifier::verify(uint8_t const* const request, uint16_t const requestLength)
{
    DiagReturnCode::Type result = Service::verify(request, requestLength);
    if (result == DiagReturnCode::OK)
    {
        if (requestLength != EXPECTED_REQUEST_LENGTH)
        {
            result = DiagReturnCode::ISO_INVALID_FORMAT;
        }
    }
    return result;
}

} // namespace uds
