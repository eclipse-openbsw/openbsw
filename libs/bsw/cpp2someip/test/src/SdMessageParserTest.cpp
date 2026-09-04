/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdMessageParser.h"

#include "someip/ISdMessageParser.h"
#include "someip/RebootTracker.h"
#include "someip/ServiceAnnouncerMock.h"
#include "someip/ServiceRegistryMock.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpMessage.h"

#include <ip/IPAddress.h>
#include <udp/DatagramPacket.h>
#include <udp/socket/AbstractDatagramSocketMock.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::someip;
using namespace ::testing;
using namespace ::ip;

static constexpr IPAddress LOCAL_IP = make_ip4(192U, 0U, 2U, 0U);
static uint8_t const SUBNET_ID      = 25U;

struct SdMessageParserTest : Test
{
    StrictMock<ServiceRegistryMock> _pRegistry;
    StrictMock<ServiceAnnouncerMock> _pAnnouncer;
    declare::RebootTracker<5U> _rebootTracker;
    SdMessageParser _pParser{_pRegistry, _pAnnouncer, _rebootTracker, SUBNET_ID, LOCAL_IP, {}};
};

static bool additionalSDCheckMockTrue(IPEndpoint const& endpoint)
{
    return (0x1122 == endpoint.getPort());
}

static bool additionalSDCheckMockFalse(IPEndpoint const& endpoint)
{
    return (0x1122 != endpoint.getPort());
}

TEST_F(SdMessageParserTest, testSubscribeReceived)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x06, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22 // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);

    IPAddress ip = make_ip4(192U, 0U, 2U, 1U);

    EXPECT_CALL(_pRegistry, rebootDetected(ip)).Times(1);
    EXPECT_CALL(
        _pAnnouncer,
        respondToSubscribe(0x1234U, 0x0001U, 0x01U, _, 0x01U, 0x0100FEU, _, _, 0x1122U, 0x11U))
        .Times(1);

    _pParser.handleMessage(message, IPEndpoint(ip, 30490U), false);

    // do not respond if received via multicast
    EXPECT_CALL(_pRegistry, rebootDetected(ip)).Times(1);
    EXPECT_CALL(_pAnnouncer, respondToSubscribe(_, _, _, _, _, _, _, _, _, _)).Times(0);

    _pParser.handleMessage(message, IPEndpoint(ip, 30490U), true);
}

#ifdef PLATFORM_SUPPORT_IPV6
TEST_F(SdMessageParserTest, testSubscribeReceivedIPv6)
{
    // clang-format off
    uint8_t data[] = {
        0x12, 0x23, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x3C, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x06, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x18, // options length
        0x00, 0x15, 0x06, 0x00, // length, type
        0x20, 0x01, 0x0D, 0xB8, // IP
        0x00, 0x00, 0x00, 0x00, // IP
        0x00, 0x00, 0x00, 0x00, // IP
        0x00, 0x00, 0x00, 0x00, // IP
        0x00, 0x11, 0x11, 0x22 // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);

    // clang-format off
    const uint8_t addr[16]
        = {0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // clang-format on
    IPAddress ip = make_ip6(addr);

    EXPECT_CALL(_pRegistry, rebootDetected(ip)).Times(1);
    EXPECT_CALL(
        _pAnnouncer,
        respondToSubscribe(0x1234U, 0x0001U, 0x01U, _, 0x01U, 0x0100FEU, _, _, 0x1122U, 0x11U))
        .Times(1);

    // clang-format off
    const uint8_t localAddr[16]
        = {0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x01,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // clang-format on
    IPAddress localIp = make_ip6(localAddr);

    declare::RebootTracker<5U> rebootTracker;

    SdMessageParser parser(_pRegistry, _pAnnouncer, rebootTracker, SUBNET_ID, localIp, {});

    parser.handleMessage(message, IPEndpoint(ip, 30490U), false);
}
#endif // OPENBSW_NO_IPV6

TEST_F(SdMessageParserTest, testSubscribeWithSecondUdpEndpointReferenced)
{
    // two options, but only one is referenced -> subscribe should be accepted
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x3C, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x06, 0x00, 0x01, 0x01, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x18, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22, // proto UDP, Port
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x33, 0x44 // proto UDP, Port
    };
    // clang-format on
    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);
    EXPECT_CALL(
        _pAnnouncer,
        respondToSubscribe(0x1234U, 0x0001U, 0x01U, _, 0x01U, 0x0100FEU, _, _, 0x3344U, 0x11U))
        .Times(1);

    _pParser.handleMessage(message, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U), false);
}

TEST_F(SdMessageParserTest, testSubscribeWithTwoUdpEndpointsReferenced)
{
    // two options referenced -> subscribe should be discarded
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x3C, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x06, 0x00, 0x01, 0x11, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x18, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22, // proto UDP, Port
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x33, 0x44 // proto UDP, Port
    };
    // clang-format on
    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);
    EXPECT_CALL(_pAnnouncer, respondToSubscribe(_, _, _, _, _, _, _, _, _, _)).Times(0);
    EXPECT_CALL(
        _pAnnouncer,
        sendSubscribeNack(0x1234U, 0x0001U, 0x01U, 0x01U, _, make_ip4(192U, 0U, 2U, 1U)))
        .Times(1);

    _pParser.handleMessage(message, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U), false);
}

TEST_F(SdMessageParserTest, testSubscribeWithIpAddressNotInLocalNetwork)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x06, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x80, // IP
        0x00, 0x11, 0x11, 0x22 // proto UDP, Port
    };
    // clang-format on
    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);
    // not called
    EXPECT_CALL(_pAnnouncer, respondToSubscribe(_, _, _, _, _, _, _, _, _, _)).Times(0);
    EXPECT_CALL(
        _pAnnouncer,
        sendSubscribeNack(0x1234U, 0x0001U, 0x01U, 0x01U, _, make_ip4(192U, 0U, 2U, 1U)))
        .Times(1);

    _pParser.handleMessage(message, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U), false);
}

TEST_F(SdMessageParserTest, testSubscribeWithWrongInterfaceVersion)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x09, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x06, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22 // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);

    // not called
    EXPECT_CALL(_pAnnouncer, respondToSubscribe(_, _, _, _, _, _, _, _, _, _)).Times(0);

    _pParser.handleMessage(message, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U), false);
}

TEST_F(SdMessageParserTest, testSubscribeWithWrongPort)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x06, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x00, 0x00 // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);
    // not called
    EXPECT_CALL(_pAnnouncer, respondToSubscribe(_, _, _, _, _, _, _, _, _, _)).Times(0);
    EXPECT_CALL(
        _pAnnouncer,
        sendSubscribeNack(0x1234U, 0x0001U, 0x01U, 0x01U, _, make_ip4(192U, 0U, 2U, 1U)))
        .Times(1);

    _pParser.handleMessage(message, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U), false);
}

TEST_F(SdMessageParserTest, testSubscribeWithWrongClientId)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x01, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x06, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22 // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);

    // not called
    EXPECT_CALL(_pAnnouncer, respondToSubscribe(_, _, _, _, _, _, _, _, _, _)).Times(0);

    _pParser.handleMessage(message, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U), false);
}

TEST_F(SdMessageParserTest, testFindReceived)
{
    _pParser.init();
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x00, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x01, 0x02, 0x03, 0x04, // minor version
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22, // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);
    EXPECT_CALL(
        _pAnnouncer, respondToFindService(0x1234U, 0x0001, 0x01, 0x01020304, 0x0100FE, _, 0x00))
        .Times(1);

    _pParser.handleMessage(message, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U), false);
}

TEST_F(SdMessageParserTest, testOfferReceived)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x01, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x01, 0x02, 0x03, 0x04, // minor version
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22, // proto UDP, Port
    };
    // clang-format on

    ServiceDescription receivedService = make<ServiceDescription>();
    auto const sdEndpoint              = IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U);
    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);
    EXPECT_CALL(_pRegistry, offerReceived(_, sdEndpoint.getAddress()))
        .Times(1)
        .WillOnce(SaveArg<0>(&receivedService));

    _pParser.handleMessage(message, sdEndpoint, false);
    EXPECT_EQ(0x1234U, receivedService.serviceId);
    EXPECT_EQ(1U, receivedService.instanceId);
    EXPECT_EQ(1U, receivedService.majorVersion);
    EXPECT_EQ(0x0100FEU, receivedService.ttl);
    EXPECT_EQ(0x01020304U, receivedService.minorVersion);
    EXPECT_EQ(0x1122U, receivedService.port);
    EXPECT_EQ(0x11U, receivedService.proto);
}

TEST_F(SdMessageParserTest, testOffer_InvalidEndpoint)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x01, 0x01, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x01, 0x02, 0x03, 0x04, // minor version
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22, // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);

    _pParser.handleMessage(message, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U), false);
}

TEST_F(SdMessageParserTest, testOffer_InvalidMulticast)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x01, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x01, 0x02, 0x03, 0x04, // minor version
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xE0, 0x01, 0xFF, 0xFF, // IP
        0x00, 0x11, 0x11, 0x22, // proto UDP, Port
    };
    // clang-format on
    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);

    _pParser.handleMessage(message, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U), false);
}

TEST_F(SdMessageParserTest, testReboot)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x03, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x01, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x01, 0x02, 0x03, 0x04, // minor version
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22, // proto UDP, Port
    };
    // clang-format on

    ServiceDescription receivedService = make<ServiceDescription>();
    auto const sdEndpoint              = IPEndpoint(make_ip4(192U, 0U, 2U, 3U), 30490U);
    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(_pRegistry, rebootDetected(_)).Times(0);
    EXPECT_CALL(_pRegistry, offerReceived(_, sdEndpoint.getAddress()))
        .Times(1)
        .WillOnce(SaveArg<0>(&receivedService));

    _pParser.handleMessage(message, sdEndpoint, false);

    EXPECT_EQ(0x1234U, receivedService.serviceId);
    EXPECT_EQ(1U, receivedService.instanceId);
    EXPECT_EQ(1U, receivedService.majorVersion);
    EXPECT_EQ(0x0100FEU, receivedService.ttl);
    EXPECT_EQ(0x01020304U, receivedService.minorVersion);
    EXPECT_EQ(0x1122U, receivedService.port);
    EXPECT_EQ(0x11U, receivedService.proto);

    --data[11]; // decrement session id
    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 3U))).Times(1);
    EXPECT_CALL(_pRegistry, offerReceived(_, sdEndpoint.getAddress()))
        .Times(1)
        .WillOnce(SaveArg<0>(&receivedService));

    _pParser.handleMessage(message, sdEndpoint, false);

    EXPECT_EQ(0x1234U, receivedService.serviceId);
    EXPECT_EQ(1U, receivedService.instanceId);
    EXPECT_EQ(1U, receivedService.majorVersion);
    EXPECT_EQ(0x0100FEU, receivedService.ttl);
    EXPECT_EQ(0x01020304U, receivedService.minorVersion);
    EXPECT_EQ(0x1122U, receivedService.port);
    EXPECT_EQ(0x11U, receivedService.proto);
}

TEST_F(SdMessageParserTest, handleEntryWithIrrelevantService)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x01, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x01, 0x02, 0x03, 0x04, // minor version
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22, // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);
    EXPECT_CALL(_pRegistry, offerReceived(_, _)).Times(0);

    _pParser.handleMessage(message, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U), false);
}

TEST_F(SdMessageParserTest, handleEntryFindEventgroup)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x04, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x01, 0x02, 0x03, 0x04, // minor version
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22, // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);

    _pParser.handleMessage(message, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U), false);
}

TEST_F(SdMessageParserTest, testSubscribeAckReceived)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x07, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x14, 0x00, // length, type
        0xE0, 0x01, 0xFF, 0xFF, // IP
        0x00, 0x11, 0x11, 0x22 // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);
    auto const sourceAddress = make_ip4(192U, 0U, 2U, 1U);

    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);
    EXPECT_CALL(
        _pRegistry,
        subscribeAckReceived(
            0x1234U,
            0x0001,
            0x01,
            0x01,
            IPEndpoint(make_ip4(224U, 1U, 255U, 255U), 0x1122),
            sourceAddress))
        .Times(1);

    _pParser.handleMessage(message, IPEndpoint(sourceAddress, 30490U), false);

    // do not respond if received via multicast
    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);
    EXPECT_CALL(_pRegistry, subscribeAckReceived(_, _, _, _, _, sourceAddress)).Times(0);

    _pParser.handleMessage(message, IPEndpoint(sourceAddress, 30490U), true);
}

TEST_F(SdMessageParserTest, testSubscribeAckReceivedTtl0)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x07, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x00, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x14, 0x00, // length, type
        0xE0, 0x01, 0xFF, 0xFF, // IP
        0x00, 0x11, 0x11, 0x22 // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);
    auto const sourceAddress = make_ip4(192U, 0U, 2U, 1U);

    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);
    EXPECT_CALL(_pRegistry, subscribeNackReceived(0x1234U, 1U, 1U, 1U, sourceAddress)).Times(1);

    _pParser.handleMessage(message, IPEndpoint(sourceAddress, 30490U), false);
}

TEST_F(SdMessageParserTest, testSubscribeReceived_StopSubscribe)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x06, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x00, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x14, 0x00, // length, type
        0xE0, 0x01, 0xFF, 0xFF, // IP
        0x00, 0x11, 0x11, 0x22 // proto UDP, Port
    };
    // clang-format on
    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);

    _pParser.handleMessage(message, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U), false);
}

TEST_F(SdMessageParserTest, testInvalidEntryType)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0xAB, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x0, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x14, 0x00, // length, type
        0xE0, 0x01, 0xFF, 0xFF, // IP
        0x00, 0x11, 0x11, 0x22 // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);

    _pParser.handleMessage(message, IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U), false);
}

/**
 * SdMessageParser receives the optional parameter additionalSDCheck.
 * As the additional parameter returns true it causes handleEntrySubscribe() to abort
 * and no respondToSubscribe() is invoked.
 */
TEST_F(SdMessageParserTest, testSubscribeReceived_with_additionalSDCheck_true)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x06, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22 // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);

    IPAddress ip = make_ip4(192U, 0U, 2U, 1U);

    EXPECT_CALL(_pRegistry, rebootDetected(ip)).Times(1);
    EXPECT_CALL(
        _pAnnouncer,
        sendSubscribeNack(
            0x1234U, 0x0001U, 0x0001U, 0x01U, 0x0000U, ip::make_ip4(192U, 0U, 2U, 1U)))
        .Times(1);

    ISdMessageParser::AdditionalSDCheck _additionalCheck
        = {::etl::delegate<bool(::ip::IPEndpoint const&)>::create<&additionalSDCheckMockTrue>()};
    SdMessageParser _pParser_additionalSDChecks{
        _pRegistry, _pAnnouncer, _rebootTracker, SUBNET_ID, LOCAL_IP, _additionalCheck};

    _pParser_additionalSDChecks.handleMessage(message, IPEndpoint(ip, 30490U), false);
}

/**
 * SdMessageParser receives the optional parameter additionalSDCheck.
 * As the additional parameter returns false it does not
 * cause handleEntrySubscribe() to abort and respondToSubscribe() is invoked.
 */
TEST_F(SdMessageParserTest, testSubscribeReceived_with_additionalSDCheck_false)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x06, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22 // proto UDP, Port
    };
    // clang-format on

    SomeIpMessage message(data);

    IPAddress ip = make_ip4(192U, 0U, 2U, 1U);

    EXPECT_CALL(_pRegistry, rebootDetected(ip)).Times(1);
    EXPECT_CALL(
        _pAnnouncer,
        respondToSubscribe(0x1234U, 0x0001U, 0x01U, _, 0x01U, 0x0100FEU, _, _, 0x1122U, 0x11U))
        .Times(1);

    ISdMessageParser::AdditionalSDCheck _additionalCheck
        = {::etl::delegate<bool(::ip::IPEndpoint const&)>::create<&additionalSDCheckMockFalse>()};
    SdMessageParser _pParser_additionalSDChecks{
        _pRegistry, _pAnnouncer, _rebootTracker, SUBNET_ID, LOCAL_IP, _additionalCheck};

    _pParser_additionalSDChecks.handleMessage(message, IPEndpoint(ip, 30490U), false);
}

/**
 * SdMessageParser receives the optional parameter additionalSDCheck.
 * As the additional parameter returns true it causes handleEntryOffer() to abort
 * and no offerReceived is invoked.
 */
TEST_F(SdMessageParserTest, testOfferReceived_with_additionalSDCheck_true)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x01, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x01, 0x02, 0x03, 0x04, // minor version
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22, // proto UDP, Port
    };
    // clang-format on

    auto const sdEndpoint = IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U);
    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);
    EXPECT_CALL(_pRegistry, offerReceived(_, sdEndpoint.getAddress())).Times(0);

    ISdMessageParser::AdditionalSDCheck _additionalCheck
        = {::etl::delegate<bool(::ip::IPEndpoint const&)>::create<&additionalSDCheckMockTrue>()};
    SdMessageParser _pParser_additionalSDChecks{
        _pRegistry, _pAnnouncer, _rebootTracker, SUBNET_ID, LOCAL_IP, _additionalCheck};

    _pParser_additionalSDChecks.handleMessage(message, sdEndpoint, false);
}

/**
 * SdMessageParser receives the optional parameter additionalSDCheck.
 * As the additional parameter returns false it does not cause
 * handleEntryOffer() to abort and offerReceived is invoked.
 */
TEST_F(SdMessageParserTest, testOfferReceived_with_additionalSDCheck_false)
{
    // clang-format off
    uint8_t data[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x80, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x01, 0x00, 0x00, 0x10, // type, options
        0x12, 0x34, 0x00, 0x01, // service id, instance id
        0x01, 0x01, 0x00, 0xFE, // major version, ttl
        0x01, 0x02, 0x03, 0x04, // minor version
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x11, 0x22, // proto UDP, Port
    };
    // clang-format on

    ServiceDescription receivedService = make<ServiceDescription>();
    auto const sdEndpoint              = IPEndpoint(make_ip4(192U, 0U, 2U, 1U), 30490U);
    SomeIpMessage message(data);

    EXPECT_CALL(_pRegistry, interestedInService(0x1234U, 0x0001, 0x01))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(_pRegistry, rebootDetected(make_ip4(192U, 0U, 2U, 1U))).Times(1);
    EXPECT_CALL(_pRegistry, offerReceived(_, sdEndpoint.getAddress()))
        .Times(1)
        .WillOnce(SaveArg<0>(&receivedService));

    ISdMessageParser::AdditionalSDCheck _additionalCheck
        = {::etl::delegate<bool(::ip::IPEndpoint const&)>::create<&additionalSDCheckMockFalse>()};
    SdMessageParser _pParser_additionalSDChecks{
        _pRegistry, _pAnnouncer, _rebootTracker, SUBNET_ID, LOCAL_IP, _additionalCheck};

    _pParser_additionalSDChecks.handleMessage(message, sdEndpoint, false);
    EXPECT_EQ(0x1234U, receivedService.serviceId);
    EXPECT_EQ(1U, receivedService.instanceId);
    EXPECT_EQ(1U, receivedService.majorVersion);
    EXPECT_EQ(0x0100FEU, receivedService.ttl);
    EXPECT_EQ(0x01020304U, receivedService.minorVersion);
    EXPECT_EQ(0x1122U, receivedService.port);
    EXPECT_EQ(0x11U, receivedService.proto);
}

} // anonymous namespace
