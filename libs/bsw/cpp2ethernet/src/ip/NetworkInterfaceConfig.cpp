// Copyright 2025 Accenture.

#include "ip/NetworkInterfaceConfig.h"

namespace ip
{
uint8_t const IPADDRESS_INDEX      = 0U;
uint8_t const NETWORKMASK_INDEX    = 1U;
uint8_t const DEFAULTGATEWAY_INDEX = 2U;

NetworkInterfaceConfig::NetworkInterfaceConfig() : _config(), _family(IPAddress::FAMILY_UNKNOWN) {}

NetworkInterfaceConfig::NetworkInterfaceConfig(
    uint32_t const ip4Address, uint32_t const networkMask, uint32_t const defaultGateway)
: _family(IPAddress::IPV4)
{
    uint8_t const freeIndex = 3U;

    _config[IPADDRESS_INDEX]      = ip4Address;
    _config[NETWORKMASK_INDEX]    = networkMask;
    _config[DEFAULTGATEWAY_INDEX] = defaultGateway;
    _config[freeIndex]            = 0U;
}

NetworkInterfaceConfig::NetworkInterfaceConfig(Ip6AddressType const& ip6Address)
: _family(IPAddress::IPV6)
{
    _config[0U] = ip6Address[0];
    _config[1U] = ip6Address[1];
    _config[2U] = ip6Address[2];
    _config[3U] = ip6Address[3];
}

IPAddress NetworkInterfaceConfig::ipAddress() const
{
    if (_family == IPAddress::IPV4)
    {
        return make_ip4(_config[IPADDRESS_INDEX]);
    }
#ifndef OPENBSW_NO_IPV6
    if (_family == IPAddress::IPV6)
    {
        return make_ip6(&_config[0U]);
    }
#endif

    return IPAddress();
}

IPAddress NetworkInterfaceConfig::networkMask() const
{
    if (_family == IPAddress::IPV4)
    {
        return make_ip4(_config[NETWORKMASK_INDEX]);
    }

    return IPAddress();
}

IPAddress NetworkInterfaceConfig::defaultGateway() const
{
    if (_family == IPAddress::IPV4)
    {
        return make_ip4(_config[DEFAULTGATEWAY_INDEX]);
    }

    return IPAddress();
}

IPAddress NetworkInterfaceConfig::broadcastAddress() const
{
    if (_family == IPAddress::IPV4)
    {
        return make_ip4(
            _config[IPADDRESS_INDEX] | (static_cast<uint32_t>(~_config[NETWORKMASK_INDEX])));
    }

    return IPAddress();
}

bool operator==(NetworkInterfaceConfig const& lhs, NetworkInterfaceConfig const& rhs)
{
    return (lhs._family == rhs._family) && (lhs._config == rhs._config);
}

} // namespace ip
