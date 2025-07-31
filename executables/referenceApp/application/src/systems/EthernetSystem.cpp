// Copyright 2025 Accenture.

#include "systems/EthernetSystem.h"

#include <bsp/SystemTime.h>
#include <ethernet/EthernetLogger.h>
#include <etl/functional.h>
#include <lwip/init.h>
#include <lwip/prot/ethernet.h>
#include <lwip/timeouts.h>
#include <lwipSocket/utils/LwipHelper.h>
#include <util/estd/big_endian.h>

extern "C"
{
int32_t vlanForNetif(void const* const vlwipNi)
{
    estd_assert(vlwipNi != nullptr);
    auto const lwipNi         = reinterpret_cast<netif const*>(vlwipNi);
    auto const ethernetSystem = reinterpret_cast<::systems::EthernetSystem*>(lwipNi->state);

    estd_assert(
        lwipNi >= ethernetSystem->netifs.netifs.begin()
        && lwipNi < ethernetSystem->netifs.netifs.end());

    auto const i = lwipNi - ethernetSystem->netifs.netifs.begin();

    auto const vlanid = ethernetSystem->netifs.vlanIds[i];
    if (vlanid == ::ethX::VLAN_UNTAGGED)
    {
        return -1; // Any value < 0 means no vlan tag
    }
    return vlanid;
}
}

static err_t linkoutput(netif* const aNetif, struct pbuf* const buf)
{
    auto const ethernetSystem = reinterpret_cast<::systems::EthernetSystem*>(aNetif->state);
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
        auto const ethernetSystem = reinterpret_cast<::systems::EthernetSystem*>(aNetif->state);
        uint8_t groupAddress[6]
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
        ::estd::memory::copy(
            ::estd::memory::make_span(netif.name).reinterpret_as<uint8_t>(),
            ::estd::memory::make_str(""));
    }
    setTransitionContext(context);
}

void EthernetSystem::init()
{
    lwip_init();

    for (size_t i = 0; i < netifs.netifs.size(); ++i)
    {
        if (!lwipnetif::initNetifIp4(
                netifs.netifs[i], netifs.ip4Configs[i], netifs.networkInterfaceConfigsIp4[i], this))
        {
            continue;
        }
        netifs.netifStates[i].state = ::lwipnetif::State::Initialised;
        netifs.netifs[i].linkoutput = &linkoutput;

        ::lwiputils::initNetifDriverParameters(::ethX::MAC_ADDRESS, netifs.netifs[i]);

        auto const lwipNetif = &netifs.netifs[i];
#if LWIP_IGMP
        netif_set_igmp_mac_filter(lwipNetif, joinMulticastGroupIpV4);
#endif
        netif_set_up(lwipNetif);
    }
    transitionDone();
}

void EthernetSystem::run()
{
    for (size_t i = 0; i < netifs.netifStates.size(); ++i)
    {
        if (netifs.netifStates[i].state == ::lwipnetif::State::Initialised)
        {
            ::lwipnetif::start(netifs.netifs[i], netifs.ip4Configs[i]);
            netifs.netifStates[i].state = ::lwipnetif::State::Started;
        }
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

                auto& linkStatus = netifs.linkStatus[i];
                auto& netif      = netifs.netifs[i];

                ::lwipnetif::onLinkStatusChanged(linkStatus, netif);
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
