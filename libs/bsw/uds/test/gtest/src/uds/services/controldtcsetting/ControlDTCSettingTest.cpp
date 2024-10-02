// Copyright 2024 Accenture.

#include "uds/services/controldtcsetting/ControlDTCSetting.h"

#include "uds/connection/IncomingDiagConnectionMock.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <transport/TransportMessageWithBuffer.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::uds;
using namespace ::testing;
using namespace ::transport::test;

class ControlDTCSettingTest : public ::testing::Test
{
public:
    ControlDTCSettingTest()
    : fControlDTCSetting(), fIncomingDiagConnection(::async::CONTEXT_INVALID), fSessionManager()
    {}

    virtual void SetUp() { fControlDTCSetting.setDefaultDiagSessionManager(fSessionManager); }

protected:
    ControlDTCSetting fControlDTCSetting;
    StrictMock<IncomingDiagConnectionMock> fIncomingDiagConnection;
    StrictMock<DiagSessionManagerMock> fSessionManager;
};

TEST_F(
    ControlDTCSettingTest,
    process_which_is_called_by_execute_should_unlock_ErrorMemoryLock_and_return_DiagReturnCode_OK)
{
    uint8_t request[] = {
        0x85U, // ControlDTCSetting request
        0x01U, // CONTROL_DTC_SETTING_ON
    };

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.fpRequestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fControlDTCSetting.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(
    ControlDTCSettingTest,
    process_which_is_called_by_execute_should_lock_ErrorMemoryLock_and_return_DiagReturnCode_OK)
{
    uint8_t request[] = {
        0x85U, // ControlDTCSetting request
        0x02U  // CONTROL_DTC_SETTING_OFF
    };

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.fpRequestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fControlDTCSetting.execute(fIncomingDiagConnection, request, sizeof(request)));
}

TEST_F(
    ControlDTCSettingTest,
    process_which_is_called_by_execute_should_return_ISO_SUBFUNCTION_NOT_SUPPORTED)
{
    uint8_t request[] = {
        0x85U, // ControlDTCSetting request
        0x00U  // not supported
    };

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, request, 0U);

    fIncomingDiagConnection.fpRequestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED,
        fControlDTCSetting.execute(fIncomingDiagConnection, request, sizeof(request)));
}

} // anonymous namespace
