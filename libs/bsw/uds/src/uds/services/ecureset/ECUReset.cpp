/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
