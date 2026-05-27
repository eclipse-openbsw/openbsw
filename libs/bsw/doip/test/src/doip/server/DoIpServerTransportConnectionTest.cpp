/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/DoIpServerTransportConnection.h"

#include "doip/common/DoIpTransportMessageProvidingListenerMock.h"
#include "doip/server/DoIpServerTransportConnectionConfig.h"
#include "doip/server/DoIpServerTransportLayerParameters.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <common/busid/BusId.h>
#include <etl/pool.h>
#include <tcp/socket/AbstractSocketMock.h>
#include <transport/BufferedTransportMessage.h>
#include <transport/TransportMessageProcessedListenerMock.h>

#include <gmock/gmock.h>

namespace doip
{
namespace test
{
using namespace ::testing;
using namespace ::transport;

class DoIpServerTransportConnectionTest : public Test
{
public:
    DoIpServerTransportConnectionTest()
    : fBusId(0x0U)
    , asyncMock()
    , asyncContext(1U)
    , fParameters(0x123U, 0x4412U, 0x2139U, 0xABCU)
    , fConfig(
          fBusId,
          0x1245,
          asyncContext,
          fMessageProvidingListenerMock,
          fMessageProcessedListenerMock,
          fParameters)
    {}

protected:
    ::tcp::AbstractSocketMock fSocketMock;
    uint8_t fBusId;
    ::testing::StrictMock<::async::AsyncMock> asyncMock;
    ::async::ContextType asyncContext;
    StrictMock<DoIpTransportMessageProvidingListenerMock> fMessageProvidingListenerMock;
    StrictMock<TransportMessageProcessedListenerMock> fMessageProcessedListenerMock;
    DoIpServerTransportLayerParameters fParameters;
    DoIpServerTransportConnectionConfig fConfig;
    ::etl::pool<DoIpTransportMessageSendJob, 4> fDiagnosticSendJobBlockPool;
    ::etl::pool<DoIpServerTransportMessageHandler::StaticPayloadSendJobType, 4>
        fProtocolSendJobBlockPool;
};

TEST_F(DoIpServerTransportConnectionTest, TestInitializationAndMarking)
{
    DoIpServerTransportConnection cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        14U,
        fSocketMock,
        fConfig,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        DoIpTcpConnection::ConnectionType::PLAIN);
    EXPECT_EQ(&fSocketMock, &cut.getConnection().getSocket());
    EXPECT_EQ(14U, cut.getSocketGroupId());
    ASSERT_FALSE(cut.isMarkedForClose());
    cut.markForClose(IDoIpTcpConnection::CloseMode::CLOSE);
    ASSERT_TRUE(cut.isMarkedForClose());
}

TEST_F(DoIpServerTransportConnectionTest, TestInitializationAndMarkingTls)
{
    DoIpServerTransportConnection cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        14U,
        fSocketMock,
        fConfig,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        DoIpTcpConnection::ConnectionType::TLS);
    EXPECT_EQ(&fSocketMock, &cut.getConnection().getSocket());
    EXPECT_EQ(14U, cut.getSocketGroupId());
    ASSERT_FALSE(cut.isMarkedForClose());
    cut.markForClose(IDoIpTcpConnection::CloseMode::CLOSE);
    ASSERT_TRUE(cut.isMarkedForClose());
}

TEST_F(DoIpServerTransportConnectionTest, TestSendMessage)
{
    DoIpServerTransportConnection cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        14U,
        fSocketMock,
        fConfig,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        DoIpTcpConnection::ConnectionType::PLAIN);
    BufferedTransportMessage<6> message;
    message.setSourceAddress(0x3345);
    message.setTargetAddress(0x4456);
    ASSERT_FALSE(cut.send(message, &fMessageProcessedListenerMock));
}

TEST_F(DoIpServerTransportConnectionTest, TestSendMessageTls)
{
    DoIpServerTransportConnection cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        14U,
        fSocketMock,
        fConfig,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        DoIpTcpConnection::ConnectionType::TLS);
    BufferedTransportMessage<6> message;
    message.setSourceAddress(0x3345);
    message.setTargetAddress(0x4456);
    ASSERT_FALSE(cut.send(message, &fMessageProcessedListenerMock));
}

} // namespace test
} // namespace doip
