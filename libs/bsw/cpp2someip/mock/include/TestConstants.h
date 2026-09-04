/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <ip/IPAddress.h>

#include <cstdint>

namespace test
{
static ::ip::IPAddress const IPV4_MULTICAST_IP = ::ip::make_ip4(239, 192, 255, 251);

static ::ip::IPAddress const IPV4_LOCAL_IP_1 = ::ip::make_ip4(127, 0, 0, 1);
static ::ip::IPAddress const IPV4_LOCAL_IP_2 = ::ip::make_ip4(127, 0, 0, 2);
static ::ip::IPAddress const IPV4_LOCAL_IP_3 = ::ip::make_ip4(127, 0, 0, 3);

static uint8_t const IPV4_SUBNET_ID = 24U; // 255.255.255.0

#ifdef PLATFORM_SUPPORT_IPV6
static uint8_t const IPV6_MULTICAST_IP_ADDR[::ip::IPAddress::IP6LENGTH]
    = { // FF0E:0000:0000:0000:0000:FFFF:EFC0:FFFB
        0xFF,
        0x0E,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xFF,
        0xFF,
        0xEF,
        0xC0,
        0xFF,
        0xFB};
static ::ip::IPAddress const IPV6_MULTICAST_IP = ::ip::make_ip6(IPV6_MULTICAST_IP_ADDR);

static uint8_t const IPV6_LOCAL_IP_1_ADDR[::ip::IPAddress::IP6LENGTH] = { // IPv6(127.0.0.1)
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xFF,
    0xFF,
    0x7F,
    0x00,
    0x00,
    0x01};
static ::ip::IPAddress const IPV6_LOCAL_IP_1 = ::ip::make_ip6(IPV6_LOCAL_IP_1_ADDR);

static uint8_t const IPV6_LOCAL_IP_2_ADDR[::ip::IPAddress::IP6LENGTH] = { // IPv6(127.0.0.2)
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xFF,
    0xFF,
    0x7F,
    0x00,
    0x00,
    0x02};
static ::ip::IPAddress const IPV6_LOCAL_IP_2 = ::ip::make_ip6(IPV6_LOCAL_IP_2_ADDR);

static uint8_t const IPV6_LOCAL_IP_3_ADDR[::ip::IPAddress::IP6LENGTH] = { // IPv6(127.0.0.3)
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xFF,
    0xFF,
    0x7F,
    0x00,
    0x00,
    0x03};
static ::ip::IPAddress const IPV6_LOCAL_IP_3 = ::ip::make_ip6(IPV6_LOCAL_IP_3_ADDR);

static uint8_t const IPV6_SUBNET_ID = 96U; // IPv6(255.255.255.0)

#endif // OPENBSW_NO_IPV6

static uint16_t const PORT_0 = 30000;
static uint16_t const PORT_1 = 30001;
static uint16_t const PORT_2 = 30002;
static uint16_t const PORT_3 = 30003;
static uint16_t const PORT_4 = 30004;
static uint16_t const PORT_5 = 30005;

static uint16_t const BUFFER_SIZE = 1500;

} // namespace test
