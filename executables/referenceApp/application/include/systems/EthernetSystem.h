/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "busid/BusId.h"
#include "ethConfig.h"
#include "lifecycle/ILifecycleManager.h"

#include <async/Async.h>
#include <async/IRunnable.h>
#include <async/util/Call.h>
#include <ethernet/NetworkInterface.h>
#include <etl/array.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <shed/ops.h>
#include <shed/table.h>
#include <systems/IEthernetDriverSystem.h>

namespace systems
{

struct NetifsSchema;
using Netifs                                              = ::shed::table<NetifsSchema>;
static constexpr size_t NUM_NETIF_CONFIG_CHANGE_LISTENERS = 2U;
using NetifConfigRegistry = ::ip::declare::NetworkInterfaceConfigRegistry<
    NUM_NETIF_CONFIG_CHANGE_LISTENERS,
    Netifs,
    ::ethernet::NetifBusId>;

struct NetifsSchema
{
    using states = ::shed::states<
        ::ethernet::NETIF_CONFIGURED,
        ::ethernet::NETIF_INITIALISED,
        ::ethernet::NETIF_STARTED,
        ::ethernet::NETIF_UP>;
    using columns = ::shed::columns<
        ::shed::column<::ethernet::NetifBusId>,
        ::shed::column<::ethernet::NetifPort>,
        ::shed::column<::ip::Ip4Config>,
        ::shed::column<::ethernet::NetifVlanId>,
        ::shed::column<::ip::NetworkInterfaceConfig>,
        ::shed::column<::ethernet::NetifState>,
        ::shed::column<::ethernet::LinkStatus>,
        ::shed::column<netif>,
        ::shed::shared<NetifConfigRegistry>>;
};

class EthernetSystem
: public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
{
public:
    explicit EthernetSystem(
        ::async::ContextType context, ::ethernet::IEthernetDriverSystem& ethernetSystem);

    EthernetSystem(EthernetSystem const&)            = delete;
    EthernetSystem& operator=(EthernetSystem const&) = delete;

    void init() override;
    void run() override;
    void shutdown() override;

    void execute() override;

    ::ethernet::IEthernetDriverSystem& ethernetDriverSystem;
    Netifs netifs;

    void onNetifStatusChanged(size_t i);

private:
    ::async::ContextType const _context;
    ::async::TimeoutType _timeout;
    std::size_t _executeCounter;
    ::etl::array<uint8_t, Netifs::memory_for(::ethX::NUM_NETIFS)> _netifsMem;
};

} // namespace systems
