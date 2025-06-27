// Copyright 2024 Accenture.

#pragma once
#include "uds/resume/IResumableDiagDispatcher.h"

#include <gmock/gmock.h>

namespace uds
{
class ResumableDiagDispatcherMock : public IResumableDiagDispatcher
{
public:
    ResumableDiagDispatcherMock(IDiagSessionManager& sessionManager, DiagJobRoot& jobRoot)
    : IResumableDiagDispatcher(sessionManager, jobRoot)
    {}

    MOCK_METHOD(
        ::transport::AbstractTransportLayer::ErrorCode,
        resume,
        (::transport::TransportMessage & transportMessage,
         ::transport::ITransportMessageProcessedListener* notificationListener),
        (override));
    MOCK_METHOD(uint16_t, getSourceId, (void), (const, override));
    MOCK_METHOD(
        uint8_t, dispatchTriggerEventRequest, (::transport::TransportMessage & msg), (override));
    MOCK_METHOD(
        ErrorCode,
        getOutgoingDiagConnection,
        (uint16_t targetId,
         OutgoingDiagConnection*& connection,
         ::transport::TransportMessage* pRequestMessage = nullptr),
        (override));

#ifdef IS_VARIANT_HANDLING_NEEDED
    MOCK_METHOD(void, setSourceId, (uint16_t), (override));
#endif
};
} // namespace uds
