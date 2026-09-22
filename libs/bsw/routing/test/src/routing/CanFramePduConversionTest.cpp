/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/util.h"

#include <can/canframes/CANFrame.h>

#include <etl/array.h>
#include <gtest/gtest.h>

namespace
{
TEST(CanFramePduConversionTest, serializesAndDeserializesCanFrame)
{
    ::etl::array<uint8_t, ::can::CANFrame::MAX_FRAME_LENGTH> payload{1U, 2U, 3U, 4U};
    ::can::CANFrame const input{0x123U, payload.data(), 4U};
    ::etl::array<uint8_t, 16U> buffer{};

    auto const serialized = ::routing::canFrameToPdu(input, ::etl::make_span(buffer));

    ASSERT_EQ(12U, serialized.size());
    ::can::CANFrame output;
    ASSERT_TRUE(::routing::pduToCanFrame(serialized, output));
    EXPECT_EQ(input, output);
}

TEST(CanFramePduConversionTest, rejectsBufferTooSmallForSerialization)
{
    ::can::CANFrame const input{0x123U};
    ::etl::array<uint8_t, ::routing::ROUTING_PDU_HEADER_SIZE - 1U> buffer{};

    EXPECT_TRUE(::routing::canFrameToPdu(input, ::etl::make_span(buffer)).empty());
}

TEST(CanFramePduConversionTest, rejectsIncompleteHeader)
{
    ::etl::array<uint8_t, ::routing::ROUTING_PDU_HEADER_SIZE - 1U> buffer{};
    ::can::CANFrame output;

    EXPECT_FALSE(::routing::pduToCanFrame(::etl::make_span(buffer), output));
}

TEST(CanFramePduConversionTest, rejectsPayloadLengthLargerThanCanFrame)
{
    ::etl::array<uint8_t, ::routing::ROUTING_PDU_HEADER_SIZE> buffer{};
    buffer[7] = static_cast<uint8_t>(::can::CANFrame::MAX_FRAME_LENGTH + 1U);
    ::can::CANFrame output;

    EXPECT_FALSE(::routing::pduToCanFrame(::etl::make_span(buffer), output));
}

TEST(CanFramePduConversionTest, rejectsBufferShorterThanDeclaredPayload)
{
    ::etl::array<uint8_t, ::routing::ROUTING_PDU_HEADER_SIZE + 1U> buffer{};
    buffer[7] = 2U;
    ::can::CANFrame output;

    EXPECT_FALSE(::routing::pduToCanFrame(::etl::make_span(buffer), output));
}
} // namespace
