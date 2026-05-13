// Copyright 2025 Accenture.

#include "systems/EthernetSystem.h"

#include <bsp/SystemTime.h>
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

    ETL_ASSERT(
        lwipNi >= ethernetSystem->netifs.netifs.begin()
            && lwipNi < ethernetSystem->netifs.netifs.end(),
        ETL_ERROR_GENERIC("netif must be part of this system"));

    auto const i = lwipNi - ethernetSystem->netifs.netifs.begin();

    auto const vlanid = ethernetSystem->netifs.vlanIds[i];
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
    auto* const sys = reinterpret_cast<::systems::EthernetSystem*>(lwipNi->state);
    ETL_ASSERT(
        lwipNi >= sys->netifs.netifs.begin() && lwipNi < sys->netifs.netifs.end(),
        ETL_ERROR_GENERIC("netif must be part of this system"));
    auto const idx = static_cast<size_t>(lwipNi - sys->netifs.netifs.begin());
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

namespace systems
{

EthernetSystem::EthernetSystem(
    ::async::ContextType const context, ::ethernet::IEthernetDriverSystem& ethernetSystem)
: ethernetDriverSystem(ethernetSystem), _context(context), _executeCounter(0)
{
#if LWIP_NETIF_SPECIFIC_TTL
    for (auto& netif : netifs)
    {
        netif._netif.ttl = IP_DEFAULT_TTL;
    }
#endif
    for (auto& netif : netifs.netifs)
    {
        netif.name[0] = 0;
    }
    setTransitionContext(context);
}

void EthernetSystem::init()
{
    lwip_init();
    transitionDone();
}

void EthernetSystem::run()
{
    for (size_t i = 0; i < netifs.netifStates.size(); ++i)
    {
        auto& state = netifs.netifStates[i].state;
        auto& netif = netifs.netifs[i];

        if (state == ::lwipnetif::State::Uninitialised)
        {
            if (!lwipnetif::initNetifIp4(
                    netif, netifs.ip4Configs[i], netifs.networkInterfaceConfigsIp4[i], this))
            {
                continue;
            }

            netif.linkoutput = &linkoutput;
            ::lwiputils::initNetifDriverParameters(::ethX::MAC_ADDRESS, netif);
#if LWIP_IGMP
            netif_set_igmp_mac_filter(&netif, joinMulticastGroupIpV4);
#endif

            state = ::lwipnetif::State::Initialised;
        }

        if (state == ::lwipnetif::State::Initialised)
        {
            ::lwipnetif::start(netif, netifs.ip4Configs[i]);
            state = ::lwipnetif::State::Started;
        }

        bool phyLink         = ethernetDriverSystem.getLinkStatus(netifs.ports[i]);
        netifs.linkStatus[i] = phyLink;

        if (phyLink)
        {
            netif_set_link_up(&netif);
        }
        else
        {
            netif_set_link_down(&netif);
        }
        netif_set_up(&netif);
    }

    ::async::scheduleAtFixedRate(_context, *this, _timeout, 1, ::async::TimeUnit::MILLISECONDS);
    transitionDone();
}

void EthernetSystem::shutdown()
{
    _timeout.cancel();
    for (size_t i = 0; i < netifs.netifs.size(); ++i)
    {
        ::lwipnetif::stop(netifs.netifs[i], netifs.netifStates[i], netifs.ip4Configs[i]);
    }
    transitionDone();
}

void EthernetSystem::onNetifStatusChanged(size_t const i)
{
    if (::lwipnetif::onStatusChangedIp4(
            netifs.netifStates[i].state, netifs.netifs[i], netifs.networkInterfaceConfigsIp4[i]))
    {
        netifConfigRegistry.configChangedSignal(
            netifs.busIds[i], netifs.networkInterfaceConfigsIp4[i]);
    }
}

void EthernetSystem::execute()
{
    // Call processPbufQueue every millisecond.
    ::lwiputils::processPbufQueue(ethernetDriverSystem.getRx(), netifs.netifs, netifs.vlanIds);

    if (_executeCounter % 10 == 0)
    {
        // Perform link status checks every 10 ms.
        for (size_t i = 0; i < netifs.netifStates.size(); ++i)
        {
            bool const link = ethernetDriverSystem.getLinkStatus(netifs.ports[i]);
            if (link != netifs.linkStatus[i])
            {
                netifs.linkStatus[i] = link;
                ::lwipnetif::onLinkStatusChanged(link, netifs.netifs[i]);
                onNetifStatusChanged(i);
            }
        }
    }
    if (_executeCounter % 50 == 0)
    {
        // Call lwip timeout checks every 50 ms.
        sys_check_timeouts();
    }
    ++_executeCounter;
}

} // namespace systems
