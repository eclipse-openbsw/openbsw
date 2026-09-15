/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/connection/IncomingDiagConnectionMock.h"
#include "uds/jobs/WriteIdentifierToMemory.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <etl/memory.h>
#include <etl/span.h>
#include <transport/TransportMessage.h>
#include <transport/TransportMessageWithBuffer.h>

#include <array>

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

    std::array<uint8_t, 4U> _testBuffer{};
    StrictMock<DiagSessionManagerMock> _sessionManager;
    StrictMock<IncomingDiagConnectionMock> _incomingDiagConnection{::async::CONTEXT_INVALID};
    WriteIdentifierToMemory _cut;
};

TEST_F(WriteIdentifierToMemoryJobTest, execute_valid_request)
{
    std::array<uint8_t, 6U> const validRequest
        = {TESTIDENTIFIER >> 8U & 0xFFU, TESTIDENTIFIER & 0xFFU, 0x42U, 0x42U, 0x42U, 0x42U};

    ::etl::mem_set(_testBuffer.data(), _testBuffer.size(), static_cast<uint8_t>(0));

    TransportMessageWithBuffer request(
        SOURCE_ID,
        TARGET_ID,
        ::etl::span<uint8_t const>(validRequest.data(), validRequest.size()),
        AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    _incomingDiagConnection.requestMessage = request.get();

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        _cut.execute(_incomingDiagConnection, validRequest.data(), validRequest.size()));

    EXPECT_THAT(_testBuffer, ElementsAre(0x42U, 0x42U, 0x42U, 0x42U));
}

TEST_F(WriteIdentifierToMemoryJobTest, execute_wrong_size_request)
{
    std::array<uint8_t, 4U> const validRequest
        = {TESTIDENTIFIER >> 8U & 0xFFU, TESTIDENTIFIER & 0xFFU, 0x42U, 0x42U};

    ::etl::mem_set(_testBuffer.data(), _testBuffer.size(), static_cast<uint8_t>(0));

    TransportMessageWithBuffer request(
        SOURCE_ID,
        TARGET_ID,
        ::etl::span<uint8_t const>(validRequest.data(), validRequest.size()),
        AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    _incomingDiagConnection.requestMessage = request.get();

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        _cut.execute(_incomingDiagConnection, validRequest.data(), validRequest.size()));

    EXPECT_THAT(_testBuffer, ElementsAre(0, 0, 0, 0));

    std::array<uint8_t, 4U> const invalidRequest = {0x00U, 0x00U, 0x00U, 0x00U};

    EXPECT_EQ(
        DiagReturnCode::NOT_RESPONSIBLE,
        _cut.execute(_incomingDiagConnection, invalidRequest.data(), invalidRequest.size()));
}

TEST_F(WriteIdentifierToMemoryJobTest, execute_too_short_request)
{
    std::array<uint8_t, 1U> const invalidRequest = {TESTIDENTIFIER >> 8U & 0xFFU};

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        _cut.execute(_incomingDiagConnection, invalidRequest.data(), invalidRequest.size()));
}

} // namespace
