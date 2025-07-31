// Copyright 2025 Accenture.

#include "lwipSocket/netif/LwipNetworkInterface.h"

#include "lwipSocket/utils/LwipHelper.h"

#include <ethernet/EthernetLogger.h>
#include <ip/IPAddress.h>
#include <ip/NetworkInterfaceConfig.h>
#include <tcp/TcpLogger.h>
#include <util/estd/memory.h>

extern "C"
{
#include "lwip/autoip.h"
#include "lwip/dhcp.h"
#include "lwip/dhcp6.h"
#include "lwip/ip_addr.h"
#include "lwip/prot/autoip.h"
#include "lwip/prot/dhcp.h"
} // extern "C"

namespace lwipnetif
{
namespace memory = ::estd::memory;
using ::util::logger::Logger;
using ::util::logger::TCP;

// LWIP needs this init callback otherwise it will assert
static err_t initNetif(netif* const /* netif */) { return ERR_OK; }

bool initNetifIp4(
    netif& lwipNetif,
    ::ip::Ip4Config const& config,
    ::ip::NetworkInterfaceConfig const& networkInterfaceConfig,
    void* state)
{
    ip4_addr ipAddress      = IPADDR4_INIT(IPADDR_ANY);
    ip4_addr networkMask    = IPADDR4_INIT(IPADDR_ANY);
    ip4_addr defaultGateway = IPADDR4_INIT(IPADDR_ANY);

    if (!config.useDhcp)
    {
        (void)memory::copy(
            memory::as_bytes(&ipAddress.addr), ip::packed(networkInterfaceConfig.ipAddress()));
        (void)memory::copy(
            memory::as_bytes(&networkMask.addr), ip::packed(networkInterfaceConfig.networkMask()));
        (void)memory::copy(
            memory::as_bytes(&defaultGateway.addr),
            ip::packed(networkInterfaceConfig.defaultGateway()));
    }

    auto const isInitialized
        = netif_add(
              &lwipNetif, &ipAddress, &networkMask, &defaultGateway, state, &initNetif, nullptr)
          != nullptr;

    lwipNetif.flags = 0U;
    return isInitialized;
}

#if LWIP_IPV6
bool initNetifWithStaticIp6Address(
    netif& lwipNetif,
    ::ip::NetworkInterfaceConfig const& netifConfig,
    bool& hasStaticIp4Address,
    void* state)
{
    bool isInitialized = false;
    if (netifConfig.ipFamily() == ::ip::IPAddress::IPV6)
    {
        isInitialized = netif_add(&lwipNetif, nullptr, nullptr, nullptr, state, &initNetif, nullptr)
                        != nullptr;

        lwipNetif.flags = 0U;
        if (!(isInitialized && (!_isStarted)))
        {
            ::util::logger::Logger::error(
                ::util::logger::TCP, "set static IP address called in wrong state");
        }

        ip_addr_t convertedAddress = lwiputils::to_lwipIp(ipAddress);
        if (IP_IS_V4(&convertedAddress))
        {
#if LWIP_IPV4
            netif_set_ipaddr(&_netif, ip_2_ip4(&convertedAddress));
            _hasStaticIp4Address = true;
#else
            estd_assert(false);
#endif
        }
        else
        {
#if LWIP_IPV4
            // set type to V6 if the main address is 0
            if (ip4_addr_isany(ip_2_ip4(&_netif.ip_addr)))
            {
                IP_SET_TYPE(&_netif.ip_addr, IPADDR_TYPE_V6);
            }
            setFlag(NETIF_FLAG_MLD6);
            netif_ip6_addr_set(&_netif, 0, ip_2_ip6(&convertedAddress));
            netif_ip6_addr_set_state(
                &_netif,
                0,
                static_cast<uint8_t>(IP6_ADDR_PREFERRED) | static_cast<uint8_t>(IP6_ADDR_VALID));
            _hasStaticIp6Address = true;
#endif
        }

        return isInitialized;
    }
}
#endif

void start(netif& ni, ::ip::Ip4Config const& config)
{
    if (config.useDhcp)
    {
#if LWIP_DHCP == 1
        ::util::logger::Logger::info(::util::logger::TCP, "dhcp_start");
        (void)dhcp_start(&ni

                         //, &lwiputils::LwipDhcpVendorOptionProvider::dhcp4OptionsReceived
        );
#endif
#if (LWIP_AUTOIP == 1) && (LWIP_DHCP_AUTOIP_COOP == 0)
        logger::Logger::info(logger::TCP, "autoip_start");
        autoip_start(&_netif);
#endif
    }

#if LWIP_IPV6_DHCP6 == 1
    if (!_hasStaticIp6Address)
    {
        logger::Logger::info(logger::TCP, "dhcp6_start");
        dhcp6_start(&_netif, &lwiputils::LwipDhcpVendorOptionProvider::dhcp6OptionsReceived);
    }
#endif
}

void stop(netif& ni, ::lwipnetif::NetifState& state, ::ip::Ip4Config& config)
{
    if (state.state != State::Started)
    {
        return;
    }

#if LWIP_DHCP == 1
    if (config.useDhcp)
    {
        dhcp_stop(&ni);
#if LWIP_AUTOIP == 1
        (void)autoip_stop(&ni);
#endif
    }
#endif
#if LWIP_IPV6_DHCP6 == 1
    if (!hasStaticIp6Address)
    {
        dhcp6_stop(&_netif);
    }
#endif

    netif_set_down(&ni);
    netif_remove(&ni);
    state.state = State::Uninitialised;
}

#if LWIP_IPV6
void createIp6Address()
{
    setFlag(NETIF_FLAG_MLD6);
#if LWIP_IPV6_AUTOCONFIG
    _netif.ip6_autoconfig_enabled = 1;
#endif
#if LWIP_IPV6_SEND_ROUTER_SOLICIT
    _netif.rs_count = LWIP_ND6_MAX_MULTICAST_SOLICIT;
#endif
    netif_create_ip6_linklocal_address(&_netif, 1);
    _hasStaticIp6Address = false;
}

#endif

void onLinkStatusChanged(bool const isLinkUp, netif& ni)
{
    ::util::logger::Logger::info(
        ::util::logger::ETHERNET, "linkStatusChanged(%s)", (isLinkUp ? "UP" : "DOWN"));
    if (isLinkUp)
    {
        netif_set_link_up(&ni);
    }
    else
    {
        netif_set_link_down(&ni);
    }
}

} // namespace lwipnetif
