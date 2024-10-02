// Copyright 2024 Accenture.

#include "uds/connection/IncomingDiagConnectionMock.h"
#include "uds/jobs/WriteIdentifierToMemory.h"
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

class WriteIdentifierToMemoryJobTest : public ::testing::Test
{
public:
    WriteIdentifierToMemoryJobTest()
    : _cut(
        TESTIDENTIFIER,
        _testBuffer,
        AbstractDiagJob::DiagSessionMask::getInstance()
            << DiagSession::APPLICATION_DEFAULT_SESSION())
    {}

    virtual void SetUp() { _cut.setDefaultDiagSessionManager(_sessionManager); }

protected:
    static uint16_t const TESTIDENTIFIER = 0x4242U;
    static uint8_t const SOURCE_ID       = 0xF1U;
    static uint8_t const TARGET_ID       = 0x10U;

    uint8_t _testBuffer[4U];
    StrictMock<DiagSessionManagerMock> _sessionManager;
    StrictMock<IncomingDiagConnectionMock> _incomingDiagConnection{::async::CONTEXT_INVALID};
    WriteIdentifierToMemory _cut;
};

TEST_F(WriteIdentifierToMemoryJobTest, execute_valid_request)
{
    uint8_t const VALID_REQUEST[]
        = {TESTIDENTIFIER >> 8U & 0xFFU, TESTIDENTIFIER & 0xFFU, 0x42U, 0x42U, 0x42U, 0x42U};

    ::estd::memory::set(_testBuffer, 0);

    TransportMessageWithBuffer request(
        SOURCE_ID, TARGET_ID, VALID_REQUEST, AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    _incomingDiagConnection.fpRequestMessage = request.get();

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        _cut.execute(_incomingDiagConnection, VALID_REQUEST, sizeof(VALID_REQUEST)));

    EXPECT_THAT(
        ::estd::slice<uint8_t const>(_testBuffer),
        ElementsAreArray(&VALID_REQUEST[2], sizeof(_testBuffer)));
}

TEST_F(WriteIdentifierToMemoryJobTest, execute_wrong_size_request)
{
    uint8_t const VALID_REQUEST[]
        = {TESTIDENTIFIER >> 8U & 0xFFU, TESTIDENTIFIER & 0xFFU, 0x42U, 0x42U};

    ::estd::memory::set(_testBuffer, 0);

    TransportMessageWithBuffer request(
        SOURCE_ID, TARGET_ID, VALID_REQUEST, AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    _incomingDiagConnection.fpRequestMessage = request.get();

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        _cut.execute(_incomingDiagConnection, VALID_REQUEST, sizeof(VALID_REQUEST)));

    EXPECT_THAT(::estd::slice<uint8_t const>(_testBuffer), ElementsAre(0, 0, 0, 0));

    uint8_t const INVALID_REQUEST[] = {0x00U, 0x00U, 0x00U, 0x00U};

    EXPECT_EQ(
        DiagReturnCode::NOT_RESPONSIBLE,
        _cut.execute(_incomingDiagConnection, INVALID_REQUEST, sizeof(INVALID_REQUEST)));
}

} // namespace
