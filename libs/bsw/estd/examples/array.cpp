// Copyright 2024 Accenture.

#include <estd/array.h>

#include <gtest/gtest.h>

// EXAMPLE_START array
int sum(estd::array<int, 5> const& a)
{
    int result = 0;
    for (size_t i = 0; i < a.size(); ++i)
    {
        result += a[i];
    }
    return result;
}

// EXAMPLE_END array

TEST(ArrayExample, sum)
{
    // EXAMPLE_START array_init
    estd::array<int, 5> a = {1, 2, 3, 4, 5};

    int const sum_of_5 = sum(a);
    // EXAMPLE_END array_init
    EXPECT_EQ(15, sum_of_5);
}
