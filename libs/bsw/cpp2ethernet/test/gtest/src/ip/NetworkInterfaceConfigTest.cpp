// Copyright 2025 Accenture.

#include "ip/NetworkInterfaceConfig.h"

#include <gtest/gtest.h>

using namespace ::testing;

using namespace ip;

namespace
{
#ifndef OPENBSW_NO_IPV6
static uint32_t nonConstIp6Address[]   = {2348U, 234U, 232U, 345743U};
static uint32_t const ip6Address1[]    = {0x123U, 0x1232U, 0x23839U, 0x23492U};
static uint32_t const ip6Address2[]    = {0x12433U, 0x133232U, 0x2383439U, 0x2492342U};
static uint32_t const ip6AddressNull[] = {0x0, 0x0, 0x0, 0x0};
#endif

TEST(NetworkInterfaceConfigTest, ConstructAndAssign){{// empty config
                                                      NetworkInterfaceConfig cut;
EXPECT_FALSE(cut.isValid());
EXPECT_EQ(IPAddress::FAMILY_UNKNOWN, cut.ipFamily());
EXPECT_EQ(IPAddress(), cut.ipAddress());
EXPECT_EQ(IPAddress(), cut.networkMask());
EXPECT_EQ(IPAddress(), cut.defaultGateway());
EXPECT_EQ(IPAddress(), cut.broadcastAddress());
// copy
NetworkInterfaceConfig copy(cut);
EXPECT_FALSE(copy.isValid());
EXPECT_EQ(IPAddress::FAMILY_UNKNOWN, copy.ipFamily());
EXPECT_EQ(IPAddress(), copy.ipAddress());
EXPECT_EQ(IPAddress(), copy.networkMask());
EXPECT_EQ(IPAddress(), copy.defaultGateway());
EXPECT_EQ(IPAddress(), copy.broadcastAddress());
// assign
NetworkInterfaceConfig assign;
assign = cut;
EXPECT_FALSE(assign.isValid());
EXPECT_EQ(IPAddress::FAMILY_UNKNOWN, assign.ipFamily());
EXPECT_EQ(IPAddress(), assign.ipAddress());
EXPECT_EQ(IPAddress(), assign.networkMask());
EXPECT_EQ(IPAddress(), assign.defaultGateway());
EXPECT_EQ(IPAddress(), assign.broadcastAddress());
// assign twice
cut = *&cut;
} // namespace

{
    // IPv4 config
    NetworkInterfaceConfig cut(0xc0a8b656U, 0xfff00000, 0x33229U);
    EXPECT_TRUE(cut.isValid());
    EXPECT_EQ(IPAddress::IPV4, cut.ipFamily());
    EXPECT_EQ(make_ip4(0xc0a8b656U), cut.ipAddress());
    EXPECT_EQ(make_ip4(0xfff00000), cut.networkMask());
    EXPECT_EQ(make_ip4(0x33229U), cut.defaultGateway());
    EXPECT_EQ(make_ip4(0xc0afffffU), cut.broadcastAddress());
    // copy
    NetworkInterfaceConfig copy(cut);
    EXPECT_TRUE(copy.isValid());
    EXPECT_EQ(IPAddress::IPV4, copy.ipFamily());
    EXPECT_EQ(make_ip4(0xc0a8b656U), copy.ipAddress());
    EXPECT_EQ(make_ip4(0xfff00000), copy.networkMask());
    EXPECT_EQ(make_ip4(0x33229U), copy.defaultGateway());
    EXPECT_EQ(make_ip4(0xc0afffffU), copy.broadcastAddress());
    // assign
    NetworkInterfaceConfig assign;
    assign = cut;
    EXPECT_TRUE(assign.isValid());
    EXPECT_EQ(IPAddress::IPV4, assign.ipFamily());
    EXPECT_EQ(make_ip4(0xc0a8b656U), assign.ipAddress());
    EXPECT_EQ(make_ip4(0xfff00000), assign.networkMask());
    EXPECT_EQ(make_ip4(0x33229U), assign.defaultGateway());
    EXPECT_EQ(make_ip4(0xc0afffffU), assign.broadcastAddress());
}
#ifndef OPENBSW_NO_IPV6
{
    // IPv6 config
    NetworkInterfaceConfig cut(ip6Address1);
    EXPECT_TRUE(cut.isValid());
    EXPECT_EQ(IPAddress::IPV6, cut.ipFamily());
    EXPECT_EQ(make_ip6(ip6Address1), cut.ipAddress());
    EXPECT_EQ(IPAddress(), cut.networkMask());
    EXPECT_EQ(IPAddress(), cut.defaultGateway());
    EXPECT_EQ(IPAddress(), cut.broadcastAddress());
    // copy
    NetworkInterfaceConfig copy(cut);
    EXPECT_TRUE(copy.isValid());
    EXPECT_EQ(IPAddress::IPV6, copy.ipFamily());
    EXPECT_EQ(make_ip6(ip6Address1), copy.ipAddress());
    EXPECT_EQ(IPAddress(), copy.networkMask());
    EXPECT_EQ(IPAddress(), copy.defaultGateway());
    EXPECT_EQ(IPAddress(), copy.broadcastAddress());
    // assign
    NetworkInterfaceConfig assign;
    assign = cut;
    EXPECT_TRUE(assign.isValid());
    EXPECT_EQ(IPAddress::IPV6, assign.ipFamily());
    EXPECT_EQ(make_ip6(ip6Address1), assign.ipAddress());
    EXPECT_EQ(IPAddress(), assign.networkMask());
    EXPECT_EQ(IPAddress(), assign.defaultGateway());
    EXPECT_EQ(IPAddress(), assign.broadcastAddress());
}
{
    // IPv6 config
    NetworkInterfaceConfig cut(nonConstIp6Address);
    EXPECT_TRUE(cut.isValid());
    EXPECT_EQ(IPAddress::IPV6, cut.ipFamily());
    EXPECT_EQ(make_ip6(nonConstIp6Address), cut.ipAddress());
    EXPECT_EQ(IPAddress(), cut.networkMask());
    EXPECT_EQ(IPAddress(), cut.defaultGateway());
    EXPECT_EQ(IPAddress(), cut.broadcastAddress());
}
#endif
}

TEST(NetworkInterfaceConfigTest, Compare)
{
    {
        // operator==
        EXPECT_TRUE(NetworkInterfaceConfig() == NetworkInterfaceConfig());
        EXPECT_FALSE(NetworkInterfaceConfig() == NetworkInterfaceConfig(0U, 0U, 0U));
#ifndef OPENBSW_NO_IPV6
        EXPECT_FALSE(NetworkInterfaceConfig() == NetworkInterfaceConfig(ip6AddressNull));
#endif
        EXPECT_TRUE(
            NetworkInterfaceConfig(0x2347U, 0x23748U, 0x454U)
            == NetworkInterfaceConfig(0x2347U, 0x23748U, 0x454U));
        EXPECT_FALSE(
            NetworkInterfaceConfig(0x2346U, 0x23748U, 0x454U)
            == NetworkInterfaceConfig(0x2347U, 0x23748U, 0x454U));
        EXPECT_FALSE(
            NetworkInterfaceConfig(0x2347U, 0x2448U, 0x454U)
            == NetworkInterfaceConfig(0x2347U, 0x23748U, 0x454U));
        EXPECT_FALSE(
            NetworkInterfaceConfig(0x2347U, 0x23748U, 0x4434U)
            == NetworkInterfaceConfig(0x2347U, 0x23748U, 0x454U));
#ifndef OPENBSW_NO_IPV6
        EXPECT_TRUE(
            NetworkInterfaceConfig(ip6AddressNull) == NetworkInterfaceConfig(ip6AddressNull));
        EXPECT_FALSE(NetworkInterfaceConfig(ip6AddressNull) == NetworkInterfaceConfig(ip6Address1));
        EXPECT_TRUE(NetworkInterfaceConfig(ip6Address1) == NetworkInterfaceConfig(ip6Address1));
        EXPECT_FALSE(NetworkInterfaceConfig(ip6Address1) == NetworkInterfaceConfig(ip6Address2));
#endif
    }
    {
        // operator!=
        EXPECT_FALSE(NetworkInterfaceConfig() != NetworkInterfaceConfig());
        EXPECT_TRUE(NetworkInterfaceConfig() != NetworkInterfaceConfig(0U, 0U, 0U));
#ifndef OPENBSW_NO_IPV6
        EXPECT_TRUE(NetworkInterfaceConfig() != NetworkInterfaceConfig(ip6AddressNull));
#endif
        EXPECT_FALSE(
            NetworkInterfaceConfig(0x2347U, 0x23748U, 0x454U)
            != NetworkInterfaceConfig(0x2347U, 0x23748U, 0x454U));
        EXPECT_TRUE(
            NetworkInterfaceConfig(0x2346U, 0x23748U, 0x454U)
            != NetworkInterfaceConfig(0x2347U, 0x23748U, 0x454U));
        EXPECT_TRUE(
            NetworkInterfaceConfig(0x2347U, 0x2448U, 0x454U)
            != NetworkInterfaceConfig(0x2347U, 0x23748U, 0x454U));
        EXPECT_TRUE(
            NetworkInterfaceConfig(0x2347U, 0x23748U, 0x4434U)
            != NetworkInterfaceConfig(0x2347U, 0x23748U, 0x454U));
#ifndef OPENBSW_NO_IPV6
        EXPECT_FALSE(
            NetworkInterfaceConfig(ip6AddressNull) != NetworkInterfaceConfig(ip6AddressNull));
        EXPECT_TRUE(NetworkInterfaceConfig(ip6AddressNull) != NetworkInterfaceConfig(ip6Address1));
        EXPECT_FALSE(NetworkInterfaceConfig(ip6Address1) != NetworkInterfaceConfig(ip6Address1));
        EXPECT_TRUE(NetworkInterfaceConfig(ip6Address1) != NetworkInterfaceConfig(ip6Address2));
#endif
    }
}

} // anonymous namespace
