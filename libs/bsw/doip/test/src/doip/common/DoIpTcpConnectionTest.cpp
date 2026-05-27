/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpTcpConnection.h"

#include "doip/common/DoIpConnectionHandlerMock.h"
#include "doip/common/DoIpHeader.h"
#include "doip/common/DoIpSendJobMock.h"
#include "doip/common/DoIpTestHelpers.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <etl/array.h>
#include <etl/span.h>
#include <tcp/socket/AbstractSocketMock.h>

#include <gmock/gmock.h>

#define BytesAreSpan(S) ElementsAreArray((S).data(), (S).size())

namespace doip
{
namespace test
{
using namespace ::testing;
using namespace ::tcp::test;

MATCHER_P(IsDoIpHeader, headerBytes, "")
{
    return is_equal(
        ::etl::span<DoIpHeader const, 1U>(&arg, 1U).reinterpret_as<uint8_t const>(),
        ::etl::span<uint8_t const>(headerBytes, 8U));
}

MATCHER_P2(Span, dataMatcher, sizeMatcher, "")
{
    return Matches(dataMatcher)(arg.data()) && Matches(sizeMatcher)(arg.size());
}

struct DoIpTcpConnectionTest : Test
{
    DoIpTcpConnectionTest()
    : asyncMock()
    , asyncContext(1U)
    , testContext(asyncContext)
    , fPayloadReceivedCallback(
          IDoIpConnection::PayloadReceivedCallbackType::
              create<DoIpTcpConnectionTest, &DoIpTcpConnectionTest::payloadReceivedCallback>(*this))
    , fDetachCallback(
          IDoIpTcpConnection::DetachCallbackType::
              create<DoIpTcpConnectionTest, &DoIpTcpConnectionTest::detachCallback>(*this))
    {}

    void SetUp() override { testContext.handleAll(); }

    virtual void TearDown() override {}

    MOCK_METHOD1(payloadReceivedCallback, void(::etl::span<uint8_t const>));
    MOCK_METHOD0(detachCallback, void());

    ::testing::StrictMock<::async::AsyncMock> asyncMock;
    ::async::ContextType asyncContext;
    ::async::TestContext testContext;
    ::tcp::AbstractSocketMock fSocketMock;
    DoIpConnectionHandlerMock fConnectionHandlerMock;
    IDoIpConnection::PayloadReceivedCallbackType fPayloadReceivedCallback;
    IDoIpTcpConnection::DetachCallbackType fDetachCallback;
};

TEST_F(DoIpTcpConnectionTest, Constructor)
{
    {
        ::etl::array<uint8_t, 10U> writeBuffer;
        DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
        ASSERT_EQ(&fSocketMock, &cut.getSocket());
    }
}

TEST_F(DoIpTcpConnectionTest, Init)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    // init on non-established socket
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(false));
    EXPECT_CALL(fConnectionHandlerMock, connectionClosed(true)).Times(1);
    cut.init(fConnectionHandlerMock);
    EXPECT_EQ(nullptr, fSocketMock.getDataListener());
    // init on established socket
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    // init again shouldn't care
    cut.init(fConnectionHandlerMock);
    // address checks
    ::ip::IPEndpoint localEndpoint(::ip::make_ip4(0x12345678), 0x4321);
    EXPECT_CALL(fSocketMock, getLocalIPAddress())
        .WillRepeatedly(Return(localEndpoint.getAddress()));
    EXPECT_CALL(fSocketMock, getLocalPort()).WillRepeatedly(Return(localEndpoint.getPort()));
    EXPECT_EQ(localEndpoint, cut.getLocalEndpoint());
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x87654321), 0x1122);
    EXPECT_CALL(fSocketMock, getRemoteIPAddress())
        .WillRepeatedly(Return(remoteEndpoint.getAddress()));
    EXPECT_CALL(fSocketMock, getRemotePort()).WillRepeatedly(Return(remoteEndpoint.getPort()));
    EXPECT_EQ(remoteEndpoint, cut.getRemoteEndpoint());
}

TEST_F(DoIpTcpConnectionTest, ReceivePayload)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    EXPECT_FALSE(cut.receivePayload(::etl::span<uint8_t>(), fPayloadReceivedCallback));
    // header describes payload of length 31
    uint8_t const header[] = {0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1f};
    EXPECT_CALL(fConnectionHandlerMock, headerReceived(IsDoIpHeader(header)))
        .WillOnce(Return(IDoIpConnectionHandler::HeaderReceivedContinuation{
            IDoIpConnectionHandler::HandledByThisHandler{}}));
    EXPECT_CALL(fSocketMock, read(_, _)).WillOnce(Invoke(ReadBytesFrom(header)));
    fSocketMock.getDataListener()->dataReceived(8U);
    // receive more than payload length (31)
    ::etl::array<uint8_t, 32> tooLongBuffer;
    EXPECT_FALSE(cut.receivePayload(tooLongBuffer, fPayloadReceivedCallback));
    ::etl::array<uint8_t, 10> perfectBuffer;
    // receive payload without callback
    EXPECT_FALSE(cut.receivePayload(perfectBuffer, IDoIpConnection::PayloadReceivedCallbackType()));
    // receive payload with valid buffer
    EXPECT_TRUE(cut.receivePayload(perfectBuffer, fPayloadReceivedCallback));
    // receive during pending reception
    EXPECT_FALSE(cut.receivePayload(perfectBuffer, fPayloadReceivedCallback));
}

TEST_F(DoIpTcpConnectionTest, IgnoreMessage)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    // message
    uint8_t const data[]
        = {0x01, 0xfe, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x10, 0x21, 0x32, 0x43, 0x54, 0x65};
    EXPECT_CALL(fConnectionHandlerMock, headerReceived(IsDoIpHeader(data)))
        .WillOnce(Return(IDoIpConnectionHandler::HeaderReceivedContinuation{
            IDoIpConnection::PayloadDiscardedCallbackType{}}));
    EXPECT_CALL(fSocketMock, read(_, 8U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(data).first(8))));
    EXPECT_CALL(fSocketMock, read(nullptr, 3U)).WillOnce(Return(3U));
    fSocketMock.getDataListener()->dataReceived(11U);
    // end receive message
    EXPECT_CALL(fSocketMock, read(nullptr, 3U)).WillOnce(Return(3U));
    EXPECT_CALL(fSocketMock, read(_, 1U)).WillOnce(Return(1U));
    fSocketMock.getDataListener()->dataReceived(4U);
}

TEST_F(DoIpTcpConnectionTest, EndReceiveMessage)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    // can be called anytime
    cut.endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
    // header describes payload of length 7
    uint8_t const data[] = {
        0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    EXPECT_CALL(fConnectionHandlerMock, headerReceived(IsDoIpHeader(data)))
        .WillOnce(Return(IDoIpConnectionHandler::HeaderReceivedContinuation{
            IDoIpConnectionHandler::HandledByThisHandler{}}));
    EXPECT_CALL(fSocketMock, read(_, 8U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(data).first(8))));
    fSocketMock.getDataListener()->dataReceived(15U);
    // end receive message immediately
    EXPECT_CALL(fSocketMock, read(nullptr, 7U)).WillOnce(Return(7U));
    cut.endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
    // next message
    uint8_t const data2[]
        = {0x01, 0xfe, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x10, 0x21, 0x32, 0x43, 0x54, 0x65};
    EXPECT_CALL(fConnectionHandlerMock, headerReceived(IsDoIpHeader(data2)))
        .WillOnce(Return(IDoIpConnectionHandler::HeaderReceivedContinuation{
            IDoIpConnectionHandler::HandledByThisHandler{}}));
    EXPECT_CALL(fSocketMock, read(_, 8U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(data2).first(8))));
    fSocketMock.getDataListener()->dataReceived(11U);
    // read some payload
    ::etl::array<uint8_t, 4U> payloadBuffer;
    EXPECT_CALL(fSocketMock, read(_, 3U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(data2).subspan(8, 3))));
    cut.receivePayload(payloadBuffer, fPayloadReceivedCallback);
    EXPECT_CALL(fSocketMock, read(_, 1U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(data2).subspan(11, 1))));
    EXPECT_CALL(
        *this,
        payloadReceivedCallback(BytesAreSpan(::etl::span<uint8_t const>(data2).subspan(8U, 4U))));

    fSocketMock.getDataListener()->dataReceived(5U);
    // end receive message
    EXPECT_CALL(fSocketMock, read(nullptr, 2U)).WillOnce(Return(2U));
    EXPECT_CALL(fSocketMock, read(NotNull(), 2U)).WillOnce(Return(2U));
    cut.endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
}

TEST_F(DoIpTcpConnectionTest, SendMessage)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    StrictMock<DoIpSendJobMock> sendJobMock;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(1U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(10U));
    EXPECT_FALSE(cut.sendMessage(sendJobMock));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getSendNotificationListener() != nullptr);
    cut.sendMessage(sendJobMock);
    uint8_t output[] = {0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0xa1, 0xb2};
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(output)));
    EXPECT_CALL(fSocketMock, send(Span(output, 10U)))
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fSocketMock, flush())
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    testContext.expireAndExecute();
}

TEST_F(DoIpTcpConnectionTest, SuspendResume)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    StrictMock<DoIpSendJobMock> sendJobMock;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(1U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(10U));
    EXPECT_FALSE(cut.sendMessage(sendJobMock));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getSendNotificationListener() != nullptr);
    cut.suspendSending();
    cut.suspendSending();
    cut.sendMessage(sendJobMock);
    testContext.expireAndExecute();
    cut.resumeSending();
    testContext.expireAndExecute();
    cut.resumeSending();
    uint8_t output[] = {0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0xa1, 0xb2};
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(output)));
    EXPECT_CALL(fSocketMock, send(Span(output, 10U)))
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fSocketMock, flush())
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    testContext.expireAndExecute();
}

TEST_F(DoIpTcpConnectionTest, SendConsecutiveMessagesWithoutFlushing)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    StrictMock<DoIpSendJobMock> sendJobMock1;
    StrictMock<DoIpSendJobMock> sendJobMock2;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    EXPECT_FALSE(cut.sendMessage(sendJobMock1));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getSendNotificationListener() != nullptr);
    cut.sendMessage(sendJobMock1);
    cut.sendMessage(sendJobMock2);
    Sequence seq;
    uint8_t output[] = {0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0xa1, 0xb2};
    EXPECT_CALL(sendJobMock1, getSendBufferCount()).WillRepeatedly(Return(1U));
    EXPECT_CALL(sendJobMock1, getTotalLength()).WillRepeatedly(Return(10U));
    EXPECT_CALL(sendJobMock1, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(output)));
    EXPECT_CALL(sendJobMock2, getSendBufferCount()).WillRepeatedly(Return(1U));
    EXPECT_CALL(sendJobMock2, getTotalLength()).WillRepeatedly(Return(10U));
    EXPECT_CALL(sendJobMock2, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(output)));
    EXPECT_CALL(fSocketMock, send(Span(output, 10U)))
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fSocketMock, send(Span(output, 10U)))
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fSocketMock, flush())
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    testContext.expireAndExecute();
}

TEST_F(DoIpTcpConnectionTest, ContinueSendingOfFlushedMessageBlockAfterDataHasBeenQueued)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    StrictMock<DoIpSendJobMock> sendJobMock1;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    EXPECT_FALSE(cut.sendMessage(sendJobMock1));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getSendNotificationListener() != nullptr);
    cut.sendMessage(sendJobMock1);
    Sequence seq;
    EXPECT_CALL(sendJobMock1, getSendBufferCount()).WillRepeatedly(Return(2U));
    EXPECT_CALL(sendJobMock1, getTotalLength()).WillRepeatedly(Return(18U));
    EXPECT_CALL(sendJobMock1, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(writeBuffer).subspan(0U, 8U)));
    EXPECT_CALL(fSocketMock, send(Span(&writeBuffer[0], 8U)))
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_NO_MORE_BUFFER));
    EXPECT_CALL(fSocketMock, flush())
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&sendJobMock1);
    Mock::VerifyAndClearExpectations(&fSocketMock);
    EXPECT_CALL(sendJobMock1, getSendBufferCount()).WillRepeatedly(Return(2U));
    EXPECT_CALL(sendJobMock1, getTotalLength()).WillRepeatedly(Return(18U));
    fSocketMock.getSendNotificationListener()->dataSent(
        1U, ::tcp::IDataSendNotificationListener::SendResult::DATA_SENT);
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&sendJobMock1);
    Mock::VerifyAndClearExpectations(&fSocketMock);
    EXPECT_CALL(sendJobMock1, getSendBufferCount()).WillRepeatedly(Return(2U));
    EXPECT_CALL(sendJobMock1, getTotalLength()).WillRepeatedly(Return(18U));
    fSocketMock.getSendNotificationListener()->dataSent(
        8U, ::tcp::IDataSendNotificationListener::SendResult::DATA_QUEUED);
    EXPECT_CALL(sendJobMock1, getSendBuffer(_, 1U))
        .WillOnce(Return(::etl::span<uint8_t const>(writeBuffer).subspan(0U, 2U)));
    EXPECT_CALL(fSocketMock, send(Span(&writeBuffer[0U], 2U)))
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fSocketMock, flush())
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    testContext.expireAndExecute();
}

TEST_F(DoIpTcpConnectionTest, ContinueSendingOfFlushedMessageBlockAfterAllDataHasBeenSent)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    StrictMock<DoIpSendJobMock> sendJobMock1;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    EXPECT_FALSE(cut.sendMessage(sendJobMock1));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getSendNotificationListener() != nullptr);
    cut.sendMessage(sendJobMock1);
    Sequence seq;
    EXPECT_CALL(sendJobMock1, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(writeBuffer).subspan(0U, 8U)));
    EXPECT_CALL(fSocketMock, send(Span(&writeBuffer[0], 8U)))
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_NO_MORE_BUFFER));
    EXPECT_CALL(fSocketMock, flush())
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(sendJobMock1, getSendBufferCount()).WillRepeatedly(Return(2U));
    EXPECT_CALL(sendJobMock1, getTotalLength()).WillRepeatedly(Return(18U));
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&sendJobMock1);
    Mock::VerifyAndClearExpectations(&fSocketMock);
    fSocketMock.getSendNotificationListener()->dataSent(
        7U, ::tcp::IDataSendNotificationListener::SendResult::DATA_QUEUED);
    EXPECT_CALL(sendJobMock1, getSendBufferCount()).WillRepeatedly(Return(2U));
    EXPECT_CALL(sendJobMock1, getTotalLength()).WillRepeatedly(Return(18U));
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&sendJobMock1);
    Mock::VerifyAndClearExpectations(&fSocketMock);
    EXPECT_CALL(sendJobMock1, getSendBufferCount()).WillRepeatedly(Return(2U));
    EXPECT_CALL(sendJobMock1, getTotalLength()).WillRepeatedly(Return(18U));
    fSocketMock.getSendNotificationListener()->dataSent(
        1U, ::tcp::IDataSendNotificationListener::SendResult::DATA_QUEUED);
    EXPECT_CALL(sendJobMock1, getSendBuffer(_, 1U))
        .WillOnce(Return(::etl::span<uint8_t const>(writeBuffer).subspan(0U, 2U)));
    EXPECT_CALL(fSocketMock, send(Span(&writeBuffer[0U], 2U)))
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fSocketMock, flush())
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    testContext.expireAndExecute();
}

TEST_F(DoIpTcpConnectionTest, RepeatSendingOfUnsentMessageBlockAfterDataHasBeenQueued)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    StrictMock<DoIpSendJobMock> sendJobMock;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    EXPECT_FALSE(cut.sendMessage(sendJobMock));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getSendNotificationListener() != nullptr);
    cut.sendMessage(sendJobMock);
    Sequence seq;
    uint8_t output[] = {0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0xa1, 0xb2};
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(output)));
    EXPECT_CALL(fSocketMock, send(Span(output, 10U)))
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_FLUSH));
    EXPECT_CALL(fSocketMock, flush())
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(1U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(10U));
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&sendJobMock);
    Mock::VerifyAndClearExpectations(&fSocketMock);
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(1U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(10U));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(output)));
    EXPECT_CALL(fSocketMock, send(Span(output, 10U)))
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fSocketMock, flush())
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    testContext.elapse(1000U);
    testContext.expireAndExecute();
    Mock::VerifyAndClearExpectations(&sendJobMock);
    Mock::VerifyAndClearExpectations(&fSocketMock);
}

TEST_F(DoIpTcpConnectionTest, Close)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    // shouldn't care
    cut.close();
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    cut.init(fConnectionHandlerMock);
    EXPECT_CALL(fConnectionHandlerMock, connectionClosed(false));
    EXPECT_CALL(fSocketMock, close())
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    cut.close();
}

TEST_F(DoIpTcpConnectionTest, SimpleLifecycleWithMessageReception)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    uint8_t const input[]
        = {0x02,
           0xfd,
           0x00,
           0x01,
           0x00,
           0x00,
           0x00,
           0x09,
           0x11,
           0x22,
           0x33,
           0x44,
           0x55,
           0x66,
           0x77,
           0x88,
           0x99};
    EXPECT_CALL(fConnectionHandlerMock, headerReceived(_)).Times(0);
    EXPECT_CALL(fSocketMock, read(_, 6U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(input).first(6))));
    fSocketMock.getDataListener()->dataReceived(6U);
    EXPECT_CALL(fSocketMock, read(_, _))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(input).subspan(6, 2))));
    EXPECT_CALL(fConnectionHandlerMock, headerReceived(IsDoIpHeader(input)))
        .WillOnce(Return(IDoIpConnectionHandler::HeaderReceivedContinuation{
            IDoIpConnectionHandler::HandledByThisHandler{}}));
    fSocketMock.getDataListener()->dataReceived(17U);
    EXPECT_CALL(fSocketMock, read(_, _))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(input).subspan(8, 9U))));
    uint8_t readInput[9];
    EXPECT_TRUE(cut.receivePayload(::etl::span<uint8_t>(readInput), fPayloadReceivedCallback));
    cut.endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
    EXPECT_CALL(fConnectionHandlerMock, connectionClosed(false)).Times(1);
    EXPECT_CALL(fSocketMock, close())
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    cut.close();
}

TEST_F(DoIpTcpConnectionTest, SimpleLifecycleWithMessageTransmission)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    StrictMock<DoIpSendJobMock> sendJobMock;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getSendNotificationListener() != nullptr);
    uint8_t const output[]
        = {0x02,
           0xfd,
           0x00,
           0x01,
           0x00,
           0x00,
           0x00,
           0x09,
           0x11,
           0x22,
           0x33,
           0x44,
           0x55,
           0x66,
           0x77,
           0x88,
           0x99};
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(3U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(17U));
    // potentially the send job is already "used"
    cut.sendMessage(sendJobMock);
    Sequence seq;
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 0U))
        .InSequence(seq)
        .WillOnce(Return(::etl::span<uint8_t const>(output, 8U)));
    EXPECT_CALL(fSocketMock, send(Span(output, 8U)))
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    // empty buffer!
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 1U))
        .InSequence(seq)
        .WillOnce(Return(::etl::span<uint8_t const>()));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 2U))
        .InSequence(seq)
        .WillOnce(Return(::etl::span<uint8_t const>(output + 8U, 9U)));
    EXPECT_CALL(fSocketMock, send(Span(output + 8U, 9U)))
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fSocketMock, flush())
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    testContext.expireAndExecute();
    fSocketMock.getSendNotificationListener()->dataSent(
        17U, ::tcp::IDataSendNotificationListener::SendResult::DATA_NOT_SENT);
    fSocketMock.getSendNotificationListener()->dataSent(
        11U, ::tcp::IDataSendNotificationListener::SendResult::DATA_SENT);
    EXPECT_CALL(sendJobMock, release(true));
    fSocketMock.getSendNotificationListener()->dataSent(
        6U, ::tcp::IDataSendNotificationListener::SendResult::DATA_SENT);
    testContext.expireAndExecute();
}

TEST_F(DoIpTcpConnectionTest, CloseConnectionDuringSend)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    StrictMock<DoIpSendJobMock> sendJobMock;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    EXPECT_TRUE(fSocketMock.getSendNotificationListener() != nullptr);
    uint8_t const output[] = {
        0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    cut.sendMessage(sendJobMock);
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(2U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(15U));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(output, 8U)));
    EXPECT_CALL(fSocketMock, send(Span(output, 8U)))
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 1U))
        .WillRepeatedly(Return(::etl::span<uint8_t const>(writeBuffer).subspan(0U, 7U)));
    EXPECT_CALL(fSocketMock, send(Span(&writeBuffer[0], 7U)))
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_FLUSH));
    EXPECT_CALL(fSocketMock, flush())
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    testContext.expireAndExecute();
    EXPECT_CALL(fConnectionHandlerMock, connectionClosed(true));
    EXPECT_CALL(sendJobMock, release(false));
    fSocketMock.getDataListener()->connectionClosed(
        ::tcp::IDataListener::ErrorCode::ERR_CONNECTION_RESET);
    testContext.expireAndExecute();
}

TEST_F(DoIpTcpConnectionTest, AbortConnection)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    EXPECT_TRUE(fSocketMock.getSendNotificationListener() != nullptr);
    EXPECT_CALL(fConnectionHandlerMock, connectionClosed(false));
    EXPECT_CALL(fSocketMock, abort());
    cut.setCloseMode(IDoIpTcpConnection::CloseMode::ABORT);
    cut.close();
    testContext.expireAndExecute();
}

TEST_F(DoIpTcpConnectionTest, DetachConnection)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    EXPECT_TRUE(fSocketMock.getSendNotificationListener() != nullptr);
    EXPECT_CALL(*this, detachCallback());
    EXPECT_CALL(fConnectionHandlerMock, connectionClosed(false));
    cut.detach(fDetachCallback);
    testContext.expireAndExecute();
}

TEST_F(DoIpTcpConnectionTest, DetachConnectionDuringSend)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    StrictMock<DoIpSendJobMock> sendJobMock;
    DoIpTcpConnection cut(asyncContext, fSocketMock, writeBuffer);
    EXPECT_CALL(fSocketMock, isEstablished()).WillOnce(Return(true));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    EXPECT_TRUE(fSocketMock.getSendNotificationListener() != nullptr);
    uint8_t const output[] = {
        0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    cut.sendMessage(sendJobMock);
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(2U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(15U));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(output, 8U)));
    EXPECT_CALL(fSocketMock, send(Span(output, 8U)))
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 1U))
        .WillRepeatedly(Return(::etl::span<uint8_t const>(output + 8U, 7U)));
    EXPECT_CALL(fSocketMock, send(Span(output + 8U, 7U)))
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fSocketMock, flush())
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    testContext.expireAndExecute();
    cut.detach(fDetachCallback);
    fSocketMock.getSendNotificationListener()->dataSent(
        11U, ::tcp::IDataSendNotificationListener::SendResult::DATA_SENT);
    EXPECT_CALL(fConnectionHandlerMock, connectionClosed(false));
    EXPECT_CALL(sendJobMock, release(true));
    EXPECT_CALL(*this, detachCallback());
    fSocketMock.getSendNotificationListener()->dataSent(
        4U, ::tcp::IDataSendNotificationListener::SendResult::DATA_SENT);
    testContext.expireAndExecute();
}

} // namespace test
} // namespace doip
