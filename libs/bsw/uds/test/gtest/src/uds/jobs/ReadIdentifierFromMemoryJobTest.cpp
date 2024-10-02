// Copyright 2024 Accenture.

#include "uds/connection/IncomingDiagConnectionMock.h"
#include "uds/jobs/ReadIdentifierFromMemory.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <transport/TransportMessage.h>
#include <transport/TransportMessageWithBuffer.h>

#include <estd/memory.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;
using namespace ::transport::test;

class ReadIdentifierFromMemoryJobTest : public ::testing::Test
{
public:
    ReadIdentifierFromMemoryJobTest()
    : _cut(
        TESTIDENTIFIER,
        TESTDATA,
        sizeof(TESTDATA),
        AbstractDiagJob::DiagSessionMask::getInstance()
            << DiagSession::APPLICATION_DEFAULT_SESSION())
    , _cutSlice(
          TESTIDENTIFIER,
          ::estd::slice<uint8_t const>(TESTDATA),
          AbstractDiagJob::DiagSessionMask::getInstance()
              << DiagSession::APPLICATION_DEFAULT_SESSION())
    {}

    virtual void SetUp() { _cut.setDefaultDiagSessionManager(_sessionManager); }

protected:
    static uint16_t const TESTIDENTIFIER = 0x4242U;
    static uint8_t const TESTDATA[4U];
    static uint8_t const SOURCE_ID = 0xF1U;
    static uint8_t const TARGET_ID = 0x10U;

    uint8_t _responseBuffer[10U];
    StrictMock<DiagSessionManagerMock> _sessionManager;
    StrictMock<IncomingDiagConnectionMock> _incomingDiagConnection{::async::CONTEXT_INVALID};
    ReadIdentifierFromMemory _cut;
    ReadIdentifierFromMemory _cutSlice;
};

uint8_t const ReadIdentifierFromMemoryJobTest::TESTDATA[] = {0x00U, 0x01U, 0x02U, 0x03U};

TEST_F(ReadIdentifierFromMemoryJobTest, execute_valid_request)
{
    uint8_t const VALID_REQUEST[] = {
        TESTIDENTIFIER >> 8U & 0xFFU,
        TESTIDENTIFIER & 0xFFU,
    };

    ::estd::memory::set(_responseBuffer, 0);

    TransportMessageWithBuffer request(
        SOURCE_ID, TARGET_ID, VALID_REQUEST, AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    ::transport::TransportMessage responseMessage(_responseBuffer, sizeof(_responseBuffer));

    _incomingDiagConnection.fpRequestMessage  = request.get();
    _incomingDiagConnection.fpResponseMessage = &responseMessage;

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        _cut.execute(_incomingDiagConnection, VALID_REQUEST, sizeof(VALID_REQUEST)));

    EXPECT_THAT(
        ::estd::slice<uint8_t const>(TESTDATA),
        ElementsAreArray(&_responseBuffer[2], sizeof(TESTDATA)));
}

TEST_F(ReadIdentifierFromMemoryJobTest, execute_valid_request_slice_constructor)
{
    uint8_t const VALID_REQUEST[] = {
        TESTIDENTIFIER >> 8U & 0xFFU,
        TESTIDENTIFIER & 0xFFU,
    };

    ::estd::memory::set(_responseBuffer, 0);

    TransportMessageWithBuffer request(
        SOURCE_ID, TARGET_ID, VALID_REQUEST, AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    ::transport::TransportMessage responseMessage(_responseBuffer, sizeof(_responseBuffer));

    _incomingDiagConnection.fpRequestMessage  = request.get();
    _incomingDiagConnection.fpResponseMessage = &responseMessage;

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        _cutSlice.execute(_incomingDiagConnection, VALID_REQUEST, sizeof(VALID_REQUEST)));

    EXPECT_THAT(
        ::estd::slice<uint8_t const>(TESTDATA),
        ElementsAreArray(&_responseBuffer[2], sizeof(TESTDATA)));
}

} // namespace
