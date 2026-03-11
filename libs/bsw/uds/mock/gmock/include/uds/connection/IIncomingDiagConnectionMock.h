#pragma once

#include "StubMock.h"

#include "uds/connection/IIncomingDiagConnection.h"


#include <gmock/gmock.h>

#include <memory>

namespace uds
{
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