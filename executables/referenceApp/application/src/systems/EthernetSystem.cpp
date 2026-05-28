/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "systems/EthernetSystem.h"

#include <ethernet/EthernetLogger.h>
#include <lwip/init.h>
#include <lwip/prot/ethernet.h>
#include <lwip/timeouts.h>
#include <lwipSocket/utils/LwipHelper.h>

#include <etl/error_handler.h>

extern "C"
{
int32_t vlanForNetif(void const* const vlwipNi)
{
    ETL_ASSERT(vlwipNi != nullptr, ETL_ERROR_GENERIC("netif must not be null"));

    auto const lwipNi         = static_cast<netif const*>(vlwipNi);
    auto const ethernetSystem = static_cast<::systems::EthernetSystem*>(lwipNi->state);

    auto const netifSpan = ::shed::get<netif>(ethernetSystem->netifs).data();
    ETL_ASSERT(
        lwipNi >= netifSpan.begin() && lwipNi < netifSpan.end(),
        ETL_ERROR_GENERIC("netif must be part of this system"));

    auto const i = lwipNi - netifSpan.begin();

    auto const vlanid = ::shed::get<::ethernet::NetifVlanId>(ethernetSystem->netifs)[i].value;
    if (vlanid == ::ethX::VLAN_UNTAGGED)
    {
        return -1; // Any value < 0 means no vlan tag
    }
    return vlanid;
}
}

static void netifStatusCallback(netif* const lwipNi)
{
    ETL_ASSERT(lwipNi != nullptr, ETL_ERROR_GENERIC("netif must not be null"));
    auto* const sys      = reinterpret_cast<::systems::EthernetSystem*>(lwipNi->state);
    auto const netifSpan = ::shed::get<netif>(sys->netifs).data();
    ETL_ASSERT(
        lwipNi >= netifSpan.begin() && lwipNi < netifSpan.end(),
        ETL_ERROR_GENERIC("netif must be part of this system"));
    auto const idx = static_cast<size_t>(lwipNi - netifSpan.begin());
    sys->onNetifStatusChanged(idx);
}

static err_t linkoutput(netif* const aNetif, struct pbuf* const buf)
{
    auto const ethernetSystem = static_cast<::systems::EthernetSystem*>(aNetif->state);
    if ((aNetif->flags & NETIF_FLAG_LINK_UP) == 0)
    {
        return ERR_VAL;
    }

    if (ethernetSystem->ethernetDriverSystem.writeFrame(aNetif, buf))
    {
        return ERR_OK;
    }

    return ERR_VAL;
}

#if LWIP_IGMP
static err_t joinMulticastGroupIpV4(
    struct netif* const aNetif,
    ip4_addr_t const* const group,
    const enum netif_mac_filter_action action)
{
    if ((aNetif == nullptr) || (group == nullptr))
    {
        return ERR_VAL;
    }
    if (action == NETIF_ADD_MAC_FILTER)
    {
        auto const ethernetSystem = static_cast<::systems::EthernetSystem*>(aNetif->state);
        ::etl::array<uint8_t, 6> groupAddress
            = {LL_IP4_MULTICAST_ADDR_0,
               LL_IP4_MULTICAST_ADDR_1,
               LL_IP4_MULTICAST_ADDR_2,
               static_cast<uint8_t>(ip4_addr2(group) & static_cast<uint8_t>(0x7fU)),
               ip4_addr3(group),
               ip4_addr4(group)};
        ethernetSystem->ethernetDriverSystem.setGroupcastAddressRecognition(groupAddress);
    }
    return ERR_OK;
}
#endif

static ::systems::EthernetSystem& ethernetSystemFor(::netif& lnetif)
{
    ETL_ASSERT(lnetif.state != nullptr, ETL_ERROR_GENERIC("netif must have an owner"));
    return *static_cast<::systems::EthernetSystem*>(lnetif.state);
}

static ::shed::move_op initNetif(::ethernet::NetifState& state, ::netif& lnetif)
{
    lnetif.linkoutput = &linkoutput;
    ::lwiputils::initNetifDriverParameters(::ethX::MAC_ADDRESS, lnetif);
    netif_set_status_callback(&lnetif, &netifStatusCallback);
#if LWIP_IGMP
    netif_set_igmp_mac_filter(&lnetif, joinMulticastGroupIpV4);
#endif

    state = ::ethernet::NetifState::Initialised;
    return ::shed::move_op::MOVE;
}

static void refreshNetifLinkStatus(
    ::ethernet::NetifPort& port, ::ethernet::LinkStatus& linkStatus, ::netif& lnetif)
{
    linkStatus = ethernetSystemFor(lnetif).ethernetDriverSystem.getLinkStatus(port.value)
                     ? ::ethernet::LinkStatus::Up
                     : ::ethernet::LinkStatus::Down;
}

namespace systems
{

EthernetSystem::EthernetSystem(
    ::async::ContextType const context, ::ethernet::IEthernetDriverSystem& ethernetSystem)
: ethernetDriverSystem(ethernetSystem)
, netifs(::shed::shared<NetifConfigRegistry>(netifs))
, _context(context)
, _executeCounter(0)
{
    netifs.init(::etl::span<uint8_t>(_netifsMem), ::ethX::NUM_NETIFS);

    ::shed::insert<::ethernet::NETIF_CONFIGURED>(
        netifs,
        [](::ethernet::NetifBusId& busId,
           ::ethernet::NetifVlanId& vlanId,
           ::ip::NetworkInterfaceConfig& config)
        {
            busId.value  = ::busid::ETH_0;
            vlanId.value = ::ethX::VLAN_UNTAGGED;
            config       = ::ip::NetworkInterfaceConfig(
                ::ip::ip4_to_u32(::eth0::IP_ADDRESS),
                ::ip::ip4_to_u32(::eth0::NETWORK_MASK),
                ::ip::ip4_to_u32(::eth0::DEFAULT_GATEWAY));
        });

    ::shed::insert<::ethernet::NETIF_CONFIGURED>(
        netifs,
        [](::ethernet::NetifBusId& busId,
           ::ethernet::NetifVlanId& vlanId,
           ::ip::NetworkInterfaceConfig& config)
        {
            busId.value  = ::busid::ETH_1;
            vlanId.value = 160U;
            config       = ::ip::NetworkInterfaceConfig(
                ::ip::ip4_to_u32(::eth1::IP_ADDRESS),
                ::ip::ip4_to_u32(::eth1::NETWORK_MASK),
                ::ip::ip4_to_u32(::eth1::DEFAULT_GATEWAY));
        });

    setTransitionContext(context);
}

void EthernetSystem::init()
{
    lwip_init();
    ::shed::for_each(
        netifs,
        [this](::netif& lnetif, ::ip::Ip4Config& ip4, ::ip::NetworkInterfaceConfig& config)
        {
            lnetif.state   = this;
            lnetif.name[0] = 0;
#if LWIP_NETIF_SPECIFIC_TTL
            lnetif.ttl = IP_DEFAULT_TTL;
#endif
            ETL_ASSERT(
                ::lwipnetif::initNetifIp4(lnetif, ip4, config, lnetif.state),
                ETL_ERROR_GENERIC("netif_add failed"));
        });
    transitionDone();
}

void EthernetSystem::run()
{
    ::shed::move_to<::ethernet::NETIF_INITIALISED>::from<::ethernet::NETIF_CONFIGURED>(
        netifs, &initNetif);
    ::shed::move_to<::ethernet::NETIF_STARTED>::from<::ethernet::NETIF_INITIALISED>(
        netifs, &::lwipnetif::startNetif);

    ::async::scheduleAtFixedRate(_context, *this, _timeout, 1, ::async::TimeUnit::MILLISECONDS);
    transitionDone();
}

void EthernetSystem::shutdown()
{
    _timeout.cancel();
    ::shed::move_to<::ethernet::NETIF_STARTED>::from<::ethernet::NETIF_UP>(
        netifs, &::lwipnetif::downNetif);
    ::shed::move_to<::ethernet::NETIF_CONFIGURED>::from<::ethernet::NETIF_STARTED>(
        netifs, &::lwipnetif::stopNetif);
    transitionDone();
}

void EthernetSystem::onNetifStatusChanged(size_t const i)
{
    if (::lwipnetif::onStatusChangedIp4(
            ::shed::get<::ethernet::NetifState>(netifs)[i],
            ::shed::get<netif>(netifs)[i],
            ::shed::get<::ip::NetworkInterfaceConfig>(netifs)[i]))
    {
        ::shed::get<NetifConfigRegistry>(netifs).value.configChangedSignal(
            ::shed::get<::ethernet::NetifBusId>(netifs)[i],
            ::shed::get<::ip::NetworkInterfaceConfig>(netifs)[i]);
    }
}

void EthernetSystem::execute()
{
    // Call processPbufQueue every millisecond.
    ::lwiputils::processPbufQueue(
        ethernetDriverSystem.getRx(),
        ::shed::get<netif>(netifs).data(),
        ::shed::get<::ethernet::NetifVlanId>(netifs).data());

    if (_executeCounter % 10 == 0)
    {
        // Perform link status checks every 10 ms.
        ::shed::for_each<::ethernet::NETIF_UP>(netifs, &refreshNetifLinkStatus);
        ::shed::for_each<::ethernet::NETIF_STARTED>(netifs, &refreshNetifLinkStatus);
        ::shed::move_to<::ethernet::NETIF_STARTED>::from<::ethernet::NETIF_UP>(
            netifs,
            &::lwipnetif::checkNetif<
                ::ethernet::LinkStatus::Down,
                NetifConfigRegistry,
                ::ethernet::NetifBusId>);
        ::shed::move_to<::ethernet::NETIF_UP>::from<::ethernet::NETIF_STARTED>(
            netifs,
            &::lwipnetif::checkNetif<
                ::ethernet::LinkStatus::Up,
                NetifConfigRegistry,
                ::ethernet::NetifBusId>);
    }
    if (_executeCounter % 50 == 0)
    {
        // Call lwip timeout checks every 50 ms.
        sys_check_timeouts();
    }
    ++_executeCounter;
}

} // namespace systems
