/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/routinecontrol/StartRoutine.h"

#include "uds/services/routinecontrol/RoutineControl.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

#include <etl/type_traits.h>

namespace uds
{
::etl::array<uint8_t, 2U> const StartRoutine::sfImplementedRequest
    = {ServiceId::ROUTINE_CONTROL,
       static_cast<::etl::underlying_type<RoutineControl::Subfunction>::type>(
           RoutineControl::Subfunction::START_ROUTINE)};

StartRoutine::StartRoutine() : Subfunction(sfImplementedRequest.data(), DiagSession::ALL_SESSIONS())
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE);
}

} // namespace uds
