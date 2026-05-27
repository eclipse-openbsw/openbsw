/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpSendJobHelper.h"
#include "doip/common/DoIpTestHelpers.h"

#include <etl/array.h>

#include <gmock/gmock.h>

namespace doip
{
namespace test
{
using namespace ::testing;

TEST(DoIpSendJobTest, TestPrepareHeaderBuffer)
{
    {
        ::etl::array<uint8_t, 8U> destBuffer;
        uint8_t expectedHeader[] = {0x03, 0xfc, 0x80, 0x02, 0x00, 0x00, 0x02, 0x10};
        EXPECT_TRUE(is_equal(
            expectedHeader,
            DoIpSendJobHelper::prepareHeaderBuffer(destBuffer, 0x03, 0x8002U, 0x210)));
    }
    {
        ::etl::array<uint8_t, 7> destBuffer;
        EXPECT_TRUE(
            DoIpSendJobHelper::prepareHeaderBuffer(destBuffer, 0x03, 0x8002U, 0x210).empty());
    }
}

} // namespace test
} // namespace doip
