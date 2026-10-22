/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/IDoIpSendJobCallback.h"
#include "doip/common/IDoIpTcpConnection.h"
#include "doip/common/IDoIpTransportMessageProvidingListener.h"
#include "doip/common/IDoIpVehicleAnnouncementListener.h"
#include "doip/server/IDoIpServerConnection.h"
#include "doip/server/IDoIpServerConnectionHandlerCallback.h"
#include "doip/server/IDoIpServerEntityStatusCallback.h"
#include "doip/server/IDoIpServerMessageHandler.h"
#include "doip/server/IDoIpServerSocketHandler.h"
#include "doip/server/IDoIpServerSocketHandlerListener.h"
#include "doip/server/IDoIpServerTransportConnectionCreator.h"
#include "doip/server/IDoIpServerTransportConnectionPool.h"
#include "doip/server/IDoIpServerTransportConnectionProviderCallback.h"
#include "doip/server/IDoIpServerTransportLayerCallback.h"
#include "doip/server/IDoIpServerVehicleIdentificationCallback.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

namespace doip
{
namespace test
{
namespace
{
struct SendJobCallback : IDoIpSendJobCallback<int>
{
    void releaseSendJob(int&, bool) override {}
};

struct MessageProvidingListener : IDoIpTransportMessageProvidingListener
{
    GetResult getTransportMessage(
        uint8_t,
        uint16_t,
        uint16_t,
        uint16_t,
        ::etl::span<uint8_t const> const&,
        ::transport::TransportMessage*&) override
    {
        return GetResult();
    }

    void releaseTransportMessage(::transport::TransportMessage&) override {}

    ReceiveResult messageReceived(
        uint8_t,
        ::transport::TransportMessage&,
        ::transport::ITransportMessageProcessedListener*) override
    {
        return ReceiveResult();
    }
};

struct AnnouncementListener : IDoIpVehicleAnnouncementListener
{
    void
    vehicleAnnouncementReceived(uint16_t, ::ip::IPEndpoint const&, ::ip::IPEndpoint const&) override
    {}
};

template<class Base>
struct Connection : Base
{
    void init(IDoIpConnectionHandler&) override {}

    ::ip::IPEndpoint getLocalEndpoint() const override { return ::ip::IPEndpoint(); }

    ::ip::IPEndpoint getRemoteEndpoint() const override { return ::ip::IPEndpoint(); }

    bool receivePayload(::etl::span<uint8_t>, IDoIpConnection::PayloadReceivedCallbackType) override
    {
        return false;
    }

    void endReceiveMessage(IDoIpConnection::PayloadDiscardedCallbackType) override {}

    bool sendMessage(IDoIpSendJob&) override { return false; }

    void close() override {}

    void suspendSending() override {}

    void resumeSending() override {}
};

struct TcpConnection : Connection<IDoIpTcpConnection>
{
    void setCloseMode(CloseMode) override {}

    void detach(DetachCallbackType) override {}
};

struct ServerConnection : Connection<IDoIpServerConnection>
{
    uint16_t getSourceAddress() const override { return 0U; }

    uint16_t getInternalSourceAddress() const override { return 0U; }

    void sendNack(uint8_t, bool) override {}
};

template<class Base>
struct ConnectionFilter : Base
{
    IDoIpServerConnectionFilter::RoutingActivationCheckResult checkRoutingActivation(
        uint16_t,
        uint8_t,
        uint8_t,
        ::ip::IPEndpoint const&,
        ::ip::IPEndpoint const&,
        ::etl::optional<uint32_t>,
        bool) override
    {
        return IDoIpServerConnectionFilter::RoutingActivationCheckResult();
    }
};

struct ConnectionHandlerCallback : ConnectionFilter<IDoIpServerConnectionHandlerCallback>
{
    void handleRoutingActivationRequest(DoIpServerConnectionHandler&) override {}

    void routingActive(DoIpServerConnectionHandler&) override {}

    void aliveCheckResponseReceived(DoIpServerConnectionHandler&, bool) override {}

    void connectionClosed(DoIpServerConnectionHandler&) override {}
};

struct TransportLayerCallback : ConnectionFilter<IDoIpServerTransportLayerCallback>
{
    uint8_t getSocketGroupId(uint8_t, ::ip::IPEndpoint const&) const override { return 0U; }

    uint8_t getMaxConnectionCount(uint8_t) const override { return 0U; }

    bool filterConnection(uint8_t, ::ip::IPEndpoint const&) override { return true; }

    void routingActive(
        uint16_t,
        uint16_t,
        ::ip::IPEndpoint const&,
        ::ip::IPEndpoint const&,
        DoIpTcpConnection::ConnectionType) override
    {}

    void routingInactive(uint16_t) override {}

    void connectionClosed(uint16_t) override {}
};

struct EntityStatusCallback : IDoIpServerEntityStatusCallback
{
    EntityStatus getEntityStatus(uint8_t) const override { return EntityStatus(0U, 0U, 0U, 0U); }
};

struct SocketHandler : IDoIpServerSocketHandler
{
    void start(IDoIpServerSocketHandlerListener&) override {}

    void stop() override {}

    ::tcp::AbstractSocket* acquireSocket() override { return nullptr; }

    void releaseSocket(::tcp::AbstractSocket&, DoIpTcpConnection::ConnectionType) override {}
};

struct SocketHandlerListener : IDoIpServerSocketHandlerListener
{
    void
    serverSocketBound(uint8_t, ::ip::IPEndpoint const&, DoIpTcpConnection::ConnectionType) override
    {}

    bool filterConnection(uint8_t, ::ip::IPEndpoint const&) override { return true; }

    void
    connectionAccepted(uint8_t, ::tcp::AbstractSocket&, DoIpTcpConnection::ConnectionType) override
    {}
};

struct ConnectionCreator : IDoIpServerTransportConnectionCreator<DoIpServerTransportConnection>
{
    DoIpServerTransportConnection& createConnection(
        DoIpServerTransportConnection* memory,
        uint8_t,
        ::tcp::AbstractSocket&,
        ::etl::ipool&,
        ::etl::ipool&,
        DoIpServerTransportConnectionConfig const&,
        DoIpTcpConnection::ConnectionType) override
    {
        return *memory;
    }
};

struct ConnectionPool : IDoIpServerTransportConnectionPool
{
    DoIpServerTransportConnection* createConnection(
        uint8_t,
        ::tcp::AbstractSocket&,
        DoIpServerTransportConnectionConfig const&,
        DoIpTcpConnection::ConnectionType) override
    {
        return nullptr;
    }

    void releaseConnection(DoIpServerTransportConnection&) override {}
};

struct ConnectionProviderCallback : IDoIpServerTransportConnectionProviderCallback
{
    uint8_t getSocketGroupId(uint8_t, ::ip::IPEndpoint const&) const override { return 0U; }

    bool filterConnection(uint8_t, ::ip::IPEndpoint const&) override { return true; }

    void connectionAccepted(DoIpServerTransportConnection&) override {}
};

struct VehicleIdentificationCallback : IDoIpServerVehicleIdentificationCallback
{
    void getVin(VinType) override {}

    void getGid(GidType) override {}

    void getEid(EidType) override {}

    DoIpConstants::DiagnosticPowerMode getPowerMode() override
    {
        return DoIpConstants::DiagnosticPowerMode();
    }

    void onVirReceived() override {}

    IDoIpUdpOemMessageHandler* getOemMessageHandler(uint16_t) const override { return nullptr; }
};

struct MessageHandler : IDoIpServerMessageHandler
{
    void connectionOpened(IDoIpServerConnection&) override {}

    void routingActive() override {}

    bool headerReceived(DoIpHeader const&) override { return false; }

    void connectionClosed() override {}
};

static_assert(!std::is_copy_constructible<MessageHandler>::value);
static_assert(!std::is_move_constructible<MessageHandler>::value);
static_assert(!std::is_copy_assignable<MessageHandler>::value);
static_assert(!std::is_move_assignable<MessageHandler>::value);

template<class Interface>
struct InterfaceWithState : Interface
{
    uint32_t value{123U};
};

template<class T>
class DoIpInterfaceCopyTest : public ::testing::Test
{};

using CopyOnlyInterfaces = ::testing::Types<
    SendJobCallback,
    MessageProvidingListener,
    AnnouncementListener,
    Connection<IDoIpConnection>,
    TcpConnection,
    ServerConnection,
    ConnectionFilter<IDoIpServerConnectionFilter>,
    ConnectionHandlerCallback,
    TransportLayerCallback,
    DoIpConcreteOemMessageHandler<0xf000U, 1U, 0xf001U, 1U>>;

TYPED_TEST_SUITE(DoIpInterfaceCopyTest, CopyOnlyInterfaces);

TYPED_TEST(DoIpInterfaceCopyTest, PreservesCopyConstructionWithoutEnablingAssignment)
{
    using Implementation = InterfaceWithState<TypeParam>;
    static_assert(std::is_copy_constructible<Implementation>::value);
    static_assert(std::is_move_constructible<Implementation>::value);
    static_assert(!std::is_copy_assignable<Implementation>::value);
    static_assert(!std::is_move_assignable<Implementation>::value);

    Implementation const original;
    auto copy = original;
    EXPECT_EQ(123U, copy.value);
    copy.value = 456U;
    EXPECT_EQ(123U, original.value);
    auto const moved = std::move(copy);
    EXPECT_EQ(456U, moved.value);
}

template<class T>
class DoIpInterfaceAssignmentTest : public ::testing::Test
{};

using AssignableInterfaces = ::testing::Types<
    EntityStatusCallback,
    SocketHandler,
    SocketHandlerListener,
    ConnectionCreator,
    ConnectionPool,
    ConnectionProviderCallback,
    VehicleIdentificationCallback>;

TYPED_TEST_SUITE(DoIpInterfaceAssignmentTest, AssignableInterfaces);

TYPED_TEST(DoIpInterfaceAssignmentTest, PreservesCopyConstructionAndAssignment)
{
    using Implementation = InterfaceWithState<TypeParam>;
    static_assert(std::is_copy_constructible<Implementation>::value);
    static_assert(std::is_move_constructible<Implementation>::value);
    static_assert(std::is_copy_assignable<Implementation>::value);
    static_assert(std::is_move_assignable<Implementation>::value);

    Implementation const original;
    auto copy = original;
    EXPECT_EQ(123U, copy.value);
    copy.value = 456U;
    EXPECT_EQ(123U, original.value);
    Implementation assigned;
    assigned = copy;
    EXPECT_EQ(456U, assigned.value);
    auto moved = std::move(copy);
    EXPECT_EQ(456U, moved.value);
    assigned = std::move(moved);
    EXPECT_EQ(456U, assigned.value);
}
} // namespace
} // namespace test
} // namespace doip
