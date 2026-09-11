/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/services/readdata/MultipleReadDataByIdentifier.h"

#include "uds/DiagDispatcher.h"
#include "uds/DiagnosisConfiguration.h"
#include "uds/async/AsyncDiagHelper.h"
#include "uds/base/AbstractDiagJobMock.h"
#include "uds/connection/IncomingDiagConnectionMock.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <transport/TransportConfiguration.h>
#include <transport/TransportMessageWithBuffer.h>

#include <array>

#include <gtest/gtest.h>

#define CONTEXT_EXECUTE fContext.execute()

namespace
{
using namespace ::uds;
using namespace ::testing;
using namespace ::transport::test;

ACTION_P3(SendPositiveResponseReadProductionDate1, connection, diagJob, errorCode)
{
    PositiveResponse& response = connection->releaseRequestGetResponse();

    // Response of ReadProductionDate dataIdentifier 0x22, 0xF1, 0x8B:
    response.appendUint8(0x10U); // production date
    response.appendUint8(0x07U);
    response.appendUint8(0x01U);

    ASSERT_EQ(
        errorCode, (connection->sendPositiveResponseInternal(response.getLength(), *diagJob)));
}

ACTION_P3(SendPositiveResponseReadProductionDate2, connection, diagJob, errorCode)
{
    PositiveResponse& response = connection->releaseRequestGetResponse();

    // Response of ReadProductionDate dataIdentifier 0x22, 0xF1, 0x8B:
    response.appendUint8(0x16U); // production date
    response.appendUint8(0x11U);
    response.appendUint8(0x15U);

    ASSERT_EQ(
        errorCode, (connection->sendPositiveResponseInternal(response.getLength(), *diagJob)));
}

ACTION_P3(SendPositiveResponseReadProductionDate1Overflow, connection, diagJob, errorCode)
{
    PositiveResponse& response = connection->releaseRequestGetResponse();

    // Response of ReadProductionDate dataIdentifier 0x22, 0xF1, 0x8B:
    response.appendUint8(0x10U); // production date
    response.appendUint8(0x07U);
    response.appendUint8(0x01U);
    response.appendUint8(0x32U); // append one byte too much

    ASSERT_EQ(errorCode, (connection.sendPositiveResponseInternal(response.getLength(), *diagJob)));
}

ACTION_P3(SendPositiveResponseReadProductionDate2Overflow, connection, diagJob, errorCode)
{
    PositiveResponse& response = connection->releaseRequestGetResponse();

    // Response of ReadProductionDate dataIdentifier 0x22, 0xF1, 0x8B:
    response.appendUint8(0x16U); // production date
    response.appendUint8(0x11U);
    response.appendUint8(0x15U);
    response.appendUint8(0x33U); // append one byte too much

    ASSERT_EQ(errorCode, (connection.sendPositiveResponseInternal(response.getLength(), *diagJob)));
}

ACTION_P3(SendNegativeResponseAfterReleaseGetResponse, connection, diagJob, errorCode)
{
    connection->releaseRequestGetResponse();
    connection->sendNegativeResponse(errorCode, *diagJob);
}

class MyMultipleReadDataByIdentifier : public MultipleReadDataByIdentifier
{
public:
    MyMultipleReadDataByIdentifier(IAsyncDiagHelper& asyncHelper)
    : MultipleReadDataByIdentifier(asyncHelper)
    {}

    MyMultipleReadDataByIdentifier(IAsyncDiagHelper& asyncHelper, AbstractDiagJob& firstJob)
    : MultipleReadDataByIdentifier(asyncHelper, firstJob)
    {}

    virtual DiagReturnCode::Type getDefaultDiagReturnCode() const
    {
        return MultipleReadDataByIdentifier::getDefaultDiagReturnCode();
    }

    virtual DiagReturnCode::Type verify(uint8_t const* request, uint16_t requestLength)
    {
        return MultipleReadDataByIdentifier::verify(request, requestLength);
    }

    virtual DiagReturnCode::Type
    process(IncomingDiagConnection& connection, uint8_t const* request, uint16_t requestLength)
    {
        return MultipleReadDataByIdentifier::process(connection, request, requestLength);
    }
};

class MultipleReadDataByIdentifierTest : public ::testing::Test
{
public:
    MultipleReadDataByIdentifierTest()
    : fContext(2U)
    , fAsyncHelper(fContext)
    , fMyMultipleReadDataByIdentifier(fAsyncHelper)
    , fMyMultipleReadDataByIdentifierWithFirstJob(fAsyncHelper, fDiagJob)
    , fDiagJob(
          READ_DATA_BY_IDENTIFIER_REQUEST_1.data(),
          READ_DATA_BY_IDENTIFIER_REQUEST_1.size(),
          1U // prefix length
          ,
          DiagSession::ALL_SESSIONS())
    , fDiagJob2(
          READ_DATA_BY_IDENTIFIER_REQUEST_2.data(),
          READ_DATA_BY_IDENTIFIER_REQUEST_2.size(),
          1U // prefix length
          ,
          DiagSession::ALL_SESSIONS())
    , fIncomingDiagConnection(fContext)
    , fUdsConfiguration{0x10U, 0xDFU, transport::TransportConfiguration::DIAG_PAYLOAD_SIZE, 0u, true, false, true, fContext}
    , fUdsDispatcher(
          _connectionPool, _sendJobQueue, fUdsConfiguration, fSessionManager, fDiagJobRoot)
    {}

    virtual void SetUp()
    {
        fContext.handleAll();

        fDiagRoot.addAbstractDiagJob(fMyMultipleReadDataByIdentifierWithFirstJob);
        fDiagRoot.addAbstractDiagJob(fDiagJob);
        fMyMultipleReadDataByIdentifier.setDefaultDiagSessionManager(fSessionManager);
        fMyMultipleReadDataByIdentifierWithFirstJob.setDefaultDiagSessionManager(fSessionManager);
        fIncomingDiagConnection.isOpen    = true;
        fIncomingDiagConnection.serviceId = ::ServiceId::READ_DATA_BY_IDENTIFIER;

        fMyMultipleReadDataByIdentifier.addAbstractDiagJob(fDiagJob);
        fDiagRoot.addAbstractDiagJob(fDiagJob);
    }

    virtual void TearDown()
    {
        fDiagRoot.removeAbstractDiagJob(fDiagJob);
        fMyMultipleReadDataByIdentifier.removeAbstractDiagJob(fDiagJob);
    }

    MOCK_METHOD(uint8_t, getDidLimit, (::transport::TransportMessage const& message));
    MOCK_METHOD(bool, checkResponse, (DiagReturnCode::Type, DiagReturnCode::Type&));

protected:
    static uint8_t const NUM_INCOMING_CONNECTIONS = 1;

    async::TestContext fContext;
    async::AsyncMock fAsyncMock;
    uds::declare::AsyncDiagHelper<2> fAsyncHelper;
    MyMultipleReadDataByIdentifier fMyMultipleReadDataByIdentifier;
    MyMultipleReadDataByIdentifier fMyMultipleReadDataByIdentifierWithFirstJob;
    DiagJobRoot fDiagRoot;
    StrictMock<AbstractDiagJobMock> fDiagJob;
    StrictMock<AbstractDiagJobMock> fDiagJob2;
    StrictMock<IncomingDiagConnectionMock> fIncomingDiagConnection;
    StrictMock<DiagSessionManagerMock> fSessionManager;
    DiagnosisConfiguration fUdsConfiguration;
    ::etl::pool<IncomingDiagConnection, NUM_INCOMING_CONNECTIONS> _connectionPool;
    ::etl::queue<TransportJob, 1> _sendJobQueue;
    DiagDispatcher fUdsDispatcher;
    DiagJobRoot fDiagJobRoot;

    static uint8_t const SOURCE_ID = 0xF1U;
    static uint8_t const TARGET_ID = 0x10U;

    static uint8_t const WRONG_SERVICE_ID = 0x31U;
    static uint8_t const SOME_DATA        = 0x11U;
    static std::array<uint8_t, 2U> const VALID_DATA_IDENTIFIER_1;
    static std::array<uint8_t, 2U> const VALID_DATA_IDENTIFIER_2;
    static std::array<uint8_t, 2U> const INVALID_DATA_IDENTIFIER_1;
    static std::array<uint8_t, 2U> const INVALID_DATA_IDENTIFIER_2;

    static std::array<uint8_t, 3U> const READ_DATA_BY_IDENTIFIER_REQUEST_1;
    static std::array<uint8_t, 3U> const READ_DATA_BY_IDENTIFIER_REQUEST_2;
};

std::array<uint8_t, 2U> const MultipleReadDataByIdentifierTest::VALID_DATA_IDENTIFIER_1
    = {0xF1U, 0x8BU}; // ReadProductionDate dataIdentifier

std::array<uint8_t, 2U> const MultipleReadDataByIdentifierTest::VALID_DATA_IDENTIFIER_2
    = {0xA0U, 0x7FU}; // ReadProductionDate dataIdentifier

std::array<uint8_t, 2U> const MultipleReadDataByIdentifierTest::INVALID_DATA_IDENTIFIER_1
    = {0x44U, 0x44U}; // random dataIdentifier

std::array<uint8_t, 2U> const MultipleReadDataByIdentifierTest::INVALID_DATA_IDENTIFIER_2
    = {0x55U, 0x55U}; // random dataIdentifier

std::array<uint8_t, 3U> const MultipleReadDataByIdentifierTest::READ_DATA_BY_IDENTIFIER_REQUEST_1
    = {
        0x22U, // ReadDataByIdentifier Service ID
        0xF1U, // ReadProductionDate dataIdentifier
        0x8BU  // ReadProductionDate dataIdentifier
};

std::array<uint8_t, 3U> const MultipleReadDataByIdentifierTest::READ_DATA_BY_IDENTIFIER_REQUEST_2
    = {
        0x22U, // ReadDataByIdentifier Service ID
        0xA0U, // ReadProductionDate dataIdentifier
        0x7FU  // ReadProductionDate dataIdentifier
};

TEST_F(MultipleReadDataByIdentifierTest, sets_default_diag_return_code_to_iso_request_out_of_range)
{
    EXPECT_EQ(
        DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE,
        fMyMultipleReadDataByIdentifier.getDefaultDiagReturnCode());

    EXPECT_EQ(
        DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE,
        fMyMultipleReadDataByIdentifierWithFirstJob.getDefaultDiagReturnCode());
}

TEST_F(
    MultipleReadDataByIdentifierTest, verify_returns_the_DiagReturnCode_OK_if_the_request_is_valid)
{
    std::array<uint8_t, 3U> const validRequest
        = {ServiceId::READ_DATA_BY_IDENTIFIER,
           VALID_DATA_IDENTIFIER_1[0U],
           VALID_DATA_IDENTIFIER_1[1U]};

    EXPECT_EQ(
        DiagReturnCode::OK,
        fMyMultipleReadDataByIdentifier.verify(validRequest.data(), validRequest.size()));
}

TEST_F(
    MultipleReadDataByIdentifierTest,
    verify_returns_the_DiagReturnCode_ISO_INVALID_FORMAT_if_the_request_length_is_invalid)
{
    std::array<uint8_t, 2U> const invalidRequest = {
        ServiceId::READ_DATA_BY_IDENTIFIER, VALID_DATA_IDENTIFIER_1[0U]
        // at least dataIdentifier 2 is missing
    };

    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        fMyMultipleReadDataByIdentifier.verify(invalidRequest.data(), invalidRequest.size()));
}

TEST_F(
    MultipleReadDataByIdentifierTest,
    verify_returns_the_DiagReturnCode_ISO_INVALID_FORMAT_if_the_request_is_empty)
{
    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT, fMyMultipleReadDataByIdentifier.verify(nullptr, 0U));
    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT, fMyMultipleReadDataByIdentifier.verify(nullptr, 1U));
}

TEST_F(
    MultipleReadDataByIdentifierTest,
    verify_returns_the_DiagReturnCode_ISO_INVALID_FORMAT_if_the_request_length_is_even)
{
    std::array<uint8_t, 4U> const invalidRequest
        = {ServiceId::READ_DATA_BY_IDENTIFIER,
           VALID_DATA_IDENTIFIER_1[0U],
           VALID_DATA_IDENTIFIER_1[1U],
           SOME_DATA};

    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        fMyMultipleReadDataByIdentifier.verify(invalidRequest.data(), invalidRequest.size()));
}

TEST_F(
    MultipleReadDataByIdentifierTest,
    verify_returns_the_DiagReturnCode_NOT_RESPONSIBLE_if_the_request_service_id_is_wrong)
{
    std::array<uint8_t, 3U> const invalidRequest
        = {WRONG_SERVICE_ID, VALID_DATA_IDENTIFIER_1[0U], VALID_DATA_IDENTIFIER_1[1U]};

    EXPECT_EQ(
        DiagReturnCode::NOT_RESPONSIBLE,
        fMyMultipleReadDataByIdentifier.verify(invalidRequest.data(), invalidRequest.size()));
}

TEST_F(
    MultipleReadDataByIdentifierTest,
    execute_deals_with_one_valid_and_one_invalid_dataIdentifier_and_return_one_positive_response)
{
    std::array<uint8_t, 5U> const dataIdentifiersRequest
        = {ServiceId::READ_DATA_BY_IDENTIFIER,
           VALID_DATA_IDENTIFIER_1[0U],
           VALID_DATA_IDENTIFIER_1[1U],
           INVALID_DATA_IDENTIFIER_1[0U],
           INVALID_DATA_IDENTIFIER_1[1U]};

    std::array<uint8_t, 6U> const expectedResponse = {
        0x62U, // positive response to 0x22 (ReadDataByIdentifier)
        0xF1U, // ReadProductionDate dataIdentifier
        0x8BU, // ReadProductionDate dataIdentifier
        0x10U,
        0x07U,
        0x01U // production date
    };

    TransportMessageWithBuffer pRequest(
        SOURCE_ID,
        TARGET_ID,
        ::etl::span<uint8_t const>(dataIdentifiersRequest.data(), dataIdentifiersRequest.size()),
        AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    fIncomingDiagConnection.requestMessage     = pRequest.get();
    fIncomingDiagConnection.messageSender      = &fUdsDispatcher;
    fIncomingDiagConnection.diagSessionManager = &fSessionManager;

    EXPECT_CALL(fDiagJob, verify(_, _)).WillOnce(Return(DiagReturnCode::OK));

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(fDiagJob, process(_, _, _))
        .WillOnce(DoAll(
            SendPositiveResponseReadProductionDate1(
                &fIncomingDiagConnection, &fDiagJob, ::uds::ErrorCode::OK),
            Return(DiagReturnCode::OK)));
    EXPECT_CALL(fIncomingDiagConnection, terminate());

    EXPECT_EQ(
        DiagReturnCode::OK,
        fMyMultipleReadDataByIdentifier.execute(
            fIncomingDiagConnection, dataIdentifiersRequest.data(), dataIdentifiersRequest.size()));
    CONTEXT_EXECUTE;

    Mock::VerifyAndClearExpectations(&fDiagJob);
    Mock::VerifyAndClearExpectations(&fIncomingDiagConnection);

    EXPECT_CALL(fDiagJob, verify(_, _)).WillOnce(Return(DiagReturnCode::OK));
    EXPECT_CALL(fDiagJob, process(_, _, _)).WillOnce(Return(DiagReturnCode::NOT_RESPONSIBLE));
    EXPECT_CALL(fIncomingDiagConnection, terminate());

    EXPECT_CALL(
        fSessionManager,
        responseSent(Ref(fIncomingDiagConnection), DiagReturnCode::OK, NotNull(), 5U));
    fIncomingDiagConnection.terminateNestedRequest();
    CONTEXT_EXECUTE;

    auto buffer = ::etl::span<uint8_t const>(
        fIncomingDiagConnection.responseMessage->getPayload(),
        fIncomingDiagConnection.responseMessage->getPayloadLength());

    EXPECT_THAT(buffer, ElementsAreArray(expectedResponse));
}

TEST_F(MultipleReadDataByIdentifierTest, processes_mixed_did_results_into_positive_response)
{
    std::array<uint8_t, 5U> const dataIdentifiersRequest
        = {ServiceId::READ_DATA_BY_IDENTIFIER,
           VALID_DATA_IDENTIFIER_2[0U],
           VALID_DATA_IDENTIFIER_2[1U],
           VALID_DATA_IDENTIFIER_1[0U],
           VALID_DATA_IDENTIFIER_1[1U]};

    std::array<uint8_t, 6U> const expectedResponse = {
        0x62U, // positive response to 0x22 (ReadDataByIdentifier)
        0xF1U, // ReadProductionDate dataIdentifier
        0x8BU, // ReadProductionDate dataIdentifier
        0x10U,
        0x07U,
        0x01U // production date
    };

    fDiagRoot.addAbstractDiagJob(fDiagJob2);

    TransportMessageWithBuffer pRequest(
        SOURCE_ID,
        TARGET_ID,
        ::etl::span<uint8_t const>(dataIdentifiersRequest.data(), dataIdentifiersRequest.size()),
        AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    fIncomingDiagConnection.requestMessage     = pRequest.get();
    fIncomingDiagConnection.messageSender      = &fUdsDispatcher;
    fIncomingDiagConnection.diagSessionManager = &fSessionManager;

    Sequence seq;

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(fDiagJob, verify(_, _))
        .InSequence(seq)
        .WillOnce(Return(DiagReturnCode::NOT_RESPONSIBLE));

    EXPECT_CALL(fDiagJob2, verify(_, _)).InSequence(seq).WillOnce(Return(DiagReturnCode::OK));
    EXPECT_CALL(fDiagJob2, process(_, _, _))
        .InSequence(seq)
        .WillOnce(DoAll(
            SendNegativeResponseAfterReleaseGetResponse(
                &fIncomingDiagConnection, &fDiagJob2, DiagReturnCode::NOT_RESPONSIBLE),
            Return(DiagReturnCode::OK)));

    EXPECT_CALL(fIncomingDiagConnection, terminate());
    EXPECT_EQ(
        DiagReturnCode::OK,
        fMyMultipleReadDataByIdentifier.execute(
            fIncomingDiagConnection, dataIdentifiersRequest.data(), dataIdentifiersRequest.size()));
    CONTEXT_EXECUTE;

    Mock::VerifyAndClearExpectations(&fDiagJob);
    Mock::VerifyAndClearExpectations(&fIncomingDiagConnection);

    EXPECT_CALL(fDiagJob, verify(_, _)).InSequence(seq).WillOnce(Return(DiagReturnCode::OK));
    EXPECT_CALL(fDiagJob, process(_, _, _))
        .InSequence(seq)
        .WillOnce(DoAll(
            SendPositiveResponseReadProductionDate1(
                &fIncomingDiagConnection, &fDiagJob, ::uds::ErrorCode::OK),
            Return(DiagReturnCode::OK)));

    EXPECT_CALL(
        fSessionManager,
        responseSent(Ref(fIncomingDiagConnection), DiagReturnCode::OK, NotNull(), 5U));
    EXPECT_CALL(fIncomingDiagConnection, terminate()).Times(2);
    fIncomingDiagConnection.terminateNestedRequest();
    CONTEXT_EXECUTE;
    fIncomingDiagConnection.terminateNestedRequest();
    CONTEXT_EXECUTE;

    auto buffer = ::etl::span<uint8_t const>(
        fIncomingDiagConnection.responseMessage->getPayload(),
        fIncomingDiagConnection.responseMessage->getPayloadLength());

    EXPECT_THAT(buffer, ElementsAreArray(expectedResponse));
}

TEST_F(
    MultipleReadDataByIdentifierTest,
    execute_deals_with_one_invalid_and_one_valid_dataIdentifier_and_return_one_positive_response)
{
    std::array<uint8_t, 5U> const dataIdentifiersRequest
        = {ServiceId::READ_DATA_BY_IDENTIFIER,
           INVALID_DATA_IDENTIFIER_1[0U],
           INVALID_DATA_IDENTIFIER_1[1U],
           VALID_DATA_IDENTIFIER_1[0U],
           VALID_DATA_IDENTIFIER_1[1U]};

    std::array<uint8_t, 6U> const expectedResponse = {
        0x62U, // positive response to 0x22 (ReadDataByIdentifier)
        0xF1U, // ReadProductionDate dataIdentifier
        0x8BU, // ReadProductionDate dataIdentifier
        0x10U,
        0x07U,
        0x01U // production date
    };

    TransportMessageWithBuffer pRequest(
        SOURCE_ID,
        TARGET_ID,
        ::etl::span<uint8_t const>(dataIdentifiersRequest.data(), dataIdentifiersRequest.size()),
        AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    fMyMultipleReadDataByIdentifier.setCheckResponse(
        MultipleReadDataByIdentifier::CheckResponseType::create<
            MultipleReadDataByIdentifierTest,
            &MultipleReadDataByIdentifierTest::checkResponse>(*this));

    fIncomingDiagConnection.requestMessage     = pRequest.get();
    fIncomingDiagConnection.messageSender      = &fUdsDispatcher;
    fIncomingDiagConnection.diagSessionManager = &fSessionManager;

    Sequence seq;

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(fDiagJob, verify(_, _)).InSequence(seq).WillOnce(Return(DiagReturnCode::OK));
    EXPECT_CALL(fDiagJob, process(_, _, _))
        .InSequence(seq)
        .WillOnce(Return(DiagReturnCode::ISO_CONTROL_UNIT_ON_SUBBUS_NOT_RESPONDING));
    EXPECT_CALL(*this, checkResponse(DiagReturnCode::ISO_CONTROL_UNIT_ON_SUBBUS_NOT_RESPONDING, _))
        .InSequence(seq)
        .WillOnce(Return(true));
    EXPECT_CALL(fIncomingDiagConnection, terminate());

    EXPECT_CALL(fDiagJob, verify(_, _)).InSequence(seq).WillOnce(Return(DiagReturnCode::OK));
    EXPECT_CALL(fDiagJob, process(_, _, _))
        .InSequence(seq)
        .WillOnce(DoAll(
            SendPositiveResponseReadProductionDate1(
                &fIncomingDiagConnection, &fDiagJob, ::uds::ErrorCode::OK),
            Return(DiagReturnCode::OK)));
    EXPECT_CALL(*this, checkResponse(DiagReturnCode::OK, _))
        .InSequence(seq)
        .WillOnce(DoAll(SetArgReferee<1>(DiagReturnCode::OK), Return(true)));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fMyMultipleReadDataByIdentifier.execute(
            fIncomingDiagConnection, dataIdentifiersRequest.data(), dataIdentifiersRequest.size()));
    CONTEXT_EXECUTE;

    EXPECT_CALL(
        fSessionManager,
        responseSent(Ref(fIncomingDiagConnection), DiagReturnCode::OK, NotNull(), 5U));
    EXPECT_CALL(fIncomingDiagConnection, terminate());
    fIncomingDiagConnection.terminateNestedRequest();
    CONTEXT_EXECUTE;

    auto buffer = ::etl::span<uint8_t const>(
        fIncomingDiagConnection.responseMessage->getPayload(),
        fIncomingDiagConnection.responseMessage->getPayloadLength());

    EXPECT_THAT(buffer, ElementsAreArray(expectedResponse));
}

TEST_F(
    MultipleReadDataByIdentifierTest,
    execute_deals_with_two_valid_dataIdentifiers_and_return_one_positive_response_for_both)
{
    std::array<uint8_t, 5U> const dataIdentifiersRequest
        = {ServiceId::READ_DATA_BY_IDENTIFIER,
           VALID_DATA_IDENTIFIER_1[0U],
           VALID_DATA_IDENTIFIER_1[1U],
           VALID_DATA_IDENTIFIER_1[0U],
           VALID_DATA_IDENTIFIER_1[1U]};

    std::array<uint8_t, 11U> const expectedResponse = {
        0x62U, // Positive response to 0x22 (ReadDataByIdentifier)
        0xF1U, // ReadProductionDate dataIdentifier
        0x8BU, // ReadProductionDate dataIdentifier
        0x10U,
        0x07U,
        0x01U, // Production date
        0xF1U, // ReadProductionDate dataIdentifier
        0x8BU, // ReadProductionDate dataIdentifier
        0x16U,
        0x11U,
        0x15U // Production date
    };

    TransportMessageWithBuffer pRequest(
        SOURCE_ID,
        TARGET_ID,
        ::etl::span<uint8_t const>(dataIdentifiersRequest.data(), dataIdentifiersRequest.size()),
        AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    fIncomingDiagConnection.requestMessage     = pRequest.get();
    fIncomingDiagConnection.messageSender      = &fUdsDispatcher;
    fIncomingDiagConnection.diagSessionManager = &fSessionManager;

    EXPECT_CALL(fDiagJob, verify(_, _)).WillOnce(Return(DiagReturnCode::OK));

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(fDiagJob, process(_, _, _))
        .WillOnce(DoAll(
            SendPositiveResponseReadProductionDate1(
                &fIncomingDiagConnection, &fDiagJob, ::uds::ErrorCode::OK),
            Return(DiagReturnCode::OK)));
    EXPECT_CALL(fIncomingDiagConnection, terminate());

    EXPECT_EQ(
        DiagReturnCode::OK,
        fMyMultipleReadDataByIdentifier.execute(
            fIncomingDiagConnection, dataIdentifiersRequest.data(), dataIdentifiersRequest.size()));
    CONTEXT_EXECUTE;

    Mock::VerifyAndClearExpectations(&fDiagJob);
    Mock::VerifyAndClearExpectations(&fIncomingDiagConnection);

    EXPECT_CALL(fDiagJob, verify(_, _)).WillOnce(Return(DiagReturnCode::OK));
    EXPECT_CALL(fDiagJob, process(_, _, _))
        .WillOnce(DoAll(
            SendPositiveResponseReadProductionDate2(
                &fIncomingDiagConnection, &fDiagJob, ::uds::ErrorCode::OK),
            Return(DiagReturnCode::OK)));

    EXPECT_CALL(fIncomingDiagConnection, terminate()); // terminate 1st request
    fIncomingDiagConnection.terminateNestedRequest();
    CONTEXT_EXECUTE;

    Mock::VerifyAndClearExpectations(&fDiagJob);
    Mock::VerifyAndClearExpectations(&fIncomingDiagConnection);

    EXPECT_CALL(
        fSessionManager,
        responseSent(Ref(fIncomingDiagConnection), DiagReturnCode::OK, NotNull(), 10U));
    EXPECT_CALL(fIncomingDiagConnection, terminate()); // terminate 2nd request
    fIncomingDiagConnection.terminateNestedRequest();
    CONTEXT_EXECUTE;

    Mock::VerifyAndClearExpectations(&fDiagJob);
    Mock::VerifyAndClearExpectations(&fIncomingDiagConnection);
    Mock::VerifyAndClearExpectations(&fSessionManager);

    auto buffer = ::etl::span<uint8_t const>(
        fIncomingDiagConnection.responseMessage->getPayload(),
        fIncomingDiagConnection.responseMessage->getPayloadLength());

    EXPECT_THAT(buffer, ElementsAreArray(expectedResponse));
}

TEST_F(
    MultipleReadDataByIdentifierTest,
    execute_deals_with_two_invalid_dataIdentifiers_and_return_negative_response)
{
    std::array<uint8_t, 5U> const invalidDataIdentifiersRequest
        = {ServiceId::READ_DATA_BY_IDENTIFIER,
           INVALID_DATA_IDENTIFIER_1[0U],
           INVALID_DATA_IDENTIFIER_1[1U],
           INVALID_DATA_IDENTIFIER_2[0U],
           INVALID_DATA_IDENTIFIER_2[1U]};

    std::array<uint8_t, 3U> const expectedResponse = {
        0x7FU, // Negative Response Identifier
        0x22U, // ReadDataByIdentifier SID
        0x31U  // Negative Response Code if no DID's are valid
    };

    TransportMessageWithBuffer pRequest(
        SOURCE_ID,
        TARGET_ID,
        ::etl::span<uint8_t const>(
            invalidDataIdentifiersRequest.data(), invalidDataIdentifiersRequest.size()),
        AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    fIncomingDiagConnection.requestMessage     = pRequest.get();
    fIncomingDiagConnection.messageSender      = &fUdsDispatcher;
    fIncomingDiagConnection.diagSessionManager = &fSessionManager;

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    Sequence seq;

    EXPECT_CALL(fDiagJob, verify(_, _)).InSequence(seq).WillOnce(Return(DiagReturnCode::OK));
    EXPECT_CALL(fDiagJob, process(_, _, _))
        .InSequence(seq)
        .WillOnce(Return(DiagReturnCode::NOT_RESPONSIBLE));

    EXPECT_CALL(fDiagJob, verify(_, _)).InSequence(seq).WillOnce(Return(DiagReturnCode::OK));
    EXPECT_CALL(fDiagJob, process(_, _, _))
        .InSequence(seq)
        .WillOnce(Return(DiagReturnCode::NOT_RESPONSIBLE));

    EXPECT_CALL(
        fSessionManager,
        responseSent(
            Ref(fIncomingDiagConnection), DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE, NotNull(), 0U));

    EXPECT_CALL(fIncomingDiagConnection, terminate());

    EXPECT_EQ(
        DiagReturnCode::OK,
        fMyMultipleReadDataByIdentifier.execute(
            fIncomingDiagConnection,
            invalidDataIdentifiersRequest.data(),
            invalidDataIdentifiersRequest.size()));
    CONTEXT_EXECUTE;

    auto buffer = ::etl::span<uint8_t const>(
        fIncomingDiagConnection.responseMessage->getPayload(),
        fIncomingDiagConnection.responseMessage->getPayloadLength());

    EXPECT_THAT(buffer, ElementsAreArray(expectedResponse));
}

TEST_F(
    MultipleReadDataByIdentifierTest,
    execute_deals_with_an_valid_dataIdentifiers_returning_negative_response)
{
    std::array<uint8_t, 5U> const invalidDataIdentifiersRequest
        = {ServiceId::READ_DATA_BY_IDENTIFIER,
           INVALID_DATA_IDENTIFIER_1[0U],
           INVALID_DATA_IDENTIFIER_1[1U],
           INVALID_DATA_IDENTIFIER_2[0U],
           INVALID_DATA_IDENTIFIER_2[1U]};

    std::array<uint8_t, 3U> const expectedResponse = {
        0x7FU, // Negative Response Identifier
        0x22U, // ReadDataByIdentifier SID
        0x36U  // Negative Response Code if no DID's are valid
    };

    TransportMessageWithBuffer pRequest(
        SOURCE_ID,
        TARGET_ID,
        ::etl::span<uint8_t const>(
            invalidDataIdentifiersRequest.data(), invalidDataIdentifiersRequest.size()),
        AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    fIncomingDiagConnection.requestMessage     = pRequest.get();
    fIncomingDiagConnection.messageSender      = &fUdsDispatcher;
    fIncomingDiagConnection.diagSessionManager = &fSessionManager;

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    Sequence seq;

    EXPECT_CALL(fDiagJob, verify(_, _)).InSequence(seq).WillOnce(Return(DiagReturnCode::OK));
    EXPECT_CALL(fDiagJob, process(_, _, _))
        .InSequence(seq)
        .WillOnce(Return(DiagReturnCode::ISO_EXCEEDED_NUMS_OF_ATTEMPTS));

    EXPECT_CALL(
        fSessionManager,
        responseSent(
            Ref(fIncomingDiagConnection),
            DiagReturnCode::ISO_EXCEEDED_NUMS_OF_ATTEMPTS,
            NotNull(),
            0U));

    EXPECT_CALL(fIncomingDiagConnection, terminate());

    EXPECT_EQ(
        DiagReturnCode::OK,
        fMyMultipleReadDataByIdentifier.execute(
            fIncomingDiagConnection,
            invalidDataIdentifiersRequest.data(),
            invalidDataIdentifiersRequest.size()));
    CONTEXT_EXECUTE;

    auto buffer = ::etl::span<uint8_t const>(
        fIncomingDiagConnection.responseMessage->getPayload(),
        fIncomingDiagConnection.responseMessage->getPayloadLength());

    EXPECT_THAT(buffer, ElementsAreArray(expectedResponse));
}

TEST_F(MultipleReadDataByIdentifierTest, get_did_limit_is_called_and_checked_if_available)
{
    std::array<uint8_t, 5U> const dataIdentifiersRequest
        = {ServiceId::READ_DATA_BY_IDENTIFIER,
           VALID_DATA_IDENTIFIER_1[0U],
           VALID_DATA_IDENTIFIER_1[1U],
           VALID_DATA_IDENTIFIER_1[0U],
           VALID_DATA_IDENTIFIER_1[1U]};

    std::array<uint8_t, 11U> const expectedResponse = {
        0x62U, // Positive response to 0x22 (ReadDataByIdentifier)
        0xF1U, // ReadProductionDate dataIdentifier
        0x8BU, // ReadProductionDate dataIdentifier
        0x10U,
        0x07U,
        0x01U, // Production date
        0xF1U, // ReadProductionDate dataIdentifier
        0x8BU, // ReadProductionDate dataIdentifier
        0x16U,
        0x11U,
        0x15U // Production date
    };

    fMyMultipleReadDataByIdentifier.setGetDidLimit(
        MultipleReadDataByIdentifier::GetDidLimitType::create<
            MultipleReadDataByIdentifierTest,
            &MultipleReadDataByIdentifierTest::getDidLimit>(*this));

    TransportMessageWithBuffer pRequest(
        SOURCE_ID,
        TARGET_ID,
        ::etl::span<uint8_t const>(dataIdentifiersRequest.data(), dataIdentifiersRequest.size()),
        AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    fIncomingDiagConnection.requestMessage     = pRequest.get();
    fIncomingDiagConnection.messageSender      = &fUdsDispatcher;
    fIncomingDiagConnection.diagSessionManager = &fSessionManager;

    EXPECT_CALL(fDiagJob, verify(_, _)).WillOnce(Return(DiagReturnCode::OK));

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(fDiagJob, process(_, _, _))
        .WillOnce(DoAll(
            SendPositiveResponseReadProductionDate1(
                &fIncomingDiagConnection, &fDiagJob, ::uds::ErrorCode::OK),
            Return(DiagReturnCode::OK)));
    EXPECT_CALL(fIncomingDiagConnection, terminate());

    EXPECT_CALL(*this, getDidLimit(Ref(*pRequest))).WillOnce(Return(2U));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fMyMultipleReadDataByIdentifier.execute(
            fIncomingDiagConnection, dataIdentifiersRequest.data(), dataIdentifiersRequest.size()));
    CONTEXT_EXECUTE;

    Mock::VerifyAndClearExpectations(&fDiagJob);
    Mock::VerifyAndClearExpectations(&fIncomingDiagConnection);

    EXPECT_CALL(fDiagJob, verify(_, _)).WillOnce(Return(DiagReturnCode::OK));
    EXPECT_CALL(fDiagJob, process(_, _, _))
        .WillOnce(DoAll(
            SendPositiveResponseReadProductionDate2(
                &fIncomingDiagConnection, &fDiagJob, ::uds::ErrorCode::OK),
            Return(DiagReturnCode::OK)));

    EXPECT_CALL(fIncomingDiagConnection, terminate()); // terminate 1st request
    fIncomingDiagConnection.terminateNestedRequest();
    CONTEXT_EXECUTE;

    Mock::VerifyAndClearExpectations(&fDiagJob);
    Mock::VerifyAndClearExpectations(&fIncomingDiagConnection);

    EXPECT_CALL(
        fSessionManager,
        responseSent(Ref(fIncomingDiagConnection), DiagReturnCode::OK, NotNull(), 10U));
    EXPECT_CALL(fIncomingDiagConnection, terminate()); // terminate 2nd request
    fIncomingDiagConnection.terminateNestedRequest();
    CONTEXT_EXECUTE;

    Mock::VerifyAndClearExpectations(&fDiagJob);
    Mock::VerifyAndClearExpectations(&fIncomingDiagConnection);
    Mock::VerifyAndClearExpectations(&fSessionManager);

    auto buffer = ::etl::span<uint8_t const>(
        fIncomingDiagConnection.responseMessage->getPayload(),
        fIncomingDiagConnection.responseMessage->getPayloadLength());

    EXPECT_THAT(buffer, ElementsAreArray(expectedResponse));
}

TEST_F(MultipleReadDataByIdentifierTest, get_did_limit_is_called_and_not_checked_if_zero)
{
    std::array<uint8_t, 5U> const dataIdentifiersRequest
        = {ServiceId::READ_DATA_BY_IDENTIFIER,
           VALID_DATA_IDENTIFIER_1[0U],
           VALID_DATA_IDENTIFIER_1[1U],
           VALID_DATA_IDENTIFIER_1[0U],
           VALID_DATA_IDENTIFIER_1[1U]};

    std::array<uint8_t, 11U> const expectedResponse = {
        0x62U, // Positive response to 0x22 (ReadDataByIdentifier)
        0xF1U, // ReadProductionDate dataIdentifier
        0x8BU, // ReadProductionDate dataIdentifier
        0x10U,
        0x07U,
        0x01U, // Production date
        0xF1U, // ReadProductionDate dataIdentifier
        0x8BU, // ReadProductionDate dataIdentifier
        0x16U,
        0x11U,
        0x15U // Production date
    };

    fMyMultipleReadDataByIdentifier.setGetDidLimit(
        MultipleReadDataByIdentifier::GetDidLimitType::create<
            MultipleReadDataByIdentifierTest,
            &MultipleReadDataByIdentifierTest::getDidLimit>(*this));

    TransportMessageWithBuffer pRequest(
        SOURCE_ID,
        TARGET_ID,
        ::etl::span<uint8_t const>(dataIdentifiersRequest.data(), dataIdentifiersRequest.size()),
        AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    fIncomingDiagConnection.requestMessage     = pRequest.get();
    fIncomingDiagConnection.messageSender      = &fUdsDispatcher;
    fIncomingDiagConnection.diagSessionManager = &fSessionManager;

    EXPECT_CALL(fDiagJob, verify(_, _)).WillOnce(Return(DiagReturnCode::OK));

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(fDiagJob, process(_, _, _))
        .WillOnce(DoAll(
            SendPositiveResponseReadProductionDate1(
                &fIncomingDiagConnection, &fDiagJob, ::uds::ErrorCode::OK),
            Return(DiagReturnCode::OK)));
    EXPECT_CALL(fIncomingDiagConnection, terminate());

    EXPECT_CALL(*this, getDidLimit(Ref(*pRequest))).WillOnce(Return(0U));

    EXPECT_EQ(
        DiagReturnCode::OK,
        fMyMultipleReadDataByIdentifier.execute(
            fIncomingDiagConnection, dataIdentifiersRequest.data(), dataIdentifiersRequest.size()));
    CONTEXT_EXECUTE;

    Mock::VerifyAndClearExpectations(&fDiagJob);
    Mock::VerifyAndClearExpectations(&fIncomingDiagConnection);

    EXPECT_CALL(fDiagJob, verify(_, _)).WillOnce(Return(DiagReturnCode::OK));
    EXPECT_CALL(fDiagJob, process(_, _, _))
        .WillOnce(DoAll(
            SendPositiveResponseReadProductionDate2(
                &fIncomingDiagConnection, &fDiagJob, ::uds::ErrorCode::OK),
            Return(DiagReturnCode::OK)));

    EXPECT_CALL(fIncomingDiagConnection, terminate()); // terminate 1st request
    fIncomingDiagConnection.terminateNestedRequest();
    CONTEXT_EXECUTE;

    Mock::VerifyAndClearExpectations(&fDiagJob);
    Mock::VerifyAndClearExpectations(&fIncomingDiagConnection);

    EXPECT_CALL(
        fSessionManager,
        responseSent(Ref(fIncomingDiagConnection), DiagReturnCode::OK, NotNull(), 10U));
    EXPECT_CALL(fIncomingDiagConnection, terminate()); // terminate 2nd request
    fIncomingDiagConnection.terminateNestedRequest();
    CONTEXT_EXECUTE;

    Mock::VerifyAndClearExpectations(&fDiagJob);
    Mock::VerifyAndClearExpectations(&fIncomingDiagConnection);
    Mock::VerifyAndClearExpectations(&fSessionManager);

    auto buffer = ::etl::span<uint8_t const>(
        fIncomingDiagConnection.responseMessage->getPayload(),
        fIncomingDiagConnection.responseMessage->getPayloadLength());

    EXPECT_THAT(buffer, ElementsAreArray(expectedResponse));
}

TEST_F(MultipleReadDataByIdentifierTest, get_did_limit_is_called_and_returns_ISO_INVALID_FORMAT)
{
    std::array<uint8_t, 5U> const dataIdentifiersRequest
        = {ServiceId::READ_DATA_BY_IDENTIFIER,
           VALID_DATA_IDENTIFIER_1[0U],
           VALID_DATA_IDENTIFIER_1[1U],
           VALID_DATA_IDENTIFIER_1[0U],
           VALID_DATA_IDENTIFIER_1[1U]};

    fMyMultipleReadDataByIdentifier.setGetDidLimit(
        MultipleReadDataByIdentifier::GetDidLimitType::create<
            MultipleReadDataByIdentifierTest,
            &MultipleReadDataByIdentifierTest::getDidLimit>(*this));

    TransportMessageWithBuffer pRequest(
        SOURCE_ID,
        TARGET_ID,
        ::etl::span<uint8_t const>(dataIdentifiersRequest.data(), dataIdentifiersRequest.size()),
        AbstractDiagJob::VARIABLE_RESPONSE_LENGTH);

    fIncomingDiagConnection.requestMessage     = pRequest.get();
    fIncomingDiagConnection.messageSender      = &fUdsDispatcher;
    fIncomingDiagConnection.diagSessionManager = &fSessionManager;

    EXPECT_CALL(fSessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    EXPECT_CALL(fSessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(*this, getDidLimit(Ref(*pRequest))).WillOnce(Return(1U));

    EXPECT_EQ(
        DiagReturnCode::ISO_INVALID_FORMAT,
        fMyMultipleReadDataByIdentifier.execute(
            fIncomingDiagConnection, dataIdentifiersRequest.data(), dataIdentifiersRequest.size()));
    CONTEXT_EXECUTE;
}

} // namespace
