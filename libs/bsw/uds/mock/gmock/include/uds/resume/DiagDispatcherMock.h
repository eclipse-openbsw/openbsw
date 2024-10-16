// Copyright 2024 Accenture.

#ifndef GUARD_0B51DD83_4F84_4853_93DA_9595EAB415D4
#define GUARD_0B51DD83_4F84_4853_93DA_9595EAB415D4

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

    MOCK_CONST_METHOD0(getSourceId, uint16_t());

    MOCK_METHOD1(dispatchTriggerEventRequest, uint8_t(transport::TransportMessage& msg));

    MOCK_METHOD2(
        resume,
        transport::AbstractTransportLayer::ErrorCode(
            transport::TransportMessage& msg,
            transport::ITransportMessageProcessedListener* notificationListener));

    MOCK_METHOD3(
        getOutgoingDiagConnection,
        IOutgoingDiagConnectionProvider::ErrorCode(
            uint16_t targetId,
            OutgoingDiagConnection*& pConnection,
            transport::TransportMessage* pRequestMessage));
};

} // namespace uds

#endif // GUARD_0B51DD83_4F84_4853_93DA_9595EAB415D4
