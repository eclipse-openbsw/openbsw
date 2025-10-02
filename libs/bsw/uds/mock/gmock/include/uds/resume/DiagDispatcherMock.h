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

    MOCK_CONST_METHOD0(getSourceId, uint16_t());

    MOCK_METHOD1(dispatchTriggerEventRequest, uint8_t(transport::TransportMessage& msg));

    MOCK_METHOD2(
        resume,
        transport::AbstractTransportLayer::ErrorCode(
            transport::TransportMessage& msg,
            transport::ITransportMessageProcessedListener* notificationListener));
};

} // namespace uds
