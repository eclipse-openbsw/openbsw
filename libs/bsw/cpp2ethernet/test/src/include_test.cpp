/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "ethernet/EthernetLogger.h"
#include "ethernet/NetworkInterface.h"
#include "ip/IPAddress.h"
#include "ip/IPEndpoint.h"
#include "ip/NetworkInterfaceConfig.h"
#include "ip/to_str.h"
#include "tcp/IDataListener.h"
#include "tcp/IDataSendNotificationListener.h"
#include "tcp/TcpLogger.h"
#include "tcp/socket/AbstractServerSocket.h"
#include "tcp/socket/AbstractSocket.h"
#include "tcp/socket/ISocketProvidingConnectionListener.h"
#include "tcp/util/BandwidthTestSocket.h"
#include "tcp/util/LoopbackTestServer.h"
#include "tcp/util/TcpIperf2Server.h"
#include "udp/DatagramPacket.h"
#include "udp/IDataListener.h"
#include "udp/IDataSentListener.h"
#include "udp/UdpLogger.h"
#include "udp/socket/AbstractDatagramSocket.h"
#include "udp/util/UdpEchoTestServer.h"
#include "udp/util/UdpIperf2Server.h"

#include <gmock/gmock.h>

#include <etl/type_traits.h>

namespace
{
using namespace ::testing;

TEST(IncludeTest, TestIncludes) {}

/**
 * Verify non-owning socket interfaces prohibit base destruction while concrete
 * listeners remain stack-destructible.
 */
TEST(IncludeTest, SocketAndListenerInterfacesPreventBaseDestruction)
{
    static_assert(!::etl::has_virtual_destructor<::tcp::IDataListener>::value);
    static_assert(!::etl::has_virtual_destructor<::tcp::IDataSendNotificationListener>::value);
    static_assert(!::etl::has_virtual_destructor<::tcp::AbstractSocket>::value);
    static_assert(!::etl::has_virtual_destructor<::tcp::AbstractServerSocket>::value);
    static_assert(!::etl::has_virtual_destructor<::udp::IDataListener>::value);
    static_assert(!::etl::has_virtual_destructor<::udp::IDataSentListener>::value);
    static_assert(!::etl::has_virtual_destructor<::udp::AbstractDatagramSocket>::value);
    static_assert(!::etl::is_destructible<::tcp::IDataListener>::value);
    static_assert(!::etl::is_destructible<::tcp::IDataSendNotificationListener>::value);
    static_assert(!::etl::is_destructible<::tcp::AbstractSocket>::value);
    static_assert(!::etl::is_destructible<::tcp::AbstractServerSocket>::value);
    static_assert(!::etl::is_destructible<::udp::IDataListener>::value);
    static_assert(!::etl::is_destructible<::udp::IDataSentListener>::value);
    static_assert(!::etl::is_destructible<::udp::AbstractDatagramSocket>::value);
    static_assert(!::etl::is_copy_constructible<::udp::UdpEchoTestServer>::value);
    static_assert(!::etl::is_move_constructible<::udp::UdpIperf2Server>::value);

    class Listener final : public ::tcp::IDataListener
    {
    public:
        explicit Listener(bool& destroyed) : _destroyed(destroyed) {}

        ~Listener() { _destroyed = true; }

        void dataReceived(uint16_t) override {}

        void connectionClosed(ErrorCode) override {}

    private:
        bool& _destroyed;
    };

    static_assert(::etl::is_destructible<Listener>::value);
    bool destroyed{false};
    {
        Listener listener(destroyed);
        EXPECT_FALSE(destroyed);
    }
    EXPECT_TRUE(destroyed);
}
} // anonymous namespace
