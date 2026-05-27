/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpSimplePayloadSendJob.h"

#include "doip/common/DoIpConstants.h"
#include "doip/common/DoIpHeader.h"
#include "doip/common/DoIpTestHelpers.h"

#include <etl/array.h>
#include <etl/span.h>

#include <gmock/gmock.h>

namespace doip
{
namespace test
{
using namespace ::testing;

class DoIpSimplePayloadSendJobMock;

class DoIpSimplePayloadSendJobMock : public DoIpSimplePayloadSendJob<DoIpSimplePayloadSendJobMock>
{
public:
    DoIpSimplePayloadSendJobMock(
        uint8_t protocolVersion,
        uint16_t payloadType,
        uint16_t payloadLength,
        ReleaseCallbackType releaseCallback)
    : DoIpSimplePayloadSendJob(protocolVersion, payloadType, payloadLength, releaseCallback)
    {}

    MOCK_CONST_METHOD1(getPayloadBuffer, ::etl::span<uint8_t const>(::etl::span<uint8_t>));

    static ::etl::span<uint8_t const> returnBuffer(::etl::span<uint8_t const> buffer)
    {
        return buffer;
    }
};

class DoIpSimplePayloadSendJobTest : public Test
{
public:
    MOCK_METHOD2(released, void(DoIpSimplePayloadSendJobMock&, bool));
};

TEST_F(DoIpSimplePayloadSendJobTest, TestAll)
{
    DoIpSimplePayloadSendJobMock cut(
        0x01,
        0x1234U,
        0xfU,
        DoIpSimplePayloadSendJobMock::ReleaseCallbackType::
            create<DoIpSimplePayloadSendJobTest, &DoIpSimplePayloadSendJobTest::released>(*this));
    ASSERT_EQ(2U, cut.getSendBufferCount());
    ASSERT_EQ(DoIpConstants::DOIP_HEADER_LENGTH + 0xf, cut.getTotalLength());
    uint8_t const expectedHeader[] = {0x01, 0xfe, 0x12, 0x34, 0x00, 0x00, 0x00, 0x0f};
    ::etl::array<uint8_t, 8U> staticBuffer;
    ::etl::span<uint8_t const> sendBuffer = cut.getSendBuffer(staticBuffer, 0U);
    EXPECT_EQ(staticBuffer.data(), sendBuffer.data());
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(expectedHeader), sendBuffer));
    // return another buffer
    uint8_t const payloadBuffer[] = {0x11, 0x22, 0x33, 0x44};
    EXPECT_CALL(cut, getPayloadBuffer(_))
        .WillOnce(Return(::etl::span<uint8_t const>(payloadBuffer)));
    sendBuffer = cut.getSendBuffer(staticBuffer, 1U);
    EXPECT_EQ(payloadBuffer, sendBuffer.data());
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(payloadBuffer), sendBuffer));
    // return static buffer
    EXPECT_CALL(cut, getPayloadBuffer(_))
        .WillOnce(Invoke(DoIpSimplePayloadSendJobMock::returnBuffer));
    sendBuffer = cut.getSendBuffer(staticBuffer, 1U);
    EXPECT_EQ(staticBuffer.data(), sendBuffer.data());
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(staticBuffer), sendBuffer));
    // last buffer
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(), cut.getSendBuffer(staticBuffer, 2U)));

    // release with success
    EXPECT_CALL(*this, released(Ref(cut), true));
    cut.release(true);
    Mock::VerifyAndClearExpectations(this);

    // release without success
    EXPECT_CALL(*this, released(Ref(cut), false));
    cut.release(false);
    Mock::VerifyAndClearExpectations(this);
}

} // namespace test
} // namespace doip
