/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "transport/AbstractTransportLayer.h"
#include "transport/ITransportMessageProcessedListener.h"
#include "uds/IDiagDispatcher.h"

#include <gmock/gmock.h>

namespace uds
{
class DiagDispatcherMock : public IDiagDispatcher
{
public:
    DiagDispatcherMock(IDiagSessionManager& sessionManager) : IDiagDispatcher(sessionManager) {}

    MOCK_METHOD(
        transport::AbstractTransportLayer::ErrorCode,
        resume,
        (transport::TransportMessage & msg,
         transport::ITransportMessageProcessedListener* notificationListener));
};

} // namespace uds
