// Copyright 2024 Accenture.

#include "uds/services/readdata/ReadDataByIdentifier.h"

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

class MyReadDataByIdentifier : public ReadDataByIdentifier
{
public:
    MyReadDataByIdentifier() : ReadDataByIdentifier() {}

    virtual DiagReturnCode::Type getDefaultDiagReturnCode() const
    {
        return ReadDataByIdentifier::getDefaultDiagReturnCode();
    }

    virtual DiagReturnCode::Type
    process(IncomingDiagConnection& connection, uint8_t const request[], uint16_t requestLength)
    {
        return DiagReturnCode::OK;
    }
};

class ReadDataByIdentifierTest : public ::testing::Test
{
public:
    ReadDataByIdentifierTest() : fMyReadDataByIdentifier() {}

    virtual void SetUp() { fMyReadDataByIdentifier.setDefaultDiagSessionManager(fSessionManager); }

protected:
    MyReadDataByIdentifier fMyReadDataByIdentifier;
    StrictMock<IncomingDiagConnectionMock> fIncomingDiagConnection{::async::CONTEXT_INVALID};
    StrictMock<DiagSessionManagerMock> fSessionManager;

    static uint8_t const SOURCE_ID                   = 0xF1U;
    static uint8_t const TARGET_ID                   = 0x10U;
    static uint8_t const READ_DATA_BY_IDENTIFIER_SID = 0x22U;
    static uint8_t const WRONG_SERVICE_ID            = 0x31U;
    static uint8_t const DATA_IDENTIFIER[2U];
};

uint8_t const ReadDataByIdentifierTest::DATA_IDENTIFIER[] = {
    0x00U, // random value
    0x01U  // random value
};

TEST_F(
    ReadDataByIdentifierTest,
    a_ReadDataByIdentifier_object_should_set_DefaultDiagReturnCode_to_ISO_REQUEST_OUT_OF_RANGE)
{
    EXPECT_EQ(
        DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE,
        fMyReadDataByIdentifier.getDefaultDiagReturnCode());
}

TEST_F(
    ReadDataByIdentifierTest,
    verify_which_is_called_by_execute_return_the_DiagReturnCode_OK_if_the_request_is_valid)
{
    uint8_t const VALID_REQUEST[] = {
        READ_DATA_BY_IDENTIFIER_SID,
        DATA_IDENTIFIER[0],
        DATA_IDENTIFIER[1],
    };

    TransportMessageWithBuffer pRequest(
        SOURCE_ID, TARGET_ID, VALID_REQUEST, AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    fIncomingDiagConnection.fpRequestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fMyReadDataByIdentifier.execute(
            fIncomingDiagConnection, VALID_REQUEST, sizeof(VALID_REQUEST)));
}

TEST_F(
    ReadDataByIdentifierTest,
    verify_which_is_called_by_execute_return_ISO_INVALID_FORMAT_if_request_length_is_wrong)
{
    uint8_t const INVALID_REQUEST[] = {
        READ_DATA_BY_IDENTIFIER_SID, DATA_IDENTIFIER[0]
        // dataIdentifier 2 is MISSING
    };

    TransportMessageWithBuffer pRequest(
        SOURCE_ID, TARGET_ID, INVALID_REQUEST, AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    fIncomingDiagConnection.fpRequestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        fMyReadDataByIdentifier.execute(
            fIncomingDiagConnection, INVALID_REQUEST, sizeof(INVALID_REQUEST)));
}

TEST_F(
    ReadDataByIdentifierTest,
    verify_which_is_called_by_execute_return_NOT_RESPONSIBLE_if_the_service_id_is_wrong)
{
    uint8_t const INVALID_REQUEST[] = {WRONG_SERVICE_ID, DATA_IDENTIFIER[0], DATA_IDENTIFIER[1]};

    TransportMessageWithBuffer pRequest(
        SOURCE_ID, TARGET_ID, INVALID_REQUEST, AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    fIncomingDiagConnection.fpRequestMessage = pRequest.get();

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_EQ(
        DiagReturnCode::NOT_RESPONSIBLE,
        fMyReadDataByIdentifier.execute(
            fIncomingDiagConnection, INVALID_REQUEST, sizeof(INVALID_REQUEST)));
}

} // anonymous namespace
