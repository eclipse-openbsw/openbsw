// Copyright 2024 Accenture.

#pragma once

#include "ResponseMock.h"
#include "StubMock.h"
#include "uds/base/AbstractDiagJob.h"
#include "uds/connection/IncomingDiagConnection.h"

#include <transport/ITransportMessageProcessedListener.h>

#include <gmock/gmock.h>

#include <memory>

namespace uds
{
class IncomingDiagConnectionMock : public IncomingDiagConnection
{
public:
    IncomingDiagConnectionMock(::async::ContextType const diagContext)
    : IncomingDiagConnection(diagContext)
    {}

    MOCK_METHOD0(terminate, void());

    void setDiagSessionManager(IDiagSessionManager& diagSessionManager)
    {
        fpDiagSessionManager = &diagSessionManager;
    }

    using IncomingDiagConnection::terminateNestedRequest;
};

class AbstractDiagConnectionMockHelper : public StubMock
{
public:
    AbstractDiagConnectionMockHelper() : StubMock() {}

    MOCK_METHOD(uint8_t, getServiceId, (), (const));
    MOCK_METHOD(uint16_t, getSourceId, (), (const));
    MOCK_METHOD(uint16_t, getTargetId, (), (const));

    static AbstractDiagConnectionMockHelper& instance()
    {
        static AbstractDiagConnectionMockHelper instance;
        return instance;
    }
};

class IncomingDiagConnectionMockHelper
: public IncomingDiagConnection
, public StubMock
{
public:
    IncomingDiagConnectionMockHelper(::async::ContextType const diagContext)
    : IncomingDiagConnection(diagContext), StubMock()
    {}

    /* NOTE: Terminate is mocked every time this way.
       It is okay for now, since it is pure virtual method,
       and is expected multiple times in the functional tests,
       Therefore no reason to make a stub version for this function.
    */
    MOCK_METHOD0(terminate, void());

    MOCK_METHOD(void, addIdentifier, ());
    MOCK_METHOD0(triggerNextNestedRequest, void());
    MOCK_METHOD(
        void,
        transportMessageProcessed,
        (transport::TransportMessage&,
         transport::ITransportMessageProcessedListener::ProcessingResult));
    MOCK_METHOD(void, asyncSendNegativeResponse, (uint8_t, AbstractDiagJob*));
    MOCK_METHOD(void, asyncSendPositiveResponse, (uint16_t, AbstractDiagJob*));
    MOCK_METHOD(
        void,
        asyncTransportMessageProcessed,
        (transport::TransportMessage*,
         transport::ITransportMessageProcessedListener::ProcessingResult));
    MOCK_METHOD(PositiveResponse&, releaseRequestGetResponse, ());
    MOCK_METHOD(::uds::ErrorCode, sendNegativeResponse, (uint8_t, AbstractDiagJob&));
    MOCK_METHOD(uint8_t, getIdentifier, (uint16_t));
    virtual ~IncomingDiagConnectionMockHelper() = default;

    static IncomingDiagConnectionMockHelper& instance()
    {
        static IncomingDiagConnectionMockHelper instance(::async::CONTEXT_INVALID);
        return instance;
    }
};

} // namespace uds

