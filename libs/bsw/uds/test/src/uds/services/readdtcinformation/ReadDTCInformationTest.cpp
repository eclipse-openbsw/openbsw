/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/readdtcinformation/ReadDTCInformation.h"

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

class ReadDTCInformationTest : public ::testing::Test
{
public:
    ReadDTCInformationTest()
    : fReadDTCInformation(), fIncomingDiagConnection(::async::CONTEXT_INVALID), fSessionManager()
    {}

    virtual void SetUp() { fReadDTCInformation.setDefaultDiagSessionManager(fSessionManager); }

protected:
    ReadDTCInformation fReadDTCInformation;
    StrictMock<IncomingDiagConnectionMock> fIncomingDiagConnection;
    StrictMock<DiagSessionManagerMock> fSessionManager;
};

TEST_F(ReadDTCInformationTest, execute_with_subfunction_0x02_and_valid_status_mask_should_return_OK)
{
    uint8_t request[] = {
        0x19U, // ReadDTCInformation SID
        0x02U, // reportDTCByStatusMask
        0xFFU  // DTCStatusMask
    };

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fReadDTCInformation.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(
    ReadDTCInformationTest,
    execute_with_subfunction_0x02_without_status_mask_should_return_ISO_INVALID_FORMAT)
{
    uint8_t request[] = {
        0x19U, // ReadDTCInformation SID
        0x02U  // reportDTCByStatusMask (no DTCStatusMask)
    };

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        fReadDTCInformation.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(
    ReadDTCInformationTest,
    execute_with_subfunction_0x04_should_return_ISO_SUBFUNCTION_NOT_SUPPORTED)
{
    uint8_t request[]
        = {0x19U, // ReadDTCInformation SID
           0x04U, // reportDTCSnapshotRecordByDTCNumber
           0xFFU};

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED,
        fReadDTCInformation.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(
    ReadDTCInformationTest,
    execute_with_subfunction_0x06_should_return_ISO_SUBFUNCTION_NOT_SUPPORTED)
{
    uint8_t request[]
        = {0x19U, // ReadDTCInformation SID
           0x06U, // reportDTCExtendedDataRecordByDTCNumber
           0xFFU};

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED,
        fReadDTCInformation.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(
    ReadDTCInformationTest,
    execute_with_subfunction_0x0A_should_return_ISO_SUBFUNCTION_NOT_SUPPORTED)
{
    uint8_t request[]
        = {0x19U, // ReadDTCInformation SID
           0x0AU, // reportSupportedDTCs
           0xFFU};

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED,
        fReadDTCInformation.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(
    ReadDTCInformationTest,
    execute_with_unsupported_subfunction_should_return_ISO_SUBFUNCTION_NOT_SUPPORTED)
{
    uint8_t request[]
        = {0x19U, // ReadDTCInformation SID
           0x00U, // unsupported subfunction
           0xFFU};

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED,
        fReadDTCInformation.execute(fIncomingDiagConnection, request, sizeof(request)));
}

} // anonymous namespace
