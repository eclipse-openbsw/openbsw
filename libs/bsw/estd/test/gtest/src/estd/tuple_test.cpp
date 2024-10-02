// Copyright 2024 Accenture.

#include "estd/tuple.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

TEST(tuple, get_by_type_const)
{
    auto const t = std::make_tuple(uint8_t(5), uint16_t(9));

    EXPECT_EQ(5U, ::estd::get<uint8_t>(t));
    EXPECT_EQ(9U, ::estd::get<uint16_t>(t));
}

TEST(tuple, get_by_type_non_const)
{
    auto t = std::make_tuple(uint8_t(5), uint16_t(10));

    ::estd::get<uint8_t>(t)  = 11;
    ::estd::get<uint16_t>(t) = 22;

    EXPECT_EQ(11U, ::estd::get<uint8_t>(t));
    EXPECT_EQ(22U, ::estd::get<uint16_t>(t));
}

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

TEST(tuple, for_each_const)
{
    auto const t = std::make_tuple(uint8_t(5), uint16_t(10));
    SumUp sum;
    ::estd::for_each(t, sum);
    EXPECT_EQ(15U, sum.value);
}

TEST(tuple, for_each_non_const)
{
    auto t = std::make_tuple(uint8_t(5), uint16_t(10));
    ::estd::for_each(t, Double());
    EXPECT_EQ(10U, ::estd::get<uint8_t>(t));
    EXPECT_EQ(20U, ::estd::get<uint16_t>(t));
}

} // namespace
