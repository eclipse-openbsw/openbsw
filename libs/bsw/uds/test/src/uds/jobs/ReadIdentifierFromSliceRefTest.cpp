// Copyright 2024 Accenture.

#include "uds/jobs/ReadIdentifierFromSliceRef.h"

#include "uds/connection/IncomingDiagConnectionMock.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <etl/memory.h>
#include <etl/span.h>
#include <transport/TransportMessage.h>
#include <transport/TransportMessageWithBuffer.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;
using namespace ::transport::test;

class ReadIdentifierFromSliceRefJobTest : public ::testing::Test
{
public:
    ReadIdentifierFromSliceRefJobTest()
    : _testSlice(TESTDATA)
    , _cut(
          TESTIDENTIFIER,
          _testSlice,
          AbstractDiagJob::DiagSessionMask::getInstance()
              << DiagSession::APPLICATION_DEFAULT_SESSION())
    {}

    virtual void SetUp() { _cut.setDefaultDiagSessionManager(_sessionManager); }

protected:
    static uint16_t const TESTIDENTIFIER = 0x4242U;
    static uint8_t const TESTDATA[4U];
    static uint8_t const TESTDATA2[6U];
    static uint8_t const SOURCE_ID = 0xF1U;
    static uint8_t const TARGET_ID = 0x10U;

    uint8_t _responseBuffer[10U];
    StrictMock<DiagSessionManagerMock> _sessionManager;
    StrictMock<IncomingDiagConnectionMock> _incomingDiagConnection{::async::CONTEXT_INVALID};
    ::etl::span<uint8_t const> _testSlice;
    ReadIdentifierFromSliceRef _cut;
};

uint8_t const ReadIdentifierFromSliceRefJobTest::TESTDATA[] = {0x00U, 0x01U, 0x02U, 0x03U};
uint8_t const ReadIdentifierFromSliceRefJobTest::TESTDATA2[]
    = {0x42U, 0x42U, 0x42, 0x42U, 0x42U, 0x42U};

TEST_F(ReadIdentifierFromSliceRefJobTest, execute_valid_request)
{
    uint8_t const VALID_REQUEST[] = {
        TESTIDENTIFIER >> 8U & 0xFFU,
        TESTIDENTIFIER & 0xFFU,
    };

    ::etl::mem_set(_responseBuffer, sizeof(_responseBuffer), static_cast<uint8_t>(0));
    _testSlice = TESTDATA;

    TransportMessageWithBuffer request(
        SOURCE_ID, TARGET_ID, VALID_REQUEST, AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    ::transport::TransportMessage responseMessage(_responseBuffer, sizeof(_responseBuffer));

    _incomingDiagConnection.requestMessage  = request.get();
    _incomingDiagConnection.responseMessage = &responseMessage;

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        _cut.execute(_incomingDiagConnection, VALID_REQUEST, sizeof(VALID_REQUEST)));

    EXPECT_THAT(
        ::etl::span<uint8_t const>(TESTDATA),
        ElementsAreArray(&_responseBuffer[2], sizeof(TESTDATA)));
}

TEST_F(ReadIdentifierFromSliceRefJobTest, execute_valid_request_and_change_slice)
{
    uint8_t const VALID_REQUEST[] = {
        TESTIDENTIFIER >> 8U & 0xFFU,
        TESTIDENTIFIER & 0xFFU,
    };

    ::etl::mem_set(_responseBuffer, sizeof(_responseBuffer), static_cast<uint8_t>(0));
    _testSlice = TESTDATA2;

    TransportMessageWithBuffer request(
        SOURCE_ID, TARGET_ID, VALID_REQUEST, AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    ::transport::TransportMessage responseMessage(_responseBuffer, sizeof(_responseBuffer));

    _incomingDiagConnection.requestMessage  = request.get();
    _incomingDiagConnection.responseMessage = &responseMessage;

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        _cut.execute(_incomingDiagConnection, VALID_REQUEST, sizeof(VALID_REQUEST)));

    EXPECT_THAT(
        ::etl::span<uint8_t const>(TESTDATA2),
        ElementsAreArray(&_responseBuffer[2], sizeof(TESTDATA2)));
}

} // namespace
