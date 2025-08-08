// Copyright 2025 Accenture.

#include "lwip/netif.h"

#include <etl/span.h>
#include <lwipSocket/utils/LwipHelper.h>
#include <util/estd/big_endian.h>
#include <util/estd/memory.h>

uint16_t const VLAN_TAG              = 0x8100U;
uint16_t const VLAN_OUTER_TAG_LEGACY = 0x9100U;
uint16_t const VLAN_OUTER_TAG        = 0x88A8;
uint16_t const VID_MASK              = 0x0FFFU;

namespace
{
bool macAddressMatches(
    ::etl::span<uint8_t const, 6> const dstMac, ::etl::span<uint8_t const, 6> const ourMac)
{
    bool const isMulticastOrBroadcast = ((dstMac[0] & 0x01U) == 0x01U);
    if (isMulticastOrBroadcast)
    {
        return true;
    }
    auto const isSentToUs = ::estd::memory::is_equal(dstMac, ourMac);
    return isSentToUs;
}
} // namespace

namespace lwiputils
{

netif* filterETHFrames(
    pbuf* const pCompleteFrame,
    ::etl::span<netif> const lwnetifs,
    ::etl::span<uint16_t const> const vlanIds)
{
    if (pCompleteFrame == nullptr)
    {
        return nullptr;
    }

    auto payload = ::etl::span<uint8_t const>(
        static_cast<uint8_t*>(pCompleteFrame->payload), pCompleteFrame->len);

    auto const dstMac = ::estd::memory::take<::etl::array<uint8_t, 6>>(payload);
    payload.advance(6); // skip src mac
    uint16_t const ethernetType = ::estd::memory::take<::estd::be_uint16_t>(payload);
    uint16_t const tci          = ::estd::memory::take<::estd::be_uint16_t>(payload);

    if ((VLAN_OUTER_TAG == ethernetType) || (VLAN_OUTER_TAG_LEGACY == ethernetType))
    {
        // ignore double tagged frames
        return nullptr;
    }

    uint16_t const vlanId = (ethernetType == VLAN_TAG) ? (tci & VID_MASK) : 0xFFFF;

    size_t netifIdx = 0;
    for (; (netifIdx < vlanIds.size()); ++netifIdx)
    {
        if (vlanIds[netifIdx] == vlanId)
        {
            break;
        }
    }

    if (netifIdx >= lwnetifs.size())
    {
        return nullptr;
    }

    auto const ourMac = ::etl::span<uint8_t const, 6>(lwnetifs[netifIdx].hwaddr);
    if (!macAddressMatches(dstMac, ourMac))
    {
        return nullptr;
    }

    return &lwnetifs[netifIdx];
}

/**
 * Input all pbufs to their respective interfaces
 */
bool processPbufQueue(
    ::lwiputils::PbufQueue::Receiver receiver,
    ::etl::span<netif> const lwnetifs,
    ::etl::span<uint16_t const> const vlanIds)
{
    // Only process as many items as are in the queue when we
    // start the loop. This prevents the loop from never terminating
    // in case the queue is filled up very fast from an interrupt.
    auto const queued = receiver.size();
    for (size_t i = 0; i < queued; ++i)
    {
        auto const p      = receiver.read();
        auto const pNetIf = filterETHFrames(p, lwnetifs, vlanIds);
        if ((pNetIf != nullptr) && (pNetIf->input != nullptr))
        {
            (void)pNetIf->input(p, pNetIf);
        }
        else
        {
            (void)pbuf_free(p);
        }
    }
    return receiver.empty();
}

void to_lwipIp(::ip::IPAddress const& ip, ip_addr_t* const dst)
{
    if (dst != nullptr)
    {
        ::etl::span<uint8_t const> const ipSlice = ::ip::packed(ip);
        if (::ip::isIp4Address(ip))
        {
            IP_SET_TYPE(dst, IPADDR_TYPE_V4);

            ::etl::span<uint32_t> const ip4Slice
                = ::etl::span<uint32_t>(&(ip_2_ip4(dst)->addr), 1U);
            (void)::estd::memory::copy(ip4Slice.reinterpret_as<uint8_t>(), ipSlice);
        }
        else
        {
#if LWIP_IPV6
            IP_SET_TYPE(dst, IPADDR_TYPE_V6);
            ::etl::span<uint32_t, 4U> const ip6Slice(ip_2_ip6(dst)->addr);
            (void)::estd::memory::copy(ip6Slice.reinterpret_as<uint8_t>(), ipSlice);
#endif
        }
    }
}

err_t initNetifDriverParameters(::etl::span<uint8_t const, 6> const macAddr, netif& lwipNetif)
{
    lwipNetif.input = &ethernet_input;
#if LWIP_IPV4
    lwipNetif.output = &etharp_output;
#endif

#if LWIP_IPV6
    lwipNetif->output_ip6 = &ethip6_output;
#endif

    lwipNetif.hwaddr_len = 6U;
    lwipNetif.mtu        = 1500U; // IP_FRAG is 0, so this should have no impact - right?

    static constexpr uint8_t DEFAULT_FLAGS
        = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;
    lwipNetif.flags |= DEFAULT_FLAGS;

    ::estd::memory::copy(lwipNetif.hwaddr, macAddr);

    return ERR_OK;
}

::ip::IPAddress from_lwipIp(ip_addr_t const& lwipIp)
{
    if (IP_IS_V4_VAL(lwipIp))
    {
        uint32_t const* const lwipIp4Ptr           = &(ip_2_ip4(&lwipIp)->addr);
        ::etl::span<uint32_t const> const ip4Slice = ::etl::span<uint32_t const>(lwipIp4Ptr, 1U);
        return ::ip::make_ip4(ip4Slice.reinterpret_as<uint8_t const>());
    }
    else
    {
#if LWIP_IPV6
        ::etl::span<uint32_t const, 4U> const ip6Slice(ip_2_ip6(&lwipIp)->addr);
        return ::ip::make_ip6(ip6Slice.reinterpret_as<uint8_t const>());
#else
        return ::ip::IPAddress();
#endif
    }
}
} // namespace lwiputils
