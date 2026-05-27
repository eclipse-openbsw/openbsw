/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpHeader.h"

#include <etl/span.h>

#include <gtest/gtest.h>

using namespace ::testing;
using namespace ::doip;

namespace
{
DoIpHeader const as_header(etl::span<uint8_t const> bytes)
{
    return bytes.reinterpret_as<DoIpHeader const>()[0];
}
} // namespace

TEST(DoIpHeaderTest, checkProtocolVersion)
{
    uint8_t const correct[] = {0x02, 0xFD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    EXPECT_TRUE(checkProtocolVersion(as_header(etl::span<uint8_t const>(correct)), 0x02));

    uint8_t const invalidInverse[] = {0x02, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    EXPECT_FALSE(checkProtocolVersion(as_header(invalidInverse), 0x02));

    uint8_t const correctInverseIncorrectVersion[]
        = {0x02, 0xFD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    EXPECT_FALSE(checkProtocolVersion(as_header(correctInverseIncorrectVersion), 0x03));
}

TEST(DoIpHeaderTest, read_fields)
{
    uint8_t const bytes[]    = {0x02, 0xFD, 0x03, 0x75, 0x12, 0x34, 0x56, 0x78};
    DoIpHeader const& header = as_header(bytes);

    EXPECT_EQ(0x02U, header.protocolVersion);
    EXPECT_EQ(0x375U, header.payloadType);
    EXPECT_EQ(0x12345678U, header.payloadLength);
}
