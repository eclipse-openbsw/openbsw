// Copyright 2025 Accenture.

#include "doip/server/DoIpServerConnectionHandler.h"

#include "doip/common/DoIpConnectionHandlerMock.h"
#include "doip/common/DoIpHeader.h"
#include "doip/common/DoIpSendJobMock.h"
#include "doip/common/DoIpTcpConnectionMock.h"
#include "doip/server/DoIpServerConnectionHandlerCallbackMock.h"
#include "doip/server/DoIpServerMessageHandlerMock.h"
#include "doip/server/DoIpServerTransportLayerParameters.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <common/busid/BusId.h>
#include <transport/TransportMessage.h>

#include <gmock/gmock.h>
#include <gtest/esr_extensions.h>

using namespace ::testing;
using namespace ::doip;

namespace
{
MATCHER_P(IsHeaderEqual, expected, "")
{
    return ::estd::memory::is_equal(
        ::estd::memory::as_bytes(&arg),
        ::estd::slice<uint8_t const>::from_pointer(expected, DoIpConstants::DOIP_HEADER_LENGTH));
}

DoIpHeader const& as_header(::estd::slice<uint8_t const> bytes)
{
    return bytes.reinterpret_as<DoIpHeader const>()[0];
}
} // namespace

struct DoIpServerConnectionHandlerTest : Test
{
    DoIpServerConnectionHandlerTest()
    : fBusId()
    , asyncMock()
    , asyncContext(1U)
    , testContext(asyncContext)
    , fParameters(0x123U, 0x4412U, 0x2139U, 0x45363U)
    , fLocalEndpoint(::ip::make_ip4(0x238492U), 343U)
    , fRemoteEndpoint(::ip::make_ip4(0x2324492U), 384U)
    {}

    void TearDown() override {}

    void SetUp() override { testContext.handleAll(); }

    void expectRoutingActivationCheck(uint8_t const socketGroupId = 0U);

    uint8_t fBusId;
    ::testing::StrictMock<::async::AsyncMock> asyncMock;
    ::async::ContextType asyncContext;
    ::async::TestContext testContext;
    DoIpServerTransportLayerParameters fParameters;
    StrictMock<DoIpTcpConnectionMock> fConnectionMock;
    StrictMock<DoIpConnectionHandlerMock> fConnectionHandlerMock;
    StrictMock<DoIpServerConnectionHandlerCallbackMock> fCallbackMock;
    StrictMock<DoIpServerMessageHandlerMock> fMessageHandlerMock;
    ::ip::IPEndpoint fLocalEndpoint;
    ::ip::IPEndpoint fRemoteEndpoint;
    uint8_t fHeaderBuffer[8U];
};

TEST_F(DoIpServerConnectionHandlerTest, TestRoutingActivationAndClose)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        123U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    EXPECT_EQ(123U, cut.getSocketGroupId());
    EXPECT_FALSE(cut.isActivating());
    EXPECT_FALSE(cut.isRouting());
    EXPECT_FALSE(cut.isClosed());
    EXPECT_EQ(uint16_t(::transport::TransportMessage::INVALID_ADDRESS), cut.getSourceAddress());
    IDoIpConnectionHandler* connectionHandler = nullptr;
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // start the connection
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    EXPECT_TRUE(serverConnection != nullptr);
    // Starting again should be neutral
    cut.start(fCallbackMock);
    // Receive an activation request
    uint8_t const activationRequest[] = {
        0x02, 0xfd, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    EXPECT_EQ(7U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(
            0x1234,
            0x03,
            123U,
            fLocalEndpoint,
            fRemoteEndpoint,
            ::estd::optional<uint32_t>(),
            false))
        .WillOnce(Return(IDoIpServerConnectionFilter::RoutingActivationCheckResult()));
    EXPECT_CALL(fCallbackMock, handleRoutingActivationRequest(Ref(cut)));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(activationRequest + 8U, 3U));
    Mock::VerifyAndClearExpectations(&fCallbackMock);
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    // Expect positive response
    EXPECT_TRUE(cut.isActivating());
    EXPECT_FALSE(cut.isRouting());
    EXPECT_FALSE(cut.isClosed());
    EXPECT_EQ(0x1234, cut.getSourceAddress());
    EXPECT_CALL(fMessageHandlerMock, routingActive());
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    uint8_t const activationResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x12,
           0x34,
           0x01,
           0x23,
           0x11,
           0x00,
           0x00,
           0x00,
           0x00};
    cut.routingActivationCompleted(true, 0x11);
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse + 8U, 9U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // Now routing should be active
    EXPECT_FALSE(cut.isActivating());
    EXPECT_TRUE(cut.isRouting());
    EXPECT_FALSE(cut.isClosed());
    EXPECT_EQ(0x1234, cut.getSourceAddress());
    // Close the connection
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fConnectionMock, close());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    connectionHandler->connectionClosed(true);
    // Closing again should be neutral
    cut.close();
    EXPECT_FALSE(cut.isActivating());
    EXPECT_FALSE(cut.isRouting());
    EXPECT_TRUE(cut.isClosed());
}

TEST_F(DoIpServerConnectionHandlerTest, TestRoutingActivationWithBigPayload)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    IDoIpConnectionHandler* connectionHandler = nullptr;
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // start the connection
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    EXPECT_TRUE(serverConnection != nullptr);
    // Starting again should be neutral
    cut.start(fCallbackMock);
    // Receive an activation request
    uint8_t const activationRequest[]
        = {0x02,
           0xfd,
           0x00,
           0x05,
           0x00,
           0x00,
           0x00,
           0x0b,
           0x12,
           0x34,
           0x03,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    EXPECT_EQ(11U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(
            0x1234,
            0x03,
            0U,
            fLocalEndpoint,
            fRemoteEndpoint,
            ::estd::optional<uint32_t>(0),
            false))
        .WillOnce(Return(IDoIpServerConnectionFilter::RoutingActivationCheckResult()));
    EXPECT_CALL(fCallbackMock, handleRoutingActivationRequest(Ref(cut)));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(activationRequest + 8U, 11U));
    Mock::VerifyAndClearExpectations(&fCallbackMock);
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    // Expect positive response
    EXPECT_TRUE(cut.isActivating());
    EXPECT_FALSE(cut.isRouting());
    EXPECT_FALSE(cut.isClosed());
    EXPECT_EQ(0x1234, cut.getSourceAddress());
}

TEST_F(DoIpServerConnectionHandlerTest, TestRoutingActivationWithBadActivationType)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    IDoIpConnectionHandler* connectionHandler = nullptr;
    // start the connection
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Receive an activation request
    uint8_t const activationRequest[] = {
        0x02, 0xfd, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    EXPECT_EQ(7U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(
            0x1234, 0x03, 0U, fLocalEndpoint, fRemoteEndpoint, ::estd::optional<uint32_t>(), false))
        .WillOnce(Return(IDoIpServerConnectionFilter::RoutingActivationCheckResult()
                             .setAction(IDoIpServerConnectionFilter::Action::REJECT)
                             .setResponseCode(0x06)));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    // Expect negative response
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(activationRequest + 8U, 3U));
    Mock::VerifyAndClearExpectations(&fCallbackMock);
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    uint8_t const activationResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x12,
           0x34,
           0x01,
           0x23,
           0x06,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse + 8U, 9U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // Expect close after release of send job
    EXPECT_CALL(fConnectionMock, close());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    sendJob->release(false);
}

TEST_F(DoIpServerConnectionHandlerTest, TestRoutingActivationWithBadSourceAddress)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    EXPECT_EQ(uint16_t(::transport::TransportMessage::INVALID_ADDRESS), cut.getSourceAddress());
    IDoIpConnectionHandler* connectionHandler = nullptr;
    // start the connection
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Receive an activation request
    uint8_t const activationRequest[] = {
        0x02, 0xfd, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    EXPECT_EQ(7U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(
            0x1234, 0x03, 0U, fLocalEndpoint, fRemoteEndpoint, ::estd::optional<uint32_t>(), false))
        .WillOnce(Return(IDoIpServerConnectionFilter::RoutingActivationCheckResult()
                             .setAction(IDoIpServerConnectionFilter::Action::REJECT)
                             .setResponseCode(0x0U)));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    // Expect negative response
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(activationRequest + 8U, 3U));
    Mock::VerifyAndClearExpectations(&fCallbackMock);
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    uint8_t const activationResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x12,
           0x34,
           0x01,
           0x23,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse + 8U, 9U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // Expect close after release of send job
    EXPECT_CALL(fConnectionMock, close());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    sendJob->release(false);
}

TEST_F(DoIpServerConnectionHandlerTest, TestRoutingActivationWithInvalidPayloadLength)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    IDoIpConnectionHandler* connectionHandler = nullptr;
    // start the connection
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Receive an activation request
    uint8_t const activationRequest[]
        = {0x02,
           0xfd,
           0x00,
           0x05,
           0x00,
           0x00,
           0x00,
           0x08,
           0x12,
           0x34,
           0x03,
           0x00,
           0x00,
           0x00,
           0x00,
           0x01};
    // Expect negative response
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    uint8_t const activationResponse[] = {0x02, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04};
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse + 8U, 1U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // Expect close after release of send job
    EXPECT_CALL(fConnectionMock, close());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    sendJob->release(false);
}

TEST_F(DoIpServerConnectionHandlerTest, TestRoutingActivationFails)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    IDoIpConnectionHandler* connectionHandler = nullptr;
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // start the connection
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Receive an activation request
    uint8_t const activationRequest[] = {
        0x02, 0xfd, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    EXPECT_EQ(7U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(
            0x1234, 0x03, 0U, fLocalEndpoint, fRemoteEndpoint, ::estd::optional<uint32_t>(), false))
        .WillOnce(Return(IDoIpServerConnectionFilter::RoutingActivationCheckResult()));
    EXPECT_CALL(fCallbackMock, handleRoutingActivationRequest(Ref(cut)));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(activationRequest + 8U, 3U));
    Mock::VerifyAndClearExpectations(&fCallbackMock);
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    // Expect negative response
    EXPECT_TRUE(cut.isActivating());
    EXPECT_FALSE(cut.isRouting());
    EXPECT_FALSE(cut.isClosed());
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    uint8_t const activationResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x12,
           0x34,
           0x01,
           0x23,
           0x05,
           0x00,
           0x00,
           0x00,
           0x00};
    cut.routingActivationCompleted(false, 0x05);
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse + 8U, 9U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // Now routing should be active
    EXPECT_FALSE(cut.isActivating());
    EXPECT_FALSE(cut.isRouting());
    EXPECT_FALSE(cut.isClosed());
    // Expect close after release of send job
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fConnectionMock, close());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    sendJob->release(false);
}

TEST_F(DoIpServerConnectionHandlerTest, TestRoutingActivationIsIgnoredIfConnectionIsClosed)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    IDoIpConnectionHandler* connectionHandler = nullptr;
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // start the connection
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Receive an activation request
    uint8_t const activationRequest[] = {
        0x02, 0xfd, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    EXPECT_EQ(7U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(
            0x1234, 0x03, 0U, fLocalEndpoint, fRemoteEndpoint, ::estd::optional<uint32_t>(), false))
        .WillOnce(Return(IDoIpServerConnectionFilter::RoutingActivationCheckResult()));
    EXPECT_CALL(fCallbackMock, handleRoutingActivationRequest(Ref(cut)));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(activationRequest + 8U, 3U));
    Mock::VerifyAndClearExpectations(&fCallbackMock);
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    // Expect negative response
    EXPECT_TRUE(cut.isActivating());
    EXPECT_FALSE(cut.isRouting());
    EXPECT_FALSE(cut.isClosed());
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    EXPECT_CALL(fConnectionMock, close());
    cut.close();
    cut.routingActivationCompleted(false, 0x05);
}

TEST_F(DoIpServerConnectionHandlerTest, TestRoutingActivationWillKeepConnection)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    IDoIpConnectionHandler* connectionHandler = nullptr;
    // start the connection
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Receive an activation request
    uint8_t const activationRequest[] = {
        0x02, 0xfd, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    EXPECT_EQ(7U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(
            0x1234, 0x03, 0U, fLocalEndpoint, fRemoteEndpoint, ::estd::optional<uint32_t>(), false))
        .WillOnce(Return(IDoIpServerConnectionFilter::RoutingActivationCheckResult()
                             .setAction(IDoIpServerConnectionFilter::Action::KEEP)
                             .setResponseCode(0x11)));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    // Expect response
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(activationRequest + 8U, 3U));
    Mock::VerifyAndClearExpectations(&fCallbackMock);
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    uint8_t const activationResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x12,
           0x34,
           0x01,
           0x23,
           0x11,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse + 8U, 9U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    sendJob->release(true);

    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    EXPECT_EQ(7U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(
            0x1234, 0x03, 0U, fLocalEndpoint, fRemoteEndpoint, ::estd::optional<uint32_t>(), false))
        .WillOnce(Return(IDoIpServerConnectionFilter::RoutingActivationCheckResult()));
    EXPECT_CALL(fCallbackMock, handleRoutingActivationRequest(Ref(cut)));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(activationRequest + 8U, 3U));
    Mock::VerifyAndClearExpectations(&fCallbackMock);
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    // Expect positive response
    EXPECT_TRUE(cut.isActivating());
    EXPECT_FALSE(cut.isRouting());
    EXPECT_FALSE(cut.isClosed());
    EXPECT_EQ(0x1234, cut.getSourceAddress());
    sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    uint8_t const activationResponse2[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x12,
           0x34,
           0x01,
           0x23,
           0x10,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    cut.routingActivationCompleted(true, 0x10);
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse2, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse2 + 8U, 9U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // Now routing should be active
    EXPECT_FALSE(cut.isActivating());
}

TEST_F(DoIpServerConnectionHandlerTest, TestInvalidHeaderVersion)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    IDoIpConnectionHandler* connectionHandler = nullptr;
    // start the connection
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Receive an activation request
    uint8_t const activationRequest[] = {
        0x03, 0xf5, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00};
    // Expect negative response
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    uint8_t const activationResponse[] = {0x02, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse + 8U, 1U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // Expect close after release of send job
    EXPECT_CALL(fConnectionMock, close());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    sendJob->release(true);
}

TEST_F(DoIpServerConnectionHandlerTest, TestUnknownPayloadType)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    IDoIpConnectionHandler* connectionHandler = nullptr;
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // start the connection
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Receive an unknown request
    uint8_t const activationRequest[] = {
        0x02, 0xfd, 0x08, 0x01, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00};
    // Expect negative response
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    EXPECT_CALL(fMessageHandlerMock, headerReceived(_)).WillOnce(Return(false));
    connectionHandler->headerReceived(as_header(activationRequest));
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    uint8_t const activationResponse[] = {0x02, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01};
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse + 8U, 1U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // No close expected
    sendJob->release(true);
}

TEST_F(DoIpServerConnectionHandlerTest, TestResume)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        111U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // Now resume
    IDoIpServerConnection* serverConnection = nullptr;
    Sequence seq;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_))
        .InSequence(seq)
        .WillOnce(SaveRef<0>(&serverConnection));
    EXPECT_CALL(fMessageHandlerMock, routingActive()).InSequence(seq);
    EXPECT_TRUE(cut.resume(0x1234U));
    // routing with valid source address
    EXPECT_TRUE(cut.isRouting());
    EXPECT_EQ(0x1234, cut.getSourceAddress());
    // And start
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck(111U);
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    cut.start(fCallbackMock);
    EXPECT_TRUE(serverConnection != nullptr);
    EXPECT_TRUE(connectionHandler != nullptr);
    // A Second resume should be neutral
    EXPECT_FALSE(cut.resume(0x4321U));
}

TEST_F(DoIpServerConnectionHandlerTest, TestSecondActivationWithSameSourceAddress)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Resume and start
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    EXPECT_TRUE(cut.resume(0x1234U));
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck();
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Receive an activation request
    uint8_t const activationRequest[] = {
        0x02, 0xfd, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    EXPECT_EQ(7U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(
            0x1234, 0x03, 0U, fLocalEndpoint, fRemoteEndpoint, ::estd::optional<uint32_t>(), false))
        .WillOnce(Return(IDoIpServerConnectionFilter::RoutingActivationCheckResult()));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    // Expect positive response
    EXPECT_TRUE(cut.isRouting());
    EXPECT_EQ(0x1234, cut.getSourceAddress());
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(activationRequest + 8U, 3U));
    Mock::VerifyAndClearExpectations(&fCallbackMock);
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    // Expect positive response
    EXPECT_TRUE(cut.isRouting());
    EXPECT_EQ(0x1234, cut.getSourceAddress());
    uint8_t const activationResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x12,
           0x34,
           0x01,
           0x23,
           0x10,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse + 8U, 9U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
}

TEST_F(DoIpServerConnectionHandlerTest, TestSecondActivationWithDifferentSourceAddress)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Resume and start
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    EXPECT_TRUE(cut.resume(0x1234U));
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck();
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Receive an activation request
    uint8_t const activationRequest[] = {
        0x02, 0xfd, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x15, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    EXPECT_EQ(7U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(
            0x1534, 0x03, 0U, fLocalEndpoint, fRemoteEndpoint, ::estd::optional<uint32_t>(), false))
        .WillOnce(Return(IDoIpServerConnectionFilter::RoutingActivationCheckResult()));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    // Receive payload
    EXPECT_TRUE(cut.isRouting());
    EXPECT_EQ(0x1234, cut.getSourceAddress());
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(activationRequest + 8U, 3U));
    Mock::VerifyAndClearExpectations(&fCallbackMock);
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    // Expect negative response
    EXPECT_FALSE(cut.isRouting());
    EXPECT_EQ(0x1234, cut.getSourceAddress());
    uint8_t const activationResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x15,
           0x34,
           0x01,
           0x23,
           0x02,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse + 8U, 9U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // Expect close after release of send job
    EXPECT_CALL(fConnectionMock, close());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    sendJob->release(true);
}

TEST_F(DoIpServerConnectionHandlerTest, TestReceptionOfCustomPayloadType)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // Now resume
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    EXPECT_CALL(fMessageHandlerMock, routingActive());
    EXPECT_TRUE(cut.resume(0x1234U));
    // And start
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck();
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    cut.start(fCallbackMock);
    EXPECT_TRUE(serverConnection != nullptr);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Receive a request header
    uint8_t const request[] = {0x02, 0xfd, 0x80, 0x01, 0x00, 0x00, 0x00, 0x07};
    EXPECT_CALL(fMessageHandlerMock, headerReceived(IsHeaderEqual(request))).WillOnce(Return(true));
    connectionHandler->headerReceived(as_header(request));
}

TEST_F(DoIpServerConnectionHandlerTest, TestAliveCheckRequestWithValidResponse)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // Now resume
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    EXPECT_CALL(fMessageHandlerMock, routingActive());
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    EXPECT_TRUE(cut.resume(0x1234U));
    // And start
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck();
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    EXPECT_TRUE(serverConnection != nullptr);
    // Expect an alive check request
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    cut.startAliveCheck();
    // Receive an alive check packet
    uint8_t const request[] = {0x02, 0xfd, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00};
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(request, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(request + 8U, 0U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // Response
    uint8_t const response[] = {0x02, 0xfd, 0x00, 0x08, 0x00, 0x00, 0x00, 0x02, 0x12, 0x34U};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(response));
    EXPECT_EQ(2U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    // Response received
    EXPECT_CALL(fCallbackMock, aliveCheckResponseReceived(Ref(cut), true));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(response + 8U, 2U));
}

TEST_F(DoIpServerConnectionHandlerTest, TestAliveCheckRequestWithInvalidResponse)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // Now resume
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    EXPECT_CALL(fMessageHandlerMock, routingActive());
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    EXPECT_TRUE(cut.resume(0x1234U));
    // And start
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck();
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    EXPECT_TRUE(serverConnection != nullptr);
    // Expect an alive check request
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    cut.startAliveCheck();
    // Receive an alive check packet
    uint8_t const request[] = {0x02, 0xfd, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00};
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(request, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(request + 8U, 0U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // Response
    uint8_t const response[] = {0x02, 0xfd, 0x00, 0x08, 0x00, 0x00, 0x00, 0x02, 0x12, 0x35U};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(response));
    EXPECT_EQ(2U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    // Response received => Expect invalid callback and close of connection
    EXPECT_CALL(fCallbackMock, aliveCheckResponseReceived(Ref(cut), false));
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fConnectionMock, close());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(response + 8U, 2U));
}

TEST_F(DoIpServerConnectionHandlerTest, TestAliveCheckResponseWithoutRequest)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // Now resume
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    EXPECT_CALL(fMessageHandlerMock, routingActive());
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    EXPECT_TRUE(cut.resume(0x1234U));
    // And start
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck();
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    EXPECT_TRUE(serverConnection != nullptr);
    // Alive check Response
    uint8_t const response[] = {0x02, 0xfd, 0x00, 0x08, 0x00, 0x00, 0x00, 0x02, 0x12, 0x34U};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(response));
    EXPECT_EQ(2U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    // Response received
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(response + 8U, 2U));
}

TEST_F(DoIpServerConnectionHandlerTest, TestAliveCheckTimeoutExpires)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // Now resume
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    EXPECT_CALL(fMessageHandlerMock, routingActive());
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    EXPECT_TRUE(cut.resume(0x1234U));
    // And start
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck();
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    EXPECT_TRUE(serverConnection != nullptr);
    // Expect an alive check request
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    cut.startAliveCheck();
    // Receive an alive check packet
    uint8_t const request[] = {0x02, 0xfd, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00};
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(request, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(request + 8U, 0U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // increase timer
    testContext.elapse(static_cast<uint64_t>(fParameters.getAliveCheckTimeout() - 1) * 1000U);
    testContext.expireAndExecute();
    testContext.elapse(1000U);
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fConnectionMock, close());
    Sequence seq;
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut))).InSequence(seq);
    EXPECT_CALL(fCallbackMock, aliveCheckResponseReceived(Ref(cut), false)).InSequence(seq);
    testContext.expireAndExecute();
}

TEST_F(DoIpServerConnectionHandlerTest, TestAliveCheckIfConnectionIsNotActive)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Start alive check in non-active connection
    cut.startAliveCheck();
    // start the connection
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    cut.start(fCallbackMock);
    // now start alive check and expect negative response
    EXPECT_CALL(fCallbackMock, aliveCheckResponseReceived(Ref(cut), false));
    cut.startAliveCheck();
}

TEST_F(DoIpServerConnectionHandlerTest, TestAliveCheckResponseWithInvalidPayloadLength)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Resume and start
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    EXPECT_TRUE(cut.resume(0x1234U));
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck();
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Send alive check response with bad length
    uint8_t const aliveCheckResponse[]
        = {0x02, 0xfd, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03, 0x12, 0x35U, 0x01U};
    // Expect negative response
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    connectionHandler->headerReceived(as_header(aliveCheckResponse));
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    uint8_t const nackResponse[] = {0x02, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04};
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(nackResponse, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(nackResponse + 8U, 1U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // Expect close after release of send job
    EXPECT_CALL(fConnectionMock, close());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    sendJob->release(true);
}

TEST_F(DoIpServerConnectionHandlerTest, TestInitialTimeout)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // start the connection
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    cut.start(fCallbackMock);

    // Receive a diagnostic message. The initial activity timeout shouldn't be reset
    uint8_t const diagnosticRequest[]
        = {0x02, 0xfd, 0x80, 0x01, 0x00, 0x00, 0x00, 0x05, 0x12, 0x34, 0x03, 0x00, 0x00};
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    EXPECT_CALL(fMessageHandlerMock, headerReceived(_)).WillOnce(Return(false));
    EXPECT_EQ(
        connectionHandler->headerReceived(as_header(diagnosticRequest)),
        IDoIpConnectionHandler::HeaderReceivedContinuation{
            IDoIpConnection::PayloadDiscardedCallbackType{}});

    // step within time
    testContext.elapse(
        static_cast<uint64_t>(fParameters.getInitialInactivityTimeout() - 1) * 1000U);
    testContext.expireAndExecute();
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fConnectionMock, close());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    testContext.elapse(1000U);
    testContext.expireAndExecute();
    // now start alive check and expect negative response
}

TEST_F(DoIpServerConnectionHandlerTest, TestGeneralInactivityTimeout)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // start the connection
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    EXPECT_TRUE(serverConnection != nullptr);
    cut.start(fCallbackMock);
    // Receive an activation request
    uint8_t const activationRequest[] = {
        0x02, 0xfd, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    EXPECT_EQ(7U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(
            0x1234, 0x03, 0U, fLocalEndpoint, fRemoteEndpoint, ::estd::optional<uint32_t>(), false))
        .WillOnce(Return(IDoIpServerConnectionFilter::RoutingActivationCheckResult()));
    EXPECT_CALL(fCallbackMock, handleRoutingActivationRequest(Ref(cut)));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(activationRequest + 8U, 3U));
    Mock::VerifyAndClearExpectations(&fCallbackMock);
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    // Expect positive response
    EXPECT_CALL(fMessageHandlerMock, routingActive());
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    uint8_t const activationResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x12,
           0x34,
           0x01,
           0x23,
           0x11,
           0x00,
           0x00,
           0x00,
           0x00};
    cut.routingActivationCompleted(true, 0x11);
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(activationResponse + 8U, 9U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // Routing is active => now general inactivity timer should expire if no message is sent
    testContext.elapse(
        static_cast<uint64_t>(fParameters.getGeneralInactivityTimeout() - 1) * 1000U);
    testContext.expireAndExecute();
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fConnectionMock, setCloseMode(IDoIpTcpConnection::CloseMode::ABORT));
    EXPECT_CALL(fConnectionMock, close());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    testContext.elapse(1000U);
    testContext.expireAndExecute();
    // now start alive check and expect negative response
}

TEST_F(DoIpServerConnectionHandlerTest, TestTimeoutDuringSendingNack)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    IDoIpConnectionHandler* connectionHandler = nullptr;
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // start the connection
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Receive an invalid activation request
    uint8_t const activationRequest[] = {
        0x02, 0xfd, 0x00, 0x05, 0x00, 0x00, 0x00, 0x08, 0x12, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00};
    // Expect negative response
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    EXPECT_TRUE(sendJob != nullptr);
    // Timeout will be ignored
    testContext.elapse(static_cast<uint64_t>(fParameters.getGeneralInactivityTimeout()) * 1000U);
    testContext.expireAndExecute();
    // close expected
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fConnectionMock, close());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    sendJob->release(true);
}

TEST_F(DoIpServerConnectionHandlerTest, TestServerConnectionSendsNackOnDemand)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // Now resume and start
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    EXPECT_CALL(fMessageHandlerMock, routingActive());
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    EXPECT_TRUE(cut.resume(0x1234U));
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck();
    cut.start(fCallbackMock);
    EXPECT_TRUE(serverConnection != nullptr);
    EXPECT_TRUE(connectionHandler != nullptr);
    // Expect negative response
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    // nack message is sent out and connection is closed afterwards
    serverConnection->sendNack(0x23U, true);
    uint8_t const nackResponse[] = {0x02, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x23};
    EXPECT_TRUE(sendJob != nullptr);
    EXPECT_EQ(2U, sendJob->getSendBufferCount());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(nackResponse, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(nackResponse + 8U, 1U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    // Expect close after release of send job
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fConnectionMock, close());
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    sendJob->release(true);
}

TEST_F(DoIpServerConnectionHandlerTest, TestServerConnectionCallsAreHandled)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    IDoIpConnectionHandler* connectionHandler = nullptr;
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // start the connection
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    cut.start(fCallbackMock);
    EXPECT_TRUE(serverConnection != nullptr);
    // expect init not to be forwarded
    serverConnection->init(fConnectionHandlerMock);
    // expect call for others
    // local endpoint:
    ::ip::IPEndpoint localEndpoint(::ip::make_ip4(0x123abcU), 12U);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(localEndpoint));
    EXPECT_EQ(localEndpoint, serverConnection->getLocalEndpoint());
    // remote endpoint
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x1abcedfU), 143U);
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(remoteEndpoint));
    EXPECT_EQ(remoteEndpoint, serverConnection->getRemoteEndpoint());
    // receive payload
    uint8_t payloadBuffer[8];
    ::estd::slice<uint8_t> receivedPayloadBuffer;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&receivedPayloadBuffer), Return(true)));
    EXPECT_TRUE(serverConnection->receivePayload(
        payloadBuffer, IDoIpConnection::PayloadReceivedCallbackType()));
    EXPECT_EQ(receivedPayloadBuffer.data(), payloadBuffer);
    EXPECT_EQ(receivedPayloadBuffer.size(), sizeof(payloadBuffer));
    // end receive message
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    serverConnection->endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
    // send message
    StrictMock<DoIpSendJobMock> sendJobMock; //(1, 1);
    EXPECT_CALL(fConnectionMock, sendMessage(Ref(sendJobMock))).WillOnce(Return(true));
    EXPECT_TRUE(serverConnection->sendMessage(sendJobMock));
    // close
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fConnectionMock, close());
    serverConnection->close();
}

TEST_F(DoIpServerConnectionHandlerTest, TestConnectionIsClosedIfNoMoreNackCanBeSent)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // Now resume and start
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    EXPECT_CALL(fMessageHandlerMock, routingActive());
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    EXPECT_TRUE(cut.resume(0x1234U));
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck();
    cut.start(fCallbackMock);
    EXPECT_TRUE(serverConnection != nullptr);
    EXPECT_TRUE(connectionHandler != nullptr);
    // send two nacks
    EXPECT_CALL(fConnectionMock, sendMessage(_)).WillOnce(Return(true));
    serverConnection->sendNack(0x11U, false);
    EXPECT_CALL(fConnectionMock, sendMessage(_)).WillOnce(Return(true));
    serverConnection->sendNack(0x12U, false);
    // close
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fConnectionMock, close());
    serverConnection->sendNack(0x13U, false);
}

TEST_F(DoIpServerConnectionHandlerTest, TestConnectionIsClosedIfNoAliveCheckRequestCanBeSent)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // Now resume and start
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    EXPECT_CALL(fMessageHandlerMock, routingActive());
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    EXPECT_TRUE(cut.resume(0x1234U));
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck();
    cut.start(fCallbackMock);
    EXPECT_TRUE(serverConnection != nullptr);
    EXPECT_TRUE(connectionHandler != nullptr);
    // send two nacks
    EXPECT_CALL(fConnectionMock, sendMessage(_)).WillOnce(Return(true));
    serverConnection->sendNack(0x11U, false);
    EXPECT_CALL(fConnectionMock, sendMessage(_)).WillOnce(Return(true));
    serverConnection->sendNack(0x12U, false);
    // close
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    EXPECT_CALL(fCallbackMock, aliveCheckResponseReceived(Ref(cut), false));
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fConnectionMock, close());
    cut.startAliveCheck();
}

TEST_F(
    DoIpServerConnectionHandlerTest, TestConnectionIsClosedIfNoRoutingActivationResponseCanBeSent)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // Now resume and start
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    EXPECT_CALL(fMessageHandlerMock, routingActive());
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    EXPECT_TRUE(cut.resume(0x1234U));
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck();
    cut.start(fCallbackMock);
    EXPECT_TRUE(serverConnection != nullptr);
    EXPECT_TRUE(connectionHandler != nullptr);
    // send two nacks
    EXPECT_CALL(fConnectionMock, sendMessage(_)).WillOnce(Return(true));
    serverConnection->sendNack(0x11U, false);
    EXPECT_CALL(fConnectionMock, sendMessage(_)).WillOnce(Return(true));
    serverConnection->sendNack(0x12U, false);
    // Receive the activation request
    uint8_t const activationRequest[] = {
        0x02, 0xfd, 0x00, 0x05, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x03, 0x00, 0x00, 0x00, 0x00};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    EXPECT_EQ(7U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(
            0x1234, 0x03, 0U, fLocalEndpoint, fRemoteEndpoint, ::estd::optional<uint32_t>(), false))
        .WillOnce(Return(IDoIpServerConnectionFilter::RoutingActivationCheckResult()));
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fConnectionMock, close());
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(activationRequest + 8U, 3U));
}

TEST_F(DoIpServerConnectionHandlerTest, TestSendJobsAreReleasedIfNotSentByConnection)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // Now resume and start
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    EXPECT_CALL(fMessageHandlerMock, routingActive());
    EXPECT_CALL(fCallbackMock, routingActive(Ref(cut)));
    EXPECT_TRUE(cut.resume(0x1234U));
    IDoIpConnectionHandler* connectionHandler = nullptr;
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    expectRoutingActivationCheck();
    cut.start(fCallbackMock);
    EXPECT_TRUE(serverConnection != nullptr);
    EXPECT_TRUE(connectionHandler != nullptr);
    // send two nacks
    EXPECT_CALL(fConnectionMock, sendMessage(_)).Times(6).WillRepeatedly(Return(false));
    serverConnection->sendNack(0x11U, false);
    serverConnection->sendNack(0x12U, false);
    serverConnection->sendNack(0x13U, false);
    serverConnection->sendNack(0x14U, false);
    serverConnection->sendNack(0x15U, false);
    serverConnection->sendNack(0x16U, false);
    // close connection
    EXPECT_CALL(fCallbackMock, connectionClosed(Ref(cut)));
    EXPECT_CALL(fMessageHandlerMock, connectionClosed());
    EXPECT_CALL(fConnectionMock, close());
    cut.close();
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    // try to send NACK again (nothing should happen)
    serverConnection->sendNack(0x11U, true);
}

TEST_F(DoIpServerConnectionHandlerTest, TestRoutingActivationOemField)
{
    DoIpServerConnectionHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        0U,
        fConnectionMock,
        asyncContext,
        0x123,
        fParameters);
    IDoIpConnectionHandler* connectionHandler = nullptr;
    // Add message handler
    cut.addMessageHandler(fMessageHandlerMock);
    // start the connection
    EXPECT_CALL(fConnectionMock, init(_)).WillOnce(SaveRef<0>(&connectionHandler));
    IDoIpServerConnection* serverConnection = nullptr;
    EXPECT_CALL(fMessageHandlerMock, connectionOpened(_)).WillOnce(SaveRef<0>(&serverConnection));
    cut.start(fCallbackMock);
    EXPECT_TRUE(connectionHandler != nullptr);
    EXPECT_TRUE(serverConnection != nullptr);
    // Starting again should be neutral
    cut.start(fCallbackMock);
    // Receive an activation request
    uint8_t const activationRequest[]
        = {0x02,
           0xfd,
           0x00,
           0x05,
           0x00,
           0x00,
           0x00,
           0x0b,
           0x12,
           0x34,
           0x03,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x00,
           0x01};
    ::estd::slice<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    connectionHandler->headerReceived(as_header(activationRequest));
    EXPECT_EQ(11U, payloadBuffer.size());
    EXPECT_TRUE(payloadCallback);
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(
            0x1234,
            0x03,
            0U,
            fLocalEndpoint,
            fRemoteEndpoint,
            ::estd::optional<uint32_t>(1),
            false))
        .WillOnce(Return(
            IDoIpServerConnectionFilter::RoutingActivationCheckResult()
                .setAction(IDoIpServerConnectionFilter::Action::REJECT)
                .setResponseCode(
                    ::doip::DoIpConstants::RoutingResponseCodes::ROUTING_MISSING_AUTHENTICATION)));
    EXPECT_CALL(fConnectionMock, sendMessage(_));
    EXPECT_CALL(fConnectionMock, endReceiveMessage(_));
    payloadCallback(::estd::slice<uint8_t const>::from_pointer(activationRequest + 8U, 11U));
    Mock::VerifyAndClearExpectations(&fCallbackMock);
    Mock::VerifyAndClearExpectations(&fConnectionMock);
    // Expect negative response
    EXPECT_FALSE(cut.isActivating());
    EXPECT_FALSE(cut.isRouting());
    EXPECT_FALSE(cut.isClosed());
}

void DoIpServerConnectionHandlerTest::expectRoutingActivationCheck(uint8_t const socketGroupId)
{
    EXPECT_CALL(fConnectionMock, getLocalEndpoint()).WillOnce(Return(fLocalEndpoint));
    EXPECT_CALL(fConnectionMock, getRemoteEndpoint()).WillOnce(Return(fRemoteEndpoint));
    EXPECT_CALL(
        fCallbackMock,
        checkRoutingActivation(_, _, socketGroupId, fLocalEndpoint, fRemoteEndpoint, _, _))
        .WillOnce(Return(IDoIpServerConnectionFilter::RoutingActivationCheckResult()));
}
