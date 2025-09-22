// Copyright 2024 Accenture.

#include "transport/AbstractTransportLayerMock.h"
#include "transport/TransportConfiguration.h"
#include "transport/TransportMessage.h"
#include "transport/TransportMessageProvidingListenerMock.h"
#include "transport/TransportMessageWithBuffer.h"
#include "uds/DiagCodes.h"
#include "uds/DiagDispatcher.h"
#include "uds/DiagnosisConfiguration.h"
#include "uds/base/AbstractDiagJobMock.h"
#include "uds/base/DiagJobMock.h"
#include "uds/base/DiagJobRoot.h"
#include "uds/connection/DiagConnectionManager.h"
#include "uds/connection/IncomingDiagConnection.h"
#include "uds/connection/NestedDiagRequestMock.h"
#include "uds/session/DiagSessionManagerMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <etl/span.h>

#include <gmock/gmock.h>

#include <cstdlib>

using namespace ::uds;
using namespace ::transport;
using namespace ::transport::test;
using namespace ::testing;

static uint8_t const DIAGNOSIS_ID = 0x10;
static uint8_t const BROADCAST_ID = 0xDF;
static uint8_t const TESTER_ID    = 0xF1;
static uint8_t const SERVICE_ID   = 0x22;

struct ManagedIncomingDiagConnectionTest : Test
{
    static uint8_t const NUM_INCOMING_CONNECTIONS  = 4;
    static uint8_t const MAX_NUM_INCOMING_MESSAGES = 4;

    async::TestContext fContext;
    async::AsyncMock fAsyncMock;
    IncomingDiagConnection* fpIncomingDiagConnection;
    DiagnosisConfiguration<NUM_INCOMING_CONNECTIONS, MAX_NUM_INCOMING_MESSAGES>*
        fpDiagnosisConfiguration;
    TransportMessageProvidingListenerMock* fpTpRouterMock;
    AbstractTransportLayerMock* fpTpLayerMock;
    DiagConnectionManager* fpDiagConnectionManager;
    DiagDispatcher2* fpDiagDispatcher;
    DiagSessionManagerMock* fpSessionProvider;
    DiagJobRoot* fpDiagJobRoot;

    ManagedIncomingDiagConnectionTest() : fContext(2U) {}

    virtual void SetUp()
    {
        fContext.handleAll();

        fpTpRouterMock    = new TransportMessageProvidingListenerMock();
        fpTpLayerMock     = new AbstractTransportLayerMock(0u);
        fpSessionProvider = new DiagSessionManagerMock();
        fpDiagJobRoot     = new DiagJobRoot();
        AbstractDiagJob::setDefaultDiagSessionManager(*fpSessionProvider);

        fpDiagnosisConfiguration
            = new DiagnosisConfiguration<NUM_INCOMING_CONNECTIONS, MAX_NUM_INCOMING_MESSAGES>(
                DIAGNOSIS_ID,
                BROADCAST_ID,
                0u,
                TransportConfiguration::DIAG_PAYLOAD_SIZE,
                false,
                true,
                fContext);

        fpDiagDispatcher = new DiagDispatcher2(
            *fpDiagnosisConfiguration, *fpSessionProvider, *fpDiagJobRoot, fContext);

        fpDiagConnectionManager = new DiagConnectionManager(
            *fpDiagnosisConfiguration,
            *fpTpLayerMock,
            *fpTpRouterMock,
            fContext,
            *fpDiagDispatcher);

        fpIncomingDiagConnection                          = new IncomingDiagConnection(fContext);
        fpIncomingDiagConnection->fpMessageSender         = fpTpLayerMock;
        fpIncomingDiagConnection->fpDiagConnectionManager = fpDiagConnectionManager;
        fpIncomingDiagConnection->setDiagSessionManager(*fpSessionProvider);
        fpIncomingDiagConnection->fpMessageSender = fpTpLayerMock;
    }

    virtual void TearDown()
    {
#if defined(__GNUC__)
#ifdef UNIT_TEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif
#endif
        delete fpIncomingDiagConnection;
        delete fpDiagJobRoot;
        delete fpDiagConnectionManager;
        delete fpDiagDispatcher;
        delete fpDiagnosisConfiguration;
        delete fpSessionProvider;
        delete fpTpRouterMock;
        delete fpTpLayerMock;
#if defined(__GNUC__)
#ifdef UNIT_TEST
#pragma GCC diagnostic pop
#endif
#endif
    }
};

/**
 * @test
 * Direction must be INCOMING after calling the constructor
 */
TEST_F(ManagedIncomingDiagConnectionTest, constructor)
{
    IncomingDiagConnection c{fContext};
    ASSERT_EQ(TransportConfiguration::INVALID_DIAG_ADDRESS, c.fSourceId);
    ASSERT_EQ(TransportConfiguration::INVALID_DIAG_ADDRESS, c.fTargetId);
    ASSERT_EQ(nullptr, c.fpRequestMessage);
    ASSERT_EQ(nullptr, c.fpResponseMessage);
}

/**
 * @test
 * SourceId, TargetId and ServiceId must equal the ones from the
 * assigned request message
 */
TEST_F(ManagedIncomingDiagConnectionTest, setRequestMessage)
{
    TransportMessageWithBuffer pRequest(1024);
    fpIncomingDiagConnection->fpRequestMessage = pRequest.get();

    ASSERT_EQ(pRequest.get(), fpIncomingDiagConnection->fpRequestMessage);
}

/**
 * @test
 * getResponseBuffer() must throw an exception if no response message has
 * been set or otherwise return its payload and length.
 */
TEST_F(ManagedIncomingDiagConnectionTest, setResponseMessage)
{
    TransportMessageWithBuffer pResponse(1024);

    fpIncomingDiagConnection->fpResponseMessage = pResponse.get();

    ASSERT_EQ(pResponse.get(), fpIncomingDiagConnection->fpResponseMessage);
}

/**
 * @test
 * Identifiers are directly added into the response message.
 */
TEST_F(ManagedIncomingDiagConnectionTest, addIdentifier)
{
    TransportMessageWithBuffer pRequest(1024);
    pRequest->resetValidBytes();
    for (uint16_t identifierCount = 1;
         identifierCount <= 6; // IncomingDiagConnection::MAXIMUM_NUMBER_OF_IDENTIFIERS;
         ++identifierCount)
    {
        pRequest->append(identifierCount);
    }
    fpIncomingDiagConnection->fpRequestMessage = pRequest.get();
    for (uint16_t identifierCount = 1;
         identifierCount <= 6; // IncomingDiagConnection::MAXIMUM_NUMBER_OF_IDENTIFIERS;
         ++identifierCount)
    {
        fpIncomingDiagConnection->addIdentifier();
        ASSERT_EQ(identifierCount, fpIncomingDiagConnection->getNumIdentifiers());
        for (uint16_t i = 1; i <= identifierCount; ++i)
        {
            ASSERT_EQ(uint8_t(i), fpIncomingDiagConnection->getIdentifier(i - 1));
        }
    }
}

/**
 * @test
 * A positive response with the correct payload must be sent.
 */
TEST_F(ManagedIncomingDiagConnectionTest, sendPositiveResponse)
{
    uint32_t const MAX_PAYLOAD_LENGTH = 1024;
    TransportMessageWithBuffer pExpectedResponse(MAX_PAYLOAD_LENGTH);
    pExpectedResponse->setTargetId(TESTER_ID);
    pExpectedResponse->setSourceId(DIAGNOSIS_ID);
    pExpectedResponse->resetValidBytes();
    pExpectedResponse->append(SERVICE_ID + DiagReturnCode::POSITIVE_RESPONSE_OFFSET);
    pExpectedResponse->append(1);
    pExpectedResponse->append(2);
    pExpectedResponse->append(3);
    pExpectedResponse->setPayloadLength(4); // +1 because of 1 identifier (service id)

    TransportMessageWithBuffer pRequest(MAX_PAYLOAD_LENGTH);
    fpIncomingDiagConnection->fSourceId        = TESTER_ID;
    fpIncomingDiagConnection->fTargetId        = DIAGNOSIS_ID;
    fpIncomingDiagConnection->fServiceId       = SERVICE_ID;
    fpIncomingDiagConnection->fpRequestMessage = pRequest.get();
    fpIncomingDiagConnection->open(false);
    // save service id
    fpIncomingDiagConnection->addIdentifier();

    PositiveResponse& response = fpIncomingDiagConnection->releaseRequestGetResponse();
    response.appendUint8(1);
    response.appendUint8(2);
    response.appendUint8(3);
    DIAG_JOB(sender, {SERVICE_ID}, 0);
    EXPECT_CALL(
        *fpSessionProvider,
        responseSent(Ref(*fpIncomingDiagConnection), Eq(DiagReturnCode::OK), NotNull(), 3))
        .With(Args<2, 3>(ElementsAre(1, 2, 3)))
        .Times(1);
    EXPECT_CALL(*fpTpLayerMock, send(Eq(*pExpectedResponse), Eq(fpIncomingDiagConnection)))
        .WillOnce(Return(AbstractTransportLayer::ErrorCode::TP_OK));
    fpIncomingDiagConnection->fpResponseMessage = pExpectedResponse.get();
    fpIncomingDiagConnection->sendPositiveResponse(sender);
    fContext.execute();
}

/**
 * @test
 * isNestedRequest() must throw an exception if resuming flag is not set as expected
 */
TEST_F(ManagedIncomingDiagConnectionTest, isNestedRequest)
{
    EXPECT_FALSE(fpIncomingDiagConnection->isNestedRequest());

    NiceMock<NestedDiagRequestMock> nestedRequestMock(1U);
    NiceMock<AbstractDiagJobMock> diagJobMock(static_cast<uint8_t const*>(nullptr), 0U, 0U, 0U);

    TransportMessageWithBuffer pRequest(10);
    fpIncomingDiagConnection->fpRequestMessage = pRequest.get();

    uint8_t const buffer[] = {1};
    EXPECT_CALL(nestedRequestMock, prepareNestedRequest(_))
        .WillOnce(Return(::etl::span<uint8_t const>(buffer)));
    EXPECT_CALL(nestedRequestMock, processNestedRequest(_, _, _))
        .WillOnce(Return(DiagReturnCode::OK));
    fpIncomingDiagConnection->startNestedRequest(diagJobMock, nestedRequestMock, nullptr, 0U);
    EXPECT_TRUE(fpIncomingDiagConnection->isNestedRequest());
}
