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
    DiagDispatcherMock(IDiagSessionManager& sessionManager, DiagJobRoot& jobRoot)
    : IDiagDispatcher(sessionManager, jobRoot)
    {}

    MOCK_METHOD(uint16_t, getSourceId, (), (const));

    MOCK_METHOD(uint8_t, dispatchTriggerEventRequest, (transport::TransportMessage & msg));

    MOCK_METHOD(
        transport::AbstractTransportLayer::ErrorCode,
        resume,
        (transport::TransportMessage & msg,
         transport::ITransportMessageProcessedListener* notificationListener));
};

} // namespace uds
