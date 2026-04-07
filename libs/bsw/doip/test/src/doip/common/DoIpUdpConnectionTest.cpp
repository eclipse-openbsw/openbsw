// Copyright 2025 Accenture.

#include "doip/common/DoIpUdpConnection.h"

#include "doip/common/DoIpConnectionHandlerMock.h"
#include "doip/common/DoIpSendJobMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <udp/socket/AbstractDatagramSocketMock.h>

#include <estd/array.h>
#include <estd/memory.h>

#include <gmock/gmock.h>

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
    return ::estd::memory::is_equal(
        ::estd::memory::as_bytes(&arg),
        ::estd::slice<uint8_t const>::from_pointer(headerBytes, DoIpConstants::DOIP_HEADER_LENGTH));
}

MATCHER_P3(IsDatagram, endpoint, buffer, exact_match, "")
{
    return (arg.getEndpoint() == endpoint)
           && (exact_match
                   ? (arg.getData() == buffer.data() && arg.getLength() == buffer.size())
                   : (::estd::memory::is_equal(
                       ::estd::slice<uint8_t const>::from_pointer(arg.getData(), arg.getLength()),
                       buffer)));
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

    MOCK_METHOD1(payloadReceivedCallback, void(::estd::slice<uint8_t const>));

    ::estd::slice<uint8_t const> getSendBuffer(::estd::slice<uint8_t> destBuffer, uint8_t index)
    {
        ::estd::slice<uint8_t const> srcBuffer = fSrcBufferArray[index];
        ::estd::memory::copy(destBuffer, srcBuffer);
        return destBuffer.subslice(srcBuffer.size());
    }

    ::testing::StrictMock<::async::AsyncMock> asyncMock;
    ::async::ContextType asyncContext;
    ::async::TestContext testContext;
    ::udp::AbstractDatagramSocketMock fSocketMock;
    DoIpConnectionHandlerMock fConnectionHandlerMock;
    IDoIpConnection::PayloadReceivedCallbackType fPayloadReceivedCallback;
    IDoIpConnection::PayloadReceivedCallbackType fNullPayloadReceivedCallback;
    ::estd::slice<uint8_t const> fSrcBufferArray[2];
};

TEST_F(DoIpUdpConnectionTest, Init)
{
    ::estd::array<uint8_t, 10U> writeBuffer;
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
    ::estd::array<uint8_t, 10U> writeBuffer;
    DoIpUdpConnection cut(asyncContext, fSocketMock, writeBuffer);
    cut.init(fConnectionHandlerMock);
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x87654321), 0x1122);
    ::ip::IPEndpoint localEndpoint(::ip::make_ip4(0x12345678), 0x4321);
    EXPECT_CALL(fSocketMock, getLocalIPAddress())
        .WillRepeatedly(Return(&localEndpoint.getAddress()));
    EXPECT_CALL(fSocketMock, getLocalPort()).WillRepeatedly(Return(localEndpoint.getPort()));
    EXPECT_TRUE(fSocketMock.getDataListener() != nullptr);
    EXPECT_FALSE(cut.receivePayload(::estd::slice<uint8_t>(), fPayloadReceivedCallback));
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
    ::estd::array<uint8_t, 32> tooLongBuffer;
    ::estd::array<uint8_t, 10> perfectBuffer;
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
        payloadReceivedCallback(BytesAreSlice(::estd::make_slice(data).offset(8).subslice(10))));
    fSocketMock.getDataListener()->dataReceived(
        fSocketMock,
        remoteEndpoint.getAddress(),
        remoteEndpoint.getPort(),
        localEndpoint.getAddress(),
        19U);
}

TEST_F(DoIpUdpConnectionTest, IgnoreMessage)
{
    ::estd::array<uint8_t, 10U> writeBuffer;
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
    ::estd::array<uint8_t, 10U> writeBuffer;
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
    ::estd::array<uint8_t, 2> payloadBuffer;
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
        payloadReceivedCallback(BytesAreSlice(::estd::make_slice(data).offset(8).subslice(2))))
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
    ::estd::array<uint8_t, 10U> writeBuffer;
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
        .WillOnce(Return(::estd::slice<uint8_t const>::from_pointer(output, 12U)));
    EXPECT_CALL(
        fSocketMock,
        send(Matcher<DatagramPacket const&>(
            IsDatagram(remoteIpEndpoint, ::estd::make_slice(output).subslice(12), true))))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(sendJobMock, release(true));
    testContext.expireAndExecute();
}

TEST_F(DoIpUdpConnectionTest, SuspendResume)
{
    ::estd::array<uint8_t, 10U> writeBuffer;
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
        .WillOnce(Return(::estd::slice<uint8_t const>::from_pointer(output, 12U)));
    EXPECT_CALL(
        fSocketMock,
        send(Matcher<DatagramPacket const&>(
            IsDatagram(remoteIpEndpoint, ::estd::make_slice(output).subslice(12), true))))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(sendJobMock, release(true));
    testContext.expireAndExecute();
}

TEST_F(DoIpUdpConnectionTest, Close)
{
    ::estd::array<uint8_t, 10U> writeBuffer;
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
    ::estd::array<uint8_t, 10U> writeBuffer;
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
    ::estd::array<uint8_t, 9U> payloadBuffer;
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
        payloadReceivedCallback(BytesAreSlice(::estd::make_slice(input).offset(8).subslice(9))));
    fSocketMock.getDataListener()->dataReceived(
        fSocketMock, remoteIpAddress, 0x1231, localIpAddress, 17U);
    EXPECT_CALL(fConnectionHandlerMock, connectionClosed(false)).Times(1);
    EXPECT_CALL(fSocketMock, close());
    cut.close();
}

TEST_F(DoIpUdpConnectionTest, SimpleLifecycleWithMessageTransmission)
{
    ::estd::array<uint8_t, 20U> writeBuffer;
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
        .WillOnce(Return(::estd::slice<uint8_t const>::from_pointer(output, 8U)));
    // empty buffer!
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 1U))
        .InSequence(seq)
        .WillOnce(Return(::estd::slice<uint8_t const>()));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 2U))
        .InSequence(seq)
        .WillOnce(Return(::estd::slice<uint8_t const>::from_pointer(output + 8U, 9U)));
    EXPECT_CALL(
        fSocketMock,
        send(Matcher<DatagramPacket const&>(
            IsDatagram(remoteIpEndpoint, ::estd::make_slice(output).subslice(17), false))))
        .InSequence(seq)
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_OK));
    EXPECT_CALL(sendJobMock, release(true));
    testContext.expireAndExecute();
}

TEST_F(DoIpUdpConnectionTest, CloseConnectionDuringReception)
{
    ::estd::array<uint8_t, 10U> writeBuffer;
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
    ::estd::array<uint8_t, 20U> writeBuffer;
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
        .WillOnce(Return(::estd::make_slice(output).subslice(8U)));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 1U))
        .WillOnce(Return(::estd::make_slice(output).offset(8U)));
    EXPECT_CALL(fSocketMock, send(An<::udp::DatagramPacket const&>()))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK));
    EXPECT_CALL(sendJobMock, release(false));
    testContext.expireAndExecute();
}

TEST_F(DoIpUdpConnectionTest, CloseConnectionDuringTransmission)
{
    ::estd::array<uint8_t, 20U> writeBuffer;
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
        .WillOnce(DoAll(Close(&cut), Return(::estd::make_slice(output).subslice(8U))));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 1U))
        .WillOnce(Return(::estd::make_slice(output).offset(8U)));
    EXPECT_CALL(fSocketMock, send(An<::udp::DatagramPacket const&>()))
        .WillOnce(Return(::udp::AbstractDatagramSocket::ErrorCode::UDP_SOCKET_NOT_OK));
    EXPECT_CALL(sendJobMock, release(false));
    EXPECT_CALL(fSocketMock, close());
    EXPECT_CALL(fConnectionHandlerMock, connectionClosed(false));
    testContext.expireAndExecute();
}

TEST_F(DoIpUdpConnectionTest, WriteIntoStaticBuffer)
{
    ::estd::array<uint8_t, 20U> writeBuffer;
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
    fSrcBufferArray[0] = ::estd::make_slice(output).subslice(8U);
    fSrcBufferArray[1] = ::estd::make_slice(output).offset(8U);
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
    ::estd::array<uint8_t, 10U> writeBuffer;
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
        .WillOnce(Return(::estd::make_slice(output).subslice(8U)));
    EXPECT_CALL(sendJobMock, getSendBuffer(_, 1U))
        .WillOnce(Return(::estd::make_slice(output).offset(8U)));
    EXPECT_CALL(sendJobMock, release(false));
    testContext.expireAndExecute();
}

} // namespace test
} // namespace doip
