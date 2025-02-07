// Copyright 2024 Accenture.

#include "uds/services/routinecontrol/RoutineControl.h"

#include "uds/connection/IncomingDiagConnectionMock.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <transport/TransportMessageWithBuffer.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::uds;
using namespace ::testing;
using namespace ::transport::test;

class MyRoutineControl : public RoutineControl
{
public:
    MyRoutineControl() : RoutineControl() {}

    virtual DiagReturnCode::Type getDefaultDiagReturnCode() const
    {
        return RoutineControl::getDefaultDiagReturnCode();
    }

    virtual DiagReturnCode::Type
    process(IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength)
    {
        return DiagReturnCode::OK;
    }
};

class RoutineControlTest : public ::testing::Test
{
public:
    RoutineControlTest() : fMyRoutineControl(), fIncomingDiagConnection(::async::CONTEXT_INVALID) {}

    virtual void SetUp() { fMyRoutineControl.setDefaultDiagSessionManager(fSessionManager); }

protected:
    MyRoutineControl fMyRoutineControl;
    StrictMock<IncomingDiagConnectionMock> fIncomingDiagConnection;
    StrictMock<DiagSessionManagerMock> fSessionManager;

    static uint8_t const SOURCE_ID                  = 0xF1U;
    static uint8_t const TARGET_ID                  = 0x10U;
    static uint8_t const ROUTINE_CONTROL_SERVICE_ID = 0x31U;
    static uint8_t const WRONG_SERVICE_ID           = 0x22U;
    static uint8_t const START_ROUTINE              = 0x01U;
    static uint8_t const ROUTINE_IDENTIFIER[2U];

    static uint16_t const RESPONSE_LENGTH = 0U;
};

uint8_t const RoutineControlTest::ROUTINE_IDENTIFIER[] = {
    0x00U, // random value
    0x01U  // random value
};

TEST_F(
    RoutineControlTest,
    a_RoutineControl_object_should_set_DefaultDiagReturnCode_to_ISO_SUBFUNCTION_NOT_SUPPORTED)
{
    EXPECT_EQ(
        DiagReturnCode::ISO_SUBFUNCTION_NOT_SUPPORTED,
        fMyRoutineControl.getDefaultDiagReturnCode());
}

TEST_F(
    RoutineControlTest,
    verify_which_is_called_by_execute_return_the_DiagReturnCode_OK_if_the_request_is_valid)
{
    uint8_t const VALID_REQUEST[]
        = {ROUTINE_CONTROL_SERVICE_ID, START_ROUTINE, ROUTINE_IDENTIFIER[0], ROUTINE_IDENTIFIER[1]};

    TransportMessageWithBuffer pRequest(SOURCE_ID, TARGET_ID, VALID_REQUEST, RESPONSE_LENGTH);

    fIncomingDiagConnection.fpRequestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fMyRoutineControl.execute(fIncomingDiagConnection, VALID_REQUEST, sizeof(VALID_REQUEST)));
}

TEST_F(
    RoutineControlTest,
    verify_which_is_called_by_execute_return_ISO_INVALID_FORMAT_if_request_length_is_smaller_than_four)
{
    uint8_t const INVALID_REQUEST[] = {
        ROUTINE_CONTROL_SERVICE_ID, START_ROUTINE, ROUTINE_IDENTIFIER[0]
        // routineIdentifier 2 is MISSING
    };

    TransportMessageWithBuffer pRequest(SOURCE_ID, TARGET_ID, INVALID_REQUEST, RESPONSE_LENGTH);

    fIncomingDiagConnection.fpRequestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        fMyRoutineControl.execute(
            fIncomingDiagConnection, INVALID_REQUEST, sizeof(INVALID_REQUEST)));
}

TEST_F(
    RoutineControlTest,
    verify_which_is_called_by_execute_return_NOT_RESPONSIBLE_if_the_service_id_is_wrong)
{
    uint8_t const INVALID_REQUEST[]
        = {WRONG_SERVICE_ID, START_ROUTINE, ROUTINE_IDENTIFIER[0], ROUTINE_IDENTIFIER[1]};

    TransportMessageWithBuffer pRequest(SOURCE_ID, TARGET_ID, INVALID_REQUEST, RESPONSE_LENGTH);

    fIncomingDiagConnection.fpRequestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::NOT_RESPONSIBLE,
        fMyRoutineControl.execute(
            fIncomingDiagConnection, INVALID_REQUEST, sizeof(INVALID_REQUEST)));
}

} // anonymous namespace
