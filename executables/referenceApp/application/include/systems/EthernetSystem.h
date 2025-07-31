// Copyright 2025 Accenture.

#pragma once

#include "busid/BusId.h"
#include "ethConfig.h"
#include "lifecycle/ILifecycleManager.h"

#include <async/Async.h>
#include <async/IRunnable.h>
#include <async/util/Call.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <systems/IEthernetDriverSystem.h>

namespace systems
{

template<size_t N>
struct Netifs
{
    ::etl::array<uint8_t, N> busIds{::busid::ETH_0, ::busid::ETH_1};
    // Mapping of netif to physical port (for link status handling).
    ::etl::array<uint8_t, N> ports{0, 0};
    ::etl::array<::ip::Ip4Config, N> ip4Configs;
    ::etl::array<uint16_t, N> vlanIds{::ethX::VLAN_UNTAGGED, 160};
    ::etl::array<::ip::NetworkInterfaceConfig, N> networkInterfaceConfigsIp4{
        ::ip::NetworkInterfaceConfig(
            ::ip::ip4_to_u32(::eth0::IP_ADDRESS),
            ::ip::ip4_to_u32(::eth0::NETWORK_MASK),
            ::ip::ip4_to_u32(::eth0::DEFAULT_GATEWAY)),
        ::ip::NetworkInterfaceConfig(
            ::ip::ip4_to_u32(::eth1::IP_ADDRESS),
            ::ip::ip4_to_u32(::eth1::NETWORK_MASK),
            ::ip::ip4_to_u32(::eth1::DEFAULT_GATEWAY)),
    };
    // ::etl::array<::ip::NetworkInterfaceConfig, N> configsIp6; // TODO
    ::etl::array<::lwipnetif::NetifState, N> netifStates;
    ::etl::array<bool, N> linkStatus{false};
    ::etl::array<netif, N> netifs;
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
    Netifs<::ethX::NUM_NETIFS> netifs;

private:
    ::async::ContextType const _context;
    ::async::TimeoutType _timeout;
    std::size_t _executeCounter;
};

} // namespace systems
