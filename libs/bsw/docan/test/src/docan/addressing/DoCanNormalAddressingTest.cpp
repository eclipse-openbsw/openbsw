// Copyright 2024 Accenture.

#include "docan/addressing/DoCanNormalAddressing.h"

#include <etl/memory.h>

#include <gmock/gmock.h>

namespace
{
using namespace docan;

/**
 * \req: [BSW_DoCAN_184], [BSW_DoCAN_184]
 */
TEST(DoCanNormalAddressingTest, testDecodeSingleFrameWithAddress)
{
    DoCanNormalAddressing<uint16_t, uint8_t> cut;
    // single frame (0 offset)
    uint8_t const payload[] = {0x02, 0x12, 0x34};
    // CAN id is the reception address
    EXPECT_EQ(0x1238934U, cut.decodeReceptionAddress(0x1238934U, payload));
}

/**
 * \req: [BSW_DoCAN_185], [BSW_DoCAN_184]
 */
TEST(DoCanNormalAddressingTest, testEncodeSingleFrameWithAddress)
{
    DoCanNormalAddressing<uint16_t, uint8_t> cut;
    // message data
    uint8_t payloadBuffer[1] = {0x33};
    // CAN id is the reception address
    uint32_t canId           = 0U;
    cut.encodeTransmissionAddress(0x1238934U, canId, payloadBuffer);
    EXPECT_EQ(0x1238934U, canId);
    // expect valid encoded single frame
    uint8_t const expectedPayload[] = {0x33};
    ::etl::span<uint8_t const> expectedPayloadSpan(expectedPayload);
    ::etl::span<uint8_t> payload = payloadBuffer;
    EXPECT_TRUE(::etl::equal(expectedPayloadSpan, payload));
}

} // anonymous namespace
