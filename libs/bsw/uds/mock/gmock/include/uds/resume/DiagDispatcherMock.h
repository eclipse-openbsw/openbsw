// Copyright 2024 Accenture.

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

    MOCK_METHOD(uint16_t, getDispatcherSourceId, (), (const));

    MOCK_METHOD(
        transport::AbstractTransportLayer::ErrorCode,
        resume,
        (transport::TransportMessage & msg,
         transport::ITransportMessageProcessedListener* notificationListener));
};

} // namespace uds
