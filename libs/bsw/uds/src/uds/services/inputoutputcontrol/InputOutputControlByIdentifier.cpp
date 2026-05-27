/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/inputoutputcontrol/InputOutputControlByIdentifier.h"

#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
InputOutputControlByIdentifier::InputOutputControlByIdentifier()
: Service(ServiceId::INPUT_OUTPUT_CONTROL_BY_IDENTIFIER, DiagSession::ALL_SESSIONS())
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE);
}

DiagReturnCode::Type
InputOutputControlByIdentifier::verify(uint8_t const* const request, uint16_t const requestLength)
{
    static constexpr uint8_t MIN_REQUEST_LEN = 4U;

    DiagReturnCode::Type result = Service::verify(request, requestLength);
    if (DiagReturnCode::OK == result)
    {
        if (requestLength < MIN_REQUEST_LEN)
        {
            result = DiagReturnCode::ISO_INVALID_FORMAT;
        }

        /**
         * ServiceId: 1byte
         * JobID: 2 bytes
         * ControlParam: 1byte
         * */
        uint8_t const controlParamOffset = 3U;
        InputOutputControlByIdentifier::IOControlParameter::ID const controlParameter
            = static_cast<IOControlParameter::ID>(request[controlParamOffset]);

        if ((controlParameter == IOControlParameter::SHORT_TERM_ADJUSTMENT)
            && (requestLength < static_cast<uint8_t>(MIN_REQUEST_LEN + 1U)))
        {
            result = DiagReturnCode::ISO_INVALID_FORMAT;
        }
    }

    return result;
}
} // namespace uds
