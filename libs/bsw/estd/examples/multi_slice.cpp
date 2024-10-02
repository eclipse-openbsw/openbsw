// Copyright 2024 Accenture.

#include <estd/multi_slice.h>

#include <gtest/gtest.h>

TEST(multi_slice_Example, Construction_and_operations)
{
    // [EXAMPLE_MULTI_SLICE_CONSTRUCTION_AND_OPERATIONS_START]
    using MS816 = estd::multi_slice<uint8_t, uint16_t const>;

    uint8_t u8_array[10U]         = {0};
    uint16_t const u16_array[10U] = {0};

    // Creating a multi_slice of size 10 of different types.
    MS816 u8u16 = MS816(u8_array, u16_array);

    // Getting the size of multi_slice.
    EXPECT_EQ(10U, u8u16.size());
    EXPECT_EQ(10U, u8u16.get<uint8_t>().size());
    EXPECT_EQ(10U, u8u16.get<uint16_t const>().size());

    // Getting data pointer from slice.
    EXPECT_EQ(u8_array, u8u16.data<uint8_t>());
    EXPECT_EQ(u8_array, u8u16.data<uint8_t const>());
    EXPECT_EQ(u16_array, u8u16.data<uint16_t>());
    EXPECT_EQ(u16_array, u8u16.data<uint16_t const>());

    // at function returns reference to an item.
    EXPECT_EQ(&u8_array[2], &u8u16.at<uint8_t>(2));
    EXPECT_EQ(&u16_array[5], &u8u16.at<uint16_t>(5));

    // idx function gives the position of an item.
    EXPECT_EQ(3U, u8u16.idx(&u8_array[3]));
    EXPECT_EQ(4U, u8u16.idx(&u16_array[4]));

    // advance function reduces the size by the value passed in the argument.
    u8u16.advance(6U);
    EXPECT_EQ(4U, u8u16.size());

    // advance function adjusts the data pointer.
    EXPECT_EQ(u8_array + 6U, u8u16.get<uint8_t>().data());
    EXPECT_EQ(u16_array + 6U, u8u16.get<uint16_t const>().data());

    // trim function reduces the size by the value passed in the argument.
    u8u16.trim(2U);
    EXPECT_EQ(2U, u8u16.size());

    // trim function sets the size to zero if passed argument is bigger than the size.
    u8u16.trim(3);
    EXPECT_EQ(0U, u8u16.size());
    // [EXAMPLE_MULTI_SLICE_CONSTRUCTION_AND_OPERATIONS_END]
}
