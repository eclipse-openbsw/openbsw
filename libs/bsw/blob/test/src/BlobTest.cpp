/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "blob/Blob.h"

#include "blob/Header.h"
#include "blob/configuration.h"
#include "blob/util.h"

#include <blob/Config.h>

#include <etl/span.h>
#include <etl/unaligned_type.h>

#include <gmock/gmock.h>

#include <cstdint>

namespace
{
using namespace ::testing;

struct TestTable
{
    ::etl::span<uint8_t const> first;

    ::etl::span<::etl::be_uint16_t const> second;

    ::etl::span<::etl::be_uint32_t const> third;
};

/**
 * \desc
 * Loading columns of a table from a config yields expected data
 */
TEST(BlobTest, load_columns_from_test_config)
{
    uint8_t const data[]
        = {0x00, 0x00, 0x00, 0x01, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00,
           0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36,
           0x00, 0x00, 0x00, 0x04, 0x19, 0x32, 0x64, 0xC8, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x77,
           0x02, 0xEE, 0x3A, 0x98, 0x75, 0x30, 0xEA, 0x60, 0x00, 0x00, 0x00, 0x18, 0x07, 0x73,
           0x59, 0x40, 0x0E, 0xE6, 0xB2, 0x80, 0x1D, 0xCD, 0x65, 0x00, 0x3B, 0x9A, 0xCA, 0x00,
           0x77, 0x35, 0x94, 0x00, 0xE6, 0xB2, 0x80, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};

    auto config = ::blob::config(data, ::blob::Config::Type::META);

    auto const header = ::blob::loadConfigHeader(config.data);
    EXPECT_EQ(header.configType, ::blob::Config::Type::META);

    TestTable table;

    config.data = ::blob::loadColumn(table.first, config.data);
    config.data = ::blob::loadColumn(table.second, config.data);
    config.data = ::blob::loadColumn(table.third, config.data);

    ::etl::array<uint8_t, 4> const expectedFirstColumn = {25, 50, 100, 200};
    ::etl::array<uint16_t, 5> const expectedSecondColumn
        = {::etl::be_uint16_t(0x177),
           ::etl::be_uint16_t(0x2EE),
           ::etl::be_uint16_t(0x3A98),
           ::etl::be_uint16_t(0x7530),
           ::etl::be_uint16_t(0xEA60)};
    ::etl::array<uint32_t, 6> const expectedThirdColumn
        = {::etl::be_uint32_t(0x7735940),
           ::etl::be_uint32_t(0xEE6B280),
           ::etl::be_uint32_t(0x1DCD6500),
           ::etl::be_uint32_t(0x3B9ACA00),
           ::etl::be_uint32_t(0x77359400),
           ::etl::be_uint32_t(0xE6B28000)};

    EXPECT_THAT(table.first, ElementsAreArray(expectedFirstColumn));
    EXPECT_THAT(table.second, ElementsAreArray(expectedSecondColumn));
    EXPECT_THAT(table.third, ElementsAreArray(expectedThirdColumn));
}

/**
 * \desc
 * Checking header of empty blob returns false
 */
TEST(BlobTest, check_header_of_empty_blob)
{
    ::etl::span<uint8_t const> emptyBlob;

    blob::Header header;

    EXPECT_FALSE(::blob::checkHeader(emptyBlob, header));
}

/**
 * \desc
 * Checking blob with valid header returns true and sets blob's version, magic number, and size
 */
TEST(BlobTest, check_valid_header)
{
    constexpr uint32_t BLOB_VERSION       = 1;
    constexpr uint32_t BLOB_MAGIC_PATTERN = 0xDEADBEEFU;

    blob::Header header;

    EXPECT_TRUE(::blob::checkHeader(::blob::CONFIGURATION_BLOB, header));

    EXPECT_EQ(header.version, BLOB_VERSION);
    EXPECT_EQ(header.magic, BLOB_MAGIC_PATTERN);
    EXPECT_EQ(header.size, sizeof(::blob::CONFIGURATION_BLOB) - blob::Blob::HEADER_SIZE);
}

/**
 * \desc
 * Checking blob with invalid version returns false
 */
TEST(BlobTest, check_invalid_version)
{
    uint8_t data[sizeof(::blob::CONFIGURATION_BLOB)];
    ::etl::copy(::etl::make_span(::blob::CONFIGURATION_BLOB), ::etl::make_span(data));

    ::etl::span<uint8_t> blobHeader       = ::etl::make_span(data);
    blobHeader.take<::etl::be_uint32_t>() = 0U; // version

    blob::Header header;

    EXPECT_FALSE(::blob::checkHeader(data, header));
}

/**
 * \desc
 * Checking blob with invalid magic number returns false
 */
TEST(BlobTest, check_invalid_magic)
{
    uint8_t data[sizeof(::blob::CONFIGURATION_BLOB)];
    ::etl::copy(::etl::make_span(::blob::CONFIGURATION_BLOB), ::etl::make_span(data));

    ::etl::span<uint8_t> blobHeader = ::etl::make_span(data);
    (void)blobHeader.take<::etl::be_uint32_t>(); // version
    blobHeader.take<::etl::be_uint32_t>() = 0U;  // magic

    blob::Header header;

    EXPECT_FALSE(::blob::checkHeader(data, header));
}

/**
 * \desc
 * Extracting a config from blob yields expected config
 */
TEST(BlobTest, routing_config)
{
    auto const config = ::blob::config(::blob::CONFIGURATION_BLOB, ::blob::Config::Type::ROUTING);
    EXPECT_EQ(config.type, ::blob::Config::Type::ROUTING);
}

/**
 * \desc
 * Iterating over blob with valid configs yields expected configs
 */
TEST(BlobTest, configs)
{
    ::blob::Config::Type const configTypes[] = {
        ::blob::Config::Type::ROUTING,       ::blob::Config::Type::RX_ADAPTER,
        ::blob::Config::Type::RX_ADAPTER,    ::blob::Config::Type::RX_ADAPTER,
        ::blob::Config::Type::RX_ADAPTER,    ::blob::Config::Type::RX_ADAPTER,
        ::blob::Config::Type::RX_ADAPTER,    ::blob::Config::Type::RX_ADAPTER,
        ::blob::Config::Type::RX_ADAPTER,    ::blob::Config::Type::RX_ADAPTER,
        ::blob::Config::Type::RX_ADAPTER,    ::blob::Config::Type::TX_ADAPTER,
        ::blob::Config::Type::TX_ADAPTER,    ::blob::Config::Type::TX_ADAPTER,
        ::blob::Config::Type::TX_ADAPTER,    ::blob::Config::Type::TX_ADAPTER,
        ::blob::Config::Type::TX_ADAPTER,    ::blob::Config::Type::TX_ADAPTER,
        ::blob::Config::Type::TX_ADAPTER,    ::blob::Config::Type::TX_ADAPTER,
        ::blob::Config::Type::TX_ADAPTER,    ::blob::Config::Type::CHANNEL,
        ::blob::Config::Type::CHANNEL,       ::blob::Config::Type::CHANNEL,
        ::blob::Config::Type::CHANNEL,       ::blob::Config::Type::CHANNEL,
        ::blob::Config::Type::CHANNEL,       ::blob::Config::Type::CHANNEL,
        ::blob::Config::Type::CHANNEL,       ::blob::Config::Type::CHANNEL,
        ::blob::Config::Type::CHANNEL_NAMES,
    };

    size_t i = 0;
    for (auto config : ::blob::Blob(::blob::CONFIGURATION_BLOB))
    {
        EXPECT_EQ(config.type, configTypes[i]) << "i=" << i;
        ++i;
    }
}

/**
 * \desc
 * Iterating over blob with unknown config yields no configs
 */
TEST(BlobTest, unknown_config)
{
    uint8_t const data[]
        = {0x00, 0x00, 0x00, 0x01, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x38, 0xFF, 0xFF,
           0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
           0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9, 0xF2, 0xDE, 0x11};

    for (auto config : ::blob::Blob(data))
    {
        EXPECT_EQ(config.data.size(), 0);
    }
}

/**
 * \desc
 * Iterating over blob with very large config header size yields no configs
 */
TEST(BlobTest, large_config_header_size)
{
    uint8_t const data[]
        = {0x00, 0x00, 0x00, 0x01, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00,
           0x00, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
           0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9, 0xF2, 0xDE, 0x11};

    for (auto config : ::blob::Blob(data))
    {
        EXPECT_EQ(config.data.size(), 0);
    }
}

/**
 * \desc
 * Iterating over blob with invalid size yields no configs
 */
TEST(BlobTest, invalid_size_blob)
{
    uint8_t data[sizeof(::blob::CONFIGURATION_BLOB)];
    ::etl::copy(::etl::make_span(::blob::CONFIGURATION_BLOB), ::etl::make_span(data));

    ::etl::span<uint8_t> blobHeader = ::etl::make_span(data);
    (void)blobHeader.take<::etl::be_uint32_t>(); // version
    (void)blobHeader.take<::etl::be_uint32_t>(); // magic
    blobHeader.take<::etl::be_uint32_t>() = 0U;  // size

    for (auto config : ::blob::Blob(data))
    {
        FAIL(); // there should be no configs
        EXPECT_EQ(config.data.size(), 0);
    }
}

/**
 * \desc
 * Iterating over empty blob yields no configs
 */
TEST(BlobTest, empty_blob)
{
    ::etl::span<uint8_t const> emptyData;

    for (auto config : ::blob::Blob(emptyData))
    {
        FAIL(); // there should be no configs
        EXPECT_EQ(config.data.size(), 0);
    }
}

/**
 * \desc
 * Checking arrow operator's usage with Blob Iterator
 */
TEST(BlobTest, check_arrow_operator_blob)
{
    constexpr uint32_t FIRST_CONFIG_SIZE  = 20;
    constexpr uint32_t SECOND_CONFIG_SIZE = 36;

    uint8_t const data[]
        = {0x00, 0x00, 0x00, 0x01, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00,
           0x00, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
           0x8B, 0x31, 0x3D, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9, 0xF2, 0xDE, 0x11};

    auto blob_it = ::blob::Blob(data).begin();

    EXPECT_EQ(blob_it->data.size(), FIRST_CONFIG_SIZE);

    ++blob_it;

    EXPECT_EQ(blob_it->data.size(), SECOND_CONFIG_SIZE);
}

/**
 * \desc
 * Checking equality operator's usage between Blob Iterators
 */
TEST(BlobTest, check_equality_operator_blob)
{
    uint8_t const data[32] = {0x00, 0x00, 0x00, 0x01, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00,
                              0x14, 0x00, 0x00, 0x00, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x8B, 0x31, 0x3D, 0x45};

    uint8_t other_data[32] = {};
    ::etl::copy(::etl::make_span(data), ::etl::make_span(other_data));

    auto first_blob_it = ::blob::Blob(data).begin();

    auto second_blob_it = ::blob::Blob(other_data).begin();

    // same sizes and same memory locations
    EXPECT_TRUE(first_blob_it == first_blob_it);

    // same sizes and different memory locations
    EXPECT_FALSE(first_blob_it == second_blob_it);
}

/**
 * \desc
 * Checking arrow operator's usage with Config
 */
TEST(BlobTest, check_arrow_operator_config)
{
    constexpr uint32_t FIRST_CONFIG_SIZE  = 20;
    constexpr uint32_t SECOND_CONFIG_SIZE = 36;

    uint8_t const data[]
        = {0x00, 0x00, 0x00, 0x01, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00,
           0x00, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
           0x8B, 0x31, 0x3D, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9, 0xF2, 0xDE, 0x11};

    uint32_t const configSizes[] = {FIRST_CONFIG_SIZE, SECOND_CONFIG_SIZE};

    size_t i = 0;
    for (auto config : ::blob::Blob(data))
    {
        EXPECT_EQ(config->data.size(), configSizes[i]);
        ++i;
    }
}

/**
 * \desc
 * Checking equality operator's usage between Configs
 */
TEST(BlobTest, check_equality_operator_config)
{
    uint8_t const data[]
        = {0x00, 0x00, 0x00, 0x01, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x00,
           0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x8B, 0x31,
           0x3D, 0x45, 0x00, 0x00, 0x00, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x04, 0x8B, 0x31, 0x3D, 0x45, 0x00, 0x00, 0x00, 0xDD, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x01, 0x00, 0x34, 0x55, 0x4E, 0x55,
           0x53, 0x45, 0x44, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};

    auto blob_it = ::blob::Blob(data).begin();

    auto first_config = (*blob_it);

    auto second_config = (*++blob_it);

    auto third_config = (*++blob_it);

    auto forth_config = (*++blob_it);

    // same type, same size, same memory locations
    EXPECT_TRUE(first_config == first_config);

    // same type, same size, different memory locations
    EXPECT_FALSE(first_config == second_config);

    // same type, different size, different memory locations
    EXPECT_FALSE(second_config == third_config);

    // different type, same size, different memory locations
    EXPECT_FALSE(third_config == forth_config);
}

/**
 * \desc
 * Loading from non-empty config returns non-empty span with valid config header
 */
TEST(BlobTest, load_config_header_from_non_empty_span)
{
    uint8_t const data[]
        = {0x00, 0x00, 0x00, 0x01, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00,
           0x00, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
           0x8B, 0x31, 0x3D, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9, 0xF2, 0xDE, 0x11};

    auto const config = ::blob::config(data, ::blob::Config::Type::ROUTING);

    auto headerData   = config.data;
    auto const header = ::blob::loadConfigHeader(headerData);

    EXPECT_NE(headerData.size(), 0);

    EXPECT_EQ(header.configType, ::blob::Config::Type::ROUTING);
    EXPECT_EQ(header.size, 20);
}

/**
 * \desc
 * Loading from empty config returns empty span
 */
TEST(BlobTest, load_config_header_from_empty_span)
{
    ::etl::span<uint8_t const> emptyConfig;
    auto const header = ::blob::loadConfigHeader(emptyConfig);

    EXPECT_EQ(emptyConfig.size(), 0);
    EXPECT_EQ(header.configType, ::blob::Config::Type::UNKNOWN);
}

/**
 * \desc
 * Checking configs with valid crcs returns trues
 */
TEST(BlobTest, check_valid_crc)
{
    size_t i = 0;
    for (auto config : ::blob::Blob(::blob::CONFIGURATION_BLOB))
    {
        EXPECT_TRUE(::blob::checkCrc(config.data)) << "Config no. " << i << " has invalid data!";
        ++i;
    }
}

/**
 * \desc
 * Checking config with invalid crc returns false
 */
TEST(BlobTest, check_invalid_crc)
{
    uint8_t const config[] = {0x00, 0x00, 0x00, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x8B, 0x31, 0x3D, 0x46};

    EXPECT_FALSE(::blob::checkCrc(config));
}

/**
 * \desc
 * Checking crc of config with invalid size returns false
 */
TEST(BlobTest, check_crc_of_invalid_size_config)
{
    ::etl::span<uint8_t const> emptyConfig;

    EXPECT_FALSE(::blob::checkCrc(emptyConfig));
}

/**
 * \desc
 * Loading blob with valid data returns non-empty blob span
 */
TEST(BlobTest, load_valid_blob) { EXPECT_NE(::blob::load(::blob::CONFIGURATION_BLOB).size(), 0); }

/**
 * \desc
 * Loading blob with invalid header returns empty blob span
 */
TEST(BlobTest, load_invalid_header_blob)
{
    uint8_t data[sizeof(::blob::CONFIGURATION_BLOB)];
    ::etl::copy(::etl::make_span(::blob::CONFIGURATION_BLOB), ::etl::make_span(data));

    {
        ::etl::span<uint8_t> blobHeader       = ::etl::make_span(data);
        blobHeader.take<::etl::be_uint32_t>() = 0U; // version
    }

    EXPECT_EQ(::blob::load(data).size(), 0);

    ::etl::copy(::etl::make_span(::blob::CONFIGURATION_BLOB), ::etl::make_span(data));
    {
        ::etl::span<uint8_t> blobHeader = ::etl::make_span(data);
        (void)blobHeader.take<::etl::be_uint32_t>(); // version
        blobHeader.take<::etl::be_uint32_t>() = 0U;  // magic
    }

    EXPECT_EQ(::blob::load(data).size(), 0);
}

/**
 * \desc
 * Loading blob with invalid data returns empty blob span
 */
TEST(BlobTest, load_invalid_data_blob)
{
    uint8_t const data[]
        = {0x00, 0x00, 0x00, 0x01, 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00,
           0x00, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
           0x8B, 0x31, 0x3D, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9, 0xF2, 0xDE, 0x11};

    EXPECT_EQ(::blob::load(data).size(), 0);
}

/**
 * \desc
 * Loading blob from empty data returns empty blob span
 */
TEST(BlobTest, load_empty_data_blob)
{
    ::etl::span<uint8_t const> emptyData;

    EXPECT_EQ(::blob::load(emptyData).size(), 0);
}

} // namespace
