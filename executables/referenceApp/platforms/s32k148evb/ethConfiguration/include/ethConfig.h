// Copyright 2025 Accenture.

#pragma once

#include "ip/IPAddress.h"

namespace ethX
{
static constexpr size_t NUM_NETIFS                    = 2;
static constexpr uint16_t VLAN_UNTAGGED               = 0xFFFF;
// TODO: Choose appropriate MAC. Locally administrated seems to cause issues on S32K1xx.
static constexpr ::etl::array<uint8_t, 6> MAC_ADDRESS = {0x10, 0x11, 0x22, 0x77, 0x77, 0x77};
} // namespace ethX

namespace eth0
{
static constexpr ::ip::IPAddress IP_ADDRESS(::ip::make_ip4(192U, 168U, 0U, 200U));
static constexpr ::ip::IPAddress NETWORK_MASK(::ip::make_ip4(255U, 255U, 255U, 0U));
static constexpr ::ip::IPAddress DEFAULT_GATEWAY(::ip::make_ip4(0U, 0U, 0U, 0U));
} // namespace eth0

namespace eth1
{

static constexpr ::ip::IPAddress IP_ADDRESS(::ip::make_ip4(192U, 168U, 2U, 200U));
static constexpr ::ip::IPAddress NETWORK_MASK(::ip::make_ip4(255U, 255U, 255U, 0U));
static constexpr ::ip::IPAddress DEFAULT_GATEWAY(::ip::make_ip4(0U, 0U, 0U, 0U));
} // namespace eth1
