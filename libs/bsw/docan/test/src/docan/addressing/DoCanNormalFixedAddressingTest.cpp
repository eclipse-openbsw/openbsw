/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "docan/addressing/DoCanNormalFixedAddressing.h"

#include <can/canframes/CanId.h>

#include <gmock/gmock.h>

namespace
{
using namespace docan;

using Addressing = DoCanNormalFixedAddressing<uint16_t, uint8_t>;

TEST(DoCanNormalFixedAddressingTest, testPackCombinesAllFieldsAndSetsExtendedQualifierBit)
{
    uint32_t const packed = Addressing::pack(Addressing::PHYSICAL_ADDRESSING_ID_BASE, 0x2AU, 0xF1U);
    EXPECT_EQ(
        ::can::CanId::EXTENDED_QUALIFIER_BIT | Addressing::PHYSICAL_ADDRESSING_ID_BASE
            | (0x2AU << 8) | 0xF1U,
        packed);
    EXPECT_TRUE(::can::CanId::isExtended(packed));
}

TEST(DoCanNormalFixedAddressingTest, testFieldAccessorsInvertPack)
{
    uint32_t const packed
        = Addressing::pack(Addressing::FUNCTIONAL_ADDRESSING_ID_BASE, 0x33U, 0x44U);
    EXPECT_EQ(Addressing::FUNCTIONAL_ADDRESSING_ID_BASE, Addressing::addressingIdBaseOf(packed));
    EXPECT_EQ(0x33U, Addressing::targetAddressOf(packed));
    EXPECT_EQ(0x44U, Addressing::sourceAddressOf(packed));
}

TEST(DoCanNormalFixedAddressingTest, testDecodeReceptionAddressReturnsCanIdUnchanged)
{
    uint32_t const canId = Addressing::pack(Addressing::PHYSICAL_ADDRESSING_ID_BASE, 0x2AU, 0xF1U);
    uint8_t const payload[] = {0x10, 0x02, 0x12, 0x34};
    EXPECT_EQ(canId, Addressing::decodeReceptionAddress(canId, payload));
}

TEST(DoCanNormalFixedAddressingTest, testEncodeTransmissionAddressOnlySetsCanId)
{
    uint32_t const transmissionAddress
        = Addressing::pack(Addressing::PHYSICAL_ADDRESSING_ID_BASE, 0x2AU, 0xF1U);
    uint32_t canId           = 0U;
    uint8_t payloadBuffer[2] = {0x11, 0x22};
    Addressing::encodeTransmissionAddress(transmissionAddress, canId, payloadBuffer);
    EXPECT_EQ(transmissionAddress, canId);
    // payload is not touched by normal fixed addressing
    EXPECT_EQ(0x11U, payloadBuffer[0]);
    EXPECT_EQ(0x22U, payloadBuffer[1]);
}

} // anonymous namespace
