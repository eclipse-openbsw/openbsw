// Copyright 2025 Accenture.

#include "doip/server/DoIpServerSocketHandler.h"

#include "doip/common/DoIpConstants.h"
#include "doip/server/DoIpServerSocketHandlerListenerMock.h"

#include <async/AsyncMock.h>
#include <ip/IPAddress.h>
#include <ip/NetworkInterfaceConfigRegistryMock.h>
#include <tcp/socket/AbstractServerSocketMock.h>
#include <tcp/socket/AbstractSocketMock.h>

#include <gmock/gmock.h>
#include <gtest/esr_extensions.h>

namespace
{
using namespace ::testing;
using namespace ::doip;
using namespace ::tcp;
using namespace ::ip;
using ConnectionType = DoIpTcpConnection::ConnectionType;

struct TestParams
{
    ConnectionType type;
    uint16_t port;
};

struct DoIpServerSocketHandlerTest : TestWithParam<TestParams>
{
    DoIpServerSocketHandlerTest() {}

    ::ip::NetworkInterfaceConfigRegistryMock fNetworkInterfaceConfigRegistryMock;
    DoIpServerSocketHandlerListenerMock fSocketHandlerListenerMock;
};

uint32_t const ip6Address2[] = {0x1234244U, 0x437773U, 0x87337U, 0xffdd243U};
} // namespace

INSTANTIATE_TEST_SUITE_P(
    ConnectionType,
    DoIpServerSocketHandlerTest,
    Values(TestParams{ConnectionType::PLAIN, DoIpConstants::Ports::TCP_DATA}));

TEST_F(DoIpServerSocketHandlerTest, AcquireSocket)
{
    declare::DoIpServerSocketHandler<AbstractServerSocketMock, AbstractSocketMock, 1, 2, 0> cut(
        fNetworkInterfaceConfigRegistryMock);

    AbstractSocket* firstSocket = cut.acquireSocket();
    EXPECT_THAT(firstSocket, NotNull());
    AbstractSocket* secondSocket = cut.acquireSocket();
    EXPECT_THAT(secondSocket, NotNull());

    EXPECT_THAT(cut.acquireSocket(), IsNull());

    cut.releaseSocket(*firstSocket, ConnectionType::PLAIN);
    firstSocket = cut.acquireSocket();
    EXPECT_THAT(firstSocket, NotNull());
    EXPECT_THAT(cut.acquireSocket(), IsNull());
    EXPECT_THAT(cut.acquireSocket(), IsNull());

    cut.releaseSocket(*secondSocket, ConnectionType::PLAIN);
    cut.releaseSocket(*firstSocket, ConnectionType::PLAIN);
}

TEST_P(DoIpServerSocketHandlerTest, SimpleServerLifecycle)
{
    declare::DoIpServerSocketHandler<AbstractServerSocketMock, AbstractSocketMock, 2, 1, 1> cut(
        fNetworkInterfaceConfigRegistryMock);

    NetworkInterfaceConfigKey key1(0U);
    NetworkInterfaceConfig config1(0x12389U, 0x2384923U, 0x23894U);
    ::AbstractServerSocketMock* serverSocketMock;
    if (GetParam().type == ConnectionType::PLAIN)
    {
        serverSocketMock = &cut.addServerSocket(13U, key1);
    }
    else
    {
        FAIL();
    }

    ::AbstractServerSocketMock& serverSocketMock1 = *serverSocketMock;

    NetworkInterfaceConfigKey key2(1U);
    NetworkInterfaceConfig config2(ip6Address2);
    if (GetParam().type == ConnectionType::PLAIN)
    {
        serverSocketMock = &cut.addServerSocket(47U, key2);
    }
    else
    {
        FAIL();
    }
    ::AbstractServerSocketMock& serverSocketMock2 = *serverSocketMock;

    // start with configured interface 2
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(key1))
        .WillOnce(Return(NetworkInterfaceConfig()));
    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(key2)).WillOnce(Return(config2));
    EXPECT_CALL(serverSocketMock1, isClosed()).WillOnce(Return(true));
    EXPECT_CALL(serverSocketMock2, isClosed()).WillOnce(Return(true));
    EXPECT_CALL(serverSocketMock2, bind(config2.ipAddress(), GetParam().port))
        .WillOnce(Return(true));
    EXPECT_CALL(serverSocketMock2, accept()).WillOnce(Return(true));
    EXPECT_CALL(
        fSocketHandlerListenerMock,
        serverSocketBound(
            47U, ::ip::IPEndpoint(config2.ipAddress(), GetParam().port), GetParam().type));
    cut.start(fSocketHandlerListenerMock);
    EXPECT_THAT(&serverSocketMock1.getSocketProvidingConnectionListener(), IsNull());
    EXPECT_THAT(&serverSocketMock2.getSocketProvidingConnectionListener(), NotNull());
    Mock::VerifyAndClearExpectations(&fNetworkInterfaceConfigRegistryMock);
    Mock::VerifyAndClearExpectations(&serverSocketMock1);
    Mock::VerifyAndClearExpectations(&serverSocketMock2);

    // bind fails
    EXPECT_CALL(serverSocketMock1, isClosed()).WillOnce(Return(true));
    EXPECT_CALL(serverSocketMock1, bind(config1.ipAddress(), GetParam().port))
        .WillOnce(Return(false));
    fNetworkInterfaceConfigRegistryMock.configChangedSignal(key1, config1);
    Mock::VerifyAndClearExpectations(&serverSocketMock1);

    // unknown config has changed
    fNetworkInterfaceConfigRegistryMock.configChangedSignal(
        static_cast<NetworkInterfaceConfigKey>(1U), config2);

    EXPECT_CALL(serverSocketMock1, isClosed()).WillOnce(Return(true));
    EXPECT_CALL(serverSocketMock2, isClosed()).WillOnce(Return(false));
    EXPECT_CALL(serverSocketMock2, close());
    cut.stop();
}

TEST_P(DoIpServerSocketHandlerTest, GetSocketLifecycle)
{
    declare::DoIpServerSocketHandler<AbstractServerSocketMock, AbstractSocketMock, 1, 2, 2> cut(
        fNetworkInterfaceConfigRegistryMock);

    NetworkInterfaceConfigKey key(0U);
    NetworkInterfaceConfig config(0x12389U, 0x2384923U, 0x23894U);
    ::AbstractServerSocketMock* serverSocketMockPtr;
    if (GetParam().type == ConnectionType::PLAIN)
    {
        serverSocketMockPtr = &cut.addServerSocket(47U, key);
    }
    else
    {
        FAIL();
    }
    ::AbstractServerSocketMock& serverSocketMock = *serverSocketMockPtr;

    EXPECT_CALL(fNetworkInterfaceConfigRegistryMock, getConfig(key)).WillOnce(Return(config));
    EXPECT_CALL(serverSocketMock, isClosed()).WillOnce(Return(true));
    EXPECT_CALL(serverSocketMock, bind(config.ipAddress(), GetParam().port)).WillOnce(Return(true));
    EXPECT_CALL(serverSocketMock, accept()).WillOnce(Return(true));
    EXPECT_CALL(
        fSocketHandlerListenerMock,
        serverSocketBound(
            47U, ::ip::IPEndpoint(config.ipAddress(), GetParam().port), GetParam().type));
    cut.start(fSocketHandlerListenerMock);
    EXPECT_THAT(&serverSocketMock.getSocketProvidingConnectionListener(), NotNull());
    Mock::VerifyAndClearExpectations(&fNetworkInterfaceConfigRegistryMock);
    Mock::VerifyAndClearExpectations(&serverSocketMock);

    ::ip::IPEndpoint sourceEndpoint(::ip::make_ip4(0x234U), 13483U);

    // reject address
    EXPECT_CALL(fSocketHandlerListenerMock, filterConnection(47U, sourceEndpoint))
        .WillOnce(Return(false));
    EXPECT_THAT(
        serverSocketMock.getSocketProvidingConnectionListener().getSocket(
            sourceEndpoint.getAddress(), sourceEndpoint.getPort()),
        IsNull());
    Mock::VerifyAndClearExpectations(&fSocketHandlerListenerMock);

    // filter and accept valid address
    EXPECT_CALL(fSocketHandlerListenerMock, filterConnection(47U, sourceEndpoint))
        .WillOnce(Return(true));
    ::tcp::AbstractSocket* socket
        = serverSocketMock.getSocketProvidingConnectionListener().getSocket(
            sourceEndpoint.getAddress(), sourceEndpoint.getPort());
    EXPECT_THAT(socket, NotNull());
    Mock::VerifyAndClearExpectations(&fSocketHandlerListenerMock);

    ::tcp::AbstractSocket* tlsSocket = nullptr;
    if (GetParam().type == ConnectionType::PLAIN)
    {
        EXPECT_CALL(
            fSocketHandlerListenerMock, connectionAccepted(47U, Ref(*socket), GetParam().type));
        serverSocketMock.getSocketProvidingConnectionListener().connectionAccepted(*socket);
        Mock::VerifyAndClearExpectations(&fSocketHandlerListenerMock);
        cut.releaseSocket(*socket, GetParam().type);
    }
    else
    {
        EXPECT_CALL(
            fSocketHandlerListenerMock,
            connectionAccepted(47U, Not((Ref(*socket))), GetParam().type))
            .WillOnce(SaveRef<1>(&tlsSocket));
        serverSocketMock.getSocketProvidingConnectionListener().connectionAccepted(*socket);
        Mock::VerifyAndClearExpectations(&fSocketHandlerListenerMock);
        cut.releaseSocket(*tlsSocket, GetParam().type);
    }

    EXPECT_CALL(serverSocketMock, isClosed()).WillOnce(Return(false));
    EXPECT_CALL(serverSocketMock, close());
    cut.stop();
}
