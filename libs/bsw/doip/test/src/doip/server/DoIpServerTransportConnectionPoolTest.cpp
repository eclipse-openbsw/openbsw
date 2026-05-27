/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/DoIpServerTransportConnectionPool.h"

#include "doip/common/DoIpTransportMessageProvidingListenerMock.h"
#include "doip/server/DoIpServerTransportConnection.h"
#include "doip/server/DoIpServerTransportConnectionConfig.h"
#include "doip/server/DoIpServerTransportLayerParameters.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <common/busid/BusId.h>
#include <etl/functional.h>
#include <etl/pool.h>
#include <tcp/socket/AbstractSocketMock.h>
#include <transport/TransportMessage.h>
#include <transport/TransportMessageProcessedListenerMock.h>

#include <gmock/gmock.h>

namespace doip
{
namespace test
{
using namespace ::testing;
using namespace ::transport;
using ConnectionType = DoIpTcpConnection::ConnectionType;

class TestTransportConnection : public DoIpServerTransportConnection
{
public:
    TestTransportConnection(
        uint8_t const socketGroupId,
        ::tcp::AbstractSocket& socket,
        DoIpServerTransportConnectionConfig const& config,
        ::etl::ipool& diagnosticSendJobBlockPool,
        ::etl::ipool& protocolSendJobBlockPool,
        DoIpTcpConnection::ConnectionType const type)
    : DoIpServerTransportConnection(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        socketGroupId,
        socket,
        config,
        diagnosticSendJobBlockPool,
        protocolSendJobBlockPool,
        type)
    {}
};

class DoIpServerTransportConnectionPoolTest
: public Test
, public IDoIpServerTransportConnectionCreator<TestTransportConnection>
{
public:
    DoIpServerTransportConnectionPoolTest()
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
    {}

protected:
    using TransportConnectionPoolType
        = declare::DoIpServerTransportConnectionPool<TestTransportConnection, 1, 2, 2>;

    virtual DoIpServerTransportConnection& createConnection(
        TestTransportConnection* memory,
        uint8_t socketGroupId,
        ::tcp::AbstractSocket& socket,
        ::etl::ipool& diagnosticBlockPool,
        ::etl::ipool& protocolBlockPool,
        DoIpServerTransportConnectionConfig const& config,
        DoIpTcpConnection::ConnectionType const type)
    {
        auto* const p = new (memory) TestTransportConnection(
            socketGroupId,
            ::etl::ref(socket),
            config,
            ::etl::ref(diagnosticBlockPool),
            ::etl::ref(protocolBlockPool),
            type);

        return *p;
    }

    uint8_t fBusId;
    ::testing::StrictMock<::async::AsyncMock> asyncMock;
    ::async::ContextType asyncContext;
    DoIpTransportMessageProvidingListenerMock fMessageProvidingListenerMock;
    TransportMessageProcessedListenerMock fMessageProcessedListenerMock;
    DoIpServerTransportLayerParameters fParameters;
    DoIpServerTransportConnectionConfig fConfig;
    ::tcp::AbstractSocketMock fSocketMock;
    ::etl::pool<DoIpTransportMessageSendJob, 2> fDiagnosticSendJobBlockPool;
    ::etl::pool<DoIpServerTransportMessageHandler::StaticPayloadSendJobType, 2>
        fProtocolSendJobBlockPool;
};

TEST_F(DoIpServerTransportConnectionPoolTest, TestCreateAndRelease)
{
    TransportConnectionPoolType cut(*this);

    // create first connection
    DoIpServerTransportConnection* connection1
        = cut.createConnection(31U, fSocketMock, fConfig, ConnectionType::PLAIN);
    EXPECT_TRUE(connection1 != nullptr);
    EXPECT_EQ(31U, connection1->getSocketGroupId());
    Mock::VerifyAndClearExpectations(this);

    // create second connection
    DoIpServerTransportConnection* connection2
        = cut.createConnection(32U, fSocketMock, fConfig, ConnectionType::PLAIN);
    EXPECT_TRUE(connection2 != nullptr);
    EXPECT_EQ(32U, connection2->getSocketGroupId());
    Mock::VerifyAndClearExpectations(this);

    // third connection should fail!
    EXPECT_TRUE(cut.createConnection(31U, fSocketMock, fConfig, ConnectionType::PLAIN) == nullptr);
    Mock::VerifyAndClearExpectations(this);

    // Release connection and expect next creation to succeed
    cut.releaseConnection(*connection2);

    connection2 = cut.createConnection(33U, fSocketMock, fConfig, ConnectionType::PLAIN);
    EXPECT_TRUE(connection2 != nullptr);
    EXPECT_EQ(33U, connection2->getSocketGroupId());
    Mock::VerifyAndClearExpectations(this);
}

} // namespace test
} // namespace doip
