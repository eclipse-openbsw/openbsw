/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "SdMessageTestConstants.h"
#include "someip/SdMessageBuilder.h"
#include "someip/SdMessageConstants.h"
#include "someip/ServiceDescription.h"
#include "someip/SomeIpConstants.h"

#include <ip/IPAddress.h>

#include <etl/array.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::ip;
using namespace ::someip;
using namespace ::testing;

struct SdMessageBuilderSmallSizeTest : Test
{
    SdMessageBuilder _builder;
};

/**
 * Make sure startMessage() detects that buffer is too small even for header
 * and therefore message build is not started.
 */
TEST_F(SdMessageBuilderSmallSizeTest, too_small_for_header)
{
    // too small for header
    ::etl::array<uint8_t, MINIMAL_MESSAGE_SIZE - 1U> buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_BUFFER_TOO_SMALL, _builder.startMessage(buffer));
    EXPECT_EQ(0U, _builder.finishMessage(0U, false).size());
}

/**
 * Tests that message build is started because header fits buffer
 * but addFind() detects that there is not enough space for entry.
 */
TEST_F(SdMessageBuilderSmallSizeTest, too_small_for_entry)
{
    // too small for entry
    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            - 1U>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE,
        _builder.addFind(0xAAAA, 0x01, 0x01, ::someip::minor_version::INVALID, 0x03));

    EXPECT_TRUE(_builder.isEmpty());

    EXPECT_EQ(28U, _builder.finishMessage(1U, true).size());
}

TEST_F(SdMessageBuilderSmallSizeTest, WriteOffer_TooSmall)
{
    // too small for entry
    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            + IPV4_ENDPOINT_OPTION_SIZE - 1U>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE,
        _builder.addOffer(
            0xAAAA,
            0x01,
            0x01,
            ::someip::minor_version::INVALID,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = _builder.finishMessage(1U, true);

    EXPECT_EQ(28U, message.size());
}

TEST_F(SdMessageBuilderSmallSizeTest, WriteFindEventGroup_TooSmall)
{
    // too small for entry
    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            - 1U>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE,
        _builder.addFindEventgroup(0xAAAA, 0x01, 0x01, 0x01, 0x03));

    ::etl::span<uint8_t const> const message = _builder.finishMessage(1U, true);

    EXPECT_EQ(28U, message.size());
}

TEST_F(SdMessageBuilderSmallSizeTest, WritePublish_TooSmall)
{
    // too small for entry
    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            + IPV4_ENDPOINT_OPTION_SIZE - 1U>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE,
        _builder.addPublish(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = _builder.finishMessage(1U, true);

    EXPECT_EQ(28U, message.size());
}

TEST_F(SdMessageBuilderSmallSizeTest, WriteSubscribe_TooSmall)
{
    // too small for entry
    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            + IPV4_ENDPOINT_OPTION_SIZE - 1U>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE,
        _builder.addSubscribe(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = _builder.finishMessage(1U, true);

    EXPECT_EQ(28U, message.size());
}

TEST_F(SdMessageBuilderSmallSizeTest, WriteSubscribeAck_TooSmall)
{
    // too small for entry
    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            - 1U>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE,
        _builder.addSubscribeAck(0xAAAA, 0x01, 0x01, 0x01, ::someip::minor_version::INVALID, 0x03));

    ::etl::span<uint8_t const> const message = _builder.finishMessage(1U, true);

    EXPECT_EQ(28U, message.size());
}

TEST_F(SdMessageBuilderSmallSizeTest, WriteSubscribeAckMulticast_TooSmall)
{
    // too small for entry
    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            + IPV4_ENDPOINT_OPTION_SIZE - 1U>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE,
        _builder.addSubscribeAckMulticast(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            ::someip::minor_version::INVALID,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = _builder.finishMessage(1U, true);

    EXPECT_EQ(28U, message.size());
}

#ifdef PLATFORM_SUPPORT_IPV6
TEST_F(SdMessageBuilderSmallSizeTest, WriteSubscribeIp6_TooSmall)
{
    // clang-format off
    const uint8_t ip6[16]
        = {0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // clang-format on

    // too small for entry
    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            + IPV6_ENDPOINT_OPTION_SIZE - 1U>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE,
        _builder.addSubscribe(
            0xAAAA, 0x01, 0x01, 0x01, 0x03, make_ip6(ip6), 0x7725, proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = _builder.finishMessage(1U, true);

    EXPECT_EQ(28U, message.size());
}

TEST_F(SdMessageBuilderSmallSizeTest, WriteSubscribeAckMulticastIp6_TooSmall)
{
    // clang-format off
    const uint8_t ip6[16]
        = {0x20, 0x01, 0x0D, 0xB8, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // clang-format on

    // too small for entry
    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            - 1U>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, _builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE,
        _builder.addSubscribeAckMulticast(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            ::someip::minor_version::INVALID,
            0x03,
            make_ip6(ip6),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = _builder.finishMessage(1U, true);

    EXPECT_EQ(28U, message.size());
}
#endif // OPENBSW_NO_IPV6

class SdMessageBuilderSmallEntriesTest : public Test
{
public:
    SdMessageBuilderSmallEntriesTest() = default;
};

TEST_F(SdMessageBuilderSmallEntriesTest, AddEntry_Full)
{
    SdMessageBuilder builder;

    // only room for one entry
    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            + IPV4_ENDPOINT_OPTION_SIZE>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_IS_FULL,
        builder.addSubscribe(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    ::etl::span<uint8_t const> const message = builder.finishMessage(1U, true);
    EXPECT_EQ(56U, message.size());
}

TEST_F(SdMessageBuilderSmallEntriesTest, AssociateEndpointOption_OptionsFull)
{
    SdMessageBuilder builder;

    // only room for two entries but one option
    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE
            + 2U * static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            + IPV4_ENDPOINT_OPTION_SIZE>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        builder.addSubscribe(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_NOT_ENOUGH_SPACE,
        builder.addSubscribe(
            0xAAAB,
            0x02,
            0x02,
            0x02,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7726,
            proto::SD_L4_PROTO_UDP));

    (void)builder.finishMessage(1U, true);
}

TEST_F(SdMessageBuilderSmallEntriesTest, AssociateEndpointOption_WrongAddressType)
{
    // only room for one option
    SdMessageBuilder builder;
    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE
            + 2U * static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            + IPV4_ENDPOINT_OPTION_SIZE>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, builder.startMessage(buffer));
    builder.addSubscribe(
        0xAAAA, 0x01, 0x01, 0x01, 0x03, make_ip4(192U, 0U, 2U, 0U), 0x7725, proto::SD_L4_PROTO_UDP);

    builder.finishMessage(1U, true);
}

TEST_F(SdMessageBuilderSmallEntriesTest, AssociateEndpointOption_DifferentIpAddress)
{
    // only room for one option
    SdMessageBuilder builder;

    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE
            + 2U * static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            + 2U * IPV4_ENDPOINT_OPTION_SIZE>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        builder.addSubscribe(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_IS_FULL,
        builder.addSubscribe(
            0xAAAB,
            0x02,
            0x02,
            0x02,
            0x03,
            make_ip4(192U, 0U, 2U, 1U),
            0x7726,
            proto::SD_L4_PROTO_UDP));

    (void)builder.finishMessage(1U, true);
}

TEST_F(SdMessageBuilderSmallEntriesTest, AssociateEndpointOption_DifferentPort)
{
    // only room for one option
    SdMessageBuilder builder;

    ::etl::array<
        uint8_t,
        MINIMAL_MESSAGE_SIZE
            + 2U * static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENTRY)
            + 2U * IPV4_ENDPOINT_OPTION_SIZE>
        buffer{};

    EXPECT_EQ(SdMessageReturnCode::SD_MESSAGE_OK, builder.startMessage(buffer));
    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_OK,
        builder.addSubscribe(
            0xAAAA,
            0x01,
            0x01,
            0x01,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7725,
            proto::SD_L4_PROTO_UDP));

    EXPECT_EQ(
        SdMessageReturnCode::SD_MESSAGE_IS_FULL,
        builder.addSubscribe(
            0xAAAB,
            0x02,
            0x02,
            0x02,
            0x03,
            make_ip4(192U, 0U, 2U, 0U),
            0x7726,
            proto::SD_L4_PROTO_UDP));

    (void)builder.finishMessage(1U, true);

    // EXPECT_EQ(2, builder.getCurrentNumberOfEndpointOptions());
}

} // anonymous namespace
