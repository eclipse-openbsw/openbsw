// Copyright 2024 Accenture.

#include "uds/services/routinecontrol/StartRoutine.h"

#include "uds/services/routinecontrol/RoutineControl.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

#include <etl/type_traits.h>

namespace uds
{
uint8_t const StartRoutine::sfImplementedRequest[2]
    = {ServiceId::ROUTINE_CONTROL,
       static_cast<::etl::underlying_type<RoutineControl::Subfunction>::type>(
           RoutineControl::Subfunction::START_ROUTINE)};

StartRoutine::StartRoutine() : Subfunction(&sfImplementedRequest[0], DiagSession::ALL_SESSIONS())
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE);
}

} // namespace uds
