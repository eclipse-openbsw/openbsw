/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdOptionParser.h"

#include "someip/SdOptions.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::someip;

TEST(SdOptionParser, ParseIpOption_TwoOptionsSections)
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
        0x06,0x00,0x01,0x11, // type, index 1, index 2, (num 1, num 2)
        0x12,0x34,0x56,0x78, // service id, instance id
        0x01,0x01,0x00,0xFE, // major version, ttl
        0x00,0x00,0x00,0x01, // eventgroup id
        // -- Options --
        0x00,0x00,0x00,0x1F, // options length 32-bit
        0x00,0x10,0x01,0x00, // configuration option
        0x05,'a','b','c',
        '=','x',0x07,'d',
        'e','f','=',1U,
        2U,3U,0U,
        0x00,0x09,0x04,0x00, // length (16-bit), type, reserved
        0xC0,0x00,0x02,0x01, // IP
        0x00,0x11,0x11,0x22, // proto UDP, Port
    };
    // clang-format on

    SdOptions options;

    // everything OK.
    EXPECT_TRUE(options.init(data, 0x10U));

    options.readIndexValues(::etl::span<uint8_t const>(data, sizeof(data)).subspan(8U));
    EXPECT_EQ(0x00, options.getOptions1Index());
    EXPECT_EQ(0x01, options.getOptions2Index());
    EXPECT_EQ(0x01, options.getOptions1Num());
    EXPECT_EQ(0x01, options.getOptions2Num());

    uint8_t udp = 0U;
    uint8_t tcp = 0U;

    SdEndpoint ep = SdOptionParser::parseIpEndpointOption(options, udp, tcp);
    EXPECT_TRUE(ep.isValid());
    EXPECT_EQ(0x1122U, ep.getPort());
}

TEST(SdOptionParser, ParseIpOption_TwoOptionsSectionsIpFirst)
{
    // clang-format off
    uint8_t const data[] = {
//            0xFF,0xFF,0x81,0x00, // message id
//            0x00,0x00,0x00,0x30, // length
//            0x00,0x00,0x00,0x00, // request id
//            0x01,0x01,0x02,0x00, // version, message type, return code
        0x80,0x00,0x00,0x00, // flags
        // -- Entries --
        0x00,0x00,0x00,0x10, // entries length
        0x06,0x00,0x00,0x20, // type, index 1, index 2, (num 1, num 2)
        0x12,0x34,0x56,0x78, // service id, instance id
        0x01,0x01,0x00,0xFE, // major version, ttl
        0x00,0x00,0x00,0x01, // eventgroup id
        // -- Options --
        0x00,0x00,0x00,0x1F, // options length 32-bit
        0x00,0x09,0x04,0x00, // length (16-bit), type, reserved
        0xC0,0x00,0x02,0x01, // IP
        0x00,0x11,0x11,0x22, // proto UDP, Port
        0x00,0x10,0x01,0x00, // configuration option
        0x05,'a','b','c',
        '=','x',0x07,'d',
        'e','f','=',1U,
        2U,3U,0U,
    };
    // clang-format on

    SdOptions options;

    // everything OK.
    EXPECT_TRUE(options.init(data, 0x10U));

    options.readIndexValues(::etl::span<uint8_t const>(data, sizeof(data)).subspan(8U));

    uint8_t udp = 0U;
    uint8_t tcp = 0U;

    SdEndpoint ep = SdOptionParser::parseIpEndpointOption(options, udp, tcp);
    EXPECT_TRUE(ep.isValid());
    EXPECT_EQ(0x1122U, ep.getPort());
}

TEST(SdOptionParser, ParseIpOption_TwoOptionsSectionsReversed)
{
    // clang-format off
    uint8_t const data[] = {
//            0xFF,0xFF,0x81,0x00, // message id
//            0x00,0x00,0x00,0x30, // length
//            0x00,0x00,0x00,0x00, // request id
//            0x01,0x01,0x02,0x00, // version, message type, return code
        0x80,0x00,0x00,0x00, // flags
        // -- Entries --
        0x00,0x00,0x00,0x10, // entries length
        0x06,0x01,0x00,0x11, // type, index 1, index 2, (num 1, num 2)
        0x12,0x34,0x56,0x78, // service id, instance id
        0x01,0x01,0x00,0xFE, // major version, ttl
        0x00,0x00,0x00,0x01, // eventgroup id
        // -- Options --
        0x00,0x00,0x00,0x1F, // options length 32-bit
        0x00,0x09,0x04,0x00, // length (16-bit), type, reserved
        0xC0,0x00,0x02,0x01, // IP
        0x00,0x11,0x11,0x22, // proto UDP, Port
        0x00,0x10,0x01,0x00, // configuration option
        0x05,'a','b','c',
        '=','x',0x07,'d',
        'e','f','=',1U,
        2U,3U,0U,
    };
    // clang-format on

    SdOptions options;

    // everything OK.
    EXPECT_TRUE(options.init(data, 0x10U));

    options.readIndexValues(::etl::span<uint8_t const>(data, sizeof(data)).subspan(8U));
    EXPECT_EQ(0x01, options.getOptions1Index());
    EXPECT_EQ(0x00, options.getOptions2Index());
    EXPECT_EQ(0x01, options.getOptions1Num());
    EXPECT_EQ(0x01, options.getOptions2Num());

    uint8_t udp = 0U;
    uint8_t tcp = 0U;

    SdEndpoint ep = SdOptionParser::parseIpEndpointOption(options, udp, tcp);
    EXPECT_TRUE(ep.isValid());
    EXPECT_EQ(0x1122U, ep.getPort());
}

TEST(SdOptionParser, ParseIpOption_ConfigurationOption)
{
    // clang-format off
    uint8_t const data[] = {
//            0xFF,0xFF,0x81,0x00, // message id
//            0x00,0x00,0x00,0x30, // length
//            0x00,0x00,0x00,0x00, // request id
//            0x01,0x01,0x02,0x00, // version, message type, return code
        0x80,0x00,0x00,0x00, // flags
        // -- Entries --
        0x00,0x00,0x00,0x10, // entries length
        0x06,0x00,0x00,0x10, // type, index 1, index 2, (num 1, num 2)
        0x12,0x34,0x56,0x78, // service id, instance id
        0x01,0x01,0x00,0xFE, // major version, ttl
        0x00,0x00,0x00,0x01, // eventgroup id
        // -- Options --
        0x00,0x00,0x00,0x13, // options length 32-bit
        0x00,0x10,0x01,0x00, // configuration option
        0x05,'a','b','c',
        '=','x',0x07,'d',
        'e','f','=',1U,
        2U,3U,0U,
    };
    // clang-format on

    SdOptions options;

    // everything OK.
    EXPECT_TRUE(options.init(data, 0x10U));

    options.readIndexValues(::etl::span<uint8_t const>(data, sizeof(data)).subspan(8U));
    uint8_t udp = 0U;
    uint8_t tcp = 0U;

    SdEndpoint ep = SdOptionParser::parseIpEndpointOption(options, udp, tcp);
    EXPECT_FALSE(ep.isValid());
}

TEST(SdOptionParser, ParseIpOption_OneOptionSection)
{
    // clang-format off
    uint8_t const data[] = {
//            0xFF,0xFF,0x81,0x00, // message id
//            0x00,0x00,0x00,0x30, // length
//            0x00,0x00,0x00,0x00, // request id
//            0x01,0x01,0x02,0x00, // version, message type, return code
        0x80,0x00,0x00,0x00, // flags
        // -- Entries --
        0x00,0x00,0x00,0x10, // entries length
        0x06,0x00,0x00,0x10, // type, index 1, index 2, (num 1, num 2)
        0x12,0x34,0x56,0x78, // service id, instance id
        0x01,0x01,0x00,0xFE, // major version, ttl
        0x00,0x00,0x00,0x01, // eventgroup id
        // -- Options --
        0x00,0x00,0x00,0x0C, // options length 32-bit
        0x00,0x09,0x04,0x00, // length (16-bit), type, reserved
        0xC0,0x00,0x02,0x01, // IP
        0x00,0x11,0x11,0x22, // proto UDP, Port
    };
    // clang-format on

    SdOptions options;

    // everything OK.
    EXPECT_TRUE(options.init(data, 0x10U));

    options.readIndexValues(::etl::span<uint8_t const>(data, sizeof(data)).subspan(8U));
    EXPECT_EQ(0x00, options.getOptions1Index());
    EXPECT_EQ(0x00, options.getOptions2Index());
    EXPECT_EQ(0x01, options.getOptions1Num());
    EXPECT_EQ(0x00, options.getOptions2Num());

    uint8_t udp = 0U;
    uint8_t tcp = 0U;

    SdEndpoint ep = SdOptionParser::parseIpEndpointOption(options, udp, tcp);
    EXPECT_TRUE(ep.isValid());
    EXPECT_EQ(0x1122U, ep.getPort());
}
} // anonymous namespace
