// Copyright 2025 Accenture.

#pragma once

#include "ip/NetworkInterfaceConfig.h"

#include <etl/functional.h>
#include <etl/optional.h>
#include <etl/span.h>

extern "C"
{
#include "lwip/netif.h"
} // extern "C"

namespace lwipnetif
{
enum class State
{
    Uninitialised,
    Initialised,
    Started
};

struct NetifState
{
    State state = State::Uninitialised;
};

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
    ::ip::NetworkInterfaceConfig const& netifConfig,
    void* state);
bool onStatusChangedIp4(
    ::lwipnetif::State state, netif& netif, ::ip::NetworkInterfaceConfig& config);
#endif

void start(netif& ni, ::ip::Ip4Config const& config);
void stop(netif& ni, ::lwipnetif::NetifState& state, ::ip::Ip4Config& config);

void onLinkStatusChanged(bool const isLinkUp, netif& ni);

#if LWIP_IPV6
void createIp6Address();
#endif

} // namespace lwipnetif
