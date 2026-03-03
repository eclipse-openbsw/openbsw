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

    MOCK_METHOD(void, terminate, (), (override));

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
    MOCK_METHOD(void, terminate, (), (override));

    MOCK_METHOD(void, addIdentifier, ());
    MOCK_METHOD(void, triggerNextNestedRequest, ());
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

class IIncomingDiagConnectionMock : 
    public IIncomingDiagConnection
    , public StubMock
{
public:    
    IIncomingDiagConnectionMock() : IIncomingDiagConnection{}, StubMock() {}
    
    MOCK_METHOD(void, open, (bool activatePending), (override));
    MOCK_METHOD(void, terminate, (), (override));
    MOCK_METHOD(void, addIdentifier, (), (override));
    MOCK_METHOD(uint16_t, getNumIdentifiers, (), (const, override));
    MOCK_METHOD(uint8_t, getIdentifier, (uint16_t idx), (const, override));
    MOCK_METHOD(uint16_t, getMaximumResponseLength, (), (const, override));
    MOCK_METHOD(PositiveResponse&, releaseRequestGetResponse, (), (override));
    MOCK_METHOD(ErrorCode, sendPositiveResponse, (AbstractDiagJob& sender), (override));
    MOCK_METHOD(ErrorCode, sendPositiveResponseInternal, (uint16_t length, AbstractDiagJob& sender), (override));
    MOCK_METHOD(ErrorCode, sendNegativeResponse, (uint8_t responseCode, AbstractDiagJob& sender), (override));
    MOCK_METHOD(void, suppressPositiveResponse, (), (override));
    MOCK_METHOD(void, disableResponseTimeout, (), (override));
    MOCK_METHOD(void, disableGlobalTimeout, (), (override));
    MOCK_METHOD(bool, isBusy, (), (const, override));
    MOCK_METHOD(DiagReturnCode::Type, startNestedRequest, (AbstractDiagJob& sender, NestedDiagRequest& nestedRequest, uint8_t const request[], uint16_t requestLength), (override));
    MOCK_METHOD(bool, terminateNestedRequest, (), (override));
    MOCK_METHOD(ErrorCode, sendResponse, (), (override));
    MOCK_METHOD(uint8_t, getServiceId, (), (const, override));
    MOCK_METHOD(uint16_t, getTargetAddress, (), (const, override));
    MOCK_METHOD(uint16_t, getSourceAddress, (), (const, override));
    MOCK_METHOD(void, setDiagSessionManager, (IDiagSessionManager* sessionManager), (override));
    MOCK_METHOD(void, setSourceAddress, (uint16_t address), (override));
    MOCK_METHOD(void, setTargetAddress, (uint16_t address), (override));
    MOCK_METHOD(void, setResponseSourceAddress, (uint16_t address), (override));
    MOCK_METHOD(void, setServiceId, (uint8_t id), (override));
    
    virtual ~IIncomingDiagConnectionMock() = default;
    
    static IIncomingDiagConnectionMock& instance()
    {
        static IIncomingDiagConnectionMock instance;
        return instance;
    }
};

} // namespace uds
