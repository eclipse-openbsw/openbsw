// Copyright 2025 Accenture.

#include "doip/server/DoIpServerTransportLayer.h"

#include "doip/common/DoIpTransportMessageProvidingListenerHelper.h"
#include "doip/common/DoIpTransportMessageProvidingListenerMock.h"
#include "doip/server/DoIpServerSocketHandlerMock.h"
#include "doip/server/DoIpServerTransportConnectionPoolMock.h"
#include "doip/server/DoIpServerTransportLayerCallbackMock.h"
#include "doip/server/DoIpServerTransportLayerParameters.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <common/busid/BusId.h>
#include <tcp/socket/AbstractSocketMock.h>
#include <transport/BufferedTransportMessage.h>
#include <transport/TransportMessageProcessedListenerMock.h>

#include <estd/memory.h>
#include <estd/slice.h>

#include <gmock/gmock.h>
#include <gtest/esr_extensions.h>

namespace doip
{
namespace test
{
using namespace ::testing;
using namespace ::transport;
using namespace ::tcp::test;
using ConnectionType = ::doip::DoIpTcpConnection::ConnectionType;

MATCHER_P2(Slice, dataMatcher, sizeMatcher, "")
{
    return Matches(dataMatcher)(arg.data()) && Matches(sizeMatcher)(arg.size());
}

struct DoIpServerTransportLayerTest : Test
{
    DoIpServerTransportLayerTest()
    : fBusId()
    , asyncMock()
    , asyncContext(1U)
    , testContext(asyncContext)
    , fParameters(0x123U, 0x4412U, 0x2139U, 0x1648U)
    , fConfig(
          fBusId,
          0x2213U,
          asyncContext,
          fMessageProvidingListenerMock,
          fMessageProcessedListenerMock,
          fParameters)
    , fConnection1(
          DoIpConstants::ProtocolVersion::version02Iso2012,
          22U,
          fSocketMock1,
          fConfig,
          fDiagnosticSendJobBlockPool,
          fProtocolSendJobBlockPool,
          ConnectionType::PLAIN)
    , fConnection1_2(
          DoIpConstants::ProtocolVersion::version02Iso2012,
          22U,
          fSocketMock2,
          fConfig,
          fDiagnosticSendJobBlockPool,
          fProtocolSendJobBlockPool,
          ConnectionType::PLAIN)
    , fConnection1_3(
          DoIpConstants::ProtocolVersion::version02Iso2012,
          22U,
          fSocketMock3,
          fConfig,
          fDiagnosticSendJobBlockPool,
          fProtocolSendJobBlockPool,
          ConnectionType::PLAIN)
    , fConnection2(
          DoIpConstants::ProtocolVersion::version02Iso2012,
          33U,
          fSocketMock2,
          fConfig,
          fDiagnosticSendJobBlockPool,
          fProtocolSendJobBlockPool,
          ConnectionType::PLAIN)
    , fConnection3(
          DoIpConstants::ProtocolVersion::version02Iso2012,
          44U,
          fSocketMock3,
          fConfig,
          fDiagnosticSendJobBlockPool,
          fProtocolSendJobBlockPool,
          ConnectionType::PLAIN)
    , fBuffer()
    , fPersistentBuffer(fBuffer)
    , fLocalEndpoint1(::ip::make_ip4(0x4338U), 0x3484U)
    , fLocalEndpoint2(::ip::make_ip4(0x4338438U), 0x5484U)
    , fLocalEndpoint3(::ip::make_ip4(0x43438U), 0x9984U)
    , fRemoteEndpoint1(::ip::make_ip4(0x83473U), 0x3344U)
    , fRemoteEndpoint2(::ip::make_ip4(0x838233U), 0x344U)
    , fRemoteEndpoint3(::ip::make_ip4(0x232233U), 0x456U)
    {}

    void SetUp() override { testContext.handleAll(); }

    void TearDown() override {}

    MOCK_METHOD1(connectionSuspended, void(::estd::slice<uint8_t const>));

    void
    prepareSocket(::tcp::AbstractSocketMock& socketMock, ::ip::IPEndpoint const& remoteEndpoint);
    void prepareSocketGroup(
        uint8_t serverSocketId,
        ::ip::IPEndpoint const& remoteEndpoint,
        uint8_t socketGroupId,
        uint8_t maxConnectionCount);

    ::estd::slice<uint8_t> prepareRoutingActivationResponse(::tcp::AbstractSocketMock& socketMock);
    void expectRoutingActivationRequest(
        ::tcp::AbstractSocketMock& socketMock,
        uint16_t sourceAddress,
        ::ip::IPEndpoint const& localEndpoint,
        ::ip::IPEndpoint const& remoteEndpoint,
        uint8_t activationType = 0x01);
    void endRoutingActivationResponse(::tcp::AbstractSocketMock& socketMock);

    void prepareAliveCheckRequest(::tcp::AbstractSocketMock& socketMock);
    void expectAliveCheckResponse(::tcp::AbstractSocketMock& socketMock, uint16_t sourceAddress);
    void endAliveCheckRequest(::tcp::AbstractSocketMock& socketMock);

    void setupReadMessage(Sequence seq, uint8_t const* diagnosticMessage, size_t length);

    ::estd::slice<uint8_t> allocateBuffer(size_t size)
    {
        ::estd::slice<uint8_t> buffer = fPersistentBuffer.subslice(size);
        fPersistentBuffer             = fPersistentBuffer.offset(size);
        return buffer;
    }

    uint8_t fBusId;
    ::testing::StrictMock<::async::AsyncMock> asyncMock;
    ::async::ContextType asyncContext;
    ::async::TestContext testContext;
    StrictMock<DoIpServerSocketHandlerMock> fSocketHandlerMock;
    StrictMock<DoIpServerTransportConnectionPoolMock> fConnectionPoolMock;
    StrictMock<::tcp::AbstractSocketMock> fSocketMock1;
    StrictMock<::tcp::AbstractSocketMock> fSocketMock2;
    StrictMock<::tcp::AbstractSocketMock> fSocketMock3;
    DoIpServerTransportLayerParameters fParameters;
    StrictMock<DoIpTransportMessageProvidingListenerMock> fMessageProvidingListenerMock;
    StrictMock<TransportMessageProcessedListenerMock> fMessageProcessedListenerMock;
    StrictMock<DoIpServerTransportLayerCallbackMock> fTransportLayerCallbackMock;
    DoIpServerTransportConnectionConfig fConfig;
    DoIpServerTransportConnection fConnection1;
    DoIpServerTransportConnection fConnection1_2;
    DoIpServerTransportConnection fConnection1_3;
    DoIpServerTransportConnection fConnection2;
    DoIpServerTransportConnection fConnection3;
    ::util::estd::declare::
        block_pool<4, DoIpServerTransportMessageHandler::MIN_DIAGNOSTIC_SENDJOB_SIZE>
            fDiagnosticSendJobBlockPool;
    ::util::estd::declare::
        block_pool<4, DoIpServerTransportMessageHandler::MIN_PROTOCOL_SENDJOB_SIZE>
            fProtocolSendJobBlockPool;
    uint8_t fBuffer[200];
    ::estd::slice<uint8_t> fPersistentBuffer;
    ::ip::IPEndpoint fLocalEndpoint1;
    ::ip::IPEndpoint fLocalEndpoint2;
    ::ip::IPEndpoint fLocalEndpoint3;
    ::ip::IPEndpoint fRemoteEndpoint1;
    ::ip::IPEndpoint fRemoteEndpoint2;
    ::ip::IPEndpoint fRemoteEndpoint3;
};

TEST_F(DoIpServerTransportLayerTest, TestInitializationAndShutdown)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId, 0x2213U, asyncContext, fSocketHandlerMock, fConnectionPoolMock, fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();
    EXPECT_TRUE(listener != nullptr);

    cut.setTransportMessageProvidingListener(&fMessageProvidingListenerMock);

    // now shutdown
    EXPECT_CALL(fSocketHandlerMock, stop());
    EXPECT_TRUE(cut.shutdown(::transport::AbstractTransportLayer::ShutdownDelegate()));
}

TEST_F(DoIpServerTransportLayerTest, TestFilterConnections)
{
    ::doip::declare::DoIpServerTransportLayer<2> cut(
        fBusId, 0x2213U, asyncContext, fSocketHandlerMock, fConnectionPoolMock, fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();
    EXPECT_TRUE(listener != nullptr);
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x123U), 17);

    // accept without filter
    EXPECT_TRUE(listener->filterConnection(13U, remoteEndpoint));

    cut.setCallback(fTransportLayerCallbackMock);
    // filter out with filter set
    EXPECT_CALL(fTransportLayerCallbackMock, getSocketGroupId(17U, remoteEndpoint))
        .WillOnce(Return(14U));
    EXPECT_CALL(fTransportLayerCallbackMock, getMaxConnectionCount(14U)).WillOnce(Return(0U));
    EXPECT_FALSE(listener->filterConnection(17U, remoteEndpoint));

    // filter out with filter set
    EXPECT_CALL(fTransportLayerCallbackMock, getSocketGroupId(17U, remoteEndpoint))
        .WillOnce(Return(14U));
    EXPECT_CALL(fTransportLayerCallbackMock, getMaxConnectionCount(14U)).WillOnce(Return(1U));
    EXPECT_CALL(fTransportLayerCallbackMock, filterConnection(14U, remoteEndpoint))
        .WillOnce(Return(false));
    EXPECT_FALSE(listener->filterConnection(17U, remoteEndpoint));

    // accept if filter agrees
    EXPECT_CALL(fTransportLayerCallbackMock, getSocketGroupId(29U, remoteEndpoint))
        .WillOnce(Return(16U));
    EXPECT_CALL(fTransportLayerCallbackMock, getMaxConnectionCount(16U)).WillOnce(Return(1U));
    EXPECT_CALL(fTransportLayerCallbackMock, filterConnection(16U, remoteEndpoint))
        .WillOnce(Return(true));
    EXPECT_TRUE(listener->filterConnection(29U, remoteEndpoint));
}

TEST_F(DoIpServerTransportLayerTest, TestRoutingActivationWithoutFilter)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();

    EXPECT_EQ(0U, cut.getConnectionCount(22U));

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(0U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    listener->connectionAccepted(29U, fSocketMock1, ConnectionType::PLAIN);

    uint8_t const expectedResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x11,
           0x22,
           0x22,
           0x13,
           0x10,
           0x00,
           0x00,
           0x00,
           0x00};
    ::estd::slice<uint8_t> response = prepareRoutingActivationResponse(fSocketMock1);
    expectRoutingActivationRequest(fSocketMock1, 0x1122, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);
    EXPECT_TRUE(::estd::memory::is_equal(expectedResponse, response));

    EXPECT_EQ(1U, cut.getConnectionCount(22U));
}

TEST_F(DoIpServerTransportLayerTest, TestRoutingActivationWithInvalidActivationTypeFails)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(0U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    listener->connectionAccepted(37U, fSocketMock1, ConnectionType::PLAIN);

    uint8_t const expectedResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x11,
           0x22,
           0x22,
           0x13,
           0x06,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_CALL(fSocketMock1, close())
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    ::estd::slice<uint8_t> response = prepareRoutingActivationResponse(fSocketMock1);
    expectRoutingActivationRequest(fSocketMock1, 0x1122, fLocalEndpoint1, fRemoteEndpoint1, 0x02);
    endRoutingActivationResponse(fSocketMock1);
    EXPECT_TRUE(::estd::memory::is_equal(expectedResponse, response));

    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnection1)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock1), ConnectionType::PLAIN));
    testContext.expireAndExecute();
}

TEST_F(DoIpServerTransportLayerTest, TestRoutingActivationWithCallbackSucceeds)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.setCallback(fTransportLayerCallbackMock);
    cut.init();

    EXPECT_EQ(0U, cut.getConnectionCount(22U));

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(22U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    prepareSocketGroup(41U, fRemoteEndpoint1, 22U, 1U);
    listener->connectionAccepted(41U, fSocketMock1, ConnectionType::PLAIN);

    uint8_t const expectedResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x11,
           0x22,
           0x22,
           0x13,
           0x10,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_CALL(
        fTransportLayerCallbackMock,
        checkRoutingActivation(
            0x1122,
            0x01,
            22U,
            fLocalEndpoint1,
            fRemoteEndpoint1,
            ::estd::optional<uint32_t>(),
            false))
        .WillOnce(Return(::doip::IDoIpServerConnectionFilter::RoutingActivationCheckResult()));
    EXPECT_CALL(
        fTransportLayerCallbackMock,
        routingActive(
            0x1122,
            0x1122,
            fLocalEndpoint1,
            fRemoteEndpoint1,
            DoIpTcpConnection::ConnectionType::PLAIN));
    ::estd::slice<uint8_t> response = prepareRoutingActivationResponse(fSocketMock1);
    expectRoutingActivationRequest(fSocketMock1, 0x1122, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);
    EXPECT_TRUE(::estd::memory::is_equal(expectedResponse, response));

    EXPECT_EQ(1U, cut.getConnectionCount(22U));
}

TEST_F(DoIpServerTransportLayerTest, TestRoutingActivationWithCallbackFails)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.setCallback(fTransportLayerCallbackMock);
    cut.init();

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(22U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    prepareSocketGroup(49U, fRemoteEndpoint1, 22U, 1U);
    listener->connectionAccepted(49U, fSocketMock1, ConnectionType::PLAIN);

    uint8_t const expectedResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x11,
           0x22,
           0x22,
           0x13,
           0x05,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_CALL(
        fTransportLayerCallbackMock,
        checkRoutingActivation(
            0x1122,
            0x01,
            22U,
            fLocalEndpoint1,
            fRemoteEndpoint1,
            ::estd::optional<uint32_t>(),
            false))
        .WillOnce(Return(::doip::IDoIpServerConnectionFilter::RoutingActivationCheckResult()
                             .setAction(::doip::IDoIpServerConnectionFilter::Action::REJECT)
                             .setResponseCode(0x05)));
    EXPECT_CALL(fSocketMock1, close())
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnection1)));
    ::estd::slice<uint8_t> response = prepareRoutingActivationResponse(fSocketMock1);
    expectRoutingActivationRequest(fSocketMock1, 0x1122, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);
    EXPECT_TRUE(::estd::memory::is_equal(expectedResponse, response));

    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock1), ConnectionType::PLAIN));
    testContext.expireAndExecute();
}

TEST_F(DoIpServerTransportLayerTest, TestThirdSocketIsRejected)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.setCallback(fTransportLayerCallbackMock);
    cut.init();

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(22U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    prepareSocketGroup(41U, fRemoteEndpoint1, 22U, 1U);
    listener->connectionAccepted(41U, fSocketMock1, ConnectionType::PLAIN);

    uint8_t const expectedResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x11,
           0x22,
           0x22,
           0x13,
           0x10,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_CALL(
        fTransportLayerCallbackMock,
        checkRoutingActivation(
            0x1122,
            0x01,
            22U,
            fLocalEndpoint1,
            fRemoteEndpoint1,
            ::estd::optional<uint32_t>(),
            false))
        .WillOnce(Return(::doip::IDoIpServerConnectionFilter::RoutingActivationCheckResult()));
    EXPECT_CALL(
        fTransportLayerCallbackMock,
        routingActive(
            0x1122,
            0x1122,
            fLocalEndpoint1,
            fRemoteEndpoint1,
            DoIpTcpConnection::ConnectionType::PLAIN));
    ::estd::slice<uint8_t> response = prepareRoutingActivationResponse(fSocketMock1);
    expectRoutingActivationRequest(fSocketMock1, 0x1122, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);
    EXPECT_TRUE(::estd::memory::is_equal(expectedResponse, response));

    EXPECT_CALL(fConnectionPoolMock, createConnection(33U, Ref(fSocketMock2), _, _))
        .WillOnce(Return(&fConnection2));
    EXPECT_CALL(fSocketMock2, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock2, fRemoteEndpoint2);
    prepareSocketGroup(41U, fRemoteEndpoint2, 33U, 1U);
    listener->connectionAccepted(41U, fSocketMock2, ConnectionType::PLAIN);

    prepareSocket(fSocketMock3, fRemoteEndpoint3);
    prepareSocketGroup(39U, fRemoteEndpoint3, 44U, 1U);
    EXPECT_CALL(fTransportLayerCallbackMock, filterConnection(44U, fRemoteEndpoint3))
        .WillOnce(Return(false));
    EXPECT_FALSE(listener->filterConnection(39U, fRemoteEndpoint3));
}

TEST_F(DoIpServerTransportLayerTest, TestReserveSocket)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.setCallback(fTransportLayerCallbackMock);
    cut.init();

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(22U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    prepareSocketGroup(41U, fRemoteEndpoint1, 22U, 1U);
    listener->connectionAccepted(41U, fSocketMock1, ConnectionType::PLAIN);

    // first socket is connected ; maxConnectionCount == 1
    uint8_t const expectedResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x11,
           0x22,
           0x22,
           0x13,
           0x10,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_CALL(
        fTransportLayerCallbackMock,
        checkRoutingActivation(
            0x1122,
            0x01,
            22U,
            fLocalEndpoint1,
            fRemoteEndpoint1,
            ::estd::optional<uint32_t>(),
            false))
        .WillOnce(Return(::doip::IDoIpServerConnectionFilter::RoutingActivationCheckResult()));
    EXPECT_CALL(
        fTransportLayerCallbackMock,
        routingActive(
            0x1122,
            0x1122,
            fLocalEndpoint1,
            fRemoteEndpoint1,
            DoIpTcpConnection::ConnectionType::PLAIN));
    ::estd::slice<uint8_t> response = prepareRoutingActivationResponse(fSocketMock1);
    expectRoutingActivationRequest(fSocketMock1, 0x1122, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);
    EXPECT_TRUE(::estd::memory::is_equal(expectedResponse, response));

    // even if maximum connection count has been reached, a second (reserve) socket could be opened
    // in the same group in the TCP layer; this allows the new connection to replace the existing
    // one, via the alive check mechanism
    prepareSocketGroup(41U, fRemoteEndpoint2, 22U, 1U);
    EXPECT_CALL(fTransportLayerCallbackMock, filterConnection(22U, fRemoteEndpoint2))
        .WillOnce(Return(true));
    EXPECT_TRUE(listener->filterConnection(41U, fRemoteEndpoint2));
    return;
}

TEST_F(DoIpServerTransportLayerTest, TestSendWithConnection)
{
    ::doip::declare::DoIpServerTransportLayer<2> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(0U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    listener->connectionAccepted(63U, fSocketMock1, ConnectionType::PLAIN);

    // routing activation
    prepareRoutingActivationResponse(fSocketMock1);
    expectRoutingActivationRequest(fSocketMock1, 0x1122, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);

    // Send to a different address
    BufferedTransportMessage<5> message;
    message.setSourceAddress(0x2143);
    message.setTargetAddress(0x1125);
    message.append(0xf1);
    message.append(0x13);
    EXPECT_EQ(
        AbstractTransportLayer::ErrorCode::TP_SEND_FAIL,
        cut.send(message, &fMessageProcessedListenerMock));

    // set valid target address
    message.setTargetAddress(0x1122);
    // send up to buffer limit
    EXPECT_EQ(
        AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &fMessageProcessedListenerMock));
    EXPECT_EQ(
        AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &fMessageProcessedListenerMock));
    EXPECT_EQ(
        AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &fMessageProcessedListenerMock));
    EXPECT_EQ(
        AbstractTransportLayer::ErrorCode::TP_OK,
        cut.send(message, &fMessageProcessedListenerMock));
    // now expect error
    EXPECT_EQ(
        AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL,
        cut.send(message, &fMessageProcessedListenerMock));
}

TEST_F(DoIpServerTransportLayerTest, TestSendWithoutConnection)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();
    EXPECT_TRUE(listener != nullptr);

    BufferedTransportMessage<3> message;
    message.setSourceAddress(0x1234U);
    message.setTargetAddress(0x3322U);
    EXPECT_EQ(AbstractTransportLayer::ErrorCode::TP_SEND_FAIL, cut.send(message, nullptr));
}

TEST_F(DoIpServerTransportLayerTest, TestReceiveMessage)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();

    EXPECT_TRUE(listener != nullptr);
    DoIpServerTransportConnectionConfig const* config = 0;
    EXPECT_CALL(fConnectionPoolMock, createConnection(0U, Ref(fSocketMock1), _, _))
        .WillOnce(DoAll(WithArg<2>(SaveRef<0>(&config)), Return(&fConnection1)));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    listener->connectionAccepted(71U, fSocketMock1, ConnectionType::PLAIN);

    // routing activation
    prepareRoutingActivationResponse(fSocketMock1);
    expectRoutingActivationRequest(fSocketMock1, 0x1122, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);

    // Prepare a message
    BufferedTransportMessage<5> message;

    // Now receive the message
    uint8_t const diagnosticMessage[] = {
        0x02, 0xfd, 0x80, 0x01, 0x00, 0x00, 0x00, 0x07, 0x11, 0x22, 0x15, 0x19, 0x23, 0x35, 0x59};
    Sequence seq;
    setupReadMessage(seq, diagnosticMessage, sizeof(diagnosticMessage));

    EXPECT_CALL(fMessageProvidingListenerMock, getTransportMessage(fBusId, 0x1122, 0x1519, 3, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&message),
            Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(
                ITransportMessageProvider::ErrorCode::TPMSG_OK))));
    ITransportMessageProcessedListener* notificationListener = nullptr;
    EXPECT_CALL(fMessageProvidingListenerMock, messageReceived(fBusId, Ref(message), NotNull()))
        .WillOnce(DoAll(
            SaveArg<2>(&notificationListener),
            Return(DoIpTransportMessageProvidingListenerHelper::createReceiveResult(
                ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR))));
    // expect diagnostic ack
    uint8_t diagnosticAck[16];
    EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 8U)))
        .InSequence(seq)
        .WillOnce(Invoke(WriteBytesTo(
            ::estd::make_slice(diagnosticAck).subslice(8),
            ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
    EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 8U)))
        .InSequence(seq)
        .WillOnce(Invoke(WriteBytesTo(
            ::estd::make_slice(diagnosticAck).offset(8).subslice(8),
            ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
    EXPECT_CALL(fSocketMock1, flush())
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    fSocketMock1.getDataListener()->dataReceived(sizeof(diagnosticMessage));

    testContext.expireAndExecute();

    EXPECT_TRUE(notificationListener != nullptr);
    uint8_t const expectedDiagnosticAck[]
        = {0x02,
           0xfd,
           0x80,
           0x02,
           0x00,
           0x00,
           0x00,
           0x08,
           0x15,
           0x19,
           0x11,
           0x22,
           0x00,
           0x23,
           0x35,
           0x59};
    EXPECT_THAT(
        diagnosticAck,
        ::testing::ElementsAreArray(expectedDiagnosticAck, sizeof(expectedDiagnosticAck)));

    // check and release message
    EXPECT_EQ(0x1122, message.getSourceId());
    EXPECT_EQ(0x1519, message.getTargetId());
    EXPECT_EQ(3U, message.getPayloadLength());
    EXPECT_TRUE(::estd::memory::is_equal(
        ::estd::slice<uint8_t const>::from_pointer(diagnosticMessage + 12, 3),
        ::estd::slice<uint8_t const>::from_pointer(message.getPayload(), 3)));

    // Release the message at the provider that originally was returned
    EXPECT_TRUE(config != nullptr);
    config->getMessageProcessedListener().transportMessageProcessed(
        message, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
}

TEST_F(DoIpServerTransportLayerTest, TestReceiveMessageFollowedByInvalidTargetAddressMessage)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();

    EXPECT_TRUE(listener != nullptr);
    DoIpServerTransportConnectionConfig const* config = 0;
    EXPECT_CALL(fConnectionPoolMock, createConnection(0U, Ref(fSocketMock1), _, _))
        .WillOnce(DoAll(WithArg<2>(SaveRef<0>(&config)), Return(&fConnection1)));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    listener->connectionAccepted(99U, fSocketMock1, ConnectionType::PLAIN);

    // routing activation
    prepareRoutingActivationResponse(fSocketMock1);
    expectRoutingActivationRequest(fSocketMock1, 0x1234, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);

    {
        BufferedTransportMessage<5> message;

        // Now receive the message
        uint8_t const diagnosticMessage[]
            = {0x02,
               0xfd,
               0x80,
               0x01,
               0x00,
               0x00,
               0x00,
               0x07,
               0x12,
               0x34,
               0x15,
               0x19,
               0x23,
               0x35,
               0x59};
        Sequence seq;
        setupReadMessage(seq, diagnosticMessage, sizeof(diagnosticMessage));

        EXPECT_CALL(
            fMessageProvidingListenerMock, getTransportMessage(fBusId, 0x1234, 0x1519, 3, _, _))
            .WillOnce(DoAll(
                SetArgReferee<5>(&message),
                Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(
                    ITransportMessageProvider::ErrorCode::TPMSG_OK))));
        ITransportMessageProcessedListener* notificationListener = nullptr;
        EXPECT_CALL(fMessageProvidingListenerMock, messageReceived(fBusId, Ref(message), NotNull()))
            .WillOnce(DoAll(
                SaveArg<2>(&notificationListener),
                Return(DoIpTransportMessageProvidingListenerHelper::createReceiveResult(
                    ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR))));
        // expect diagnostic ack
        uint8_t diagnosticAck[16];
        {
            InSequence s;
            EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 8U)))
                .WillOnce(Invoke(WriteBytesTo(
                    ::estd::slice<uint8_t>::from_pointer(diagnosticAck, 8),
                    ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
            EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 8U)))
                .WillOnce(Invoke(WriteBytesTo(
                    ::estd::slice<uint8_t>::from_pointer(diagnosticAck + 8, 8),
                    ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
        }
        EXPECT_CALL(fSocketMock1, flush())
            .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
        fSocketMock1.getDataListener()->dataReceived(sizeof(diagnosticMessage));

        testContext.expireAndExecute();

        EXPECT_TRUE(notificationListener != nullptr);
        uint8_t const expectedDiagnosticAck[] = {
            0x02,
            0xfd,
            0x80,
            0x02,
            0x00,
            0x00,
            0x00,
            0x08,
            0x15,
            0x19,
            0x12,
            0x34,
            0x00,
            0x23,
            0x35,
            0x59,
        };
        EXPECT_THAT(
            diagnosticAck,
            ::testing::ElementsAreArray(expectedDiagnosticAck, sizeof(expectedDiagnosticAck)));

        // Release the message at the provider that originally was returned
        EXPECT_TRUE(config != nullptr);
        config->getMessageProcessedListener().transportMessageProcessed(
            message, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    }

    // Now send invalid target ID message
    {
        uint8_t const diagnosticMessage[]
            = {0x02,
               0xfd,
               0x80,
               0x01,
               0x00,
               0x00,
               0x00,
               0x09,
               0x12,
               0x34,
               0x15,
               0x19,
               0x12,
               0x34,
               0x45,
               0x56,
               0x67};
        Sequence seq;
        setupReadMessage(seq, diagnosticMessage, sizeof(diagnosticMessage));

        EXPECT_CALL(
            fMessageProvidingListenerMock, getTransportMessage(fBusId, 0x1234, 0x1519, 5, _, _))
            .WillOnce(Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(
                ITransportMessageProvider::ErrorCode::TPMSG_INVALID_TGT_ADDRESS)));

        // expect diagnostic ack
        uint8_t diagnosticNack[18];
        EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 8U)))
            .WillOnce(Invoke(WriteBytesTo(
                ::estd::slice<uint8_t>::from_pointer(diagnosticNack, 8),
                ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));

        EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 10U)))
            .WillOnce(Invoke(WriteBytesTo(
                ::estd::slice<uint8_t>::from_pointer(diagnosticNack + 8, 10),
                ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
        EXPECT_CALL(fSocketMock1, flush())
            .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));

        fSocketMock1.getDataListener()->dataReceived(sizeof(diagnosticMessage));

        testContext.expireAndExecute();

        uint8_t const expectedDiagnosticNack[]
            = {0x02,
               0xfd,
               0x80,
               0x03,
               0x00,
               0x00,
               0x00,
               0x0a,
               0x15,
               0x19,
               0x12,
               0x34,
               0x03,
               0x12,
               0x34,
               0x45,
               0x56,
               0x67};
        EXPECT_THAT(
            diagnosticNack,
            ::testing::ElementsAreArray(expectedDiagnosticNack, sizeof(expectedDiagnosticNack)));
    }

    // send a valid message now
    {
        BufferedTransportMessage<5> message;
        // Now receive the message
        uint8_t const diagnosticMessage[]
            = {0x02,
               0xfd,
               0x80,
               0x01,
               0x00,
               0x00,
               0x00,
               0x07,
               0x12,
               0x34,
               0x15,
               0x19,
               0x23,
               0x35,
               0x59};
        Sequence seq;
        setupReadMessage(seq, diagnosticMessage, sizeof(diagnosticMessage));

        EXPECT_CALL(
            fMessageProvidingListenerMock, getTransportMessage(fBusId, 0x1234, 0x1519, 3, _, _))
            .WillOnce(DoAll(
                SetArgReferee<5>(&message),
                Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(
                    ITransportMessageProvider::ErrorCode::TPMSG_OK))));
        ITransportMessageProcessedListener* notificationListener = nullptr;
        EXPECT_CALL(fMessageProvidingListenerMock, messageReceived(fBusId, Ref(message), NotNull()))
            .WillOnce(DoAll(
                SaveArg<2>(&notificationListener),
                Return(DoIpTransportMessageProvidingListenerHelper::createReceiveResult(
                    ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR))));
        // expect diagnostic ack
        uint8_t diagnosticAck[16];
        EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 8U)))
            .InSequence(seq)
            .WillOnce(Invoke(WriteBytesTo(
                ::estd::slice<uint8_t>::from_pointer(diagnosticAck, 8),
                ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
        EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 8U)))
            .InSequence(seq)
            .WillOnce(Invoke(WriteBytesTo(
                ::estd::make_slice(diagnosticAck).offset(8).subslice(8),
                ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
        EXPECT_CALL(fSocketMock1, flush())
            .InSequence(seq)
            .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
        fSocketMock1.getDataListener()->dataReceived(sizeof(diagnosticMessage));

        testContext.expireAndExecute();

        EXPECT_TRUE(notificationListener != nullptr);
        uint8_t const expectedDiagnosticAck[]
            = {0x02,
               0xfd,
               0x80,
               0x02,
               0x00,
               0x00,
               0x00,
               0x08,
               0x15,
               0x19,
               0x12,
               0x34,
               0x00,
               0x23,
               0x35,
               0x59};
        EXPECT_THAT(
            diagnosticAck,
            ::testing::ElementsAreArray(expectedDiagnosticAck, sizeof(expectedDiagnosticAck)));

        // Release the message at the provider that originally was returned
        EXPECT_TRUE(config != nullptr);
        config->getMessageProcessedListener().transportMessageProcessed(
            message, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    }
}

TEST_F(
    DoIpServerTransportLayerTest,
    TestReceiveMessageFollowedByInvalidTargetAddressMessageSinglePayload)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();

    EXPECT_TRUE(listener != nullptr);
    DoIpServerTransportConnectionConfig const* config = 0;
    EXPECT_CALL(fConnectionPoolMock, createConnection(0U, Ref(fSocketMock1), _, _))
        .WillOnce(DoAll(WithArg<2>(SaveRef<0>(&config)), Return(&fConnection1)));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    listener->connectionAccepted(107U, fSocketMock1, ConnectionType::PLAIN);

    // routing activation
    prepareRoutingActivationResponse(fSocketMock1);
    expectRoutingActivationRequest(fSocketMock1, 0x1234, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);

    {
        BufferedTransportMessage<5> message1;
        BufferedTransportMessage<5> message4;

        // Now receive the message
        uint8_t const diagnosticMessage[] = {
            0x02,
            0xfd,
            0x80,
            0x01,
            0x00,
            0x00,
            0x00,
            0x07,
            0x12,
            0x34,
            0x15,
            0x19,
            0x23,
            0x35,
            0x59,

            // invalid target ID with less than five bytes payload
            0x02,
            0xfd,
            0x80,
            0x01,
            0x00,
            0x00,
            0x00,
            0x08,
            0x12,
            0x34,
            0x15,
            0x16,
            0x12,
            0x34,
            0x45,
            0x56,

            // invalid target ID with more than five bytes payload
            0x02,
            0xfd,
            0x80,
            0x01,
            0x00,
            0x00,
            0x00,
            0x0d,
            0x12,
            0x34,
            0x15,
            0x17,
            0x12,
            0x34,
            0x45,
            0x56,
            0x67,
            0x78,
            0x89,
            0x9a,
            0xab,

            // valid message
            0x02,
            0xfd,
            0x80,
            0x01,
            0x00,
            0x00,
            0x00,
            0x08,
            0x12,
            0x34,
            0x15,
            0x19,
            0x23,
            0x35,
            0x59,
            0x60,
        };

        // handle first valid message
        uint8_t const* messagePtr = diagnosticMessage;

        Sequence seq;

        // read header
        EXPECT_CALL(fSocketMock1, read(NotNull(), 8U))
            .InSequence(seq)
            .WillOnce(
                Invoke(ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(messagePtr, 8))));
        messagePtr += 8;
        // read source/target IDs
        EXPECT_CALL(fSocketMock1, read(NotNull(), 4U))
            .InSequence(seq)
            .WillOnce(
                Invoke(ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(messagePtr, 4))));
        messagePtr += 4;
        // read payload (peek of size 3 bytes)
        EXPECT_CALL(fSocketMock1, read(NotNull(), 3U))
            .InSequence(seq)
            .WillOnce(
                Invoke(ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(messagePtr, 3U))));
        messagePtr += 3;

        // Now handle invalid target ID
        // read header
        EXPECT_CALL(fSocketMock1, read(NotNull(), 8U))
            .InSequence(seq)
            .WillOnce(
                Invoke(ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(messagePtr, 8))));
        messagePtr += 8;
        // read source/target IDs
        EXPECT_CALL(fSocketMock1, read(NotNull(), 4U))
            .InSequence(seq)
            .WillOnce(
                Invoke(ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(messagePtr, 4))));
        messagePtr += 4;
        // read payload (peek of size 4 bytes)
        EXPECT_CALL(fSocketMock1, read(NotNull(), 4U))
            .InSequence(seq)
            .WillOnce(
                Invoke(ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(messagePtr, 4U))));
        messagePtr += 4;
        // // skip rest of payload
        // EXPECT_CALL(fSocketMock1, read(nullptr, 5U)).InSequence(seq).WillOnce(Return(5U));
        // messagePtr += 5;

        // Now handle invalid target ID
        // read header
        EXPECT_CALL(fSocketMock1, read(NotNull(), 8U))
            .InSequence(seq)
            .WillOnce(
                Invoke(ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(messagePtr, 8))));
        messagePtr += 8;
        // read source/target IDs
        EXPECT_CALL(fSocketMock1, read(NotNull(), 4U))
            .InSequence(seq)
            .WillOnce(
                Invoke(ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(messagePtr, 4))));
        messagePtr += 4;
        // read the full peek of 5 bytes (4 bytes for peek + 1 byte for possible nack)
        EXPECT_CALL(fSocketMock1, read(NotNull(), 5U))
            .InSequence(seq)
            .WillOnce(
                Invoke(ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(messagePtr, 5U))));
        messagePtr += 5;
        // skip rest of payload
        EXPECT_CALL(fSocketMock1, read(nullptr, 4U)).InSequence(seq).WillOnce(Return(4U));
        messagePtr += 4;

        // now valid message
        // read header
        EXPECT_CALL(fSocketMock1, read(NotNull(), 8U))
            .InSequence(seq)
            .WillOnce(
                Invoke(ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(messagePtr, 8))));
        messagePtr += 8;
        // read source/target IDs
        EXPECT_CALL(fSocketMock1, read(NotNull(), 4U))
            .InSequence(seq)
            .WillOnce(
                Invoke(ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(messagePtr, 4))));
        messagePtr += 4;
        // read payload (full peek of 4 bytes)
        EXPECT_CALL(fSocketMock1, read(NotNull(), 4U))
            .InSequence(seq)
            .WillOnce(
                Invoke(ReadBytesFrom(::estd::slice<uint8_t const>::from_pointer(messagePtr, 4U))));

        EXPECT_CALL(
            fMessageProvidingListenerMock, getTransportMessage(fBusId, 0x1234, 0x1519, 3, _, _))
            .WillOnce(DoAll(
                SetArgReferee<5>(&message1),
                Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(
                    ITransportMessageProvider::ErrorCode::TPMSG_OK))));

        EXPECT_CALL(
            fMessageProvidingListenerMock, getTransportMessage(fBusId, 0x1234, 0x1519, 4, _, _))
            .WillOnce(DoAll(
                SetArgReferee<5>(&message4),
                Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(
                    ITransportMessageProvider::ErrorCode::TPMSG_OK))));

        EXPECT_CALL(
            fMessageProvidingListenerMock, getTransportMessage(fBusId, 0x1234, 0x1516, 4, _, _))
            .WillOnce(Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(
                ITransportMessageProvider::ErrorCode::TPMSG_INVALID_TGT_ADDRESS)));

        EXPECT_CALL(
            fMessageProvidingListenerMock, getTransportMessage(fBusId, 0x1234, 0x1517, 9, _, _))
            .WillOnce(Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(
                ITransportMessageProvider::ErrorCode::TPMSG_INVALID_TGT_ADDRESS)));

        ITransportMessageProcessedListener* notificationListener = nullptr;
        EXPECT_CALL(
            fMessageProvidingListenerMock, messageReceived(fBusId, Ref(message1), NotNull()))
            .WillOnce(DoAll(
                SaveArg<2>(&notificationListener),
                Return(DoIpTransportMessageProvidingListenerHelper::createReceiveResult(
                    ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR))));
        EXPECT_CALL(
            fMessageProvidingListenerMock, messageReceived(fBusId, Ref(message4), NotNull()))
            .WillOnce(DoAll(
                SaveArg<2>(&notificationListener),
                Return(DoIpTransportMessageProvidingListenerHelper::createReceiveResult(
                    ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR))));

        // expect diagnostic ack
        uint8_t diagnosticAckValid[16];
        uint8_t* targetMessagePtr = diagnosticAckValid;

        EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 8U)))
            .InSequence(seq)
            .WillOnce(Invoke(WriteBytesTo(
                ::estd::slice<uint8_t>::from_pointer(targetMessagePtr, 8),
                ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
        targetMessagePtr += 8;
        EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 8U)))
            .InSequence(seq)
            .WillOnce(Invoke(WriteBytesTo(
                ::estd::slice<uint8_t>::from_pointer(targetMessagePtr, 8),
                ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
        EXPECT_CALL(fSocketMock1, flush())
            .InSequence(seq)
            .WillRepeatedly(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));

        // expect invalid ACK
        uint8_t diagnosticAckInvalid[17];
        targetMessagePtr = diagnosticAckInvalid;

        EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 8U)))
            .InSequence(seq)
            .WillOnce(Invoke(WriteBytesTo(
                ::estd::slice<uint8_t>::from_pointer(targetMessagePtr, 8),
                ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
        targetMessagePtr += 8;

        EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 9U)))
            .InSequence(seq)
            .WillOnce(Invoke(WriteBytesTo(
                ::estd::slice<uint8_t>::from_pointer(targetMessagePtr, 9),
                ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
        EXPECT_CALL(fSocketMock1, flush())
            .InSequence(seq)
            .WillRepeatedly(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));

        // expect invalid ACK
        uint8_t diagnosticNackInvalid2[18];
        targetMessagePtr = diagnosticNackInvalid2;

        EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 8U)))
            .InSequence(seq)
            .WillOnce(Invoke(WriteBytesTo(
                ::estd::slice<uint8_t>::from_pointer(targetMessagePtr, 8),
                ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
        targetMessagePtr += 8;

        EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 10U)))
            .InSequence(seq)
            .WillOnce(Invoke(WriteBytesTo(
                ::estd::slice<uint8_t>::from_pointer(targetMessagePtr, 10),
                ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
        EXPECT_CALL(fSocketMock1, flush())
            .InSequence(seq)
            .WillRepeatedly(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));

        // expect valid ACK
        uint8_t diagnosticAckValid2[17];
        targetMessagePtr = diagnosticAckValid2;

        EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 8U)))
            .InSequence(seq)
            .WillOnce(Invoke(WriteBytesTo(
                ::estd::slice<uint8_t>::from_pointer(targetMessagePtr, 8),
                ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
        targetMessagePtr += 8;
        EXPECT_CALL(fSocketMock1, send(Slice(NotNull(), 9U)))
            .InSequence(seq)
            .WillOnce(Invoke(WriteBytesTo(
                ::estd::slice<uint8_t>::from_pointer(targetMessagePtr, 9),
                ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
        EXPECT_CALL(fSocketMock1, flush())
            .InSequence(seq)
            .WillRepeatedly(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));

        fSocketMock1.getDataListener()->dataReceived(sizeof(diagnosticMessage));

        testContext.expireAndExecute();

        EXPECT_TRUE(notificationListener != nullptr);
        uint8_t const expectedDiagnosticAckValid[] = {
            0x02,
            0xfd,
            0x80,
            0x02,
            0x00,
            0x00,
            0x00,
            0x08,
            0x15,
            0x19,
            0x12,
            0x34,
            0x00,
            0x23,
            0x35,
            0x59,
        };
        uint8_t const expectedDiagnosticAckInvalid[] = {
            0x02,
            0xfd,
            0x80,
            0x03,
            0x00,
            0x00,
            0x00,
            0x09,
            0x15,
            0x16,
            0x12,
            0x34,
            0x03,
            0x12,
            0x34,
            0x45,
            0x56,
        };
        uint8_t const expectedDiagnosticNackInvalid2[] = {
            0x02,
            0xfd,
            0x80,
            0x03,
            0x00,
            0x00,
            0x00,
            0x0a,
            0x15,
            0x17,
            0x12,
            0x34,
            0x03,
            0x12,
            0x34,
            0x45,
            0x56,
            0x67,
        };
        uint8_t const expectedDiagnosticAckValid2[] = {
            0x02,
            0xfd,
            0x80,
            0x02,
            0x00,
            0x00,
            0x00,
            0x09,
            0x15,
            0x19,
            0x12,
            0x34,
            0x00,
            0x23,
            0x35,
            0x59,
            0x60,
        };
        EXPECT_THAT(
            diagnosticAckValid,
            ::testing::ElementsAreArray(
                expectedDiagnosticAckValid, sizeof(expectedDiagnosticAckValid)));

        EXPECT_THAT(
            diagnosticAckInvalid,
            ::testing::ElementsAreArray(
                expectedDiagnosticAckInvalid, sizeof(expectedDiagnosticAckInvalid)));

        EXPECT_THAT(
            diagnosticNackInvalid2,
            ::testing::ElementsAreArray(
                expectedDiagnosticNackInvalid2, sizeof(expectedDiagnosticNackInvalid2)));

        EXPECT_THAT(
            diagnosticAckValid2,
            ::testing::ElementsAreArray(
                expectedDiagnosticAckValid2, sizeof(expectedDiagnosticAckValid2)));

        // Release the message at the provider that originally was returned
        EXPECT_TRUE(config != nullptr);
        config->getMessageProcessedListener().transportMessageProcessed(
            message1, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
        config->getMessageProcessedListener().transportMessageProcessed(
            message4, ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    }
}

TEST_F(DoIpServerTransportLayerTest, TestCloseConnection)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.setCallback(fTransportLayerCallbackMock);
    cut.init();

    EXPECT_CALL(fTransportLayerCallbackMock, checkRoutingActivation(0x1234, _, _, _, _, _, false))
        .WillRepeatedly(Return(
            IDoIpServerConnectionFilter::RoutingActivationCheckResult().setInternalSourceAddress(
                0x1122)));

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(22U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    prepareSocketGroup(76U, fRemoteEndpoint1, 22U, 2U);
    listener->connectionAccepted(76U, fSocketMock1, ConnectionType::PLAIN);

    // routing activation
    prepareRoutingActivationResponse(fSocketMock1);
    EXPECT_CALL(
        fTransportLayerCallbackMock,
        routingActive(
            0x1234,
            0x1122,
            fLocalEndpoint1,
            fRemoteEndpoint1,
            DoIpTcpConnection::ConnectionType::PLAIN));
    expectRoutingActivationRequest(fSocketMock1, 0x1234, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);

    EXPECT_CALL(fTransportLayerCallbackMock, checkRoutingActivation(0x4321, _, 22U, _, _, _, false))
        .WillRepeatedly(Return(
            IDoIpServerConnectionFilter::RoutingActivationCheckResult().setInternalSourceAddress(
                0x2211)));

    // Before looping: accept a new connection. This shouldn't be closed later
    EXPECT_CALL(fConnectionPoolMock, createConnection(22U, Ref(fSocketMock2), _, _))
        .WillOnce(Return(&fConnection1_2));
    EXPECT_CALL(fSocketMock2, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock2, fRemoteEndpoint2);
    prepareSocketGroup(61U, fRemoteEndpoint2, 22U, 2U);
    listener->connectionAccepted(61U, fSocketMock2, ConnectionType::PLAIN);

    // routing activation
    prepareRoutingActivationResponse(fSocketMock2);
    EXPECT_CALL(
        fTransportLayerCallbackMock,
        routingActive(
            0x4321,
            0x2211,
            fLocalEndpoint2,
            fRemoteEndpoint2,
            DoIpTcpConnection::ConnectionType::PLAIN));
    expectRoutingActivationRequest(fSocketMock2, 0x4321, fLocalEndpoint2, fRemoteEndpoint2);
    endRoutingActivationResponse(fSocketMock2);

    // Now close a connection
    cut.closeConnection(0x1122, IDoIpTcpConnection::CloseMode::CLOSE);

    EXPECT_CALL(fSocketMock1, close())
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnection1)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock1), ConnectionType::PLAIN));
    EXPECT_CALL(fTransportLayerCallbackMock, routingInactive(0x1122));
    EXPECT_CALL(fTransportLayerCallbackMock, connectionClosed(0x1234));
    testContext.expireAndExecute();

    // Nothing should happen if an non-existing connection is closed
    cut.closeConnection(0x2356, IDoIpTcpConnection::CloseMode::CLOSE);
    testContext.expireAndExecute();
}

TEST_F(DoIpServerTransportLayerTest, TestCloseAllConnections)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.setCallback(fTransportLayerCallbackMock);
    cut.init();

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(18U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    prepareSocketGroup(59U, fRemoteEndpoint1, 18U, 2U);
    listener->connectionAccepted(59U, fSocketMock1, ConnectionType::PLAIN);

    // Call close connections
    cut.closeAllConnections(IDoIpTcpConnection::CloseMode::ABORT);

    // Before looping: accept a new connection. This shouldn't be closed later
    EXPECT_CALL(fConnectionPoolMock, createConnection(18U, Ref(fSocketMock2), _, _))
        .WillOnce(Return(&fConnection2));
    EXPECT_CALL(fSocketMock2, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock2, fRemoteEndpoint2);
    prepareSocketGroup(51U, fRemoteEndpoint2, 18U, 2U);
    listener->connectionAccepted(51U, fSocketMock2, ConnectionType::PLAIN);

    EXPECT_CALL(fSocketMock1, abort());
    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnection1)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock1), ConnectionType::PLAIN));
    testContext.expireAndExecute();
}

TEST_F(DoIpServerTransportLayerTest, TestCloseSocketGroupConnections)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.setCallback(fTransportLayerCallbackMock);
    cut.init();

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(22U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    prepareSocketGroup(59U, fRemoteEndpoint1, 22U, 2U);
    listener->connectionAccepted(59U, fSocketMock1, ConnectionType::PLAIN);

    EXPECT_CALL(fConnectionPoolMock, createConnection(33U, Ref(fSocketMock2), _, _))
        .WillOnce(Return(&fConnection2));
    EXPECT_CALL(fSocketMock2, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock2, fRemoteEndpoint2);
    prepareSocketGroup(61U, fRemoteEndpoint2, 33U, 2U);
    listener->connectionAccepted(61U, fSocketMock2, ConnectionType::PLAIN);

    // Call close connections
    cut.closeSocketGroupConnections(22U, IDoIpTcpConnection::CloseMode::CLOSE);

    // Before looping: accept a new connection. This shouldn't be closed later
    EXPECT_CALL(fConnectionPoolMock, createConnection(22U, Ref(fSocketMock3), _, _))
        .WillOnce(Return(&fConnection1_3));
    EXPECT_CALL(fSocketMock3, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock3, fRemoteEndpoint3);
    prepareSocketGroup(51U, fRemoteEndpoint3, 22U, 2U);
    listener->connectionAccepted(51U, fSocketMock3, ConnectionType::PLAIN);

    EXPECT_CALL(fSocketMock1, close())
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnection1)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock1), ConnectionType::PLAIN));
    testContext.expireAndExecute();
}

TEST_F(DoIpServerTransportLayerTest, TestAliveCheckWithAlreadyRegisteredSource)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();
    cut.setCallback(fTransportLayerCallbackMock);

    EXPECT_CALL(fTransportLayerCallbackMock, checkRoutingActivation(0x1234, _, 22U, _, _, _, false))
        .WillRepeatedly(Return(
            IDoIpServerConnectionFilter::RoutingActivationCheckResult().setInternalSourceAddress(
                0x1122)));

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(22U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    prepareSocketGroup(43U, fRemoteEndpoint1, 22U, 2U);
    listener->connectionAccepted(43U, fSocketMock1, ConnectionType::PLAIN);

    EXPECT_CALL(fConnectionPoolMock, createConnection(22U, Ref(fSocketMock2), _, _))
        .WillOnce(Return(&fConnection1_2));
    EXPECT_CALL(fSocketMock2, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock2, fRemoteEndpoint2);
    EXPECT_CALL(fTransportLayerCallbackMock, getSocketGroupId(41U, fRemoteEndpoint2))
        .WillOnce(Return(22U));
    EXPECT_CALL(fTransportLayerCallbackMock, getMaxConnectionCount(22U)).WillRepeatedly(Return(2U));
    listener->connectionAccepted(41U, fSocketMock2, ConnectionType::PLAIN);

    // Activate routing for first
    prepareRoutingActivationResponse(fSocketMock1);
    EXPECT_CALL(
        fTransportLayerCallbackMock,
        routingActive(
            0x1234,
            0x1122,
            fLocalEndpoint1,
            fRemoteEndpoint1,
            DoIpTcpConnection::ConnectionType::PLAIN));
    expectRoutingActivationRequest(fSocketMock1, 0x1234, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);

    prepareAliveCheckRequest(fSocketMock1);
    // Try to activate routing for second connection
    expectRoutingActivationRequest(fSocketMock2, 0x1234, fLocalEndpoint2, fRemoteEndpoint2);

    EXPECT_CALL(fSocketMock2, close())
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnection1_2)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock2), ConnectionType::PLAIN));

    // Second connection should respond with error and close
    ::estd::slice<uint8_t> response = prepareRoutingActivationResponse(fSocketMock2);
    expectAliveCheckResponse(fSocketMock1, 0x1234);
    endRoutingActivationResponse(fSocketMock2);
    uint8_t const expectedResponse[]
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
           0x22,
           0x13,
           0x03,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_TRUE(::estd::memory::is_equal(response, expectedResponse));
    testContext.expireAndExecute();

    // Expect connection to get inactive on close
    EXPECT_CALL(fTransportLayerCallbackMock, connectionClosed(0x1234));
    fSocketMock1.getDataListener()->connectionClosed(
        ::tcp::IDataListener::ErrorCode::ERR_CONNECTION_CLOSED);
    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnection1)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock1), ConnectionType::PLAIN));
    EXPECT_CALL(fTransportLayerCallbackMock, routingInactive(0x1122));
    testContext.expireAndExecute();
}

TEST_F(DoIpServerTransportLayerTest, TestAliveCheckWithInactiveRegisteredSource)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(0U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    listener->connectionAccepted(22U, fSocketMock1, ConnectionType::PLAIN);

    EXPECT_CALL(fConnectionPoolMock, createConnection(0U, Ref(fSocketMock2), _, _))
        .WillOnce(Return(&fConnection1_2));
    EXPECT_CALL(fSocketMock2, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock2, fRemoteEndpoint2);
    listener->connectionAccepted(33U, fSocketMock2, ConnectionType::PLAIN);

    // Activate routing for first
    prepareRoutingActivationResponse(fSocketMock1);
    expectRoutingActivationRequest(fSocketMock1, 0x1122, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);

    prepareAliveCheckRequest(fSocketMock1);
    // Try to activate routing for second connection
    expectRoutingActivationRequest(fSocketMock2, 0x1122, fLocalEndpoint2, fRemoteEndpoint2);

    // First connection times out
    ::estd::slice<uint8_t> response = prepareRoutingActivationResponse(fSocketMock2);
    // expectAliveCheckResponse(fSocketMock1, 0x1122);
    uint8_t const expectedResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x11,
           0x22,
           0x22,
           0x13,
           0x10,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_CALL(fSocketMock1, close())
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnection1)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock1), ConnectionType::PLAIN));
    testContext.elapse(static_cast<uint64_t>(fParameters.getAliveCheckTimeout()) * 1000U);
    testContext.expireAndExecute();
    ASSERT_THAT(response, ElementsAreArray(expectedResponse));
    endRoutingActivationResponse(fSocketMock2);
    testContext.expireAndExecute();

    // Expect connection to get inactive on close
    fSocketMock2.getDataListener()->connectionClosed(
        ::tcp::IDataListener::ErrorCode::ERR_CONNECTION_CLOSED);
    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnection1_2)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock2), ConnectionType::PLAIN));
    testContext.expireAndExecute();
}

TEST_F(DoIpServerTransportLayerTest, TestAliveCheckWithActivatingConnectionClosingBeforeResult)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(0U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    listener->connectionAccepted(22U, fSocketMock1, ConnectionType::PLAIN);

    EXPECT_CALL(fConnectionPoolMock, createConnection(0U, Ref(fSocketMock2), _, _))
        .WillOnce(Return(&fConnection1_2));
    EXPECT_CALL(fSocketMock2, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock2, fRemoteEndpoint2);
    listener->connectionAccepted(33U, fSocketMock2, ConnectionType::PLAIN);

    // Activate routing for first
    prepareRoutingActivationResponse(fSocketMock1);
    expectRoutingActivationRequest(fSocketMock1, 0x1122, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);

    prepareAliveCheckRequest(fSocketMock1);
    // Try to activate routing for second connection
    expectRoutingActivationRequest(fSocketMock2, 0x1122, fLocalEndpoint2, fRemoteEndpoint2);

    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnection1_2)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock2), ConnectionType::PLAIN));
    fSocketMock2.getDataListener()->connectionClosed(
        ::tcp::IDataListener::ErrorCode::ERR_CONNECTION_CLOSED);

    // Receive alive check response
    expectAliveCheckResponse(fSocketMock1, 0x1122);

    testContext.expireAndExecute();
}

TEST_F(DoIpServerTransportLayerTest, TestAliveCheckWithAnotherRegisteredSource)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();
    cut.setCallback(fTransportLayerCallbackMock);

    EXPECT_CALL(fTransportLayerCallbackMock, checkRoutingActivation(0x1234, _, 22U, _, _, _, false))
        .WillRepeatedly(Return(
            IDoIpServerConnectionFilter::RoutingActivationCheckResult().setInternalSourceAddress(
                0x1122)));
    EXPECT_CALL(fTransportLayerCallbackMock, checkRoutingActivation(0x2345, _, 22U, _, _, _, false))
        .WillRepeatedly(Return(
            IDoIpServerConnectionFilter::RoutingActivationCheckResult().setInternalSourceAddress(
                0x1124)));

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(22U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    prepareSocketGroup(43U, fRemoteEndpoint1, 22U, 1U);
    listener->connectionAccepted(43U, fSocketMock1, ConnectionType::PLAIN);

    EXPECT_CALL(fConnectionPoolMock, createConnection(22U, Ref(fSocketMock2), _, _))
        .WillOnce(Return(&fConnection1_2));
    EXPECT_CALL(fSocketMock2, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock2, fRemoteEndpoint2);
    prepareSocketGroup(41U, fRemoteEndpoint2, 22U, 1U);
    listener->connectionAccepted(41U, fSocketMock2, ConnectionType::PLAIN);

    EXPECT_CALL(fConnectionPoolMock, createConnection(33U, Ref(fSocketMock3), _, _))
        .WillOnce(Return(&fConnection3));
    EXPECT_CALL(fSocketMock3, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock3, fRemoteEndpoint3);
    prepareSocketGroup(45U, fRemoteEndpoint3, 33U, 1U);
    listener->connectionAccepted(45U, fSocketMock3, ConnectionType::PLAIN);

    // Activate routing for first
    prepareRoutingActivationResponse(fSocketMock1);
    EXPECT_CALL(
        fTransportLayerCallbackMock,
        routingActive(
            0x1234,
            0x1122,
            fLocalEndpoint1,
            fRemoteEndpoint1,
            DoIpTcpConnection::ConnectionType::PLAIN));
    expectRoutingActivationRequest(fSocketMock1, 0x1234, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);

    prepareAliveCheckRequest(fSocketMock1);
    // Try to activate routing for second connection
    expectRoutingActivationRequest(fSocketMock2, 0x2345, fLocalEndpoint2, fRemoteEndpoint2);

    EXPECT_CALL(fSocketMock2, close())
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnection1_2)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock2), ConnectionType::PLAIN));

    // Second connection should respond with error and close
    ::estd::slice<uint8_t> response = prepareRoutingActivationResponse(fSocketMock2);
    expectAliveCheckResponse(fSocketMock1, 0x1234);
    endRoutingActivationResponse(fSocketMock2);
    uint8_t const expectedResponse[]
        = {0x02,
           0xfd,
           0x00,
           0x06,
           0x00,
           0x00,
           0x00,
           0x09,
           0x23,
           0x45,
           0x22,
           0x13,
           0x01,
           0x00,
           0x00,
           0x00,
           0x00};
    EXPECT_TRUE(::estd::memory::is_equal(response, expectedResponse));
    testContext.expireAndExecute();

    // Expect connection to get inactive on close
    EXPECT_CALL(fTransportLayerCallbackMock, connectionClosed(0x1234));
    fSocketMock1.getDataListener()->connectionClosed(
        ::tcp::IDataListener::ErrorCode::ERR_CONNECTION_CLOSED);
    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnection1)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock1), ConnectionType::PLAIN));
    EXPECT_CALL(fTransportLayerCallbackMock, routingInactive(0x1122));
    testContext.expireAndExecute();
}

TEST_F(DoIpServerTransportLayerTest, TestAliveCheckResponseWithoutAliveCheck)
{
    ::doip::declare::DoIpServerTransportLayer<1> cut(
        fBusId,
        fConfig.getLogicalEntityAddress(),
        asyncContext,
        fSocketHandlerMock,
        fConnectionPoolMock,
        fParameters);
    // initialize
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.init();

    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fConnectionPoolMock, createConnection(0U, Ref(fSocketMock1), _, _))
        .WillOnce(Return(&fConnection1));
    EXPECT_CALL(fSocketMock1, isEstablished()).WillOnce(Return(true));
    prepareSocket(fSocketMock1, fRemoteEndpoint1);
    listener->connectionAccepted(19U, fSocketMock1, ConnectionType::PLAIN);

    // Activate routing for first
    prepareRoutingActivationResponse(fSocketMock1);
    expectRoutingActivationRequest(fSocketMock1, 0x1122, fLocalEndpoint1, fRemoteEndpoint1);
    endRoutingActivationResponse(fSocketMock1);

    // now receive alive check response
    expectAliveCheckResponse(fSocketMock1, 0x1122);

    testContext.expireAndExecute();
}

void DoIpServerTransportLayerTest::setupReadMessage(
    Sequence seq, uint8_t const* diagnosticMessage, size_t length)
{
    auto diagMessage = ::estd::slice<uint8_t const>::from_pointer(diagnosticMessage, length);
    EXPECT_CALL(fSocketMock1, read(NotNull(), 8U))
        .InSequence(seq)
        .WillOnce(Invoke(ReadBytesFrom(diagMessage.subslice(8U))));
    diagMessage.advance(8U);
    EXPECT_CALL(fSocketMock1, read(NotNull(), 4U))
        .InSequence(seq)
        .WillOnce(Invoke(ReadBytesFrom(diagMessage.subslice(4U))));
    diagMessage.advance(4U);
    if (diagMessage.size() == 0)
    {
        return;
    }
    size_t remainingSize = diagMessage.size();
    size_t peekSize      = std::min(remainingSize, static_cast<size_t>(5U));
    EXPECT_CALL(fSocketMock1, read(NotNull(), peekSize))
        .InSequence(seq)
        .WillOnce(Invoke(ReadBytesFrom(diagMessage.subslice(peekSize))));
    if (remainingSize > peekSize)
    {
        diagMessage.advance(peekSize);
        remainingSize -= peekSize;
        EXPECT_CALL(fSocketMock1, read(NotNull(), remainingSize))
            .InSequence(seq)
            .WillOnce(Invoke(ReadBytesFrom(diagMessage.subslice(remainingSize))));
    }
}

void DoIpServerTransportLayerTest::prepareSocket(
    ::tcp::AbstractSocketMock& socketMock, ::ip::IPEndpoint const& remoteEndpoint)
{
    EXPECT_CALL(socketMock, getRemoteIPAddress())
        .WillRepeatedly(Return(remoteEndpoint.getAddress()));
    EXPECT_CALL(socketMock, getRemotePort()).WillRepeatedly(Return(remoteEndpoint.getPort()));
}

void DoIpServerTransportLayerTest::prepareSocketGroup(
    uint8_t serverSocketId,
    ::ip::IPEndpoint const& remoteEndpoint,
    uint8_t socketGroupId,
    uint8_t maxConnectionCount)
{
    EXPECT_CALL(fTransportLayerCallbackMock, getSocketGroupId(serverSocketId, remoteEndpoint))
        .WillRepeatedly(Return(socketGroupId));
    EXPECT_CALL(fTransportLayerCallbackMock, getMaxConnectionCount(socketGroupId))
        .WillRepeatedly(Return(maxConnectionCount));
}

::estd::slice<uint8_t> DoIpServerTransportLayerTest::prepareRoutingActivationResponse(
    ::tcp::AbstractSocketMock& socketMock)
{
    ::estd::slice<uint8_t> responseBuffer = allocateBuffer(17U);
    Sequence seq;
    EXPECT_CALL(socketMock, send(Slice(NotNull(), 8U)))
        .InSequence(seq)
        .WillOnce(Invoke(WriteBytesTo(
            responseBuffer.subslice(8), ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
    EXPECT_CALL(socketMock, send(Slice(NotNull(), responseBuffer.size() - 8)))
        .InSequence(seq)
        .WillOnce(Invoke(WriteBytesTo(
            responseBuffer.offset(8), ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
    EXPECT_CALL(socketMock, flush())
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
    return responseBuffer;
}

void DoIpServerTransportLayerTest::expectRoutingActivationRequest(
    ::tcp::AbstractSocketMock& socketMock,
    uint16_t sourceAddress,
    ::ip::IPEndpoint const& localEndpoint,
    ::ip::IPEndpoint const& remoteEndpoint,
    uint8_t activationType)
{
    uint8_t const requestData[]
        = {0x02,
           0xfd,
           0x00,
           0x05,
           0x00,
           0x00,
           0x00,
           0x07,
           uint8_t((sourceAddress >> 8) & 0xff),
           uint8_t(sourceAddress & 0xff),
           activationType,
           0x00,
           0x00,
           0x00,
           0x00};
    ::estd::slice<uint8_t> request = allocateBuffer(sizeof(requestData));
    ::estd::memory::copy(request, requestData);

    Sequence seq;
    EXPECT_CALL(socketMock, read(NotNull(), 8U))
        .InSequence(seq)
        .WillOnce(Invoke(ReadBytesFrom(request.subslice(8))));
    EXPECT_CALL(socketMock, read(NotNull(), 7U))
        .InSequence(seq)
        .WillOnce(Invoke(ReadBytesFrom(request.offset(8).subslice(7))));
    EXPECT_CALL(socketMock, getLocalPort()).WillRepeatedly(Return(localEndpoint.getPort()));
    EXPECT_CALL(socketMock, getLocalIPAddress()).WillRepeatedly(Return(localEndpoint.getAddress()));
    EXPECT_CALL(socketMock, getRemotePort()).WillRepeatedly(Return(remoteEndpoint.getPort()));
    EXPECT_CALL(socketMock, getRemoteIPAddress())
        .WillRepeatedly(Return(remoteEndpoint.getAddress()));

    socketMock.getDataListener()->dataReceived(request.size());

    testContext.expireAndExecute();
}

void DoIpServerTransportLayerTest::endRoutingActivationResponse(
    ::tcp::AbstractSocketMock& socketMock)
{
    socketMock.getSendNotificationListener()->dataSent(
        17, ::tcp::IDataSendNotificationListener::SendResult::DATA_SENT);
}

void DoIpServerTransportLayerTest::prepareAliveCheckRequest(::tcp::AbstractSocketMock& socketMock)
{
    uint8_t const requestData[]    = {0x02, 0xfd, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00};
    ::estd::slice<uint8_t> request = allocateBuffer(sizeof(requestData));
    Sequence seq;
    EXPECT_CALL(socketMock, send(Slice(NotNull(), Eq(8U))))
        .InSequence(seq)
        .WillOnce(Invoke(WriteBytesTo(request, ::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK)));
    EXPECT_CALL(socketMock, flush())
        .InSequence(seq)
        .WillOnce(Return(::tcp::AbstractSocket::ErrorCode::SOCKET_ERR_OK));
}

void DoIpServerTransportLayerTest::expectAliveCheckResponse(
    ::tcp::AbstractSocketMock& socketMock, uint16_t sourceAddress)
{
    uint8_t const responseData[]
        = {0x02,
           0xfd,
           0x00,
           0x08,
           0x00,
           0x00,
           0x00,
           0x02,
           uint8_t((sourceAddress >> 8) & 0xff),
           uint8_t(sourceAddress & 0xff)};
    ::estd::slice<uint8_t> response = allocateBuffer(sizeof(responseData));
    ::estd::memory::copy(response, responseData);

    Sequence seq;
    EXPECT_CALL(socketMock, read(NotNull(), 8U))
        .InSequence(seq)
        .WillOnce(Invoke(ReadBytesFrom(response.subslice(8))));
    EXPECT_CALL(socketMock, read(NotNull(), 2U))
        .InSequence(seq)
        .WillOnce(Invoke(ReadBytesFrom(response.offset(8))));

    socketMock.getDataListener()->dataReceived(response.size());

    testContext.expireAndExecute();
}

void DoIpServerTransportLayerTest::endAliveCheckRequest(::tcp::AbstractSocketMock& socketMock)
{
    socketMock.getSendNotificationListener()->dataSent(
        8, ::tcp::IDataSendNotificationListener::SendResult::DATA_SENT);
}

} // namespace test
} // namespace doip
