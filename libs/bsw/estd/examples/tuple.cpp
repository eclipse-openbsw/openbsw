// Copyright 2024 Accenture.

#include "estd/tuple.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

using namespace ::testing;

//[EXAMPLE_GET_CONSTRAINT_START]

TEST(tuple, get)
{
    auto const t = std::make_tuple(uint8_t(5), uint16_t(9));

    EXPECT_EQ(5U, ::estd::get<uint8_t>(t));
    EXPECT_EQ(9U, ::estd::get<uint16_t>(t));
}

//[EXAMPLE_GET_CONSTRAINT_END]

struct Double
{
    template<typename T>
    void operator()(T& v)
    {
        v = v * 2;
    }
};

struct SumUp
{
    size_t value = 0;

    template<typename T>
    void operator()(T v)
    {
        value += v;
    }
};

//[EXAMPLE_FOR_EACH_CONSTRAINT_START]

TEST(tuple, for_each)
{
    auto const t = std::make_tuple(uint8_t(5), uint16_t(10));
    SumUp sum;
    ::estd::for_each(t, sum);
    EXPECT_EQ(15U, sum.value);
}

//[EXAMPLE_FOR_EACH_CONSTRAINT_END]
