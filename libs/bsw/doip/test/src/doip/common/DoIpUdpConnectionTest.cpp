/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpUdpConnection.h"

#include "doip/common/DoIpConnectionHandlerMock.h"
#include "doip/common/DoIpSendJobMock.h"
#include "doip/common/DoIpTestHelpers.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <etl/array.h>
#include <etl/memory.h>
#include <etl/span.h>
#include <udp/socket/AbstractDatagramSocketMock.h>

#include <gmock/gmock.h>

#define BytesAreSpan(S) ElementsAreArray((S).data(), (S).size())

namespace doip
{
namespace test
{
using namespace ::testing;
using namespace ::udp;
using namespace ::udp::test;

ACTION_P4(ReceivePayload, cut, buffer, callback, result)
{
    ASSERT_EQ(result, cut->receivePayload(*buffer, *callback));
}

ACTION_P3(CheckEndpoint, cut, local, endpoint)
{
    ::ip::IPEndpoint expectedEndpoint = local ? cut->getLocalEndpoint() : cut->getRemoteEndpoint();
    ASSERT_EQ(expectedEndpoint.getAddress(), endpoint->getAddress());
    ASSERT_EQ(expectedEndpoint.getPort(), endpoint->getPort());
}

ACTION_P(EndReceiveMessage, cut)
{
    cut->endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
}

ACTION_P(Close, cut) { cut->close(); }

MATCHER_P(IsDoIpHeader, headerBytes, "")
{
    return is_equal(
        ::etl::span<DoIpHeader const, 1U>(&arg, 1U).reinterpret_as<uint8_t const>(),
        ::etl::span<uint8_t const>(headerBytes, DoIpConstants::DOIP_HEADER_LENGTH));
}

MATCHER_P3(IsDatagram, endpoint, buffer, exact_match, "")
{
    return (arg.getEndpoint() == endpoint)
           && (exact_match
                   ? (arg.getData() == buffer.data() && arg.getLength() == buffer.size())
                   : (is_equal(
                       ::etl::span<uint8_t const>(arg.getData(), arg.getLength()), buffer)));
}

struct DoIpUdpConnectionTest : Test
{
    DoIpUdpConnectionTest()
    : asyncMock()
    , asyncContext(1U)
    , testContext(asyncContext)
    , fPayloadReceivedCallback(
          IDoIpConnection::PayloadReceivedCallbackType::
              create<DoIpUdpConnectionTest, &DoIpUdpConnectionTest::payloadReceivedCallback>(*this))
    {}

    void SetUp() override { testContext.handleAll(); }

    virtual void TearDown() override {}

    MOCK_METHOD1(payloadReceivedCallback, void(::etl::span<uint8_t const>));

    ::etl::span<uint8_t const> getSendBuffer(::etl::span<uint8_t> destBuffer, uint8_t index)
    {
        ::etl::span<uint8_t const> srcBuffer = fSrcBufferArray[index];
        ::etl::mem_copy(srcBuffer.begin(), srcBuffer.end(), destBuffer.begin());
        return destBuffer.subspan(0U, srcBuffer.size());
    }

    ::testing::StrictMock<::async::AsyncMock> asyncMock;
    ::async::ContextType asyncContext;
    ::async::TestContext testContext;
    ::udp::AbstractDatagramSocketMock fSocketMock;
    DoIpConnectionHandlerMock fConnectionHandlerMock;
    IDoIpConnection::PayloadReceivedCallbackType fPayloadReceivedCallback;
    IDoIpConnection::PayloadReceivedCallbackType fNullPayloadReceivedCallback;
    ::etl::span<uint8_t const> fSrcBufferArray[2];
};

TEST_F(DoIpUdpConnectionTest, Init)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    ASSERT_EQ(&fSocketMock, &cut.getSocket());
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    // init again shouldn't care
    cut.init(fConnectionHandlerMock);
    // check the addresses
    EXPECT_FALSE(cut.getLocalEndpoint().isSet());
    EXPECT_FALSE(cut.getRemoteEndpoint().isSet());
}

TEST_F(DoIpUdpConnectionTest, ReceivePayload)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    cut.init(fConnectionHandlerMock);
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x87654321), 0x1122);
    ::ip::IPEndpoint localEndpoint(::ip::make_ip4(0x12345678), 0x4321);
    EXPECT_CALL(fSocketMock, getLocalIPAddress())
        .WillRepeatedly(Return(&localEndpoint.getAddress()));
    EXPECT_CALL(fSocketMock, getLocalPort()).WillRepeatedly(Return(localEndpoint.getPort()));
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    EXPECT_FALSE(cut.receivePayload(::etl::span<uint8_t>(), fPayloadReceivedCallback));
    // too short message
    EXPECT_CALL(fSocketMock, read(nullptr, 7U)).WillOnce(Return(7U));
    fSocketMock.getDataListener()->dataReceived(
        fSocketMock,
        remoteEndpoint.getAddress(),
        remoteEndpoint.getPort(),
        localEndpoint.getAddress(),
        7U);
    // now a valid message with payload of 11 bytes
    uint8_t const data[]
        = {0x02,
           0xfd,
           0x00,
           0x01,
           0x00,
           0x00,
           0x00,
           0xb,
           0x12,
           0x34,
           0x56,
           0x78,
           0x9a,
           0xbc,
           0xde,
           0xf0,
           0xe1,
           0xd2,
           0xc3};
    ::etl::array<uint8_t, 32> tooLongBuffer;
    ::etl::array<uint8_t, 10> perfectBuffer;
    EXPECT_CALL(fConnectionHandlerMock, headerReceived(IsDoIpHeader(data)))
        .WillOnce(DoAll(
            CheckEndpoint(&cut, false, &remoteEndpoint),
            CheckEndpoint(&cut, true, &localEndpoint),
            ReceivePayload(&cut, &perfectBuffer, &fNullPayloadReceivedCallback, false),
            ReceivePayload(&cut, &tooLongBuffer, &fPayloadReceivedCallback, false),
            ReceivePayload(&cut, &perfectBuffer, &fPayloadReceivedCallback, true),
            ReceivePayload(&cut, &perfectBuffer, &fPayloadReceivedCallback, false),
            Return(IDoIpConnectionHandler::HeaderReceivedContinuation{
                IDoIpConnectionHandler::HandledByThisHandler{}})));
    EXPECT_CALL(fSocketMock, read(NotNull(), 8U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(data).first(8))));
    EXPECT_CALL(fSocketMock, read(NotNull(), 10U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(data).subspan(8U, 10U))));
    EXPECT_CALL(fSocketMock, read(nullptr, 1U)).WillOnce(Return(1U));
    EXPECT_CALL(
        *this,
        payloadReceivedCallback(BytesAreSpan(::etl::span<uint8_t const>(data).subspan(8U, 10U))));
    fSocketMock.getDataListener()->dataReceived(
        fSocketMock,
        remoteEndpoint.getAddress(),
        remoteEndpoint.getPort(),
        localEndpoint.getAddress(),
        19U);
}

TEST_F(DoIpUdpConnectionTest, IgnoreMessage)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    cut.init(fConnectionHandlerMock);
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x87654321), 0x1122);
    ::ip::IPEndpoint localEndpoint(::ip::make_ip4(0x12345678), 0x4321);
    EXPECT_CALL(fSocketMock, getLocalIPAddress())
        .WillRepeatedly(Return(&localEndpoint.getAddress()));
    EXPECT_CALL(fSocketMock, getLocalPort()).WillRepeatedly(Return(localEndpoint.getPort()));
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    // data contains payload of 11 bytes
    uint8_t const data[]
        = {0x02,
           0xfd,
           0x00,
           0x01,
           0x00,
           0x00,
           0x00,
           0xb,
           0x12,
           0x34,
           0x56,
           0x78,
           0x9a,
           0xbc,
           0xde,
           0xf0,
           0xe1,
           0xd2,
           0xc3};
    EXPECT_CALL(fConnectionHandlerMock, headerReceived(IsDoIpHeader(data)))
        .WillOnce(DoAll(
            CheckEndpoint(&cut, false, &remoteEndpoint),
            CheckEndpoint(&cut, true, &localEndpoint),
            Return(IDoIpConnectionHandler::HeaderReceivedContinuation{
                IDoIpConnection::PayloadDiscardedCallbackType{}})));
    EXPECT_CALL(fSocketMock, read(NotNull(), 8U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(data).first(8))));
    EXPECT_CALL(fSocketMock, read(nullptr, 11U)).WillOnce(Return(11U));
    fSocketMock.getDataListener()->dataReceived(
        fSocketMock,
        remoteEndpoint.getAddress(),
        remoteEndpoint.getPort(),
        localEndpoint.getAddress(),
        19U);
}

TEST_F(DoIpUdpConnectionTest, EndReceiveMessage)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    cut.init(fConnectionHandlerMock);
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x87654321), 0x1122);
    ::ip::IPEndpoint localEndpoint(::ip::make_ip4(0x12345678), 0x4321);
    EXPECT_CALL(fSocketMock, getLocalIPAddress())
        .WillRepeatedly(Return(&localEndpoint.getAddress()));
    EXPECT_CALL(fSocketMock, getLocalPort()).WillRepeatedly(Return(localEndpoint.getPort()));
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    // can be called anytime
    cut.endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType{});
    // data contains payload of 11 bytes
    uint8_t const data[]
        = {0x02,
           0xfd,
           0x00,
           0x01,
           0x00,
           0x00,
           0x00,
           0xb,
           0x12,
           0x34,
           0x56,
           0x78,
           0x9a,
           0xbc,
           0xde,
           0xf0,
           0xe1,
           0xd2,
           0xc3};
    ::etl::array<uint8_t, 2> payloadBuffer;
    EXPECT_CALL(fConnectionHandlerMock, headerReceived(IsDoIpHeader(data)))
        .WillOnce(DoAll(
            CheckEndpoint(&cut, false, &remoteEndpoint),
            CheckEndpoint(&cut, true, &localEndpoint),
            ReceivePayload(&cut, &payloadBuffer, &fPayloadReceivedCallback, true),
            Return(IDoIpConnectionHandler::HeaderReceivedContinuation{
                IDoIpConnectionHandler::HandledByThisHandler{}})));
    EXPECT_CALL(fSocketMock, read(NotNull(), 8U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(data).first(8))));
    EXPECT_CALL(fSocketMock, read(NotNull(), 2U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(data).subspan(8, 2))));
    EXPECT_CALL(fSocketMock, read(nullptr, 9U)).WillOnce(Return(9U));
    EXPECT_CALL(
        *this,
        payloadReceivedCallback(BytesAreSpan(::etl::span<uint8_t const>(data).subspan(8U, 2U))))
        .WillOnce(EndReceiveMessage(&cut));
    fSocketMock.getDataListener()->dataReceived(
        fSocketMock,
        remoteEndpoint.getAddress(),
        remoteEndpoint.getPort(),
        localEndpoint.getAddress(),
        19U);
}

TEST_F(DoIpUdpConnectionTest, SendMessage)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    StrictMock<DoIpSendJobMock> sendJobMock;
    ::ip::IPEndpoint remoteIpEndpoint(::ip::make_ip4(0x87654321), 0x1135U);
    EXPECT_CALL(sendJobMock, getDestinationEndpoint())
        .WillOnce(Return(static_cast<::ip::IPEndpoint*>(nullptr)));
    EXPECT_FALSE(cut.sendMessage(sendJobMock));
    EXPECT_CALL(sendJobMock, getDestinationEndpoint()).WillRepeatedly(Return(&remoteIpEndpoint));
    EXPECT_FALSE(cut.sendMessage(sendJobMock));
    cut.init(fConnectionHandlerMock);
    EXPECT_TRUE(cut.sendMessage(sendJobMock));
    uint8_t const output[]
        = {0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0xa1, 0xb2, 0xc3, 0xd4};
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(1U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(10U));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(output, 12U)));
    EXPECT_CALL(
        fSocketMock,
        send(Matcher<DatagramPacket const&>(IsDatagram(
            remoteIpEndpoint, ::etl::span<uint8_t const>(output).subspan(0U, 12U), true))))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(sendJobMock, release(true));
    testContext.expireAndExecute();
}

TEST_F(DoIpUdpConnectionTest, SuspendResume)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    StrictMock<DoIpSendJobMock> sendJobMock;
    ::ip::IPEndpoint remoteIpEndpoint(::ip::make_ip4(0x87654321), 0x1135U);
    EXPECT_CALL(sendJobMock, getDestinationEndpoint())
        .WillOnce(Return(static_cast<::ip::IPEndpoint*>(nullptr)));
    EXPECT_FALSE(cut.sendMessage(sendJobMock));
    EXPECT_CALL(sendJobMock, getDestinationEndpoint()).WillRepeatedly(Return(&remoteIpEndpoint));
    EXPECT_FALSE(cut.sendMessage(sendJobMock));
    cut.init(fConnectionHandlerMock);
    cut.suspendSending();
    cut.suspendSending();
    EXPECT_TRUE(cut.sendMessage(sendJobMock));
    testContext.expireAndExecute();
    cut.resumeSending();
    testContext.expireAndExecute();
    cut.resumeSending();
    uint8_t const output[]
        = {0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0xa1, 0xb2, 0xc3, 0xd4};
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(1U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(10U));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(output, 12U)));
    EXPECT_CALL(
        fSocketMock,
        send(Matcher<DatagramPacket const&>(IsDatagram(
            remoteIpEndpoint, ::etl::span<uint8_t const>(output).subspan(0U, 12U), true))))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(sendJobMock, release(true));
    testContext.expireAndExecute();
}

TEST_F(DoIpUdpConnectionTest, Close)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    // shouldn't care
    cut.close();
    cut.init(fConnectionHandlerMock);
    EXPECT_CALL(fConnectionHandlerMock, connectionClosed(false));
    EXPECT_CALL(fSocketMock, close());
    cut.close();
}

TEST_F(DoIpUdpConnectionTest, SimpleLifecycleWithMessageReception)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    cut.init(fConnectionHandlerMock);
    ::ip::IPAddress remoteIpAddress = ::ip::make_ip4(0x87654321);
    ::ip::IPAddress localIpAddress  = ::ip::make_ip4(0x12345678);
    EXPECT_CALL(fSocketMock, getLocalIPAddress()).WillRepeatedly(Return(&localIpAddress));
    EXPECT_CALL(fSocketMock, getLocalPort()).WillRepeatedly(Return(0x4321));
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
    ::etl::array<uint8_t, 9U> payloadBuffer;
    EXPECT_CALL(fConnectionHandlerMock, headerReceived(IsDoIpHeader(input)))
        .WillOnce(DoAll(
            ReceivePayload(&cut, &payloadBuffer, &fPayloadReceivedCallback, true),
            Return(IDoIpConnectionHandler::HeaderReceivedContinuation{
                IDoIpConnectionHandler::HandledByThisHandler{}})));
    EXPECT_CALL(fSocketMock, read(_, 8U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(input).first(8))));
    EXPECT_CALL(fSocketMock, read(_, 9U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(input).subspan(8, 9))));
    EXPECT_CALL(
        *this,
        payloadReceivedCallback(BytesAreSpan(::etl::span<uint8_t const>(input).subspan(8U, 9U))));
    fSocketMock.getDataListener()->dataReceived(
        fSocketMock, remoteIpAddress, 0x1231, localIpAddress, 17U);
    EXPECT_CALL(fConnectionHandlerMock, connectionClosed(false)).Times(1);
    EXPECT_CALL(fSocketMock, close());
    cut.close();
}

TEST_F(DoIpUdpConnectionTest, SimpleLifecycleWithMessageTransmission)
{
    ::etl::array<uint8_t, 20U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    cut.init(fConnectionHandlerMock);
    ::ip::IPEndpoint remoteIpEndpoint(::ip::make_ip4(0x87654321), 0x1135U);
    ::ip::IPAddress localIpAddress = ::ip::make_ip4(0x12345678);
    EXPECT_CALL(fSocketMock, getLocalIPAddress()).WillRepeatedly(Return(&localIpAddress));
    EXPECT_CALL(fSocketMock, getLocalPort()).WillRepeatedly(Return(0x4321));
    // prepare send job
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
    StrictMock<DoIpSendJobMock> sendJobMock;
    EXPECT_CALL(sendJobMock, getDestinationEndpoint()).WillRepeatedly(Return(&remoteIpEndpoint));
    cut.sendMessage(sendJobMock);
    Sequence seq;
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(3U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(17U));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 0U))
        .InSequence(seq)
        .WillOnce(Return(::etl::span<uint8_t const>(output, 8U)));
    // empty buffer!
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 1U))
        .InSequence(seq)
        .WillOnce(Return(::etl::span<uint8_t const>()));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 2U))
        .InSequence(seq)
        .WillOnce(Return(::etl::span<uint8_t const>(output + 8U, 9U)));
    EXPECT_CALL(
        fSocketMock,
        send(Matcher<DatagramPacket const&>(IsDatagram(
            remoteIpEndpoint, ::etl::span<uint8_t const>(output).subspan(0U, 17U), false))))
        .InSequence(seq)
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(sendJobMock, release(true));
    testContext.expireAndExecute();
}

TEST_F(DoIpUdpConnectionTest, CloseConnectionDuringReception)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    cut.init(fConnectionHandlerMock);
    ::ip::IPAddress remoteIpAddress = ::ip::make_ip4(0x87654321);
    ::ip::IPAddress localIpAddress  = ::ip::make_ip4(0x12345678);
    EXPECT_CALL(fSocketMock, getLocalIPAddress()).WillRepeatedly(Return(&localIpAddress));
    EXPECT_CALL(fSocketMock, getLocalPort()).WillRepeatedly(Return(0x4321));
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
    EXPECT_CALL(fConnectionHandlerMock, headerReceived(IsDoIpHeader(input)))
        .WillOnce(DoAll(
            Close(&cut),
            Return(IDoIpConnectionHandler::HeaderReceivedContinuation{
                IDoIpConnectionHandler::HandledByThisHandler{}})));
    EXPECT_CALL(fSocketMock, read(_, 8U))
        .WillOnce(Invoke(ReadBytesFrom(::etl::span<uint8_t const>(input).first(8))));
    EXPECT_CALL(fSocketMock, read(nullptr, 9U)).WillOnce(Return(9U));
    EXPECT_CALL(fSocketMock, close());
    EXPECT_CALL(fConnectionHandlerMock, connectionClosed(false)).Times(1);
    fSocketMock.getDataListener()->dataReceived(
        fSocketMock, remoteIpAddress, 0x1231, localIpAddress, 17U);
}

TEST_F(DoIpUdpConnectionTest, ReleaseMessageOnFailedSendMessage)
{
    ::etl::array<uint8_t, 20U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    cut.init(fConnectionHandlerMock);
    ::ip::IPEndpoint remoteIpEndpoint(::ip::make_ip4(0x87654321), 0x1135U);
    ::ip::IPAddress localIpAddress = ::ip::make_ip4(0x12345678);
    EXPECT_CALL(fSocketMock, getLocalIPAddress()).WillRepeatedly(Return(&localIpAddress));
    EXPECT_CALL(fSocketMock, getLocalPort()).WillRepeatedly(Return(0x4321));
    uint8_t const output[] = {
        0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    StrictMock<DoIpSendJobMock> sendJobMock;
    EXPECT_CALL(sendJobMock, getDestinationEndpoint()).WillRepeatedly(Return(&remoteIpEndpoint));
    cut.sendMessage(sendJobMock);
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(2U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(sizeof(output)));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(output).subspan(0U, 8U)));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 1U))
        .WillOnce(Return(::etl::span<uint8_t const>(output).subspan(8U)));
    EXPECT_CALL(fSocketMock, send(An<::udp::DatagramPacket const&>()))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK));
    EXPECT_CALL(sendJobMock, release(false));
    testContext.expireAndExecute();
}

TEST_F(DoIpUdpConnectionTest, CloseConnectionDuringTransmission)
{
    ::etl::array<uint8_t, 20U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    cut.init(fConnectionHandlerMock);
    ::ip::IPEndpoint remoteIpEndpoint(::ip::make_ip4(0x87654321), 0x1135U);
    ::ip::IPAddress localIpAddress = ::ip::make_ip4(0x12345678);
    EXPECT_CALL(fSocketMock, getLocalIPAddress()).WillRepeatedly(Return(&localIpAddress));
    EXPECT_CALL(fSocketMock, getLocalPort()).WillRepeatedly(Return(0x4321));
    uint8_t const output[] = {
        0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    StrictMock<DoIpSendJobMock> sendJobMock;
    EXPECT_CALL(sendJobMock, getDestinationEndpoint()).WillRepeatedly(Return(&remoteIpEndpoint));
    cut.sendMessage(sendJobMock);
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(2U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(sizeof(output)));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 0U))
        .WillOnce(DoAll(Close(&cut), Return(::etl::span<uint8_t const>(output).subspan(0U, 8U))));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 1U))
        .WillOnce(Return(::etl::span<uint8_t const>(output).subspan(8U)));
    EXPECT_CALL(fSocketMock, send(An<::udp::DatagramPacket const&>()))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK));
    EXPECT_CALL(sendJobMock, release(false));
    EXPECT_CALL(fSocketMock, close());
    EXPECT_CALL(fConnectionHandlerMock, connectionClosed(false));
    testContext.expireAndExecute();
}

TEST_F(DoIpUdpConnectionTest, WriteIntoStaticBuffer)
{
    ::etl::array<uint8_t, 20U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    cut.init(fConnectionHandlerMock);
    ::ip::IPEndpoint remoteIpEndpoint(::ip::make_ip4(0x87654321), 0x1135U);
    ::ip::IPAddress localIpAddress = ::ip::make_ip4(0x12345678);
    EXPECT_CALL(fSocketMock, getLocalIPAddress()).WillRepeatedly(Return(&localIpAddress));
    EXPECT_CALL(fSocketMock, getLocalPort()).WillRepeatedly(Return(0x4321));
    uint8_t const output[] = {
        0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    StrictMock<DoIpSendJobMock> sendJobMock;
    EXPECT_CALL(sendJobMock, getDestinationEndpoint()).WillRepeatedly(Return(&remoteIpEndpoint));
    cut.sendMessage(sendJobMock);
    fSrcBufferArray[0] = ::etl::span<uint8_t const>(output).subspan(0U, 8U);
    fSrcBufferArray[1] = ::etl::span<uint8_t const>(output).subspan(8U);
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(2U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(sizeof(output)));
    EXPECT_CALL(fSocketMock, send(An<::udp::DatagramPacket const&>()))
        .WillRepeatedly(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, _))
        .WillRepeatedly(Invoke(this, &DoIpUdpConnectionTest::getSendBuffer));
    EXPECT_CALL(sendJobMock, release(true));
    testContext.expireAndExecute();
}

TEST_F(DoIpUdpConnectionTest, SendTooLongMessage)
{
    ::etl::array<uint8_t, 10U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    cut.init(fConnectionHandlerMock);
    ::ip::IPEndpoint remoteIpEndpoint(::ip::make_ip4(0x87654321), 0x1135U);
    ::ip::IPAddress localIpAddress = ::ip::make_ip4(0x12345678);
    EXPECT_CALL(fSocketMock, getLocalIPAddress()).WillRepeatedly(Return(&localIpAddress));
    EXPECT_CALL(fSocketMock, getLocalPort()).WillRepeatedly(Return(0x4321));
    uint8_t const output[] = {
        0x02, 0xfd, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    StrictMock<DoIpSendJobMock> sendJobMock;
    EXPECT_CALL(sendJobMock, getDestinationEndpoint()).WillOnce(Return(&remoteIpEndpoint));
    cut.sendMessage(sendJobMock);
    EXPECT_CALL(sendJobMock, getSendBufferCount()).WillRepeatedly(Return(2U));
    EXPECT_CALL(sendJobMock, getTotalLength()).WillRepeatedly(Return(sizeof(output)));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 0U))
        .WillOnce(Return(::etl::span<uint8_t const>(output).subspan(0U, 8U)));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 1U))
        .WillOnce(Return(::etl::span<uint8_t const>(output).subspan(8U)));
    EXPECT_CALL(sendJobMock, release(false));
    testContext.expireAndExecute();
}

} // namespace test
} // namespace doip
