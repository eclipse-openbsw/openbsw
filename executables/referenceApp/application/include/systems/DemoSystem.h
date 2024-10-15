// Copyright 2024 Accenture.

#pragma once

#include <async/Async.h>
#include <async/IRunnable.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <lifecycle/console/LifecycleControlCommand.h>
#ifdef PLATFORM_SUPPORT_CAN
#include "app/CanDemoListener.h"

#include <can/console/CanCommand.h>
#include <console/AsyncCommandWrapper.h>
#include <systems/ICanSystem.h>
#endif

#ifdef PLATFORM_SUPPORT_ETHERNET
#include <lwipSocket/netif/LwipNetworkInterface.h>
#include <lwipSocket/tcp/LwipServerSocket.h>
#include <lwipSocket/tcp/LwipSocket.h>
#include <lwipSocket/udp/LwipDatagramSocket.h>
#include <tcp/socket/AbstractServerSocket.h>
#include <tcp/util/LoopbackTestServer.h>
#include <tcp/util/TcpIperf2Server.h>
#include <udp/socket/AbstractDatagramSocket.h>
#include <udp/util/UdpEchoTestServer.h>
#include <udp/util/UdpIperf2Server.h>
#endif

namespace systems
{

class DemoSystem
: public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
{
public:
    explicit DemoSystem(
        ::async::ContextType context,
        ::lifecycle::ILifecycleManager& lifecycleManager
#ifdef PLATFORM_SUPPORT_CAN
        ,
        ::can::ICanSystem& canSystem
#endif
    );

    DemoSystem(DemoSystem const&)            = delete;
    DemoSystem& operator=(DemoSystem const&) = delete;

    void init() override;
    void run() override;
    void shutdown() override;

    void cyclic();

private:
    void execute() override;

private:
    ::async::ContextType const _context;
    ::async::TimeoutType _timeout;
#ifdef PLATFORM_SUPPORT_CAN
    ::can::ICanSystem& _canSystem;
    ::can::CanDemoListener _canDemoListener;
    ::can::CanCommand _canCommand;
    ::console::AsyncCommandWrapper _asyncCommandWrapperForCanCommand;
#endif

#ifdef PLATFORM_SUPPORT_ETHERNET
    ::udp::LwipDatagramSocket _udpEchoSocket;
    ::udp::LwipDatagramSocket _udpIperfSocket;
    ::udp::UdpEchoTestServer _echoServer;
    ::udp::UdpIperf2Server _udpIperfServer;

    ::tcp::LwipSocket _tcpLoopbackSocket;
    ::tcp::LwipSocket _tcpIperfSocket;
    ::tcp::LoopbackTestServer _tcpLoopbackListener;
    ::tcp::LwipServerSocket _loopbackServer;
    ::tcp::TcpIperf2Server _tcpIperfListener;
    ::tcp::LwipServerSocket _iperfServer;
#endif
};

} // namespace systems
