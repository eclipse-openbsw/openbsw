/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpStaticPayloadSendJob.h"

#include "doip/common/DoIpHeader.h"
#include "doip/common/DoIpTestHelpers.h"

#include <etl/array.h>
#include <etl/memory.h>
#include <etl/span.h>

#include <gmock/gmock.h>

namespace doip
{
namespace test
{
using namespace ::testing;

class DoIpStaticPayloadSendJobTest : public Test
{
public:
    MOCK_METHOD2(released, void(DoIpStaticPayloadSendJob&, bool));
};

TEST_F(DoIpStaticPayloadSendJobTest, TestAll)
{
    uint8_t const payloadBuffer[] = {0x11, 0x22, 0x33, 0x44};
    DoIpStaticPayloadSendJob cut(
        0x01,
        0x3321,
        payloadBuffer,
        DoIpStaticPayloadSendJob::ReleaseCallbackType::
            create<DoIpStaticPayloadSendJobTest, &DoIpStaticPayloadSendJobTest::released>(*this));
    ASSERT_EQ(2U, cut.getSendBufferCount());
    ASSERT_EQ(DoIpConstants::DOIP_HEADER_LENGTH + 0x4, cut.getTotalLength());
    uint8_t const expectedHeader[] = {0x01, 0xfe, 0x33, 0x21, 0x00, 0x00, 0x00, 0x04};
    ::etl::array<uint8_t, 8U> staticBuffer;
    ::etl::span<uint8_t const> sendBuffer = cut.getSendBuffer(staticBuffer, 0U);
    EXPECT_EQ(staticBuffer.data(), sendBuffer.data());
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(expectedHeader), sendBuffer));
    // return payload buffer
    sendBuffer = cut.getSendBuffer(staticBuffer, 1U);
    EXPECT_EQ(payloadBuffer, sendBuffer.data());
    EXPECT_TRUE(is_equal(payloadBuffer, sendBuffer));
    // invalid index
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(), cut.getSendBuffer(staticBuffer, 2U)));
    // release
    EXPECT_CALL(*this, released(Ref(cut), true));
    cut.release(true);
}

TEST_F(DoIpStaticPayloadSendJobTest, TestDeclareWithPayloadLength)
{
    uint8_t const payload[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    declare::DoIpStaticPayloadSendJob<15U> cut(
        0x03,
        0x3521,
        sizeof(payload),
        DoIpStaticPayloadSendJob::ReleaseCallbackType::
            create<DoIpStaticPayloadSendJobTest, &DoIpStaticPayloadSendJobTest::released>(*this));
    ASSERT_EQ(DoIpConstants::DOIP_HEADER_LENGTH + sizeof(payload), cut.getTotalLength());
    ::etl::span<uint8_t> payloadBuffer = cut.accessPayloadBuffer();
    ::etl::mem_copy(payload, sizeof(payload), payloadBuffer.begin());
    uint8_t const expectedHeader[] = {0x03, 0xfc, 0x35, 0x21, 0x00, 0x00, 0x00, sizeof(payload)};
    // retrieve
    ::etl::array<uint8_t, 8U> staticBuffer;
    ::etl::span<uint8_t const> sendBuffer = cut.getSendBuffer(staticBuffer, 0U);
    EXPECT_EQ(staticBuffer.data(), sendBuffer.data());
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(expectedHeader), sendBuffer));
    // return payload buffer
    sendBuffer = cut.getSendBuffer(staticBuffer, 1U);
    EXPECT_EQ(payloadBuffer.data(), sendBuffer.data());
    EXPECT_TRUE(is_equal(payload, sendBuffer));
    // invalid index
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(), cut.getSendBuffer(staticBuffer, 2U)));
    // release
    EXPECT_CALL(*this, released(Ref(cut), false));
    cut.release(false);
}

TEST_F(DoIpStaticPayloadSendJobTest, TestDeclareWithPayload)
{
    uint8_t const payload[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
    declare::DoIpStaticPayloadSendJob<15U> cut(
        0x02,
        0x3521,
        ::etl::span<uint8_t const>(payload),
        DoIpStaticPayloadSendJob::ReleaseCallbackType::
            create<DoIpStaticPayloadSendJobTest, &DoIpStaticPayloadSendJobTest::released>(*this));
    ASSERT_EQ(DoIpConstants::DOIP_HEADER_LENGTH + sizeof(payload), cut.getTotalLength());
    uint8_t const expectedHeader[] = {0x02, 0xfd, 0x35, 0x21, 0x00, 0x00, 0x00, sizeof(payload)};
    // retrieve
    ::etl::array<uint8_t, 8U> staticBuffer;
    ::etl::span<uint8_t const> sendBuffer = cut.getSendBuffer(staticBuffer, 0U);
    EXPECT_EQ(staticBuffer.data(), sendBuffer.data());
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(expectedHeader), sendBuffer));
    // return payload buffer
    sendBuffer = cut.getSendBuffer(staticBuffer, 1U);
    EXPECT_EQ(cut.accessPayloadBuffer().data(), sendBuffer.data());
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(payload), sendBuffer));
    // invalid index
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(), cut.getSendBuffer(staticBuffer, 2U)));
    // release
    EXPECT_CALL(*this, released(Ref(cut), true));
    cut.release(true);
}

} // namespace test
} // namespace doip
