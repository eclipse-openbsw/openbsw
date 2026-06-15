/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/cleardiagnosticinformation/ClearDiagnosticInformation.h"

#include "uds/connection/IncomingDiagConnectionMock.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <transport/TransportMessageWithBuffer.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::uds;
using namespace ::testing;
using namespace ::transport::test;

class ClearDiagnosticInformationTest : public ::testing::Test
{
public:
    ClearDiagnosticInformationTest()
    : fClearDiagnosticInformation()
    , fIncomingDiagConnection(::async::CONTEXT_INVALID)
    , fSessionManager()
    {}

    virtual void SetUp()
    {
        fClearDiagnosticInformation.setDefaultDiagSessionManager(fSessionManager);
    }

protected:
    ClearDiagnosticInformation fClearDiagnosticInformation;
    StrictMock<IncomingDiagConnectionMock> fIncomingDiagConnection;
    StrictMock<DiagSessionManagerMock> fSessionManager;
};

TEST_F(ClearDiagnosticInformationTest, execute_with_group_all_dtcs_should_return_DiagReturnCode_OK)
{
    uint8_t request[] = {
        0x14U, // ClearDiagnosticInformation SID
        0xFFU, // groupOfDTC high byte
        0xFFU, // groupOfDTC middle byte
        0xFFU  // groupOfDTC low byte
    };

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fClearDiagnosticInformation.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(
    ClearDiagnosticInformationTest,
    execute_with_unsupported_group_should_return_ISO_REQUEST_OUT_OF_RANGE)
{
    uint8_t request[] = {
        0x14U, // ClearDiagnosticInformation SID
        0x12U, // groupOfDTC high byte
        0x34U, // groupOfDTC middle byte
        0x56U  // groupOfDTC low byte
    };

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE,
        fClearDiagnosticInformation.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(
    ClearDiagnosticInformationTest,
    execute_with_powertrain_group_should_return_ISO_REQUEST_OUT_OF_RANGE)
{
    uint8_t request[] = {
        0x14U, // ClearDiagnosticInformation SID
        0x00U, // groupOfDTC high byte
        0x00U, // groupOfDTC middle byte
        0x00U  // groupOfDTC low byte
    };

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE,
        fClearDiagnosticInformation.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(ClearDiagnosticInformationTest, execute_with_invalid_length_should_return_ISO_INVALID_FORMAT)
{
    uint8_t request[] = {
        0x14U, // ClearDiagnosticInformation SID
        0xFFU, // groupOfDTC high byte
        0xFFU  // groupOfDTC middle byte (low byte missing)
    };

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        fClearDiagnosticInformation.execute(fIncomingDiagConnection, request, sizeof(request)));
}

} // anonymous namespace
