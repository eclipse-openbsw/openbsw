// Copyright 2024 Accenture.

#include "estd/little_endian.h"

#include "estd/memory.h"

#include <gmock/gmock.h>

using namespace ::testing;

void example_little_endian_read()
{
    // [EXAMPLE_BEGIN read le]
    // Example demonstrating reading little-endian values of various types
    uint8_t const b1[] = {0xAB, 0x14};
    EXPECT_EQ(0x14ABU, ::estd::read_le<uint16_t>(&b1[0]));

    uint8_t const b2[] = {0x13, 0x42, 0x32, 0x05};
    EXPECT_EQ(0x05324213U, ::estd::read_le<uint32_t>(&b2[0]));

    uint8_t const b3[] = {0x13, 0xAB};
    EXPECT_EQ(-21741, ::estd::read_le<int16_t>(&b3[0]));

    uint8_t const b4[] = {0x49, 0x56, 0xA6, 0x5B, 0x34, 0xBB, 0x66, 0xFF};
    EXPECT_EQ(-43141312863119799, ::estd::read_le<int64_t>(&b4[0]));
    // [EXAMPLE_END read le]
}

void example_little_endian_write()
{
    // [EXAMPLE_BEGIN write le]
    // Example demonstrating writing little-endian values of various types
    uint8_t b1[1] = {0};
    ::estd::write_le<uint8_t>(&b1[0], 0x14);
    EXPECT_THAT(b1, ElementsAre(0x14));

    uint8_t b2[2] = {0};
    ::estd::write_le<uint16_t>(&b2[0], 0x1435);
    EXPECT_THAT(b2, ElementsAre(0x35, 0x14));

    uint8_t b3[8] = {0};
    ::estd::write_le<uint64_t>(&b3[0], 0x9876543210012409);
    EXPECT_THAT(b3, ElementsAre(0x09, 0x24, 0x01, 0x10, 0x32, 0x54, 0x76, 0x98));

    uint8_t b4[4] = {0};
    ::estd::write_le<int32_t>(&b4[0], -12345678);
    EXPECT_THAT(b4, ElementsAre(0xB2, 0x9E, 0x43, 0xFF));
    // [EXAMPLE_END write le]
}

void example_little_endian_assignment()
{
    // [EXAMPLE_BEGIN assignment]
    // Assigning a little-endian value to a structure and checking the byte representation
    ::estd::le_uint64_t t;
    t = 0x1432968812610350;
    EXPECT_THAT(t.bytes, ElementsAre(0x50, 0x03, 0x61, 0x12, 0x88, 0x96, 0x32, 0x14));
    // [EXAMPLE_END assignment]
}

void example_little_endian_cast_to_primitive()
{
    // [EXAMPLE_BEGIN cast to primitive]
    // Casting a little-endian structure to a primitive type and checking the result
    ::estd::le_uint64_t const t = {0xFF, 0xFF, 0xAB, 0xBA, 0x00, 0x09, 0x67, 0xE4};
    EXPECT_EQ(0xE4670900BAABFFFFU, t);
    // [EXAMPLE_END cast to primitive]
}

void example_little_endian_make()
{
    // [EXAMPLE_BEGIN make]
    // Creating a little-endian structure from a primitive value and checking the byte
    // representation
    ::estd::le_uint64_t const t = ::estd::le_uint64_t::make(0x0798349078563412);
    EXPECT_THAT(t.bytes, ElementsAre(0x12, 0x34, 0x56, 0x78, 0x90, 0x34, 0x98, 0x07));
    // [EXAMPLE_END make]
}

void example_little_endian_make_le()
{
    // [EXAMPLE_BEGIN make le]
    // Creating a little-endian structure from a primitive value using a template function and
    // checking the byte representation
    ::estd::le_uint64_t const t = ::estd::make_le<uint64_t>(0x0798349078563412);
    EXPECT_THAT(t.bytes, ElementsAre(0x12, 0x34, 0x56, 0x78, 0x90, 0x34, 0x98, 0x07));
    // [EXAMPLE_END make le]
}

void example_little_endian_read_le_24()
{
    // [EXAMPLE_BEGIN read le 24]
    // Reading a little-endian 24-bit value from a byte array and checking the result
    uint8_t const buffer[] = {0x13, 0x32, 0x05};
    EXPECT_EQ(0x00053213U, ::estd::read_le_24(&buffer[0]));
    // [EXAMPLE_END read le 24]
}

void example_little_endian_write_le_24()
{
    // [EXAMPLE_BEGIN write le 24]
    // Writing a little-endian 24-bit value to a byte array and checking the result
    uint8_t buffer[3] = {0};
    ::estd::write_le_24(&buffer[0], 0x13562897U);
    EXPECT_THAT(buffer, ElementsAre(0x97, 0x28, 0x56));
    // [EXAMPLE_END write le 24]
}

void example_little_endian_read_le_48()
{
    // [EXAMPLE_BEGIN read le 48]
    // Reading a little-endian 48-bit value from a byte array and checking the result
    uint8_t const buffer[] = {0x13, 0x32, 0x11, 0x44, 0x59, 0x73};
    EXPECT_EQ(0x0000735944113213U, ::estd::read_le_48(&buffer[0]));
    // [EXAMPLE_END read le 48]
}

void example_little_endian_write_le_48()
{
    // [EXAMPLE_BEGIN write le 48]
    // Writing a little-endian 48-bit value to a byte array and checking the result
    uint8_t buffer[6] = {0};
    ::estd::write_le_48(&buffer[0], 0x5199881234567899U);
    EXPECT_THAT(buffer, ElementsAre(0x99, 0x78, 0x56, 0x34, 0x12, 0x88));
    // [EXAMPLE_END write le 48]
}

void example_little_endian_read_le_bits()
{
    // [EXAMPLE_BEGIN read le bits]
    constexpr auto NUM_BITS            = sizeof(uint8_t) * 8;
    uint8_t const buffer[NUM_BITS * 4] = {0xA5, 0xA5, 0xA5, 0xA5};

    // Reading 8 bits from the buffer starting at position 8 and expecting the result to be 0xA5
    EXPECT_EQ(0xA5, ::estd::read_le_bits<uint8_t>(&buffer[0], 8, 8));
    // Reading 8 bits from the buffer starting at position 12 and expecting the result to be 0xA5
    EXPECT_EQ(0xA5, ::estd::read_le_bits<uint8_t>(&buffer[0], 12, 8));
    // Reading 8 bits from the buffer starting at position 16 and expecting the result to be 0xA5
    EXPECT_EQ(0xA5, ::estd::read_le_bits<uint8_t>(&buffer[0], 16, 8));

    // Reading 16 bits from the buffer starting at position 12 and expecting the result to be 0xAA55
    EXPECT_EQ(0xAA55, ::estd::read_le_bits<uint16_t>(&buffer[0], 12, 16));
    // Reading 16 bits from the buffer starting at position 16 and expecting the result to be 0xAA55
    EXPECT_EQ(0xA5A5, ::estd::read_le_bits<uint16_t>(&buffer[0], 16, 16));
    // [EXAMPLE_END read le bits]
}

void example_little_endian_write_le_bits()
{
    // [EXAMPLE_BEGIN write le bits]

    constexpr auto NUM_BYTES = sizeof(uint32_t);
    uint8_t dest[NUM_BYTES];
    // Writing 24 bits (3 bytes) to the destination array starting at position 8
    ::estd::memory::set(static_cast<::estd::slice<uint8_t>>(dest), 0);
    ::estd::write_le_bits<uint64_t>(&dest[0], 0x32589945, 8, 24);
    EXPECT_THAT(dest, ElementsAre(0x00, 0x45, 0x99, 0x58));
    // Writing 24 bits (3 bytes) to the destination array starting at position 4
    ::estd::memory::set(static_cast<::estd::slice<uint8_t>>(dest), 0);
    ::estd::write_le_bits<uint64_t>(&dest[0], 0x32589945, 4, 24);
    EXPECT_THAT(dest, ElementsAre(0x05, 0x94, 0x89, 0x50));

    // [EXAMPLE_END write le bits]
}

TEST(little_endian, run_examples)
{
    example_little_endian_read();
    example_little_endian_write();
    example_little_endian_assignment();
    example_little_endian_cast_to_primitive();
    example_little_endian_make();
    example_little_endian_make_le();
    example_little_endian_read_le_24();
    example_little_endian_write_le_24();
    example_little_endian_read_le_48();
    example_little_endian_write_le_48();
    example_little_endian_read_le_bits();
    example_little_endian_write_le_bits();
}
