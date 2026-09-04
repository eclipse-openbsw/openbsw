/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdOptions.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

using ::someip::SdOptions;

TEST(SdOptions, PayloadSizeExactForZeroOptions)
{
    // clang-format off
    uint8_t const data[] = {
        0x80,0x00,0x00,0x00, // flags
        // -- Entries --
        0x00,0x00,0x00,0x10, // entries length
        0x06,0x02,0x03,0x12, // type, options
        0x12,0x34,0x56,0x78, // service id, instance id
        0x01,0x01,0x00,0xFE, // major version, ttl
        0x00,0x00,0x00,0x01, // eventgroup id
        // -- Options --
        0x00,0x00,0x00,0x00  // options length
    };
    // clang-format on

    SdOptions options;
    // everything OK.
    EXPECT_TRUE(options.init(data, 0x10U));
}

TEST(SdOptions, PayloadTooSmall)
{
    // clang-format off
    uint8_t data[] = {
//            0xFF,0xFF,0x81,0x00, // message id
//            0x00,0x00,0x00,0x30, // length
//            0x00,0x00,0x00,0x00, // request id
//            0x01,0x01,0x02,0x00, // version, message type, return code
        0x80,0x00,0x00,0x00, // flags
        // -- Entries --
        0x00,0x00,0x00,0x10, // entries length
        0x06,0x02,0x03,0x12, // type, options
        0x12,0x34,0x56,0x78, // service id, instance id
        0x01,0x01,0x00,0xFE, // major version, ttl
        0x00,0x00,0x00,0x01, // eventgroup id
        // -- Options --
        0x00,0x00,0x00,0x0C, // options length
        0x00,0x09,0x04,0x00, // length, type
        0xC0,0x00,0x02,0x01, // IP
        0x00,0x11,0x11,0x22  // proto UDP, Port
    };
    // clang-format on

    SdOptions options;
    // everything OK.
    EXPECT_TRUE(options.init(data, 0x10U));

    options.readIndexValues(::etl::span<uint8_t const>(data, sizeof(data)).subspan(8U));
    EXPECT_EQ(0x02, options.getOptions1Index());
    EXPECT_EQ(0x03, options.getOptions2Index());
    EXPECT_EQ(0x01, options.getOptions1Num());
    EXPECT_EQ(0x02, options.getOptions2Num());

    // payload is too small for reading options length
    EXPECT_FALSE(options.init(::etl::span<uint8_t const>(data, sizeof(data)).first(10U), 10U));

    // payload is too small for length of options specified
    data[27] = 0xCC;
    EXPECT_FALSE(options.init(data, 0x10));
}
} // anonymous namespace
