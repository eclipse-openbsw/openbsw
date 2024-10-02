// Copyright 2024 Accenture.

#include "estd/limited.h"

#include <gtest/gtest.h>

TEST(Limited, Limited)
{
    ::estd::limited<size_t> l1;

    EXPECT_FALSE(l1.has_value());
    EXPECT_EQ(l1.value_or(1234U), 1234U);

    l1 = 1U;

    EXPECT_TRUE(l1.has_value());
    EXPECT_EQ(l1.get(), 1U);

    auto const l2 = l1;

    EXPECT_TRUE(l2.has_value());
    EXPECT_EQ(l2.get(), 1U);

    l1.reset();
    EXPECT_FALSE(l1.has_value());

    // By default, max() value is treated as special
    l1 = std::numeric_limits<size_t>::max();
    EXPECT_FALSE(l1.has_value());
}

TEST(Limited, LimitedWithEnum)
{
    enum class Error
    {
        Problem,
        Malfunction,
        Miscalculation,
        Blunder,
    };

    ::estd::limited<Error> l1;

    EXPECT_FALSE(l1.has_value());
    EXPECT_EQ(l1.value_or(Error::Problem), Error::Problem);

    l1 = Error::Miscalculation;

    EXPECT_TRUE(l1.has_value());
    EXPECT_EQ(l1.get(), Error::Miscalculation);
}
