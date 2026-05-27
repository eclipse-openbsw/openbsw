/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpVehicleIdentificationRequestSendJob.h"

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

class DoIpVehicleIdentificationRequestSendJobTest : public Test
{
public:
    MOCK_METHOD2(released, void(DoIpStaticPayloadSendJob&, bool));
};

TEST_F(DoIpVehicleIdentificationRequestSendJobTest, TestAll)
{
    DoIpVehicleIdentificationRequestSendJob cut(
        DoIpVehicleIdentificationRequestSendJob::ReleaseCallbackType ::create<
            DoIpVehicleIdentificationRequestSendJobTest,
            &DoIpVehicleIdentificationRequestSendJobTest::released>(*this));
    ASSERT_EQ(2U, cut.getSendBufferCount());
    ASSERT_EQ(uint16_t(DoIpConstants::DOIP_HEADER_LENGTH), cut.getTotalLength());
    uint8_t const expectedHeader[] = {0xff, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    ::etl::array<uint8_t, 8U> staticBuffer;
    ::etl::span<uint8_t const> sendBuffer = cut.getSendBuffer(staticBuffer, 0U);
    EXPECT_EQ(staticBuffer.data(), sendBuffer.data());
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(expectedHeader), sendBuffer));
    // return another buffer
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(), cut.getSendBuffer(staticBuffer, 1U)));
    // release
    EXPECT_CALL(*this, released(Ref(cut), true));
    cut.release(true);
}

} // namespace test
} // namespace doip
