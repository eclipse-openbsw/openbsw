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
#include "uds/diagmgr/DiagMgr.h"
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

    virtual void SetUp()
    {
        fReadDTCInformation.setDefaultDiagSessionManager(fSessionManager);
        // Wire a real DiagMgr (RAM-only, no NVM backend) so tests exercise the
        // production code path instead of the "no DiagMgr wired" stub path.
        fDiagMgr.init();
        fReadDTCInformation.setDiagMgr(&fDiagMgr);
    }

protected:
    ReadDTCInformation fReadDTCInformation;
    StrictMock<IncomingDiagConnectionMock> fIncomingDiagConnection;
    StrictMock<DiagSessionManagerMock> fSessionManager;
    DiagMgr fDiagMgr;
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
    execute_with_subfunction_0x04_and_unknown_DTC_should_return_ISO_REQUEST_OUT_OF_RANGE)
{
    // Full request: [SID | subFunc | DTC_Hi | DTC_Mid | DTC_Lo | recordNumber]
    uint8_t request[] = {
        0x19U, // ReadDTCInformation SID
        0x04U, // reportDTCSnapshotRecordByDTCNumber
        0x12U, // DTC high byte
        0x34U, // DTC middle byte
        0x56U, // DTC low byte
        0xFFU  // record number (all records)
    };

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    // DiagMgr is wired but has no fault records - DTC 0x123456 is unknown.
    EXPECT_EQ(
        DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE,
        fReadDTCInformation.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(
    ReadDTCInformationTest,
    execute_with_subfunction_0x06_and_unknown_DTC_should_return_ISO_REQUEST_OUT_OF_RANGE)
{
    // Full request: [SID | subFunc | DTC_Hi | DTC_Mid | DTC_Lo | recordNumber]
    uint8_t request[] = {
        0x19U, // ReadDTCInformation SID
        0x06U, // reportDTCExtendedDataRecordByDTCNumber
        0x12U, // DTC high byte
        0x34U, // DTC middle byte
        0x56U, // DTC low byte
        0xFFU  // record number (all records)
    };

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.requestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    // DiagMgr is wired but has no fault records - DTC 0x123456 is unknown.
    EXPECT_EQ(
        DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE,
        fReadDTCInformation.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(ReadDTCInformationTest, execute_with_subfunction_0x0A_should_return_OK)
{
    uint8_t request[] = {
        0x19U, // ReadDTCInformation SID
        0x0AU  // reportSupportedDTCs
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
