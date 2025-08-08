// Copyright 2025 Accenture.

#pragma once

#include <etl/span.h>
#include <ip/IPAddress.h>
#include <util/estd/memory.h>
#include <util/spsc/Queue.h>

extern "C"
{
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "netif/etharp.h"
} // extern "C"

namespace lwiputils
{

struct RxCustomPbuf
{
    // `buf` always needs to stay the first member of this
    // struct so we can upcast from `pbuf` to `RxCustomPbuf`
    pbuf_custom buf;
    void* driver;
    void* slot;
};

using PbufQueue = ::util::spsc::Queue<pbuf*, 10>;

err_t initNetifDriverParameters(::etl::span<uint8_t const, 6> const macAddr, netif& lwipNetif);
::ip::IPAddress from_lwipIp(ip_addr_t const& lwipIp);
void to_lwipIp(::ip::IPAddress const& ip, ip_addr_t* const dst);

inline ip_addr_t to_lwipIp(::ip::IPAddress const& ip)
{
    ip_addr_t lwipIp = {};
    to_lwipIp(ip, &lwipIp);
    return lwipIp;
}

netif* filterETHFrames(pbuf* const pCompleteFrame, bool const enableVlanTagging);

bool processPbufQueue(
    ::lwiputils::PbufQueue::Receiver receiver,
    ::etl::span<netif> lwnetifs,
    ::etl::span<uint16_t const> vlanIds);
} // namespace lwiputils
