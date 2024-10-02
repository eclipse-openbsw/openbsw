// Copyright 2024 Accenture.

#include "uds/services/ecureset/ECUReset.h"

#include "uds/session/DiagSession.h"
#include "uds/session/IDiagSessionManager.h"

namespace uds
{
ECUReset::ECUReset() : Service(ServiceId::ECU_RESET, DiagSession::ALL_SESSIONS())
{
    enableSuppressPositiveResponse();
}

} // namespace uds
