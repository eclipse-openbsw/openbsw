/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "docan/addressing/DoCanExtendedAddressing.h"

#include <etl/memory.h>

#include <gmock/gmock.h>

namespace
{
using namespace docan;

TEST(DoCanExtendedAddressingTest, testPackCombinesCanIdAndAddressExtension)
{
    using Addressing = DoCanExtendedAddressing<uint16_t, uint8_t>;
    EXPECT_EQ((0x2AU << 11U) | 0x7F1U, Addressing::pack(0x7F1U, 0x2AU));
    EXPECT_EQ(0U, Addressing::pack(0U, 0U));
}

TEST(DoCanExtendedAddressingTest, testCanIdOfAndAddressExtensionOfInvertPack)
{
    using Addressing      = DoCanExtendedAddressing<uint16_t, uint8_t>;
    uint32_t const packed = Addressing::pack(0x7F1U, 0x2AU);
    EXPECT_EQ(0x7F1U, Addressing::canIdOf(packed));
    EXPECT_EQ(0x2AU, Addressing::addressExtensionOf(packed));
}

TEST(DoCanExtendedAddressingTest, testDecodeSingleFrameWithAddress)
{
    DoCanExtendedAddressing<uint16_t, uint8_t> cut;
    // first payload byte is the address extension, rest is the (offset) encoded frame
    uint8_t const payload[] = {0x2A, 0x02, 0x12, 0x34};
    EXPECT_EQ(cut.pack(0x7F1U, 0x2AU), cut.decodeReceptionAddress(0x7F1U, payload));
}

TEST(DoCanExtendedAddressingTest, testDecodeIgnoresQualifierBitsOfCanId)
{
    DoCanExtendedAddressing<uint16_t, uint8_t> cut;
    uint8_t const payload[] = {0xF1};
    // only the lower 11 bits of the given CAN identifier are relevant
    EXPECT_EQ(cut.pack(0x72AU, 0xF1U), cut.decodeReceptionAddress(0xFFFFF800U | 0x72AU, payload));
}

TEST(DoCanExtendedAddressingTest, testDecodeWithEmptyPayloadAssumesZeroAddressExtension)
{
    DoCanExtendedAddressing<uint16_t, uint8_t> cut;
    EXPECT_EQ(
        cut.pack(0x7F1U, 0U), cut.decodeReceptionAddress(0x7F1U, ::etl::span<uint8_t const>()));
}

TEST(DoCanExtendedAddressingTest, testEncodeSingleFrameWithAddress)
{
    DoCanExtendedAddressing<uint16_t, uint8_t> cut;
    uint8_t payloadBuffer[2] = {0x00, 0x33};
    uint32_t canId           = 0U;
    cut.encodeTransmissionAddress(cut.pack(0x72AU, 0xF1U), canId, payloadBuffer);
    EXPECT_EQ(0x72AU, canId);
    // expect the address extension byte to be written to the first payload byte, the rest of the
    // (already encoded) payload is left untouched.
    uint8_t const expectedPayload[] = {0xF1, 0x33};
    ::etl::span<uint8_t const> expectedPayloadSpan(expectedPayload);
    ::etl::span<uint8_t> payload = payloadBuffer;
    EXPECT_TRUE(::etl::equal(expectedPayloadSpan, payload));
}

TEST(DoCanExtendedAddressingTest, testEncodeWithEmptyPayloadOnlyDecodesCanId)
{
    DoCanExtendedAddressing<uint16_t, uint8_t> cut;
    uint32_t canId = 0U;
    cut.encodeTransmissionAddress(cut.pack(0x7F1U, 0x2AU), canId, ::etl::span<uint8_t>());
    EXPECT_EQ(0x7F1U, canId);
}

} // anonymous namespace
