/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/routinecontrol/RequestRoutineResults.h"

#include "uds/services/routinecontrol/RoutineControl.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

#include <etl/type_traits.h>

namespace uds
{
::etl::array<uint8_t, 2U> const RequestRoutineResults::sfImplementedRequest
    = {ServiceId::ROUTINE_CONTROL,
       static_cast<::etl::underlying_type<RoutineControl::Subfunction>::type>(
           RoutineControl::Subfunction::REQUEST_ROUTINE_RESULTS)};

RequestRoutineResults::RequestRoutineResults()
: Subfunction(sfImplementedRequest.data(), DiagSession::ALL_SESSIONS())
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE);
}

} // namespace uds
