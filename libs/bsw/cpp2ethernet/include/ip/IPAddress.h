// Copyright 2025 Accenture.

#pragma once

#include <etl/span.h>
#include <util/estd/big_endian.h>

#include <platform/estdint.h>

#include <cstring>

namespace ip
{
struct IPAddress
{
    enum Family
    {
        FAMILY_UNKNOWN = 0U, ///< \deprecated FAMILY_UNKNOWN will be removed in future
        IPV4           = 4U,
        IPV6           = 6U
    };

    static constexpr uint8_t IP4LENGTH = 4U;
    static constexpr uint8_t IP6LENGTH = 16U;

#ifndef OPENBSW_NO_IPV6
    static constexpr size_t MAX_IP_LENGTH = IP6LENGTH;
#else
    static constexpr size_t MAX_IP_LENGTH = IP4LENGTH;
#endif

    ::estd::be_uint32_t raw[MAX_IP_LENGTH / sizeof(uint32_t)];
};

constexpr IPAddress make_ip4(uint32_t ip4addr);

IPAddress make_ip4(::etl::span<uint8_t const> const& ip4addr);

#ifndef OPENBSW_NO_IPV6
constexpr IPAddress make_ip6(uint32_t addr0, uint32_t addr1, uint32_t addr2, uint32_t addr3);

constexpr IPAddress make_ip6(uint32_t const ip6addr[IPAddress::IP6LENGTH / sizeof(uint32_t)]);

IPAddress make_ip6(::etl::span<uint8_t const> const& ip6addr);
#endif

/** \deprecated  Use one of the other make_ip4 factory functions instead. */
constexpr IPAddress make_ip4(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);

::etl::span<uint8_t const> packed(IPAddress const& ipAddr);

uint32_t ip4_to_u32(IPAddress const& ipAddr);

#ifndef OPENBSW_NO_IPV6
uint32_t ip6_to_u32(IPAddress const& ipAddr, size_t offset);
#endif

bool isUnspecified(IPAddress const& ipAddr);

/** \deprecated  Test (addressFamilyOf(ipAddr) != IPAddress::IPV4) instead. */
bool isIp4Address(IPAddress const& ipAddr);

/** \deprecated  Test (addressFamilyOf(ipAddr) != IPAddress::IPV6) instead.  */
bool isIp6Address(IPAddress const& ipAddr);

bool isMulticastAddress(IPAddress const& ipAddr);

bool isLinkLocalAddress(IPAddress const& ipAddr);

bool isLoopbackAddress(IPAddress const& ipAddr);

bool isNetworkLocal(IPAddress const& ipAddr1, IPAddress const& ipAddr2, uint8_t networkId);

IPAddress::Family addressFamilyOf(IPAddress const& ipAddr);

bool operator==(IPAddress const& ip1, IPAddress const& ip2);

bool operator!=(IPAddress const& ip1, IPAddress const& ip2);

/** \deprecated  Implement your own comparator logic if needed */
struct IPAddressCompareLess
{
    bool operator()(IPAddress const& ipAddr1, IPAddress const& ipAddr2) const;
};

namespace internal
{
static constexpr size_t RAW_IP4_IDX
    = IPAddress::MAX_IP_LENGTH / static_cast<uint8_t>(sizeof(uint32_t)) - 1U;
} // namespace internal

inline constexpr IPAddress make_ip4(uint32_t const ip4addr)
{
    return {{
#ifndef OPENBSW_NO_IPV6
        {{0x00U, 0x00U, 0x00U, 0x00U}},
        {{0x00U, 0x00U, 0x00U, 0x00U}},
        {{0x00U, 0x00U, 0xFFU, 0xFFU}},
#endif
        {{static_cast<uint8_t>(ip4addr >> 24),
          static_cast<uint8_t>(ip4addr >> 16),
          static_cast<uint8_t>(ip4addr >> 8),
          static_cast<uint8_t>(ip4addr)}}}};
}

inline constexpr IPAddress
make_ip4(uint8_t const byte0, uint8_t const byte1, uint8_t const byte2, uint8_t const byte3)
{
    return {{
#ifndef OPENBSW_NO_IPV6
        {{0x00U, 0x00U, 0x00U, 0x00U}},
        {{0x00U, 0x00U, 0x00U, 0x00U}},
        {{0x00U, 0x00U, 0xFFU, 0xFFU}},
#endif
        {{byte0, byte1, byte2, byte3}}}};
}

inline IPAddress make_ip4(::etl::span<uint8_t const> const& ip4addr)
{
    estd_assert(ip4addr.size() == IPAddress::IP4LENGTH);

    // clang-format off
    IPAddress const newAddr = {{
#ifndef OPENBSW_NO_IPV6
        {{0x00U, 0x00U, 0x00U, 0x00U}},
        {{0x00U, 0x00U, 0x00U, 0x00U}},
        {{0x00U, 0x00U, 0xFFU, 0xFFU}},
#endif
        {{ip4addr[0U], ip4addr[1U], ip4addr[2U], ip4addr[3U]}}
    }};
    // clang-format on

    return newAddr;
}

#ifndef OPENBSW_NO_IPV6
inline constexpr IPAddress
make_ip6(uint32_t const addr0, uint32_t const addr1, uint32_t const addr2, uint32_t const addr3)
{
    return {
        {{{static_cast<uint8_t>(addr0 >> 24),
           static_cast<uint8_t>(addr0 >> 16),
           static_cast<uint8_t>(addr0 >> 8),
           static_cast<uint8_t>(addr0)}},
         {{static_cast<uint8_t>(addr1 >> 24),
           static_cast<uint8_t>(addr1 >> 16),
           static_cast<uint8_t>(addr1 >> 8),
           static_cast<uint8_t>(addr1)}},
         {{static_cast<uint8_t>(addr2 >> 24),
           static_cast<uint8_t>(addr2 >> 16),
           static_cast<uint8_t>(addr2 >> 8),
           static_cast<uint8_t>(addr2)}},
         {{static_cast<uint8_t>(addr3 >> 24),
           static_cast<uint8_t>(addr3 >> 16),
           static_cast<uint8_t>(addr3 >> 8),
           static_cast<uint8_t>(addr3)}}}};
}

inline constexpr IPAddress make_ip6(uint32_t const ip6addr[IPAddress::IP6LENGTH / sizeof(uint32_t)])
{
    return {
        {{{static_cast<uint8_t>(ip6addr[0] >> 24),
           static_cast<uint8_t>(ip6addr[0] >> 16),
           static_cast<uint8_t>(ip6addr[0] >> 8),
           static_cast<uint8_t>(ip6addr[0])}},
         {{static_cast<uint8_t>(ip6addr[1] >> 24),
           static_cast<uint8_t>(ip6addr[1] >> 16),
           static_cast<uint8_t>(ip6addr[1] >> 8),
           static_cast<uint8_t>(ip6addr[1])}},
         {{static_cast<uint8_t>(ip6addr[2] >> 24),
           static_cast<uint8_t>(ip6addr[2] >> 16),
           static_cast<uint8_t>(ip6addr[2] >> 8),
           static_cast<uint8_t>(ip6addr[2])}},
         {{static_cast<uint8_t>(ip6addr[3] >> 24),
           static_cast<uint8_t>(ip6addr[3] >> 16),
           static_cast<uint8_t>(ip6addr[3] >> 8),
           static_cast<uint8_t>(ip6addr[3])}}}};
}

inline IPAddress make_ip6(::etl::span<uint8_t const> const& ip6addr)
{
    estd_assert(ip6addr.size() == IPAddress::IP6LENGTH);

    IPAddress newAddr;
    (void)memcpy(newAddr.raw[0U].bytes, ip6addr.data(), IPAddress::IP6LENGTH);
    return newAddr;
}
#endif

inline ::etl::span<uint8_t const, IPAddress::IP4LENGTH> ip4_bytes(IPAddress const& ipAddr)
{
    return ::etl::span<uint8_t const, IPAddress::IP4LENGTH>(
        &ipAddr.raw[internal::RAW_IP4_IDX].bytes[0], IPAddress::IP4LENGTH);
}

inline ::etl::span<uint8_t const, IPAddress::IP6LENGTH> ip6_bytes(IPAddress const& ipAddr)
{
    return ::etl::span<uint8_t const, IPAddress::IP6LENGTH>(
        &ipAddr.raw[0].bytes[0], IPAddress::IP6LENGTH);
}

inline ::etl::span<uint8_t const> packed(IPAddress const& ipAddr)
{
    if (addressFamilyOf(ipAddr) == IPAddress::IPV4)
    {
        return ip4_bytes(ipAddr);
    }
    return ip6_bytes(ipAddr);
}

inline uint32_t ip4_to_u32(IPAddress const& ipAddr) { return ipAddr.raw[internal::RAW_IP4_IDX]; }

#ifndef OPENBSW_NO_IPV6
inline uint32_t ip6_to_u32(IPAddress const& ipAddr, size_t const offset)
{
    estd_assert(offset <= 3U);
    return ipAddr.raw[offset];
}
#endif

inline bool isUnspecified(IPAddress const& ipAddr)
{
    if (addressFamilyOf(ipAddr) == IPAddress::IPV4)
    {
        return (ipAddr.raw[internal::RAW_IP4_IDX] == 0U);
    }

    for (auto const i : ipAddr.raw)
    {
        if (0U != i)
        {
            return false;
        }
    }
    return true;
}

inline bool isIp4Address(IPAddress const& ipAddr)
{
    return (IPAddress::IPV4 == addressFamilyOf(ipAddr));
}

inline bool isIp6Address(IPAddress const& ipAddr)
{
    return (IPAddress::IPV6 == addressFamilyOf(ipAddr));
}

inline bool isMulticastAddress(IPAddress const& ipAddr)
{
    IPAddress::Family const family = addressFamilyOf(ipAddr);

    if (IPAddress::IPV6 == family)
    {
        uint8_t const IP6_MULTICAST_PREFIX = 0xFFU;
        return (ipAddr.raw[0U].bytes[0U] == IP6_MULTICAST_PREFIX);
    }

    // IPAddress::IPv4
    uint32_t const IP4_MULTICAST_MASK   = 0xF0000000U;
    uint32_t const IP4_MULTICAST_PREFIX = 0xE0000000U;
    uint32_t const addressPrefix        = (ipAddr.raw[internal::RAW_IP4_IDX] & IP4_MULTICAST_MASK);
    return (addressPrefix == IP4_MULTICAST_PREFIX);
}

inline bool isLinkLocalAddress(IPAddress const& ipAddr)
{
    IPAddress::Family const family = addressFamilyOf(ipAddr);

    if (IPAddress::IPV6 == family)
    {
        uint32_t const IP6_LINK_LOCAL_MASK   = 0xFFC00000U;
        uint32_t const IP6_LINK_LOCAL_PREFIX = 0xFE800000U;
        uint32_t const addressPrefix         = (ipAddr.raw[0U] & IP6_LINK_LOCAL_MASK);
        return (addressPrefix == IP6_LINK_LOCAL_PREFIX);
    }

    // IPAddress::IPv4
    uint32_t const IP4_LINK_LOCAL_MASK   = 0xFFFF0000U;
    uint32_t const IP4_LINK_LOCAL_PREFIX = 0xA9FE0000U;
    uint32_t const addressPrefix = (ipAddr.raw[internal::RAW_IP4_IDX] & IP4_LINK_LOCAL_MASK);
    return (addressPrefix == IP4_LINK_LOCAL_PREFIX);
}

inline bool isLoopbackAddress(IPAddress const& ipAddr)
{
    IPAddress::Family const family = addressFamilyOf(ipAddr);

    if (IPAddress::IPV6 == family)
    {
        return (ipAddr.raw[0U] == 0U) && (ipAddr.raw[1U] == 0U) && (ipAddr.raw[2U] == 0U)
               && (ipAddr.raw[3U] == 1U);
    }

    // IPAddress::IPv4
    uint32_t const IP4_LOOPBACK_PREFIX = 0x7F000000U;
    uint32_t const IP4_LOOPBACK_MASK   = 0xFFFFFF00U;
    uint32_t const addressPrefix       = (ipAddr.raw[internal::RAW_IP4_IDX] & IP4_LOOPBACK_MASK);
    bool const hasLastByteSet          = (ipAddr.raw[internal::RAW_IP4_IDX].bytes[3U] != 0U);
    return ((addressPrefix == IP4_LOOPBACK_PREFIX) && hasLastByteSet);
}

inline bool
isNetworkLocal(IPAddress const& ipAddr1, IPAddress const& ipAddr2, uint8_t const networkId)
{
    IPAddress::Family const family1 = addressFamilyOf(ipAddr1);
    IPAddress::Family const family2 = addressFamilyOf(ipAddr2);

    if ((family1 != family2) || isUnspecified(ipAddr1) || isUnspecified(ipAddr2))
    {
        return false; // invalid IPs or IPv4/6 mix
    }

    if (networkId == 0U)
    {
        return true; // don't care
    }

    size_t const ip4PrefixLengthInBits = internal::RAW_IP4_IDX * sizeof(uint32_t) * 8U;
    size_t const netMaskBits
        = (IPAddress::IPV6 == family1) ? networkId : (networkId + ip4PrefixLengthInBits);

    if (netMaskBits > (IPAddress::MAX_IP_LENGTH * 8U))
    {
        return false; // invalid networkId
    }

    size_t const netMaskFullBytes = netMaskBits / 8U;
    if (0 != memcmp(&ipAddr1.raw[0U], &ipAddr2.raw[0U], netMaskFullBytes))
    {
        return false;
    }

    uint8_t const mask = ~(0xFFU >> (netMaskBits % 8U));
    if (mask == 0U)
    {
        return true;
    }

    return (
        (ipAddr1.raw[netMaskFullBytes / 4].bytes[netMaskFullBytes % 4] & mask)
        == (ipAddr2.raw[netMaskFullBytes / 4].bytes[netMaskFullBytes % 4] & mask));
}

inline IPAddress::Family addressFamilyOf(IPAddress const& ipAddr)
{
#ifndef OPENBSW_NO_IPV6
    uint32_t const IP4_PREFIX[] = {0U, 0U, 0xFFFFU};

    bool const isIp4MappedIp6 = (IP4_PREFIX[0U] == ipAddr.raw[0U])
                                && (IP4_PREFIX[1U] == ipAddr.raw[1U])
                                && (IP4_PREFIX[2U] == ipAddr.raw[2U]);

    return isIp4MappedIp6 ? IPAddress::IPV4 : IPAddress::IPV6;
#else
    return IPAddress::IPV4;
#endif
}

inline bool operator==(IPAddress const& ip1, IPAddress const& ip2)
{
#ifndef OPENBSW_NO_IPV6
    return (
        (ip1.raw[3] == ip2.raw[3]) && (ip1.raw[2] == ip2.raw[2]) && (ip1.raw[1] == ip2.raw[1])
        && (ip1.raw[0] == ip2.raw[0]));
#else
    return (ip1.raw[0] == ip2.raw[0]);
#endif
}

inline bool operator!=(IPAddress const& ip1, IPAddress const& ip2) { return !(ip1 == ip2); }

inline bool
IPAddressCompareLess::operator()(IPAddress const& ipAddr1, IPAddress const& ipAddr2) const
{
    IPAddress::Family const family1 = addressFamilyOf(ipAddr1);
    IPAddress::Family const family2 = addressFamilyOf(ipAddr2);

    if (family1 != family2)
    {
        return (static_cast<uint8_t>(family1) < static_cast<uint8_t>(family2));
    }

    for (uint8_t i = 0U; i < (IPAddress::MAX_IP_LENGTH / sizeof(uint32_t)); ++i)
    {
        if (ipAddr1.raw[i] != ipAddr2.raw[i])
        {
            return ipAddr1.raw[i] < ipAddr2.raw[i];
        }
    }

    return false;
}

} // namespace ip
