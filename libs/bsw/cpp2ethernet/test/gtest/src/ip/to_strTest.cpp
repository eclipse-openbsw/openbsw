// Copyright 2025 Accenture.

#include "ip/to_str.h"

#include <gmock/gmock.h>

using namespace ::testing;
using namespace ::ip;

TEST(StringConversion, to_str_ip4)
{
    IPAddress ip = make_ip4(241, 242, 243, 244);

    char strBuffer[IP4_MAX_STRING_LENGTH];
    EXPECT_THAT(to_str(ip, strBuffer).data(), StrEq("241.242.243.244"));

    char smallBuffer[1];
    EXPECT_EQ(0U, to_str(ip, smallBuffer).size());
}

#ifndef OPENBSW_NO_IPV6
TEST(StringConversion, to_str_ip6)
{
    // clang-format off
    uint8_t addr[] = {
            0x11, 0x12, 0x21, 0x22,
            0x31, 0x32, 0x41, 0x42,
            0x51, 0x52, 0x61, 0x62,
            0x71, 0x72, 0x81, 0x82
    };
    // clang-format on
    IPAddress ip = make_ip6(addr);

    char strBuffer[IP6_MAX_STRING_LENGTH];
    EXPECT_THAT(to_str(ip, strBuffer).data(), StrEq("1112:2122:3132:4142:5152:6162:7172:8182"));

    char smallBuffer[1];
    EXPECT_EQ(0U, to_str(ip, smallBuffer).size());
}
#endif

TEST(StringConversion, to_str_ip4_endpoint)
{
    uint8_t ipBuffer[] = {0xF1, 0xF2, 0xF3, 0xF4};
    IPAddress ip       = make_ip4(ipBuffer);

    uint16_t port = 0xFFFF;
    IPEndpoint endpoint(ip, port);

    char strBuffer[IP4_ENDPOINT_MAX_STRING_LENGTH];
    EXPECT_THAT(to_str(endpoint, strBuffer).data(), StrEq("241.242.243.244:65535"));

    char smallBuffer[1];
    EXPECT_EQ(0U, to_str(endpoint, smallBuffer).size());
}

#ifndef OPENBSW_NO_IPV6
TEST(StringConversion, to_str_ip6_endpoint)
{
    // clang-format off
    uint8_t ipBuffer[] = {
            0x11, 0x12, 0x21, 0x22,
            0x31, 0x32, 0x41, 0x42,
            0x51, 0x52, 0x61, 0x62,
            0x71, 0x72, 0x81, 0x82
    };
    // clang-format on
    IPAddress ip = make_ip6(ipBuffer);

    uint16_t port = 0xFFFF;
    IPEndpoint endpoint(ip, port);

    char strBuffer[MAX_ENDPOINT_STRING_LENGTH];
    EXPECT_THAT(
        to_str(endpoint, strBuffer).data(),
        StrEq("[1112:2122:3132:4142:5152:6162:7172:8182]:65535"));

    char smallBuffer[1];
    EXPECT_EQ(0U, to_str(endpoint, smallBuffer).size());
}
#endif
