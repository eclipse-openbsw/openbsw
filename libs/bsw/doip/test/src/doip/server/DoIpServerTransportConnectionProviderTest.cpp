/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/DoIpServerTransportConnectionProvider.h"

#include "doip/common/DoIpTransportMessageProvidingListenerMock.h"
#include "doip/server/DoIpServerSocketHandlerMock.h"
#include "doip/server/DoIpServerTransportConnection.h"
#include "doip/server/DoIpServerTransportConnectionConfig.h"
#include "doip/server/DoIpServerTransportConnectionPoolMock.h"
#include "doip/server/DoIpServerTransportConnectionProviderCallbackMock.h"
#include "doip/server/DoIpServerTransportLayerParameters.h"
#include "doip/server/IDoIpServerSocketHandlerListener.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <etl/pool.h>
#include <tcp/socket/AbstractSocketMock.h>
#include <transport/TransportMessage.h>
#include <transport/TransportMessageProcessedListenerMock.h>

#include <gmock/gmock.h>
#include <gtest/esr_extensions.h>

using namespace ::testing;
using namespace ::transport;
using namespace ::tcp::test;
using ConnectionType = ::doip::DoIpTcpConnection::ConnectionType;

namespace doip
{
namespace test
{
struct DoIpServerTransportConnectionProviderTest : Test
{
    DoIpServerTransportConnectionProviderTest()
    : fBusId()
    , asyncMock()
    , asyncContext(1U)
    , fParameters(100, 200, 300, 400)
    , fConfig(
          fBusId,
          0x1234,
          asyncContext,
          fMessageProvidingListenerMock,
          fMessageProcessedListenerMock,
          fParameters)
    , fConnection(
          DoIpConstants::ProtocolVersion::version02Iso2012,
          13U,
          fSocketMock,
          fConfig,
          fDiagnosticSendJobBlockPool,
          fProtocolSendJobBlockPool,
          DoIpTcpConnection::ConnectionType::PLAIN)
    , fConnectionTls(
          DoIpConstants::ProtocolVersion::version02Iso2012,
          13U,
          fSocketMock,
          fConfig,
          fDiagnosticSendJobBlockPool,
          fProtocolSendJobBlockPool,
          DoIpTcpConnection::ConnectionType::TLS)
    {}

    DoIpServerTransportConnectionProviderCallbackMock fCallbackMock;
    DoIpServerSocketHandlerMock fSocketHandlerMock;
    DoIpServerTransportConnectionPoolMock fConnectionPoolMock;
    uint8_t fBusId;
    ::testing::StrictMock<::async::AsyncMock> asyncMock;
    ::async::ContextType asyncContext;
    DoIpTransportMessageProvidingListenerMock fMessageProvidingListenerMock;
    TransportMessageProcessedListenerMock fMessageProcessedListenerMock;
    DoIpServerTransportLayerParameters fParameters;
    DoIpServerTransportConnectionConfig fConfig;
    ::tcp::AbstractSocketMock fSocketMock;
    DoIpServerTransportConnection fConnection;
    DoIpServerTransportConnection fConnectionTls;
    ::etl::pool<DoIpTransportMessageSendJob, 4> fDiagnosticSendJobBlockPool;
    ::etl::pool<DoIpServerTransportMessageHandler::StaticPayloadSendJobType, 4>
        fProtocolSendJobBlockPool;
};

TEST_F(DoIpServerTransportConnectionProviderTest, TestStartAndStop)
{
    DoIpServerTransportConnectionProvider cut(
        fCallbackMock, fSocketHandlerMock, fConnectionPoolMock, fConfig);
    // Start the connection provider
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.start();
    EXPECT_TRUE(listener != nullptr);
    EXPECT_CALL(fSocketHandlerMock, stop());
    cut.stop();
}

TEST_F(DoIpServerTransportConnectionProviderTest, TestFilterConnections)
{
    DoIpServerTransportConnectionProvider cut(
        fCallbackMock, fSocketHandlerMock, fConnectionPoolMock, fConfig);
    // Start the connection provider
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.start();
    EXPECT_TRUE(listener != nullptr);
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x12382), 232);

    // first return false and expect false to be the result
    EXPECT_CALL(fCallbackMock, getSocketGroupId(49U, remoteEndpoint)).WillOnce(Return(0U));
    EXPECT_CALL(fCallbackMock, filterConnection(0U, remoteEndpoint)).WillOnce(Return(false));
    EXPECT_FALSE(listener->filterConnection(49U, remoteEndpoint));
    // now return true and expect it to be the result
    EXPECT_CALL(fCallbackMock, getSocketGroupId(51U, remoteEndpoint)).WillOnce(Return(140U));
    EXPECT_CALL(fCallbackMock, filterConnection(140U, remoteEndpoint)).WillOnce(Return(true));
    EXPECT_TRUE(listener->filterConnection(51U, remoteEndpoint));
}

TEST_F(DoIpServerTransportConnectionProviderTest, TestFilterConnectionReturnsFalse)
{
    DoIpServerTransportConnectionProvider cut(
        fCallbackMock, fSocketHandlerMock, fConnectionPoolMock, fConfig);
    // Start the connection provider
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.start();
    EXPECT_TRUE(listener != nullptr);
    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x12382), 232);

    // first return false and expect false to be the result
    EXPECT_CALL(fCallbackMock, getSocketGroupId(49U, remoteEndpoint)).WillOnce(Return(0U));
    EXPECT_CALL(fCallbackMock, filterConnection(0U, remoteEndpoint)).WillOnce(Return(false));
    EXPECT_FALSE(listener->filterConnection(49U, remoteEndpoint));
    // now return true and expect it to be the result
    EXPECT_CALL(fCallbackMock, getSocketGroupId(51U, remoteEndpoint)).WillOnce(Return(14U));
    EXPECT_CALL(fCallbackMock, filterConnection(14U, remoteEndpoint)).WillOnce(Return(false));
    EXPECT_FALSE(listener->filterConnection(51U, remoteEndpoint));
}

TEST_F(DoIpServerTransportConnectionProviderTest, TestAcceptConnection)
{
    DoIpServerTransportConnectionProvider cut(
        fCallbackMock, fSocketHandlerMock, fConnectionPoolMock, fConfig);
    // Start the connection provider
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.start();
    EXPECT_TRUE(listener != nullptr);

    ::ip::IPEndpoint remoteEndpoint(::ip::make_ip4(0x12382), 232);
    EXPECT_CALL(fSocketMock, getRemoteIPAddress())
        .WillRepeatedly(Return(remoteEndpoint.getAddress()));
    EXPECT_CALL(fSocketMock, getRemotePort()).WillRepeatedly(Return(remoteEndpoint.getPort()));

    // Release socket if no connection is available
    EXPECT_CALL(fConnectionPoolMock, createConnection(16U, Ref(fSocketMock), Ref(fConfig), _))
        .WillOnce(Return(static_cast<DoIpServerTransportConnection*>(nullptr)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock), ConnectionType::PLAIN));
    EXPECT_CALL(fCallbackMock, getSocketGroupId(53U, remoteEndpoint)).WillOnce(Return(16U));
    listener->connectionAccepted(53U, fSocketMock, ConnectionType::PLAIN);

    // Forward new connection if available
    EXPECT_CALL(fConnectionPoolMock, createConnection(17U, Ref(fSocketMock), Ref(fConfig), _))
        .WillOnce(Return(&fConnection));
    EXPECT_CALL(fCallbackMock, getSocketGroupId(55U, remoteEndpoint)).WillOnce(Return(17U));
    EXPECT_CALL(fCallbackMock, connectionAccepted(Ref(fConnection)));
    listener->connectionAccepted(55U, fSocketMock, ConnectionType::PLAIN);
}

TEST_F(DoIpServerTransportConnectionProviderTest, TestReleaseConnection)
{
    DoIpServerTransportConnectionProvider cut(
        fCallbackMock, fSocketHandlerMock, fConnectionPoolMock, fConfig);
    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnection)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock), ConnectionType::PLAIN));
    cut.releaseConnection(fConnection);
}

TEST_F(DoIpServerTransportConnectionProviderTest, TestReleaseConnectionTls)
{
    DoIpServerTransportConnectionProvider cut(
        fCallbackMock, fSocketHandlerMock, fConnectionPoolMock, fConfig);
    EXPECT_CALL(fConnectionPoolMock, releaseConnection(Ref(fConnectionTls)));
    EXPECT_CALL(fSocketHandlerMock, releaseSocket(Ref(fSocketMock), ConnectionType::TLS));
    cut.releaseConnection(fConnectionTls);
}

TEST_F(DoIpServerTransportConnectionProviderTest, TestServerSocketBound)
{
    DoIpServerTransportConnectionProvider cut(
        fCallbackMock, fSocketHandlerMock, fConnectionPoolMock, fConfig);
    // Start the connection provider
    IDoIpServerSocketHandlerListener* listener = nullptr;
    EXPECT_CALL(fSocketHandlerMock, start(_)).WillOnce(SaveRef<0>(&listener));
    cut.start();
    ::ip::IPEndpoint localEndpoint(::ip::make_ip4(0x3839), 238);

    // bound server socket should lead to nothing
    listener->serverSocketBound(21U, localEndpoint, ConnectionType::PLAIN);
}

} // namespace test
} // namespace doip
