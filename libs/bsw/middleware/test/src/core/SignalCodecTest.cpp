/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/core/SignalCodec.h"

#include <etl/bitset.h>
#include <gtest/gtest.h>

namespace
{
constexpr size_t NARROW_WIDTH     = 5U;
constexpr size_t ENUM_WIDTH       = 7U;
constexpr size_t MIXED_BYTE_COUNT = 15U;
constexpr uint8_t NARROW_VALUE    = 0x15U;
constexpr uint8_t BITSET_VALUE    = 0x13U;
constexpr uint64_t WIDE_VALUE     = 0x0123456789ABCDEFU;
constexpr uint64_t SPARSE_VALUE   = 0x8000000000000001U;
template<typename Value>
constexpr size_t STORAGE_BITS = etl::numeric_limits<Value>::digits;

enum class SignalEnum : uint8_t
{
    Value = 0x55U
};

template<typename Underlying, size_t BitCount>
class SignalBits
{
public:
    explicit SignalBits(Underlying const raw) : bits_(raw) {}

    Underlying value() const { return bits_.template value<Underlying>(); }

private:
    etl::bitset<BitCount> bits_;
};

using NarrowBits  = SignalBits<uint8_t, NARROW_WIDTH>;
using SparseBits  = SignalBits<uint64_t, STORAGE_BITS<uint64_t>>;
using SignalArray = etl::array<uint16_t, 2U>;
using middleware::core::codec::readSignal;
using middleware::core::codec::writeSignal;

SignalArray const ARRAY_VALUE{{0x1234U, 0xABCDU}};
etl::array<uint8_t, MIXED_BYTE_COUNT> const MIXED_BYTES{
    {0xAEU,
     0xACU,
     0xC4U,
     0x8DU,
     0x2AU,
     0xF3U,
     0x40U,
     0x48U,
     0xD1U,
     0x59U,
     0xE2U,
     0x6AU,
     0xF3U,
     0x7BU,
     0xC0U}};

TEST(SignalCodecTest, WritesMixedUnalignedSignalsAsExpectedBytes)
{
    etl::array<uint8_t, MIXED_BYTE_COUNT> bytes{};
    etl::bit_stream_writer stream{
        etl::make_span(bytes), etl::bit_order::msb_first, {}, etl::endian::big};
    writeSignal<NARROW_WIDTH>(stream, NARROW_VALUE);
    writeSignal<1U>(stream, true);
    writeSignal<ENUM_WIDTH>(stream, SignalEnum::Value);
    writeSignal<NARROW_WIDTH>(stream, NarrowBits{BITSET_VALUE});
    writeSignal<SignalArray::SIZE * STORAGE_BITS<uint16_t>>(stream, ARRAY_VALUE);
    writeSignal<STORAGE_BITS<uint64_t>>(stream, WIDE_VALUE);
    EXPECT_EQ(bytes, MIXED_BYTES);
}

TEST(SignalCodecTest, ReadsMixedSignalsFromExpectedBytes)
{
    etl::bit_stream_reader stream{
        etl::make_span(MIXED_BYTES), etl::bit_order::msb_first, etl::endian::big};
    EXPECT_EQ((readSignal<uint8_t, NARROW_WIDTH>(stream)), NARROW_VALUE);
    EXPECT_TRUE((readSignal<bool, 1U>(stream)));
    EXPECT_EQ((readSignal<SignalEnum, ENUM_WIDTH>(stream)), SignalEnum::Value);
    EXPECT_EQ((readSignal<NarrowBits, NARROW_WIDTH>(stream)).value(), BITSET_VALUE);
    EXPECT_EQ(
        (readSignal<SignalArray, SignalArray::SIZE * STORAGE_BITS<uint16_t>>(stream)), ARRAY_VALUE);
    EXPECT_EQ((readSignal<uint64_t, STORAGE_BITS<uint64_t>>(stream)), WIDE_VALUE);
}

TEST(SignalCodecTest, FullWidthUnsignedScalarsPreserveAllBits)
{
    etl::array<uint8_t, MIXED_BYTE_COUNT> bytes{};
    etl::bit_stream_writer writer{
        etl::make_span(bytes), etl::bit_order::msb_first, {}, etl::endian::big};
    writeSignal<STORAGE_BITS<uint8_t>>(writer, etl::numeric_limits<uint8_t>::max());
    writeSignal<STORAGE_BITS<uint16_t>>(writer, etl::numeric_limits<uint16_t>::max());
    writeSignal<STORAGE_BITS<uint32_t>>(writer, etl::numeric_limits<uint32_t>::max());
    writeSignal<STORAGE_BITS<uint64_t>>(writer, etl::numeric_limits<uint64_t>::max());
    for (auto const byte : bytes)
    {
        EXPECT_EQ(byte, 0xFFU);
    }
    etl::bit_stream_reader reader{
        etl::make_span(bytes), etl::bit_order::msb_first, etl::endian::big};
    EXPECT_EQ(
        (readSignal<uint8_t, STORAGE_BITS<uint8_t>>(reader)), etl::numeric_limits<uint8_t>::max());
    EXPECT_EQ(
        (readSignal<uint16_t, STORAGE_BITS<uint16_t>>(reader)),
        etl::numeric_limits<uint16_t>::max());
    EXPECT_EQ(
        (readSignal<uint32_t, STORAGE_BITS<uint32_t>>(reader)),
        etl::numeric_limits<uint32_t>::max());
    EXPECT_EQ(
        (readSignal<uint64_t, STORAGE_BITS<uint64_t>>(reader)),
        etl::numeric_limits<uint64_t>::max());
}

TEST(SignalCodecTest, NarrowValuesKeepOnlyWireBitsAndPreserveUnknownEnums)
{
    etl::array<uint8_t, 2U> bytes{};
    etl::bit_stream_writer writer{
        etl::make_span(bytes), etl::bit_order::msb_first, {}, etl::endian::big};
    writeSignal<1U>(writer, false);
    writeSignal<3U>(writer, etl::numeric_limits<uint8_t>::max());
    writeSignal<ENUM_WIDTH>(writer, static_cast<SignalEnum>(etl::numeric_limits<uint8_t>::max()));
    writeSignal<NARROW_WIDTH>(writer, NarrowBits{etl::numeric_limits<uint8_t>::max()});
    etl::array<uint8_t, 2U> const expected{{0x7FU, 0xFFU}};
    EXPECT_EQ(bytes, expected);
    etl::bit_stream_reader reader{
        etl::make_span(expected), etl::bit_order::msb_first, etl::endian::big};
    EXPECT_FALSE((readSignal<bool, 1U>(reader)));
    EXPECT_EQ((readSignal<uint8_t, 3U>(reader)), 7U);
    EXPECT_EQ((readSignal<SignalEnum, ENUM_WIDTH>(reader)), static_cast<SignalEnum>(0x7FU));
    EXPECT_EQ((readSignal<NarrowBits, NARROW_WIDTH>(reader)).value(), 0x1FU);
}

TEST(SignalCodecTest, SparseBitsetPreservesHighestAndLowestBits)
{
    etl::array<uint8_t, sizeof(uint64_t)> bytes{};
    etl::bit_stream_writer writer{
        etl::make_span(bytes), etl::bit_order::msb_first, {}, etl::endian::big};
    writeSignal<STORAGE_BITS<uint64_t>>(writer, SparseBits{SPARSE_VALUE});
    etl::array<uint8_t, sizeof(uint64_t)> const expected{{0x80U, 0U, 0U, 0U, 0U, 0U, 0U, 1U}};
    EXPECT_EQ(bytes, expected);
    etl::bit_stream_reader reader{
        etl::make_span(expected), etl::bit_order::msb_first, etl::endian::big};
    EXPECT_EQ((readSignal<SparseBits, STORAGE_BITS<uint64_t>>(reader)).value(), SPARSE_VALUE);
}

TEST(SignalCodecTest, ArraysUseFullElementWidthsInIndexOrder)
{
    using ByteArray = etl::array<uint8_t, 2U>;
    using WordArray = etl::array<uint32_t, 1U>;
    using WideArray = etl::array<uint64_t, 1U>;
    ByteArray const byteValues{{0x12U, 0x34U}};
    WordArray const wordValues{{0xABCDEF01U}};
    WideArray const wideValues{{WIDE_VALUE}};
    constexpr size_t byteCount = ByteArray::SIZE * sizeof(uint8_t)
                                 + WordArray::SIZE * sizeof(uint32_t)
                                 + WideArray::SIZE * sizeof(uint64_t);
    etl::array<uint8_t, byteCount> bytes{};
    etl::array<uint8_t, byteCount> const expected{
        {0x12U,
         0x34U,
         0xABU,
         0xCDU,
         0xEFU,
         0x01U,
         0x01U,
         0x23U,
         0x45U,
         0x67U,
         0x89U,
         0xABU,
         0xCDU,
         0xEFU}};
    etl::bit_stream_writer writer{
        etl::make_span(bytes), etl::bit_order::msb_first, {}, etl::endian::big};
    writeSignal<ByteArray::SIZE * STORAGE_BITS<uint8_t>>(writer, byteValues);
    writeSignal<WordArray::SIZE * STORAGE_BITS<uint32_t>>(writer, wordValues);
    writeSignal<WideArray::SIZE * STORAGE_BITS<uint64_t>>(writer, wideValues);
    EXPECT_EQ(bytes, expected);
    etl::bit_stream_reader reader{
        etl::make_span(expected), etl::bit_order::msb_first, etl::endian::big};
    EXPECT_EQ((readSignal<ByteArray, ByteArray::SIZE * STORAGE_BITS<uint8_t>>(reader)), byteValues);
    EXPECT_EQ(
        (readSignal<WordArray, WordArray::SIZE * STORAGE_BITS<uint32_t>>(reader)), wordValues);
    EXPECT_EQ(
        (readSignal<WideArray, WideArray::SIZE * STORAGE_BITS<uint64_t>>(reader)), wideValues);
}
} // namespace
