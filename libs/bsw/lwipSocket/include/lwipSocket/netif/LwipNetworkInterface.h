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

#include "ethernet/NetworkInterface.h"
#include "ip/NetworkInterfaceConfig.h"

#include <etl/functional.h>
#include <etl/optional.h>
#include <etl/span.h>
#include <shed/move_op.h>

extern "C"
{
#include "lwip/netif.h"
} // extern "C"

namespace lwipnetif
{
#if LWIP_IPV4 && LWIP_IPV6
enum
{
    DHCP_CONTEXT_DEPRECATED   = 1,
    AUTOIP_CONTEXT_DEPRECATED = 2,
    DHCP_AUTOIP_CONTEXT       = 3
};
#endif

#if LWIP_IPV4
bool initNetifIp4(
    netif& lwipNetif,
    ::ip::Ip4Config const& config,
    ::ip::NetworkInterfaceConfig const& networkInterfaceConfig,
    void* state);

bool onStatusChangedIp4(
    ::ethernet::NetifState state, netif& netif, ::ip::NetworkInterfaceConfig& config);
#endif

::shed::move_op startNetif(::ethernet::NetifState& state, netif& ni, ::ip::Ip4Config const& config);
::shed::move_op stopNetif(netif& ni, ::ethernet::NetifState& state, ::ip::Ip4Config& config);
::shed::move_op downNetif(netif& ni);

void onLinkStatusChanged(bool const isLinkUp, netif& ni);

template<::ethernet::LinkStatus ExpectedLinkStatus, typename ConfigRegistry, typename BusId>
::shed::move_op checkNetif(
    ConfigRegistry& netifConfigRegistry,
    ::ethernet::LinkStatus const& linkStatus,
    netif& ni,
    BusId const& busId,
    ::ip::NetworkInterfaceConfig& config)
{
    if (linkStatus != ExpectedLinkStatus)
    {
        return ::shed::move_op::SKIP;
    }

    if constexpr (ExpectedLinkStatus == ::ethernet::LinkStatus::Up)
    {
        onLinkStatusChanged(true, ni);
        netif_set_up(&ni);
    }
    else
    {
        onLinkStatusChanged(false, ni);
        netif_set_down(&ni);
    }
    if (onStatusChangedIp4(::ethernet::NetifState::Started, ni, config))
    {
        netifConfigRegistry.configChangedSignal(busId, config);
    }
    return ::shed::move_op::MOVE;
}

#if LWIP_IPV6
void createIp6Address();
#endif

} // namespace lwipnetif
