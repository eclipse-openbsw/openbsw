/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/routinecontrol/RoutineControl.h"

#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
RoutineControl::RoutineControl() : Service(ServiceId::ROUTINE_CONTROL, DiagSession::ALL_SESSIONS())
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED);
    enableSuppressPositiveResponse();
}

DiagReturnCode::Type
RoutineControl::verify(uint8_t const* const request, uint16_t const requestLength)
{
    DiagReturnCode::Type result = Service::verify(request, requestLength);
    if (result == DiagReturnCode::OK)
    {
        if (requestLength < 4U)
        {
            result = DiagReturnCode::ISO_INVALID_FORMAT;
        }
    }
    return result;
}

} // namespace uds
