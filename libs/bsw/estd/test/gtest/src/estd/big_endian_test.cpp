// Copyright 2024 Accenture.

#include "estd/big_endian.h"

#include "estd/memory.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

TEST(big_endian, read_u8)
{
    uint8_t const buffer[] = {0x12};
    EXPECT_EQ(0x12U, ::estd::read_be<uint8_t>(&buffer[0]));
}

TEST(big_endian, read_u16)
{
    uint8_t const buffer[] = {0xAB, 0x12};
    EXPECT_EQ(0xAB12U, ::estd::read_be<uint16_t>(&buffer[0]));
}

TEST(big_endian, read_u32)
{
    uint8_t const buffer[] = {0x13, 0x42, 0x32, 0x05};
    EXPECT_EQ(0x13423205U, ::estd::read_be<uint32_t>(&buffer[0]));
}

TEST(big_endian, read_u64)
{
    uint8_t const buffer[] = {0x49, 0x56, 0xA6, 0x5B, 0x34, 0xBB, 0x66, 0x15};
    EXPECT_EQ(0x4956A65B34BB6615U, ::estd::read_be<uint64_t>(&buffer[0]));
}

TEST(big_endian, write_u8)
{
    uint8_t buffer[1] = {0};
    ::estd::write_be<uint8_t>(&buffer[0], 0x14);
    EXPECT_THAT(buffer, ElementsAre(0x14));
}

TEST(big_endian, write_u16)
{
    uint8_t buffer[2] = {0};
    ::estd::write_be<uint16_t>(&buffer[0], 0x1435);
    EXPECT_THAT(buffer, ElementsAre(0x14, 0x35));
}

TEST(big_endian, write_u32)
{
    uint8_t buffer[4] = {0};
    ::estd::write_be<uint32_t>(&buffer[0], 0x12345678);
    EXPECT_THAT(buffer, ElementsAre(0x12, 0x34, 0x56, 0x78));
}

TEST(big_endian, write_u64)
{
    uint8_t buffer[8] = {0};
    ::estd::write_be<uint64_t>(&buffer[0], 0x9876543210012409);
    EXPECT_THAT(buffer, ElementsAre(0x98, 0x76, 0x54, 0x32, 0x10, 0x01, 0x24, 0x09));
}

TEST(big_endian, read_8)
{
    uint8_t const buffer[] = {0x12};
    EXPECT_EQ(18, ::estd::read_be<int8_t>(&buffer[0]));
}

TEST(big_endian, read_16)
{
    uint8_t const buffer[] = {0xAB, 0x12};
    EXPECT_EQ(-21742, ::estd::read_be<int16_t>(&buffer[0]));
}

TEST(big_endian, read_32)
{
    uint8_t const buffer[] = {0x13, 0x42, 0x32, 0x05};
    EXPECT_EQ(323105285, ::estd::read_be<int32_t>(&buffer[0]));
}

TEST(big_endian, read_64)
{
    uint8_t const buffer[] = {0x49, 0x56, 0xA6, 0x5B, 0x34, 0xBB, 0x66, 0x15};
    EXPECT_EQ(5284594123422787093, ::estd::read_be<int64_t>(&buffer[0]));
}

TEST(big_endian, write_8)
{
    uint8_t buffer[1] = {0};
    ::estd::write_be<int8_t>(&buffer[0], 0x14);
    EXPECT_THAT(buffer, ElementsAre(0x14));
}

TEST(big_endian, write_16)
{
    uint8_t buffer[2] = {0};
    ::estd::write_be<int16_t>(&buffer[0], 0x1435);
    EXPECT_THAT(buffer, ElementsAre(0x14, 0x35));
}

TEST(big_endian, write_32)
{
    uint8_t buffer[4] = {0};
    ::estd::write_be<int32_t>(&buffer[0], 0x12345678);
    EXPECT_THAT(buffer, ElementsAre(0x12, 0x34, 0x56, 0x78));
}

TEST(big_endian, write_64)
{
    uint8_t buffer[8] = {0};
    ::estd::write_be<int64_t>(&buffer[0], 0x9876543210012409);
    EXPECT_THAT(buffer, ElementsAre(0x98, 0x76, 0x54, 0x32, 0x10, 0x01, 0x24, 0x09));
}

TEST(big_endian, be_uint16_assignment)
{
    ::estd::be_uint16_t t;
    t = 0x2764;
    EXPECT_THAT(t.bytes, ElementsAre(0x27, 0x64));
}

TEST(big_endian, be_uint16_cast_to_primitive)
{
    ::estd::be_uint16_t const t = {0x11, 0x54};
    EXPECT_EQ(0x1154U, t);
}

TEST(big_endian, be_uint16_make)
{
    ::estd::be_uint16_t const t = ::estd::be_uint16_t::make(0x6754);
    EXPECT_THAT(t.bytes, ElementsAre(0x67, 0x54));
}

TEST(big_endian, be_uint32_assignment)
{
    ::estd::be_uint32_t t;
    t = 0x23456789;
    EXPECT_THAT(t.bytes, ElementsAre(0x23, 0x45, 0x67, 0x89));
}

TEST(big_endian, be_uint32_cast_to_primitive)
{
    ::estd::be_uint32_t const t = {0x69, 0x23, 0x49, 0x26};
    EXPECT_EQ(0x69234926U, t);
}

TEST(big_endian, be_uint32_make)
{
    ::estd::be_uint32_t const t = ::estd::be_uint32_t::make(0x90349807);
    EXPECT_THAT(t.bytes, ElementsAre(0x90, 0x34, 0x98, 0x07));
}

TEST(big_endian, be_uint64_assignment)
{
    ::estd::be_uint64_t t;
    t = 0x5003611288963214;
    EXPECT_THAT(t.bytes, ElementsAre(0x50, 0x03, 0x61, 0x12, 0x88, 0x96, 0x32, 0x14));
}

TEST(big_endian, be_uint64_cast_to_primitive)
{
    ::estd::be_uint64_t const t = {0xFF, 0xFF, 0xAB, 0xBA, 0x00, 0x09, 0x67, 0xE4};
    EXPECT_EQ(0xFFFFABBA000967E4U, t);
}

TEST(big_endian, be_uint64_make)
{
    ::estd::be_uint64_t const t = ::estd::be_uint64_t::make(0x1234567890349807);
    EXPECT_THAT(t.bytes, ElementsAre(0x12, 0x34, 0x56, 0x78, 0x90, 0x34, 0x98, 0x07));
}

TEST(big_endian, be_uint24_assignment)
{
    ::estd::be_uint24_t t;
    t = 0x12276456;
    EXPECT_THAT(t.bytes, ElementsAre(0x27, 0x64, 0x56));
}

TEST(big_endian, be_uint24_cast_to_primitive)
{
    ::estd::be_uint24_t const t = {0x11, 0x54, 0x78};
    EXPECT_EQ(0x115478U, t);
}

TEST(big_endian, be_uint24_make)
{
    ::estd::be_uint24_t const t = ::estd::be_uint24_t::make(0x67541235);
    EXPECT_THAT(t.bytes, ElementsAre(0x54, 0x12, 0x35));
}

TEST(big_endian, read_24)
{
    uint8_t const buffer[] = {0x13, 0x32, 0x05};
    EXPECT_EQ(0x00133205U, ::estd::read_be_24(&buffer[0]));
}

TEST(big_endian, write_24)
{
    uint8_t buffer[3] = {0};
    ::estd::write_be_24(&buffer[0], 0x13562897U);
    EXPECT_THAT(buffer, ElementsAre(0x56, 0x28, 0x97));
}

TEST(big_endian, be_uint48_assignment)
{
    ::estd::be_uint48_t t;
    t = 0x1227645612304590;
    EXPECT_THAT(t.bytes, ElementsAre(0x64, 0x56, 0x12, 0x30, 0x45, 0x90));
}

TEST(big_endian, be_uint48_cast_to_primitive)
{
    ::estd::be_uint48_t const t = {0x11, 0x54, 0x46, 0x11, 0x51, 0x78};
    EXPECT_EQ(0x115446115178U, t);
}

TEST(big_endian, be_uint48_make)
{
    ::estd::be_uint48_t const t = ::estd::be_uint48_t::make(0x5008019934541235);
    EXPECT_THAT(t.bytes, ElementsAre(0x01, 0x99, 0x34, 0x54, 0x12, 0x35));
}

TEST(big_endian, read_48)
{
    uint8_t const buffer[] = {0x49, 0x56, 0x34, 0xBB, 0x66, 0x15};
    EXPECT_EQ(0x0000495634BB6615U, ::estd::read_be_48(&buffer[0]));
}

TEST(big_endian, write_48)
{
    uint8_t buffer[6] = {0};
    ::estd::write_be_48(&buffer[0], 0x8978675645342312U);
    EXPECT_THAT(buffer, ElementsAre(0x67, 0x56, 0x45, 0x34, 0x23, 0x12));
}

/**
 * \desc
 * Tests reading 0 length BE values from buffer, defaults to 0
 */
TEST(big_endian, read_be_bits_zero_length)
{
    constexpr auto NUM_BITS        = sizeof(uint64_t) * 8;
    uint8_t const buffer[NUM_BITS] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    for (size_t offset = 0; offset < NUM_BITS; ++offset)
    {
        EXPECT_EQ(0, ::estd::read_be_bits<uint8_t>(&buffer[0], offset, 0));
        EXPECT_EQ(0, ::estd::read_be_bits<uint16_t>(&buffer[0], offset, 0));
        EXPECT_EQ(0, ::estd::read_be_bits<uint32_t>(&buffer[0], offset, 0));
        EXPECT_EQ(0, ::estd::read_be_bits<uint64_t>(&buffer[0], offset, 0));
    }
}

template<typename T, T ExpectValue>
struct ReadBeBitsParams
{
    using ValueType                         = T;
    static constexpr ValueType EXPECT_VALUE = ExpectValue;
    static constexpr auto NUM_BITS          = sizeof(T) * 8;
    uint8_t _buffer[sizeof(T) * 2];

    ReadBeBitsParams()
    {
        auto stackValue = ::estd::big_endian<T>::make(ExpectValue);
        auto s          = ::estd::slice<uint8_t>(_buffer);
        (void)::estd::memory::copy(
            ::estd::memory::split(s, sizeof(stackValue.bytes)), stackValue.bytes);
        (void)::estd::memory::copy(
            ::estd::memory::split(s, sizeof(stackValue.bytes)), stackValue.bytes);
    }
};

template<typename Parameters>
struct ReadBeBitsTest : public ::testing::Test
{
    using Params = Parameters;
    Params params;
};

using ReadBeBitsTypes = testing::Types<
    ReadBeBitsParams<uint8_t, 0xA5>,
    ReadBeBitsParams<uint16_t, 0xFADE>,
    ReadBeBitsParams<uint32_t, 0xCAFEBABE>,
    ReadBeBitsParams<uint64_t, 0xDEADBEEFFEEDFACE>>;

TYPED_TEST_SUITE(ReadBeBitsTest, ReadBeBitsTypes);

/**
 * \desc
 * Tests reading BE 0-NUM_BITS bits with 0-NUM_BITS offsets into a templated Type
 */
TYPED_TEST(ReadBeBitsTest, read_be_bits)
{
    using Params            = typename TestFixture::Params;
    using Type              = typename Params::ValueType;
    constexpr auto NUM_BITS = Params::NUM_BITS;
    using NumBitsType       = typename ::std::remove_const<decltype(NUM_BITS)>::type;

    for (NumBitsType offset = 0; offset < NUM_BITS; ++offset)
    {
        for (NumBitsType numBits = 1; numBits <= (NUM_BITS - offset); ++numBits)
        {
            Type mask         = std::numeric_limits<Type>::max() >> (NUM_BITS - numBits);
            Type shift        = NUM_BITS - offset - numBits;
            Type const expect = (Params::EXPECT_VALUE >> shift) & mask;
            EXPECT_EQ(
                expect, ::estd::read_be_bits<Type>(&this->params._buffer[0], offset, numBits));
        }
    }
}

/**
 * \desc
 * Tests reading BE bits with incremental offsets into a uint8_t and uint16_t
 */
TEST(big_endian, read_be_bits_larger_offset)
{
    constexpr auto NUM_BITS            = sizeof(uint8_t) * 8;
    uint8_t const buffer[NUM_BITS * 4] = {0xA5, 0xA5, 0xA5, 0xA5};

    EXPECT_EQ(0xA5, ::estd::read_be_bits<uint8_t>(&buffer[0], 0, 8));
    EXPECT_EQ(0x5A, ::estd::read_be_bits<uint8_t>(&buffer[0], 4, 8));
    EXPECT_EQ(0xA5, ::estd::read_be_bits<uint8_t>(&buffer[0], 8, 8));
    EXPECT_EQ(0x5A, ::estd::read_be_bits<uint8_t>(&buffer[0], 12, 8));
    EXPECT_EQ(0xA5, ::estd::read_be_bits<uint8_t>(&buffer[0], 16, 8));
    EXPECT_EQ(0x5A, ::estd::read_be_bits<uint8_t>(&buffer[0], 20, 8));
    EXPECT_EQ(0xA5, ::estd::read_be_bits<uint8_t>(&buffer[0], 24, 8));
    EXPECT_EQ(0x05, ::estd::read_be_bits<uint8_t>(&buffer[0], 28, 4));

    EXPECT_EQ(0xA5A5, ::estd::read_be_bits<uint16_t>(&buffer[0], 0, 16));
    EXPECT_EQ(0x5A5A, ::estd::read_be_bits<uint16_t>(&buffer[0], 4, 16));
    EXPECT_EQ(0xA5A5, ::estd::read_be_bits<uint16_t>(&buffer[0], 8, 16));
    EXPECT_EQ(0x5A5A, ::estd::read_be_bits<uint16_t>(&buffer[0], 12, 16));
    EXPECT_EQ(0xA5A5, ::estd::read_be_bits<uint16_t>(&buffer[0], 16, 16));
}

/**
 * \desc
 * Tests writing 0 length BE bits, should not change the dest buffer.
 */
TEST(big_endian, write_be_bits_zero_length)
{
    constexpr auto NUM_BYTES = sizeof(uint64_t);
    constexpr auto NUM_BITS  = NUM_BYTES * 8;

    uint8_t dest[NUM_BYTES];
    ::estd::memory::set(dest, 0);
    uint8_t expect[NUM_BYTES];
    ::estd::memory::set(expect, 0);

    for (size_t offset = 0; offset <= NUM_BITS; ++offset)
    {
        ::estd::write_be_bits<uint8_t>(&dest[0], 0xFF, offset, 0);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_be_bits<uint16_t>(&dest[0], 0xFFFF, offset, 0);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_be_bits<uint32_t>(&dest[0], 0xFFFFFFFF, offset, 0);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_be_bits<uint64_t>(&dest[0], 0xFFFFFFFFFFFFFFFF, offset, 0);
        EXPECT_THAT(dest, ElementsAreArray(expect));
    }
}

/**
 * \desc
 * Tests writing 1 BE bit at a time.
 */
TEST(big_endian, write_be_bits_one_bit_into_one_byte)
{
    constexpr auto NUM_BYTES = sizeof(uint64_t);
    constexpr auto NUM_BITS  = NUM_BYTES * 8;
    uint8_t dest[NUM_BYTES];
    uint8_t expect[NUM_BYTES];

    for (size_t offset = 0; offset < NUM_BITS; ++offset)
    {
        ::estd::memory::set(dest, 0);
        ::estd::memory::set(expect, 0);
        expect[offset / 8] = 0b10000000;
        expect[offset / 8] >>= offset % 8;
        ::estd::write_be_bits<uint8_t>(&dest[0], 0xFF, offset, 1);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_be_bits<uint16_t>(&dest[0], 0xFFFF, offset, 1);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_be_bits<uint32_t>(&dest[0], 0xFFFFFFFF, offset, 1);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_be_bits<uint64_t>(&dest[0], 0xFFFFFFFFFFFFFFFF, offset, 1);
        EXPECT_THAT(dest, ElementsAreArray(expect));
    }
}

/**
 * \desc
 * Tests writing 1 byte and 1 bit as BE bits at a time.
 */
TEST(big_endian, write_be_bits_one_byte_and_a_bit_length)
{
    constexpr auto NUM_BYTES = sizeof(uint64_t);
    constexpr auto NUM_BITS  = NUM_BYTES * 8;
    constexpr auto LENGTH    = 9;
    uint8_t dest[NUM_BYTES];
    uint8_t expect[NUM_BYTES];

    for (size_t offset = 0; offset <= NUM_BITS - LENGTH; ++offset)
    {
        ::estd::memory::set(dest, 0);
        ::estd::memory::set(expect, 0);
        expect[offset / 8] = 0xFF;
        expect[offset / 8] >>= offset % 8;
        expect[offset / 8 + 1] = 0xFF;
        expect[offset / 8 + 1] <<= 7 - (offset % 8);
        ::estd::write_be_bits<uint16_t>(&dest[0], 0xFFFF, offset, LENGTH);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_be_bits<uint32_t>(&dest[0], 0xFFFFFFFF, offset, LENGTH);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_be_bits<uint64_t>(&dest[0], 0xFFFFFFFFFFFFFFFF, offset, LENGTH);
        EXPECT_THAT(dest, ElementsAreArray(expect));
    }
}

template<typename T, T TestValue>
struct WriteBeBitsParams
{
    using ValueType                       = T;
    static constexpr ValueType TEST_VALUE = TestValue;
    static constexpr auto NUM_BITS        = sizeof(T) * 8;
    uint8_t _dest[sizeof(T) * 2];
    uint8_t _expectArray[sizeof(T) * 2];

    WriteBeBitsParams()
    {
        ::estd::memory::set(_dest, 0);
        ::estd::memory::set(_expectArray, 0);
    }
};

template<typename Parameters>
struct WriteBeBitsTest : public ::testing::Test
{
    using Params = Parameters;
    Params params;
};

using WriteBeBitsTypes = testing::Types<
    WriteBeBitsParams<uint8_t, 0xA5>,
    WriteBeBitsParams<uint16_t, 0xFADE>,
    WriteBeBitsParams<uint32_t, 0xCAFEBABE>,
    WriteBeBitsParams<uint64_t, 0xDEADBEEFFEEDFACE>>;

TYPED_TEST_SUITE(WriteBeBitsTest, WriteBeBitsTypes);

/**
 * \desc
 * Tests writing BE 0-NUM_BITS bits with 0-NUM_BITS offsets into a templated Type
 */
TYPED_TEST(WriteBeBitsTest, write_be_bits)
{
    using Params            = typename TestFixture::Params;
    using Type              = typename Params::ValueType;
    constexpr auto NUM_BITS = Params::NUM_BITS;
    auto& dest              = this->params._dest;
    auto& expectArray       = this->params._expectArray;

    for (size_t offset = 0; offset <= NUM_BITS; ++offset)
    {
        for (size_t length = 1; length <= NUM_BITS; ++length)
        {
            // This is all test code to get to the same results as the code under test
            uint8_t tempOffset = offset;
            uint8_t tempLength = length;
            Type tempVal       = Params::TEST_VALUE;
            ::estd::memory::set(expectArray, 0);
            size_t idx = 0;
            idx += tempOffset / 8;
            tempOffset %= 8;
            idx += (tempOffset + length) / 8;

            // Only write to the final byte if the write spans multiple bytes
            if (tempOffset + tempLength >= 8)
            {
                uint8_t const lastLength = (tempOffset + tempLength - 8) % 8;
                if (lastLength != 0)
                {
                    uint8_t const lastDstShift = 8 - lastLength;
                    uint8_t const lastSrcMask
                        = ::std::numeric_limits<uint8_t>::max() >> lastDstShift;
                    if (idx < sizeof(expectArray))
                    {
                        expectArray[idx] = (tempVal & lastSrcMask) << lastDstShift;
                    }
                    tempVal >>= lastLength;
                    tempLength -= lastLength;
                }
                --idx;
            }

            // Full byte copies
            while (tempLength >= 8)
            {
                expectArray[idx] = tempVal & 0xFF;
                if (sizeof(Type) > 1)
                {
                    tempVal >>= 8;
                }
                --idx;
                tempLength -= 8;
            }

            // First byte write. Only write if there's anything to write at the beginning
            if (tempOffset < 8 && tempLength != 0)
            {
                uint8_t const firstSrcMask
                    = ::std::numeric_limits<uint8_t>::max() >> (8 - tempLength);
                uint8_t const firstDstShift = 8 - (tempLength + tempOffset);
                expectArray[idx]            = (tempVal & firstSrcMask) << firstDstShift;
            }

            ::estd::memory::set(dest, 0);
            ::estd::write_be_bits<Type>(&dest[0], Params::TEST_VALUE, offset, length);
            EXPECT_THAT(dest, ElementsAreArray(expectArray));
        }
    }
}
