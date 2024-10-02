// Copyright 2024 Accenture.

#include "estd/little_endian.h"

#include "estd/big_endian.h"
#include "estd/memory.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

TEST(little_endian, read_u8)
{
    uint8_t const buffer[] = {0x12};
    EXPECT_EQ(0x12U, ::estd::read_le<uint8_t>(&buffer[0]));
}

TEST(little_endian, read_u16)
{
    uint8_t const buffer[] = {0xAB, 0x12};
    EXPECT_EQ(0x12ABU, ::estd::read_le<uint16_t>(&buffer[0]));
}

TEST(little_endian, read_u32)
{
    uint8_t const buffer[] = {0x13, 0x42, 0x32, 0x05};
    EXPECT_EQ(0x05324213U, ::estd::read_le<uint32_t>(&buffer[0]));
}

TEST(little_endian, read_u64)
{
    uint8_t const buffer[] = {0x49, 0x56, 0xA6, 0x5B, 0x34, 0xBB, 0x66, 0x15};
    EXPECT_EQ(0x1566BB345BA65649U, ::estd::read_le<uint64_t>(&buffer[0]));
}

TEST(little_endian, write_u8)
{
    uint8_t buffer[1] = {0};
    ::estd::write_le<uint8_t>(&buffer[0], 0x14);
    EXPECT_THAT(buffer, ElementsAre(0x14));
}

TEST(little_endian, write_u16)
{
    uint8_t buffer[2] = {0};
    ::estd::write_le<uint16_t>(&buffer[0], 0x1435);
    EXPECT_THAT(buffer, ElementsAre(0x35, 0x14));
}

TEST(little_endian, write_u32)
{
    uint8_t buffer[4] = {0};
    ::estd::write_le<uint32_t>(&buffer[0], 0x12345678);
    EXPECT_THAT(buffer, ElementsAre(0x78, 0x56, 0x34, 0x12));
}

TEST(little_endian, write_u64)
{
    uint8_t buffer[8] = {0};
    ::estd::write_le<uint64_t>(&buffer[0], 0x9876543210012409);
    EXPECT_THAT(buffer, ElementsAre(0x09, 0x24, 0x01, 0x10, 0x32, 0x54, 0x76, 0x98));
}

TEST(little_endian, read_8)
{
    uint8_t const buffer[] = {0xFF};
    EXPECT_EQ(-1, ::estd::read_le<int8_t>(&buffer[0]));
}

TEST(little_endian, read_16)
{
    uint8_t const buffer[] = {0x12, 0xAB};
    EXPECT_EQ(-21742, ::estd::read_le<int16_t>(&buffer[0]));
}

TEST(little_endian, read_32)
{
    uint8_t const buffer[] = {0x13, 0x42, 0x32, 0xF5};
    EXPECT_EQ(-181255661, ::estd::read_le<int32_t>(&buffer[0]));
}

TEST(little_endian, read_64)
{
    uint8_t const buffer[] = {0x49, 0x56, 0xA6, 0x5B, 0x34, 0xBB, 0x66, 0xFF};
    EXPECT_EQ(-43141312863119799, ::estd::read_le<int64_t>(&buffer[0]));
}

TEST(little_endian, write_8)
{
    uint8_t buffer[1] = {0};
    ::estd::write_le<int8_t>(&buffer[0], -5);
    EXPECT_THAT(buffer, ElementsAre(0xFB));
}

TEST(little_endian, write_16)
{
    uint8_t buffer[2] = {0};
    ::estd::write_le<int16_t>(&buffer[0], -4561);
    EXPECT_THAT(buffer, ElementsAre(0x2F, 0xEE));
}

TEST(little_endian, write_32)
{
    uint8_t buffer[4] = {0};
    ::estd::write_le<int32_t>(&buffer[0], -12345678);
    EXPECT_THAT(buffer, ElementsAre(0xB2, 0x9E, 0x43, 0xFF));
}

TEST(little_endian, write_64)
{
    uint8_t buffer[8] = {0};
    ::estd::write_le<int64_t>(&buffer[0], -8562340781075);
    EXPECT_THAT(buffer, ElementsAre(0xED, 0x23, 0xB8, 0x6C, 0x36, 0xF8, 0xFF, 0xFF));
}

TEST(little_endian, le_uint16_assignment)
{
    ::estd::le_uint16_t t;
    t = 0x2764;
    EXPECT_THAT(t.bytes, ElementsAre(0x64, 0x27));
}

TEST(little_endian, le_uint16_cast_to_primitive)
{
    ::estd::le_uint16_t const t = {0x11, 0x54};
    EXPECT_EQ(0x5411U, t);
}

TEST(little_endian, le_uint16_make)
{
    ::estd::le_uint16_t const t = ::estd::le_uint16_t::make(0x6754);
    EXPECT_THAT(t.bytes, ElementsAre(0x54, 0x67));
}

TEST(little_endian, le_make_uint16_templated)
{
    ::estd::le_uint16_t const t = ::estd::make_le<uint16_t>(0x6754);
    EXPECT_THAT(t.bytes, ElementsAre(0x54, 0x67));
}

TEST(little_endian, le_uint32_assignment)
{
    ::estd::le_uint32_t t;
    t = 0x89674523;
    EXPECT_THAT(t.bytes, ElementsAre(0x23, 0x45, 0x67, 0x89));
}

TEST(little_endian, le_uint32_cast_to_primitive)
{
    ::estd::le_uint32_t const t = {0x69, 0x23, 0x49, 0x26};
    EXPECT_EQ(0x26492369U, t);
}

TEST(little_endian, le_uint32_make)
{
    ::estd::le_uint32_t const t = ::estd::le_uint32_t::make(0x07983490);
    EXPECT_THAT(t.bytes, ElementsAre(0x90, 0x34, 0x98, 0x07));
}

TEST(little_endian, le_make_uint32_templated)
{
    ::estd::le_uint32_t const t = ::estd::make_le<uint32_t>(0x07983490);
    EXPECT_THAT(t.bytes, ElementsAre(0x90, 0x34, 0x98, 0x07));
}

TEST(little_endian, le_uint64_assignment)
{
    ::estd::le_uint64_t t;
    t = 0x1432968812610350;
    EXPECT_THAT(t.bytes, ElementsAre(0x50, 0x03, 0x61, 0x12, 0x88, 0x96, 0x32, 0x14));
}

TEST(little_endian, le_uint64_cast_to_primitive)
{
    ::estd::le_uint64_t const t = {0xFF, 0xFF, 0xAB, 0xBA, 0x00, 0x09, 0x67, 0xE4};
    EXPECT_EQ(0xE4670900BAABFFFFU, t);
}

TEST(little_endian, le_uint64_make)
{
    ::estd::le_uint64_t const t = ::estd::le_uint64_t::make(0x0798349078563412);
    EXPECT_THAT(t.bytes, ElementsAre(0x12, 0x34, 0x56, 0x78, 0x90, 0x34, 0x98, 0x07));
}

TEST(little_endian, le_make_uint64_templated)
{
    ::estd::le_uint64_t const t = ::estd::make_le<uint64_t>(0x0798349078563412);
    EXPECT_THAT(t.bytes, ElementsAre(0x12, 0x34, 0x56, 0x78, 0x90, 0x34, 0x98, 0x07));
}

TEST(little_endian, read_24)
{
    uint8_t const buffer[] = {0x13, 0x32, 0x05};
    EXPECT_EQ(0x00053213U, ::estd::read_le_24(&buffer[0]));
}

TEST(little_endian, write_24)
{
    uint8_t buffer[3] = {0};
    ::estd::write_le_24(&buffer[0], 0x13562897U);
    EXPECT_THAT(buffer, ElementsAre(0x97, 0x28, 0x56));
}

TEST(little_endian, read_48)
{
    uint8_t const buffer[] = {0x13, 0x32, 0x11, 0x44, 0x59, 0x73};
    EXPECT_EQ(0x0000735944113213U, ::estd::read_le_48(&buffer[0]));
}

TEST(little_endian, write_48)
{
    uint8_t buffer[6] = {0};
    ::estd::write_le_48(&buffer[0], 0x5199881234567899U);
    EXPECT_THAT(buffer, ElementsAre(0x99, 0x78, 0x56, 0x34, 0x12, 0x88));
}

/**
 * \desc
 * Tests reading 0 length LE values from buffer, defaults to 0
 */
TEST(little_endian, read_le_bits_zero_length)
{
    constexpr auto NUM_BITS        = sizeof(uint64_t) * 8;
    uint8_t const buffer[NUM_BITS] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    for (size_t offset = 0; offset < NUM_BITS; ++offset)
    {
        EXPECT_EQ(0, ::estd::read_le_bits<uint8_t>(&buffer[0], offset, 0));
        EXPECT_EQ(0, ::estd::read_le_bits<uint16_t>(&buffer[0], offset, 0));
        EXPECT_EQ(0, ::estd::read_le_bits<uint32_t>(&buffer[0], offset, 0));
        EXPECT_EQ(0, ::estd::read_le_bits<uint64_t>(&buffer[0], offset, 0));
    }
}

template<typename T, T ExpectValue>
struct ReadLeBitsParams
{
    using ValueType                         = T;
    static constexpr ValueType EXPECT_VALUE = ExpectValue;
    static constexpr auto NUM_BITS          = sizeof(T) * 8;
    uint8_t _buffer[sizeof(T) * 2];

    ReadLeBitsParams()
    {
        auto stackValue = ::estd::little_endian<T>::make(ExpectValue);
        auto s          = ::estd::slice<uint8_t>(_buffer);
        (void)::estd::memory::copy(
            ::estd::memory::split(s, sizeof(stackValue.bytes)), stackValue.bytes);
        (void)::estd::memory::copy(
            ::estd::memory::split(s, sizeof(stackValue.bytes)), stackValue.bytes);
    }
};

template<typename Parameters>
struct ReadLeBitsTest : public ::testing::Test
{
    using Params = Parameters;
    Params params;
};

using ReadLeBitsTypes = testing::Types<
    ReadLeBitsParams<uint8_t, 0xA5>,
    ReadLeBitsParams<uint16_t, 0xFADE>,
    ReadLeBitsParams<uint32_t, 0xCAFEBABE>,
    ReadLeBitsParams<uint64_t, 0xDEADBEEFFEEDFACE>>;

TYPED_TEST_SUITE(ReadLeBitsTest, ReadLeBitsTypes);

/**
 * \desc
 * Tests reading LE 0-NUM_BITS bits with 0-NUM_BITS offsets into a templated Type
 */
TYPED_TEST(ReadLeBitsTest, read_le_bits)
{
    using Params            = typename TestFixture::Params;
    using Type              = typename Params::ValueType;
    constexpr auto NUM_BITS = Params::NUM_BITS;
    using NumBitsType       = typename ::std::remove_const<decltype(NUM_BITS)>::type;
    auto& source            = this->params._buffer;

    for (NumBitsType offset = 0; offset < NUM_BITS; ++offset)
    {
        for (NumBitsType numBits = 1;
             numBits <= ::estd::min(NUM_BITS - offset, static_cast<NumBitsType>(1));
             ++numBits)
        {
            Type mask  = std::numeric_limits<Type>::max() >> (NUM_BITS - numBits);
            Type shift = NUM_BITS - offset - numBits;
            uint8_t tempBuffer[sizeof(Type)];
            ::estd::write_be<Type>(&tempBuffer[0], Params::EXPECT_VALUE);
            auto const tempExpect = ::estd::read_le<Type>(tempBuffer);
            Type const expect     = (tempExpect >> shift) & mask;
            EXPECT_EQ(expect, ::estd::read_le_bits<Type>(&source[0], offset, numBits));
        }
    }
}

/**
 * \desc
 * Tests reading LE bits with incremental offsets into a uint8_t and uint16_t
 */
TEST(little_endian, read_le_bits_larger_offset)
{
    constexpr auto NUM_BITS            = sizeof(uint8_t) * 8;
    uint8_t const buffer[NUM_BITS * 4] = {0xA5, 0xA5, 0xA5, 0xA5};

    EXPECT_EQ(0xA5, ::estd::read_le_bits<uint8_t>(&buffer[0], 0, 8));
    EXPECT_EQ(0xA5, ::estd::read_le_bits<uint8_t>(&buffer[0], 4, 8));
    EXPECT_EQ(0xA5, ::estd::read_le_bits<uint8_t>(&buffer[0], 8, 8));
    EXPECT_EQ(0xA5, ::estd::read_le_bits<uint8_t>(&buffer[0], 12, 8));
    EXPECT_EQ(0xA5, ::estd::read_le_bits<uint8_t>(&buffer[0], 16, 8));
    EXPECT_EQ(0xA5, ::estd::read_le_bits<uint8_t>(&buffer[0], 20, 8));
    EXPECT_EQ(0xA5, ::estd::read_le_bits<uint8_t>(&buffer[0], 24, 8));
    EXPECT_EQ(0x05, ::estd::read_le_bits<uint8_t>(&buffer[0], 28, 4));

    EXPECT_EQ(0xA5A5, ::estd::read_le_bits<uint16_t>(&buffer[0], 0, 16));
    EXPECT_EQ(0xAA55, ::estd::read_le_bits<uint16_t>(&buffer[0], 4, 16));
    EXPECT_EQ(0xA5A5, ::estd::read_le_bits<uint16_t>(&buffer[0], 8, 16));
    EXPECT_EQ(0xAA55, ::estd::read_le_bits<uint16_t>(&buffer[0], 12, 16));
    EXPECT_EQ(0xA5A5, ::estd::read_le_bits<uint16_t>(&buffer[0], 16, 16));
}

/**
 * \desc
 * Tests writing 0 length LE bits, should not change the dest buffer.
 */
TEST(little_endian, write_le_bits_zero_length)
{
    constexpr auto NUM_BYTES = sizeof(uint64_t);
    constexpr auto NUM_BITS  = NUM_BYTES * 8;

    uint8_t dest[NUM_BYTES];
    ::estd::memory::set(dest, 0);
    uint8_t expect[NUM_BYTES];
    ::estd::memory::set(expect, 0);

    for (size_t offset = 0; offset < NUM_BITS; ++offset)
    {
        ::estd::write_le_bits<uint8_t>(&dest[0], 0xFF, offset, 0);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_le_bits<uint16_t>(&dest[0], 0xFFFF, offset, 0);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_le_bits<uint32_t>(&dest[0], 0xFFFFFFFF, offset, 0);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_le_bits<uint64_t>(&dest[0], 0xFFFFFFFFFFFFFFFF, offset, 0);
        EXPECT_THAT(dest, ElementsAreArray(expect));
    }
}

/**
 * \desc
 * Tests writing 1 LE bit at a time.
 */
TEST(little_endian, write_le_bits_one_bit_into_one_byte)
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
        ::estd::write_le_bits<uint8_t>(&dest[0], 0xFF, offset, 1);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_le_bits<uint16_t>(&dest[0], 0xFFFF, offset, 1);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_le_bits<uint32_t>(&dest[0], 0xFFFFFFFF, offset, 1);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_le_bits<uint64_t>(&dest[0], 0xFFFFFFFFFFFFFFFF, offset, 1);
        EXPECT_THAT(dest, ElementsAreArray(expect));
    }
}

/**
 * \desc
 * Tests writing 1 byte and 1 bit as LE bits at a time.
 */
TEST(little_endian, write_le_bits_one_byte_and_a_bit_length)
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
        ::estd::write_le_bits<uint16_t>(&dest[0], 0xFFFF, offset, LENGTH);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_le_bits<uint32_t>(&dest[0], 0xFFFFFFFF, offset, LENGTH);
        EXPECT_THAT(dest, ElementsAreArray(expect));
        ::estd::write_le_bits<uint64_t>(&dest[0], 0xFFFFFFFFFFFFFFFF, offset, LENGTH);
        EXPECT_THAT(dest, ElementsAreArray(expect));
    }
}

template<typename T, T TestValue>
struct WriteLeBitsParams
{
    using ValueType                       = T;
    static constexpr ValueType TEST_VALUE = TestValue;
    static constexpr auto NUM_BITS        = sizeof(T) * 8;
    uint8_t _dest[sizeof(T) * 2];
    uint8_t _expectArray[sizeof(T) * 2];

    WriteLeBitsParams()
    {
        ::estd::memory::set(_dest, 0);
        ::estd::memory::set(_expectArray, 0);
    }
};

template<typename Parameters>
struct WriteLeBitsTest : public ::testing::Test
{
    using Params = Parameters;
    Params params;
};

using WriteLeBitsTypes = testing::Types<
    WriteLeBitsParams<uint8_t, 0xA5>,
    WriteLeBitsParams<uint16_t, 0xFADE>,
    WriteLeBitsParams<uint32_t, 0xCAFEBABE>,
    WriteLeBitsParams<uint64_t, 0xDEADBEEFFEEDFACE>>;

TYPED_TEST_SUITE(WriteLeBitsTest, WriteLeBitsTypes);

/**
 * \desc
 * Tests writing LE 0-NUM_BITS bits with 0-NUM_BITS offsets into a templated Type
 */
TYPED_TEST(WriteLeBitsTest, write_le_bits)
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
            auto tempVal       = Params::TEST_VALUE;
            ::estd::memory::set(expectArray, 0);
            size_t idx = tempOffset / 8;
            tempOffset %= 8;

            // Only write to the final byte if the write spans multiple bytes
            uint8_t const firstLength
                = ::estd::min(tempLength, static_cast<uint8_t>(8 - tempOffset));
            uint8_t const firstSrcMask = ~(::std::numeric_limits<uint8_t>::max() << firstLength);

            uint8_t const firstDstShift = 8 - ::estd::min(8, tempOffset + tempLength);
            expectArray[idx]            = (tempVal & firstSrcMask) << firstDstShift;
            tempVal >>= firstLength;
            tempLength -= firstLength;
            ++idx;

            // Full byte copies
            while (tempLength >= 8)
            {
                expectArray[idx] = tempVal & 0xFF;
                if (sizeof(Type) > 1)
                {
                    tempVal >>= 8;
                }
                ++idx;
                tempLength -= 8;
            }

            // Last byte write. Only write if there's anything to write at the beginning
            if (tempLength != 0)
            {
                uint8_t const firstDstShift = 8 - tempLength;
                uint8_t const firstSrcMask = ::std::numeric_limits<uint8_t>::max() >> firstDstShift;
                expectArray[idx]           = (tempVal & firstSrcMask) << firstDstShift;
            }

            ::estd::memory::set(dest, 0);
            ::estd::write_le_bits<Type>(&dest[0], Params::TEST_VALUE, offset, length);
            EXPECT_THAT(dest, ElementsAreArray(expectArray));
        }
    }
}
