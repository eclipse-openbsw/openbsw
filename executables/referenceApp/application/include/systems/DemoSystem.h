/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <cstddef>

#include <async/Async.h>
#include <async/IRunnable.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <lifecycle/ILifecycleManager.h>
#ifdef PLATFORM_SUPPORT_CAN
#include "app/CanDemoListener.h"

#include <can/console/CanCommand.h>
#include <console/AsyncCommandWrapper.h>
#include <systems/ICanSystem.h>
#endif
#ifdef PLATFORM_SUPPORT_STORAGE
#include <storage/IStorage.h>
#include <storage/StorageJob.h>
#include <util/buffer/LinkedBuffer.h>
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

#ifdef PLATFORM_SUPPORT_MIDDLEWARE
#include "middleware/ClusterCore0.h"
#include "middleware/ClusterCore1.h"
#include "middleware/FooProxyWrapper.h"
#include "middleware/FooSkeletonWrapper.h"
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
#ifdef PLATFORM_SUPPORT_STORAGE
        ,
        ::storage::IStorage& storage
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

#ifdef PLATFORM_SUPPORT_STORAGE
    void storageJobDone(::storage::StorageJob&);

    ::storage::IStorage& _storage;
    ::storage::StorageJob _storageJob;
    ::util::buffer::LinkedBuffer<uint8_t> _storageReadBuf;
    ::util::buffer::LinkedBuffer<uint8_t const> _storageWriteBuf;
    ::storage::StorageJob::JobDoneCallback const _jobDoneCallback;

    // BEGIN storage data
    struct StorageData
    {
        uint32_t intParam;
        uint8_t charParam0;
        uint8_t charParam1;
        uint16_t reserved;
    };

    static_assert(
        offsetof(StorageData, charParam0) == sizeof(uint32_t),
        "StorageData layout must keep charParam0 immediately after intParam");

    // END storage data

    StorageData _storageData;
#endif

#ifdef PLATFORM_SUPPORT_MIDDLEWARE
    FooSkeletonWrapper _fooSkeletonWrapper;
    FooProxyWrapper _fooProxyWrapper;
    uint32_t _middlewareCycleCount{0U};
#endif
};

} // namespace systems
