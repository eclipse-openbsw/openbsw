// Copyright 2024 Accenture.

#pragma once

#include "transport/AbstractTransportLayer.h"
#include "uds/resume/IResumableDiagDispatcher.h"

#include <gmock/gmock.h>

namespace uds
{
class DiagDispatcherMock : public IResumableDiagDispatcher
{
public:
    DiagDispatcherMock(IDiagSessionManager& sessionManager, DiagJobRoot& jobRoot)
    : IResumableDiagDispatcher(sessionManager, jobRoot)
    {}

    MOCK_METHOD(uint16_t, getSourceId, (), (const, override));

    MOCK_METHOD(
        uint8_t, dispatchTriggerEventRequest, (transport::TransportMessage & msg), (override));

    MOCK_METHOD(
        ::transport::AbstractTransportLayer::ErrorCode,
        resume,
        (::transport::TransportMessage & msg,
         ::transport::ITransportMessageProcessedListener* notificationListener),
        (override));

    MOCK_METHOD(
        IOutgoingDiagConnectionProvider::ErrorCode,
        getOutgoingDiagConnection,
        (uint16_t targetId,
         OutgoingDiagConnection*& pConnection,
         transport::TransportMessage* pRequestMessage),
        (override));
};

} // namespace uds
