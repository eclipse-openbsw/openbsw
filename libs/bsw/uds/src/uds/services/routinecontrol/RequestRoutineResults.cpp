// Copyright 2024 Accenture.

#include "uds/services/routinecontrol/RequestRoutineResults.h"

#include "uds/services/routinecontrol/RoutineControl.h"
#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

#include <type_traits>

namespace uds
{
uint8_t const RequestRoutineResults::sfImplementedRequest[2]
    = {ServiceId::ROUTINE_CONTROL,
       static_cast<::std::underlying_type<RoutineControl::Subfunction>::type>(
           RoutineControl::Subfunction::REQUEST_ROUTINE_RESULTS)};

RequestRoutineResults::RequestRoutineResults()
: Subfunction(&sfImplementedRequest[0], DiagSession::ALL_SESSIONS())
{
    setDefaultDiagReturnCode(DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE);
}

} // namespace uds
