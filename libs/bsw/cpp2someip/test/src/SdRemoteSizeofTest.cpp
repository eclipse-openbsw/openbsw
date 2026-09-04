/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/RebootTracker.h"
#include "someip/RebootTrackerEndpoint.h"
#include "someip/SdMessageBuilder.h"
#include "someip/SdMessageParser.h"
#include "someip/SdOptionParser.h"
#include "someip/SdOptions.h"
#include "someip/SdReceiver.h"
#include "someip/SdServiceRegistry.h"
#include "someip/SdSomeIpStack.h"
#include "someip/ServiceAnnouncer.h"
#include "someip/ServiceAnnouncerTask.h"
#include "someip/SessionManager.h"

#include <gtest/gtest.h>

namespace
{

class SocketDummy : public ::tcp::AbstractSocket
{
public:
    ::ip::IPAddress getRemoteIPAddress() const override
    {
#ifdef PLATFORM_SUPPORT_IPV6
        return ::ip::make_ip6(0U);
#else
        return ::ip::make_ip4(0U);
#endif
    }

    ::ip::IPAddress getLocalIPAddress() const override
    {
#ifdef PLATFORM_SUPPORT_IPV6
        return ::ip::make_ip6(0U);
#else
        return ::ip::make_ip4(0U);
#endif
    }

    ErrorCode bind(::ip::IPAddress const&, uint16_t) override
    {
        return ErrorCode::SOCKET_ERR_NOT_OK;
    }

    ErrorCode close() override { return ErrorCode::SOCKET_ERR_NOT_OK; }

    ErrorCode connect(::ip::IPAddress const&, uint16_t, ConnectedDelegate) override
    {
        return ErrorCode::SOCKET_ERR_NOT_OK;
    }

    ErrorCode flush() override { return ErrorCode::SOCKET_ERR_NOT_OK; }

    void discardData() override {}

    ErrorCode send(::etl::span<uint8_t const> const&) override
    {
        return ErrorCode::SOCKET_ERR_NOT_OK;
    }

    bool isClosed() const override { return false; }

    bool isEstablished() const override { return false; }

    size_t available() override { return size_t(); }

    size_t read(uint8_t*, size_t) override { return size_t(); }

    uint16_t getLocalPort() const override { return uint16_t(); }

    uint16_t getRemotePort() const override { return uint16_t(); }

    uint8_t read(uint8_t&) override { return uint8_t(); }

    void abort() override {}

    void disableNagleAlgorithm() override {}

    void enableKeepAlive(uint32_t const, uint32_t const, uint32_t const) override {}

    void disableKeepAlive() override {}
};

class ServerSocketDummy : public ::tcp::AbstractServerSocket
{
public:
    bool accept() override { return false; }

    bool bind(::ip::IPAddress const&, uint16_t) override { return false; }

    void close() override {}

    bool isClosed() const override { return true; }
};

/**
 * Verify sizeof RebootTracker, RebootTrackerEndpoint, ServiceAnnouncer, ServiceAnnouncerTask
 * SdServiceRegistry, SessionManager<1U>, SdMessageBuilder, SdMessageParser, SdOptionParser,
 * SdOptions, SdReceiver and SdSomeIpStack<1U, 1U, 1U, 1U, 1U, 1U>.
 */
TEST(SdRemoteSizeof, Sizeof)
{
    using UdpSocketType                              = SocketDummy;
    constexpr uint8_t NumUdpRpcSockets               = 1U;
    using TcpServerSocketType                        = ServerSocketDummy;
    constexpr uint8_t NumTcpServerSockets            = 0U;
    using TcpSocketType                              = SocketDummy;
    constexpr uint8_t NumTcpRpcSockets               = 0U;
    constexpr size_t BufferSize                      = 1500U;
    constexpr uint8_t NumTpStreams                   = 0U;
    constexpr bool SdEndpointOption                  = false;
    constexpr size_t InternalTcpReassembleBufferSize = 1U;

    constexpr uint16_t NumRemoteEndpoints      = 1U;
    constexpr uint16_t NumRemoteServices       = 1U;
    constexpr uint16_t NumRemoteSubscriptions  = 1U;
    constexpr uint16_t NumLocalServices        = 1U;
    constexpr uint16_t NumLocalQueries         = 1U;
    constexpr uint8_t NumSubscriptionEndpoints = 1U;
    constexpr uint8_t NumEventBuffers          = 1U;
    constexpr uint8_t NumMulticastReceptions   = 2U;

    using SdSomeIpStackType = ::someip::declare::SdSomeIpStack<
        UdpSocketType,
        NumUdpRpcSockets,
        TcpServerSocketType,
        NumTcpServerSockets,
        TcpSocketType,
        NumTcpRpcSockets,
        BufferSize,
        NumTpStreams,
        SdEndpointOption,
        InternalTcpReassembleBufferSize,
        NumRemoteEndpoints,
        NumRemoteServices,
        NumRemoteSubscriptions,
        NumLocalServices,
        NumLocalQueries,
        NumSubscriptionEndpoints,
        NumEventBuffers,
        NumMulticastReceptions>;

    if (sizeof(size_t) == 8U)
    {
#ifdef PLATFORM_SUPPORT_IPV6
        EXPECT_EQ(8U, sizeof(::someip::RebootTracker));
        EXPECT_EQ(6U, sizeof(::someip::RebootTrackerEndpoint));
        EXPECT_EQ(3944U, sizeof(::someip::ServiceAnnouncer));
        EXPECT_EQ(72U, sizeof(::someip::ServiceAnnouncerTask));
        EXPECT_EQ(80U, sizeof(::someip::SdServiceRegistry));
        EXPECT_EQ(136U, sizeof(::someip::declare::SessionManager<NumRemoteEndpoints>));
        EXPECT_EQ(24U, sizeof(::someip::SdMessageBuilder));
        EXPECT_EQ(72U, sizeof(::someip::SdMessageParser));
        EXPECT_EQ(1U, sizeof(::someip::SdOptionParser));
        EXPECT_EQ(24U, sizeof(::someip::SdOptions));
        EXPECT_EQ(16U, sizeof(::someip::SdReceiver));

        EXPECT_EQ(12064, sizeof(SdSomeIpStackType));
#else
        EXPECT_EQ(8U, sizeof(::someip::RebootTracker));
        EXPECT_EQ(6U, sizeof(::someip::RebootTrackerEndpoint));
        EXPECT_EQ(3168U, sizeof(::someip::ServiceAnnouncer));
        EXPECT_EQ(48U, sizeof(::someip::ServiceAnnouncerTask));
        EXPECT_EQ(88U, sizeof(::someip::SdServiceRegistry));
        EXPECT_EQ(120U, sizeof(::someip::declare::SessionManager<NumRemoteEndpoints>));
        EXPECT_EQ(24U, sizeof(::someip::SdMessageBuilder));
        EXPECT_EQ(72U, sizeof(::someip::SdMessageParser));
        EXPECT_EQ(1U, sizeof(::someip::SdOptionParser));
        EXPECT_EQ(24U, sizeof(::someip::SdOptions));
        EXPECT_EQ(16U, sizeof(::someip::SdReceiver));

        EXPECT_EQ(11200U, sizeof(SdSomeIpStackType));
#endif
    }
}

} // anonymous namespace
