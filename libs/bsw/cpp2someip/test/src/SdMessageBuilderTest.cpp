/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdMessageBuilder.h"

#include "SdMessageTestConstants.h"
#include "someip/SdEndpoint.h"
#include "someip/ServiceDescription.h"
#include "someip/SomeIpConstants.h"

#include <ip/IPAddress.h>

#include <etl/array.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{

using namespace ::testing;
using namespace ::ip;
using namespace ::someip;

struct SdMessageBuilderTest : Test
{
    ::etl::array<uint8_t, SD_PACKET_MAX_SIZE> _buffer{};
    SdMessageBuilder _builder;

    void verify(uint8_t const* a1, int len1, uint8_t const* a2, int len2)
    {
        EXPECT_EQ(len1, len2) << "Payload length";
        for (int i = 0; i < len1; ++i)
        {
            EXPECT_EQ((int)a1[i], (int)a2[i]) << "Payload " << (int)i;
        }
    }
};

TEST_F(SdMessageBuilderTest, testStartCalledTwice)
{
    // header gets added here
    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));

    // clang-format off
   const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x00, // length
        0x00, 0x00, 0x00, 0x00, // request id
        0x01, 0x01, 0x02, 0x00}; // version, message type, return code
    // clang-format on

    EXPECT_THAT(
        ::etl::span<uint8_t>(_buffer.data(), _buffer.size()).first(16),
        ElementsAreArray(expectedPayload));
    EXPECT_THAT(::etl::span<uint8_t>(_buffer.data(), _buffer.size()).subspan(16), Each(Eq(0U)));
}

TEST_F(SdMessageBuilderTest, testBuildSubscribe)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x06, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x00, // IP
        0x00, 0x11, 0x77, 0x25 // proto UDP, port
    };
    // clang-format on

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addSubscribe(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    EXPECT_THAT(_builder.finishMessage(1U, true), ElementsAreArray(expectedPayload));
}

#ifdef PLATFORM_SUPPORT_IPV6
TEST_F(SdMessageBuilderTest, testBuildSubscribeIPv6)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x3C, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x06, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x18, // options length
        0x00, 0x15, 0x06, 0x00, // length, type
        0x20, 0x01, 0x0D, 0xB8, // IP
        0x00, 0x00, 0x00, 0x00, // IP
        0x00, 0x00, 0x00, 0x00, // IP
        0x00, 0x00, 0x00, 0x00, // IP
        0x00, 0x11, 0x77, 0x25 // proto UDP, Port
    };

    const uint8_t ip6[16]
        = {0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // clang-format on

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addSubscribe(
            0xAAAA, 0x01, 0x01, 0x01, 0x03, make_ip6(ip6), 0x7725, proto::SD_L4_PROTO_UDP));

    EXPECT_THAT(_builder.finishMessage(1U, true), ElementsAreArray(expectedPayload));
}
#endif // OPENBSW_NO_IPV6

TEST_F(SdMessageBuilderTest, testBuildTwoSubscribes)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x40, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x20, // entries length
        0x06, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        0x06, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAB, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x00, // IP
        0x00, 0x11, 0x77, 0x25 // proto UDP, port
    };
    // clang-format on

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addSubscribe(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            30501U,
            proto::SD_L4_PROTO_UDP));

    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addSubscribe(
            0xAAAB,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            30501U,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = _builder.finishMessage(1U, true);

    verify(expectedPayload, sizeof(expectedPayload), message.data(), message.size());
}

TEST_F(SdMessageBuilderTest, testBuildUnicastSubscribeAndMulticastAck)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x4C, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x20, // entries length
        0x06, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        0x07, 0x01, 0x00, 0x10, // type, options
        0xAA, 0xAB, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0xFF, 0xFF, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x18, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x00, // IP
        0x00, 0x11, 0x77, 0x25, // proto UDP, port
        0x00, 0x09, 0x14, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x77, 0x25 // proto UDP, port
    };
    // clang-format on

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addSubscribe(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            30501U,
            proto::SD_L4_PROTO_UDP));

    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addSubscribeAckMulticast(
            0xAAAB,
            0x01,
            0x01,
            0x01,
            ::someip::minor_version::INVALID,
            0x03,
            make_ip4(192U, 0U, 2U, 1U),
            30501U,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = _builder.finishMessage(1U, true);

    verify(expectedPayload, sizeof(expectedPayload), message.data(), message.size());
}

TEST_F(SdMessageBuilderTest, testBuildMulticastAckAndUnicastSubscribe)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x4C, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x20, // entries length
        0x07, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAB, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0xFF, 0xFF, 0x00, 0x01, // eventgroup id
        0x06, 0x01, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x18, // options length
        0x00, 0x09, 0x14, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x00, // IP
        0x00, 0x11, 0x77, 0x25, // proto UDP, port
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x77, 0x25, // proto UDP, port
    };
    // clang-format on

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addSubscribeAckMulticast(
            0xAAAB,
            0x01,
            0x01,
            0x01,
            ::someip::minor_version::INVALID,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            30501U,
            proto::SD_L4_PROTO_UDP));

    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addSubscribe(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 1U),
            30501U,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = _builder.finishMessage(1U, true);

    verify(expectedPayload, sizeof(expectedPayload), message.data(), message.size());
}

TEST_F(SdMessageBuilderTest, testAddFindEventgroup)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x24, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x04, 0x00, 0x00, 0x00, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // reserved, eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x00, // options length
    };
    // clang-format on

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addFindEventgroup(0xAAAA, 0x01, 0x01, 0x01, 0x03));

    EXPECT_THAT(_builder.finishMessage(1U, true), ElementsAreArray(expectedPayload));
}

TEST_F(SdMessageBuilderTest, testAddPublish)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x05, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // reserved, eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x00, // IP
        0x00, 0x11, 0x77, 0x25 // proto UDP, port
    };
    // clang-format on

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addPublish(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            30501U,
            proto::SD_L4_PROTO_UDP));

    EXPECT_THAT(_builder.finishMessage(1U, true), ElementsAreArray(expectedPayload));
}

TEST_F(SdMessageBuilderTest, testAddUnPublish)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x05, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x00, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // reserved, eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x00, // IP
        0x00, 0x11, 0x77, 0x25 // proto UDP, port
    };
    // clang-format on

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addUnpublish(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            30501U,
            proto::SD_L4_PROTO_UDP));

    EXPECT_THAT(_builder.finishMessage(1U, true), ElementsAreArray(expectedPayload));
}

TEST_F(SdMessageBuilderTest, testAddSubscribeAckMulticast)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x07, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x01, 0x00, 0x01, // minor version, eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x14, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x00, // IP
        0x00, 0x11, 0x77, 0x25 // proto UDP, port
    };
    // clang-format on

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addSubscribeAckMulticast(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            1,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            30501U,
            proto::SD_L4_PROTO_UDP));

    EXPECT_THAT(_builder.finishMessage(1U, true), ElementsAreArray(expectedPayload));
}

TEST_F(SdMessageBuilderTest, testAddSubscribeNack)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x24, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x07, 0x00, 0x00, 0x00, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x00, // major version, ttl
        0x00, 0x01, 0x00, 0x01, // minor version, eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x000 // options length
    };
    // clang-format on

    _builder.startMessage(_buffer);
    _builder.addSubscribeNack(0xAAAA, 0x01, 0x01, 0x01, 1U, 0x03);

    EXPECT_THAT(_builder.finishMessage(1U, true), ElementsAreArray(expectedPayload));
}

TEST(SdMessageBuilderSpecialTest, testAddSubscribeAckMulticastEndpointsFull)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x07, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x01, 0x00, 0x01, // minor version, eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x14, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x00, // IP
        0x00, 0x11, 0x77, 0x25 // proto UDP, port
    };
    // clang-format on

    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE
            + 2U * static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            + IPV4_ENDPOINT_OPTION_SIZE>
        buffer{};
    SdMessageBuilder builder;

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        builder.addSubscribeAckMulticast(
            0xAAAA,
            1U,
            1U,
            1U,
            1U,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    // what happens if we add another endpoint but we are already full
    // can't add this endpoint
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE,
        builder.addSubscribeAckMulticast(
            0xABAB,
            2U,
            2U,
            2U,
            2U,
            0x04,
            make_ip4(192U, 0U, 2U, 1U),
            0x7726,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = builder.finishMessage(1U, true);

    EXPECT_THAT(message, ElementsAreArray(expectedPayload));
}

TEST(SdMessageBuilderSpecialTest, testAddSubscribeAckMulticastSameEndpoint)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x40, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x20, // entries length
        0x07, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x01, 0x00, 0x01, // minor version, eventgroup id
        // second entry
        0x07, 0x00, 0x00, 0x10, // type, options
        0xAB, 0xAB, 0x00, 0x02, // service id, instance id
        0x02, 0x00, 0x00, 0x04, // major version, ttl
        0x00, 0x02, 0x00, 0x02, // minor version, eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0C, // options length
        0x00, 0x09, 0x14, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x00, // IP
        0x00, 0x11, 0x77, 0x25 // proto UDP, port
    };
    // clang-format on

    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE
            + 2U * static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            + IPV4_ENDPOINT_OPTION_SIZE>
        buffer{};
    SdMessageBuilder builder;

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        builder.addSubscribeAckMulticast(
            0xAAAA,
            1U,
            1U,
            1U,
            1U,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    // can't add this endpoint
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_IS_FULL,
        builder.addSubscribeAckMulticast(
            0xABAB,
            2U,
            2U,
            2U,
            2U,
            0x04,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = builder.finishMessage(1U, true);

    EXPECT_THAT(message, ElementsAreArray(expectedPayload));
}

#ifdef PLATFORM_SUPPORT_IPV6
TEST_F(SdMessageBuilderTest, testBuildIp6MulticastAck)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x3C, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x40, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x07, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0xFF, 0xFF, 0x00, 0x01, // eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x18, // options length
        0x00, 0x15, 0x16, 0x00, // length, type
        0x20, 0x01, 0x0D, 0xB8, // IP
        0x00, 0x00, 0x00, 0x00, // IP
        0x00, 0x00, 0x00, 0x00, // IP
        0x00, 0x00, 0x00, 0x00, // IP
        0x00, 0x11, 0x77, 0x25, // proto UDP, port
    };

    const uint8_t ip6[16]
        = {0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // clang-format on

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addSubscribeAckMulticast(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            ::someip::minor_version::INVALID,
            0x03,
            make_ip6(ip6),
            30501U,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = _builder.finishMessage(1U, false);

    verify(expectedPayload, sizeof(expectedPayload), message.data(), message.size());
}

TEST_F(SdMessageBuilderTest, testAssociateMulticastOptionInvalidIp)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x14, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0x40, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x00, // entries length
        // no entries
        // -- Options --
        0x00, 0x00, 0x00, 0x00, // options length
        // no options
    };

    const uint8_t ip6[16] = {0x00};
    // clang-format on

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_INVALID_ADDRESS,
        _builder.addSubscribeAckMulticast(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            ::someip::minor_version::INVALID,
            0x03,
            make_ip6(ip6),
            30501U,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = _builder.finishMessage(1U, false);

    verify(expectedPayload, sizeof(expectedPayload), message.data(), message.size());
}
#endif // OPENBSW_NO_IPV6

TEST_F(SdMessageBuilderTest, testAddSubscribeAckMulticast_PortDifferent)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x4C, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x20, // entries length
        0x07, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x01, 0x00, 0x01, // minor version, eventgroup id
        // second entry
        0x07, 0x01, 0x00, 0x10, // type, options
        0xAB, 0xAB, 0x00, 0x02, // service id, instance id
        0x02, 0x00, 0x00, 0x04, // major version, ttl
        0x00, 0x02, 0x00, 0x02, // minor version, eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x18, // options length
        0x00, 0x09, 0x14, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x00, // IP
        0x00, 0x11, 0x77, 0x25, // proto UDP, port
        0x00, 0x09, 0x14, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x11, 0x77, 0x26 // proto UDP, port
    };
    // clang-format on

    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE
            + 2U
                  * (static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                     + IPV4_ENDPOINT_OPTION_SIZE)>
        buffer{};
    SdMessageBuilder builder;

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        builder.addSubscribeAckMulticast(
            0xAAAA,
            1U,
            1U,
            1U,
            1U,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    // what happens if we add another endpoint but we are already full
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_IS_FULL,
        builder.addSubscribeAckMulticast(
            0xABAB,
            2U,
            2U,
            2U,
            2U,
            0x04,
            make_ip4(192U, 0U, 2U, 1U),
            0x7726,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = builder.finishMessage(1U, true);

    EXPECT_THAT(message, ElementsAreArray(expectedPayload));
}

TEST_F(SdMessageBuilderTest, testAddSubscribeAckMulticast_ProtoDifferent)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x4C, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x20, // entries length
        0x07, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x01, 0x00, 0x01, // minor version, eventgroup id
        // second entry
        0x07, 0x01, 0x00, 0x10, // type, options
        0xAB, 0xAB, 0x00, 0x02, // service id, instance id
        0x02, 0x00, 0x00, 0x04, // major version, ttl
        0x00, 0x02, 0x00, 0x02, // minor version, eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x18, // options length
        0x00, 0x09, 0x14, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x00, // IP
        0x00, 0x11, 0x77, 0x25, // proto UDP, port
        0x00, 0x09, 0x14, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x06, 0x77, 0x25 // proto TCP, port
    };
    // clang-format on

    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE
            + 2U
                  * (static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                     + IPV4_ENDPOINT_OPTION_SIZE)>
        buffer{};
    SdMessageBuilder builder;

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        builder.addSubscribeAckMulticast(
            0xAAAA,
            1U,
            1U,
            1U,
            1U,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_IS_FULL,
        builder.addSubscribeAckMulticast(
            0xABAB,
            2U,
            2U,
            2U,
            2U,
            0x04,
            make_ip4(192U, 0U, 2U, 1U),
            0x7725,
            proto::SD_L4_PROTO_TCP));

    ::etl::span<uint8_t const> const message = builder.finishMessage(1U, true);

    EXPECT_THAT(message, ElementsAreArray(expectedPayload));
}

TEST_F(SdMessageBuilderTest, testAddPublish_ProtoDifferent)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x4C, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x20, // entries length
        0x05, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // reserved, eventgroup id
        // second entry
        0x05, 0x01, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // reserved, eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x18, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x00, // IP
        0x00, 0x11, 0x77, 0x25, // proto UDP, port
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x01, // IP
        0x00, 0x06, 0x77, 0x25 // proto TCP, port
    };
    // clang-format on

    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE
            + 2U
                  * (static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
                     + IPV4_ENDPOINT_OPTION_SIZE)>
        buffer{};
    SdMessageBuilder builder;

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        builder.addPublish(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            30501U,
            proto::SD_L4_PROTO_UDP));

    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_IS_FULL,
        builder.addPublish(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 1U),
            30501U,
            proto::SD_L4_PROTO_TCP));

    EXPECT_THAT(builder.finishMessage(1U, true), ElementsAreArray(expectedPayload));
}

TEST_F(SdMessageBuilderTest, testDenounce)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x30, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x01, 0x00, 0x00, 0x10, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x00, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // reserved, eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x0c, // options length
        0x00, 0x09, 0x04, 0x00, // length, type
        0xC0, 0x00, 0x02, 0x00, // IP
        0x00, 0x11, 0x77, 0x25, // proto UDP, port
    };
    // clang-format on

    SdEndpoint endpoint(make_ip4(192U, 0U, 2U, 0U), 30501U, proto::SD_L4_PROTO_UDP);

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        _builder.addDenounce(
            0xAAAA,
            0x01,
            0x01,
            1U,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            30501U,
            proto::SD_L4_PROTO_UDP));

    EXPECT_THAT(_builder.finishMessage(1U, true), ElementsAreArray(expectedPayload));
}

TEST_F(SdMessageBuilderTest, testWriteServiceDescription)
{
    // clang-format off
    const uint8_t expectedPayload[] = {
        0xFF, 0xFF, 0x81, 0x00, // message id
        0x00, 0x00, 0x00, 0x24, // length
        0x00, 0x00, 0x00, 0x01, // request id
        0x01, 0x01, 0x02, 0x00, // version, message type, return code
        0xC0, 0x00, 0x00, 0x00, // flags
        // -- Entries --
        0x00, 0x00, 0x00, 0x10, // entries length
        0x00, 0x00, 0x00, 0x00, // type, options
        0xAA, 0xAA, 0x00, 0x01, // service id, instance id
        0x01, 0x00, 0x00, 0x03, // major version, ttl
        0x00, 0x00, 0x00, 0x01, // reserved, eventgroup id
        // -- Options --
        0x00, 0x00, 0x00, 0x00 // options length
    };
    // clang-format on

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(_buffer));
    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.addFind(0xAAAA, 0x01, 0x01, 1U, 0x03));

    EXPECT_THAT(_builder.finishMessage(1U, true), ElementsAreArray(expectedPayload));
}

} // anonymous namespace
