// Copyright 2025 Accenture.

#include "ip/IPAddress.h"

#include <gmock/gmock.h>

using namespace ::testing;
using namespace ::ip;

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, defaults_to_ipv6)
{
    IPAddress ip = {{{0}}};
    EXPECT_EQ(IPAddress::IPV6, addressFamilyOf(ip));
    EXPECT_THAT(ip.raw, Each(Eq(0U)));
}
#endif

TEST(IPAddressTest, zero_ip_address_ip4)
{
    constexpr IPAddress ip = make_ip4(0U);
    EXPECT_EQ(IPAddress::IPV4, addressFamilyOf(ip));
    EXPECT_THAT(ip.raw[ip::internal::RAW_IP4_IDX], Eq(0U));
    EXPECT_TRUE(isUnspecified(ip));
}

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, zero_ip_address_ip6)
{
    constexpr uint32_t addr[] = {0U, 0U, 0U, 0U};
    constexpr IPAddress ip    = make_ip6(addr);

    EXPECT_EQ(IPAddress::IPV6, addressFamilyOf(ip));
    EXPECT_THAT(ip.raw, Each(Eq(0U)));
    EXPECT_TRUE(isUnspecified(ip));
}

TEST(IPAddressTest, non_zero_ip_address_ip6)
{
    constexpr uint32_t addr[] = {0U, 0U, 0U, 1U};
    constexpr IPAddress ip    = make_ip6(addr);

    EXPECT_EQ(IPAddress::IPV6, addressFamilyOf(ip));
    EXPECT_FALSE(isUnspecified(ip));
}
#endif

TEST(IPAddressTest, factory_make_ip4_uint32_t)
{
    constexpr uint32_t addr = 0xABCDEF01;
    constexpr IPAddress ip  = make_ip4(addr);

    EXPECT_EQ(IPAddress::IPV4, addressFamilyOf(ip));
    EXPECT_EQ(addr, ip4_to_u32(ip));
}

TEST(IPAddressTest, factory_make_ip4_bytes)
{
    constexpr IPAddress ip = make_ip4(0xAB, 0xCD, 0xEF, 0x01);

    EXPECT_EQ(IPAddress::IPV4, addressFamilyOf(ip));
    EXPECT_EQ(0xABCDEF01, ip4_to_u32(ip));
}

TEST(IPAddressTest, factory_make_ip4_slice)
{
    uint8_t const bytes[] = {0xAB, 0xCD, 0xEF, 0x01};
    IPAddress ip          = make_ip4(bytes);

    EXPECT_EQ(IPAddress::IPV4, addressFamilyOf(ip));
    EXPECT_EQ(0xABCDEF01, ip4_to_u32(ip));

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    uint8_t const bytes_wrong_length[] = {0xAB, 0xCD, 0xEF, 0x01, 0x02};
    EXPECT_THROW({ make_ip4(bytes_wrong_length); }, ::estd::assert_exception);
}

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, factory_make_ip6_array)
{
    constexpr uint32_t addr[] = {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};
    constexpr IPAddress ip    = make_ip6(addr);

    EXPECT_EQ(IPAddress::IPV6, addressFamilyOf(ip));
    EXPECT_EQ(0x11223344U, ip.raw[0U]);
    EXPECT_EQ(0x55667788U, ip.raw[1U]);
    EXPECT_EQ(0x99AABBCCU, ip.raw[2U]);
    EXPECT_EQ(0xDDEEFF00U, ip.raw[3U]);
}
#endif

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, factory_make_ip6_integers)
{
    constexpr IPAddress ip = make_ip6(0x11223344U, 0x55667788U, 0x99AABBCCU, 0xDDEEFF00U);

    EXPECT_EQ(IPAddress::IPV6, addressFamilyOf(ip));
    EXPECT_EQ(0x11223344U, ip.raw[0U]);
    EXPECT_EQ(0x55667788U, ip.raw[1U]);
    EXPECT_EQ(0x99AABBCCU, ip.raw[2U]);
    EXPECT_EQ(0xDDEEFF00U, ip.raw[3U]);
}
#endif

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, factory_make_ip6_slice)
{
    // clang-format off
    const uint8_t addr[] = {
        0x00, 0x01, 0x02, 0x03,
        0x10, 0x11, 0x12, 0x13,
        0x20, 0x21, 0x22, 0x23,
        0x30, 0x31, 0x32, 0x33,
    };
    // clang-format on
    IPAddress ip = make_ip6(addr);

    EXPECT_EQ(IPAddress::IPV6, addressFamilyOf(ip));
    EXPECT_EQ(0x00010203U, ip.raw[0U]);
    EXPECT_EQ(0x10111213U, ip.raw[1U]);
    EXPECT_EQ(0x20212223U, ip.raw[2U]);
    EXPECT_EQ(0x30313233U, ip.raw[3U]);

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    // clang-format off
    const uint8_t addr_wrong_length[] = {
        0x00, 0x01, 0x02, 0x03,
        0x10, 0x11, 0x12, 0x13,
        0x20, 0x21, 0x22, 0x23,
        0x30, 0x31, 0x32, 0x33,
        0x40, 0x41, 0x42, 0x43,
    };
    // clang-format on
    EXPECT_THROW({ make_ip6(addr_wrong_length); }, ::estd::assert_exception);
}
#endif

TEST(IPAddressTest, ip4_as_bytes)
{
    constexpr uint32_t addr = 0xABCDEF01;
    constexpr IPAddress ip  = make_ip4(addr);

    auto ip_bytes = ip4_bytes(ip);
    EXPECT_EQ(4U, ip_bytes.size());
    EXPECT_EQ(0xAB, ip_bytes[0]);
    EXPECT_EQ(0xCD, ip_bytes[1]);
    EXPECT_EQ(0xEF, ip_bytes[2]);
    EXPECT_EQ(0x01, ip_bytes[3]);
}

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, ip6_as_bytes)
{
    uint32_t const addr[] = {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};

    IPAddress ip = make_ip6(addr);

    auto ip_bytes = ip6_bytes(ip);
    EXPECT_EQ(16U, ip_bytes.size());
    EXPECT_EQ(0x11, ip_bytes[0]);
    EXPECT_EQ(0x22, ip_bytes[1]);
    EXPECT_EQ(0x33, ip_bytes[2]);
    EXPECT_EQ(0x44, ip_bytes[3]);
    EXPECT_EQ(0x55, ip_bytes[4]);
    EXPECT_EQ(0x66, ip_bytes[5]);
    EXPECT_EQ(0x77, ip_bytes[6]);
    EXPECT_EQ(0x88, ip_bytes[7]);
    EXPECT_EQ(0x99, ip_bytes[8]);
    EXPECT_EQ(0xAA, ip_bytes[9]);
    EXPECT_EQ(0xBB, ip_bytes[10]);
    EXPECT_EQ(0xCC, ip_bytes[11]);
    EXPECT_EQ(0xDD, ip_bytes[12]);
    EXPECT_EQ(0xEE, ip_bytes[13]);
    EXPECT_EQ(0xFF, ip_bytes[14]);
    EXPECT_EQ(0x00, ip_bytes[15]);
}
#endif

TEST(IPAddressTest, ip4_packed)
{
    constexpr uint32_t addr = 0xABCDEF01;
    constexpr IPAddress ip  = make_ip4(addr);

    auto ip_bytes = packed(ip);
    EXPECT_EQ(4U, ip_bytes.size());
    EXPECT_EQ(0xAB, ip_bytes[0]);
    EXPECT_EQ(0xCD, ip_bytes[1]);
    EXPECT_EQ(0xEF, ip_bytes[2]);
    EXPECT_EQ(0x01, ip_bytes[3]);
}

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, ip6_packed)
{
    uint32_t const addr[] = {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};

    IPAddress ip = make_ip6(addr);

    auto ip_bytes = packed(ip);
    EXPECT_EQ(16U, ip_bytes.size());
    EXPECT_EQ(0x11, ip_bytes[0]);
    EXPECT_EQ(0x22, ip_bytes[1]);
    EXPECT_EQ(0x33, ip_bytes[2]);
    EXPECT_EQ(0x44, ip_bytes[3]);
    EXPECT_EQ(0x55, ip_bytes[4]);
    EXPECT_EQ(0x66, ip_bytes[5]);
    EXPECT_EQ(0x77, ip_bytes[6]);
    EXPECT_EQ(0x88, ip_bytes[7]);
    EXPECT_EQ(0x99, ip_bytes[8]);
    EXPECT_EQ(0xAA, ip_bytes[9]);
    EXPECT_EQ(0xBB, ip_bytes[10]);
    EXPECT_EQ(0xCC, ip_bytes[11]);
    EXPECT_EQ(0xDD, ip_bytes[12]);
    EXPECT_EQ(0xEE, ip_bytes[13]);
    EXPECT_EQ(0xFF, ip_bytes[14]);
    EXPECT_EQ(0x00, ip_bytes[15]);
}
#endif

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, ip_address_type)
{
    constexpr uint32_t addr4   = 0xABCDEF01;
    constexpr uint32_t addr6[] = {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};
    constexpr IPAddress ip4    = make_ip4(addr4);
    constexpr IPAddress ip6    = make_ip6(addr6);

    EXPECT_TRUE(isIp4Address(ip4));
    EXPECT_TRUE(isIp6Address(ip6));
    EXPECT_FALSE(isIp6Address(ip4));
    EXPECT_FALSE(isIp4Address(ip6));
}
#endif

TEST(IPAddressTest, ip4_to_u32)
{
    uint32_t const addr = 0xABCDEF01;
    IPAddress ip        = make_ip4(addr);

    EXPECT_EQ(addr, ip4_to_u32(ip));
}

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, ip6_to_u32)
{
    uint32_t const addr[] = {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};

    IPAddress ip = make_ip6(addr);

    EXPECT_EQ(0x11223344U, ip6_to_u32(ip, 0));
    EXPECT_EQ(0x55667788U, ip6_to_u32(ip, 1));
    EXPECT_EQ(0x99AABBCCU, ip6_to_u32(ip, 2));
    EXPECT_EQ(0xDDEEFF00U, ip6_to_u32(ip, 3));

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    EXPECT_THROW({ ip6_to_u32(ip, 4); }, ::estd::assert_exception);
}
#endif

TEST(IPAddressTest, equality)
{
    uint32_t addr = 0xABCDEF01;

    IPAddress ip1 = make_ip4(addr);
    IPAddress ip2 = make_ip4(addr);
    EXPECT_EQ(ip1, ip2);

    IPAddress ip3 = make_ip4(++addr);
    EXPECT_NE(ip1, ip3);
}

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, inequality_ip4_ip6)
{
    uint32_t const ip6_addr[] = {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};
    IPAddress ip6             = make_ip6(ip6_addr);
    EXPECT_EQ(IPAddress::IPV6, addressFamilyOf(ip6));

    uint32_t const ip4_addr = 0x11223344;
    IPAddress ip4           = make_ip4(ip4_addr);
    EXPECT_EQ(IPAddress::IPV4, addressFamilyOf(ip4));

    EXPECT_FALSE(ip6 == ip4);
}
#endif

TEST(IPAddressTest, isMulticastAddress_ip4)
{
    IPAddress ip1 = make_ip4(0xABCDEF01);
    EXPECT_FALSE(isMulticastAddress(ip1));

    IPAddress ip2 = make_ip4(0xEFCDEF01);
    EXPECT_TRUE(isMulticastAddress(ip2));
}

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, isMulticastAddress_ip6)
{
    uint32_t const addr[] = {0x11223344, 0x55667788, 0x99AABBCC, 0xDDEEFF00};

    IPAddress ip = make_ip6(addr);
    EXPECT_FALSE(isMulticastAddress(ip));

    ip.raw[0].bytes[0U] = 0xFF;
    EXPECT_TRUE(isMulticastAddress(ip));
}
#endif

TEST(IPAddressTest, isLinkLocalAddress_ip4)
{
    uint8_t addr[] = {1, 2, 3, 4};
    EXPECT_FALSE(isLinkLocalAddress(make_ip4(addr)));
    addr[0] = 169;
    EXPECT_FALSE(isLinkLocalAddress(make_ip4(addr)));
    addr[1] = 254;
    EXPECT_TRUE(isLinkLocalAddress(make_ip4(addr)));
}

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, isLinkLocalAddress_ip6)
{
    // clang-format off
    uint8_t addr[] = {
        0x00, 0x01, 0x02, 0x03,
        0x10, 0x11, 0x12, 0x13,
        0x20, 0x21, 0x22, 0x23,
        0x30, 0x31, 0x32, 0x33,
    };
    // clang-format on
    EXPECT_FALSE(isLinkLocalAddress(make_ip6(addr)));
    addr[0] = 0xFEU;
    EXPECT_FALSE(isLinkLocalAddress(make_ip6(addr)));
    addr[1] = 0xBFU;
    EXPECT_TRUE(isLinkLocalAddress(make_ip6(addr)));
}
#endif

TEST(IPAddressTest, isLoopbackAddress_ip4)
{
    IPAddress emptyIp = {};
    EXPECT_FALSE(isLoopbackAddress(emptyIp));

    IPAddress realIp = make_ip4(172, 31, 2, 78);
    EXPECT_FALSE(isLoopbackAddress(realIp));

    IPAddress ip4 = make_ip4(127, 0, 0, 0);
    EXPECT_FALSE(isLoopbackAddress(ip4));

    for (uint8_t i = 1U; i < 255U; ++i)
    {
        EXPECT_TRUE(isLoopbackAddress(make_ip4(127, 0, 0, i)));
    }
}

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, isLoopbackAddress_ip6)
{
    IPAddress ip6 = {};
    ip6.raw[3U]   = 0x01;
    EXPECT_TRUE(isLoopbackAddress(ip6));

    ip6.raw[3U] = 0x02;
    EXPECT_FALSE(isLoopbackAddress(ip6));
}
#endif

TEST(IPAddressTest, isNetworkLocal_ip4)
{
    IPAddress emptyIp = {};
    IPAddress ip41    = make_ip4(0x01020304);
    IPAddress ip42    = make_ip4(0x01020380);
    IPAddress ip43    = make_ip4(0x01020401);

    uint8_t const maxIP4Prefix = IPAddress::IP4LENGTH * 8U;

    EXPECT_TRUE(isNetworkLocal(ip41, ip41, maxIP4Prefix));
    for (uint8_t prefix = 0; prefix < maxIP4Prefix; ++prefix)
    {
        EXPECT_EQ((prefix < maxIP4Prefix - 7U), isNetworkLocal(ip41, ip42, prefix));
    }

    // invalid ip
    EXPECT_FALSE(isNetworkLocal(emptyIp, ip41, 0));

    // invalid ip (different order)
    EXPECT_FALSE(isNetworkLocal(ip41, emptyIp, 0));

    // don't care
    EXPECT_TRUE(isNetworkLocal(ip41, ip42, 0));

    // invalid prefix
    EXPECT_FALSE(isNetworkLocal(ip41, ip41, maxIP4Prefix + 1U));

    // different network
    EXPECT_FALSE(isNetworkLocal(ip41, ip43, 24));
}

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, isNetworkLocal_ip6)
{
    IPAddress emptyIp = {};

    // clang-format off
    const uint8_t buffer[] = {
        0x01, 0x02, 0x03, 0x04,
        0x11, 0x12, 0x13, 0x14,
        0x21, 0x22, 0x23, 0x24,
        0x31, 0x32, 0x33, 0x34
    };
    // clang-format on
    IPAddress ip61 = make_ip6(buffer);
    IPAddress ip62(ip61);
    ip62.raw[3U].bytes[3U] = 0x80;

    uint8_t const maxIP6Prefix = IPAddress::IP6LENGTH * 8U;

    EXPECT_TRUE(isNetworkLocal(ip61, ip61, maxIP6Prefix));
    for (uint8_t prefix = 0; prefix < maxIP6Prefix; ++prefix)
    {
        EXPECT_EQ((prefix < maxIP6Prefix - 7U), isNetworkLocal(ip61, ip62, prefix));
    }

    // invalid ip
    EXPECT_FALSE(isNetworkLocal(emptyIp, ip61, 0));

    // don't care
    EXPECT_TRUE(isNetworkLocal(ip61, ip62, 0));

    // invalid prefix
    EXPECT_FALSE(isNetworkLocal(ip61, ip61, maxIP6Prefix + 1U));
}
#endif

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, isNetworkLocal_mixed)
{
    IPAddress ip41 = make_ip4(0x01020304);
    // clang-format off
    const uint8_t buffer[] = {
        0x01, 0x02, 0x03, 0x04,
        0x11, 0x12, 0x13, 0x14,
        0x21, 0x22, 0x23, 0x24,
        0x31, 0x32, 0x33, 0x34
    };
    // clang-format on
    IPAddress ip61 = make_ip6(buffer);

    EXPECT_FALSE(isNetworkLocal(ip41, ip61, 0));
    EXPECT_FALSE(isNetworkLocal(ip61, ip41, 0));
}
#endif

TEST(IPAddressDefaultCompareTest, compare_ip4)
{
    IPAddressCompareLess const compare;

    IPAddress ip1 = make_ip4(0x11223344);
    EXPECT_FALSE(compare(ip1, ip1));

    IPAddress ip2 = make_ip4(0x44332211);
    EXPECT_TRUE(compare(ip1, ip2));
    EXPECT_FALSE(compare(ip2, ip1));
}

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressDefaultCompareTest, compare_ip6)
{
    IPAddressCompareLess const compare;

    uint32_t const addr1[] = {1, 2, 3, 4};
    uint32_t const addr2[] = {4, 3, 2, 1};

    IPAddress ip1 = make_ip6(addr1);
    EXPECT_FALSE(compare(ip1, ip1));

    IPAddress ip2 = make_ip6(addr2);
    EXPECT_TRUE(compare(ip1, ip2));
    EXPECT_FALSE(compare(ip2, ip1));
}
#endif

#ifndef OPENBSW_NO_IPV6
TEST(IPAddressTest, OrderIPv4IPv6)
{
    IPAddressCompareLess const compare;

    IPAddress ipv4         = make_ip4(0x22222222);
    uint32_t const addr2[] = {0, 0, 0, 1};
    IPAddress ipv6         = make_ip6(addr2);

    EXPECT_TRUE(compare(ipv4, ipv6));
    EXPECT_FALSE(compare(ipv6, ipv4));
}
#endif
