/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/PduTransportConfig.h"

#include <blob/Config.h>

namespace routing
{
bool load(::etl::span<uint8_t const> mem, PduTransportConfig& config)
{
    if (mem.empty())
    {
        return false;
    }

    Header header;
    (void)load(mem, header);
    if (header.configType != ::blob::Config::Type::CHANNEL)
    {
        return false;
    }
    if (header.channelType != ::routing::ChannelType::PDU_TRANSPORT)
    {
        return false;
    }
    config.channelId = header.channelId;

    if (mem.size() < sizeof(config.type) + sizeof(config.mode) + sizeof(config.vlanId)
                         + sizeof(config.localPort) + sizeof(config.remotePort)
                         + sizeof(::etl::be_uint16_t))
    {
        return false;
    }

    config.type = mem.take<uint8_t const>();

    auto const modeValue = mem.take<uint8_t const>();
    if ((modeValue != static_cast<uint8_t>(PduTransportConfig::Mode::RX))
        && (modeValue != static_cast<uint8_t>(PduTransportConfig::Mode::TX))
        && (modeValue != static_cast<uint8_t>(PduTransportConfig::Mode::RX_TX)))
    {
        return false;
    }
    config.mode       = static_cast<PduTransportConfig::Mode>(modeValue);
    config.vlanId     = mem.take<::etl::be_uint16_t const>();
    config.localPort  = mem.take<::etl::be_uint16_t const>();
    config.remotePort = mem.take<::etl::be_uint16_t const>();

    uint16_t const ipVersionValue = mem.take<::etl::be_uint16_t const>();
    if ((ipVersionValue != static_cast<uint16_t>(::ip::IPAddress::Family::IPV4))
        && (ipVersionValue != static_cast<uint16_t>(::ip::IPAddress::Family::IPV6)))
    {
        return false;
    }
    auto const ipVersion = static_cast<::ip::IPAddress::Family>(ipVersionValue);
    bool const isIpV4    = ipVersion == ::ip::IPAddress::Family::IPV4;
#ifdef ESR_NO_IPV6
    if (!isIpV4 || (mem.size() < ::ip::IPAddress::IP4LENGTH))
    {
        return false;
    }
    auto const ipAddressLength = ::ip::IPAddress::IP4LENGTH;
    config.ipAddress           = ::ip::make_ip4(mem.take<uint8_t const>(ipAddressLength));
#else
    auto const ipAddressLength = isIpV4 ? ::ip::IPAddress::IP4LENGTH : ::ip::IPAddress::IP6LENGTH;
    if (mem.size() < ipAddressLength)
    {
        return false;
    }
    config.ipAddress = isIpV4 ? ::ip::make_ip4(mem.take<uint8_t const>(ipAddressLength))
                              : ::ip::make_ip6(mem.take<uint8_t const>(ipAddressLength));
#endif

    if (mem.size() < sizeof(config.pcp) + sizeof(config.transmissionTimeout) + sizeof(uint8_t))
    {
        return false;
    }

    config.pcp                      = mem.take<uint8_t const>();
    config.transmissionTimeout      = mem.take<::etl::be_uint16_t const>();
    auto const remoteIpAddressCount = static_cast<size_t>(mem.take<uint8_t const>());
    if (mem.size() < ipAddressLength * remoteIpAddressCount)
    {
        return false;
    }
    config.remoteIpAddresses = mem.take<uint8_t const>(ipAddressLength * remoteIpAddressCount);

    return true;
}
} // namespace routing
