// Copyright 2024 Accenture.

#pragma once

#include "StubMock.h"
#include "uds/authentication/DefaultDiagAuthenticator.h"
#include "uds/authentication/IDiagAuthenticator.h"
#include "uds/jobs/DataIdentifierJob.h"
#include "uds/jobs/RoutineControlJob.h"

#include <gmock/gmock.h>

namespace uds
{
class AbstractDiagJobMockHelper : public StubMock
{
public:
    AbstractDiagJobMockHelper() : StubMock() {}

    MOCK_METHOD(IDiagAuthenticator const&, getDiagAuthenticator, (), (const));
    MOCK_METHOD(IDiagSessionManager&, getDiagSessionManager, (), ());
    MOCK_METHOD(IDiagSessionManager const&, getDiagSessionManager, (), (const));
    MOCK_METHOD(IDiagAuthenticator const&, getDefaultDiagAuthenticator, ());

    MOCK_METHOD3(
        execute,
        DiagReturnCode::Type(
            IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength));
    MOCK_METHOD(
        DiagReturnCode::Type, process, (IncomingDiagConnection&, uint8_t const[], uint16_t));
    MOCK_METHOD(void, responseSent, (IncomingDiagConnection&, AbstractDiagJob::ResponseSendResult));
    MOCK_METHOD(void, setDefaultDiagSessionManager, (IDiagSessionManager&));
    MOCK_METHOD(bool, compare, (uint8_t const[], uint8_t const[], uint16_t));

    static AbstractDiagJobMockHelper& instance()
    {
        static AbstractDiagJobMockHelper instance;
        return instance;
    }
};

class RoutineControlJobMockHelper : public StubMock
{
public:
    RoutineControlJobMockHelper() : StubMock() {}

    MOCK_METHOD(DiagReturnCode::Type, verify, (uint8_t const[], uint16_t));
    MOCK_METHOD(
        DiagReturnCode::Type, process, (IncomingDiagConnection&, uint8_t const[], uint16_t));

    static RoutineControlJobMockHelper& instance()
    {
        static RoutineControlJobMockHelper instance;
        return instance;
    }
};

class DataIdentifierJobMockHelper : public StubMock
{
public:
    DataIdentifierJobMockHelper() : StubMock() {}

    MOCK_METHOD(DiagReturnCode::Type, verify, (uint8_t const[], uint16_t));

    static DataIdentifierJobMockHelper& instance()
    {
        static DataIdentifierJobMockHelper instance;
        return instance;
    }
};

} // namespace uds
