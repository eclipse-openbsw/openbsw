// Copyright 2024 Accenture.

#include "estd/algorithm.h"

#include "estd/array.h"
#include "estd/deque.h"
#include "estd/vector.h"

#include <gtest/gtest.h>

namespace
{
void Usage()
{
    // EXAMPLE_START container
    // example to create container c from array
    ::estd::array<int32_t, 3> c = {{1, 2, 3}};

    // example to create container c from vector
    ::estd::declare::vector<int32_t, 10U> c0;

    // example to create container c1 from deque
    ::estd::declare::deque<int32_t, 10U> c1;

    // Checks if all of the value in array c are equal to 1.
    ::estd::all_of(c.cbegin(), c.cend(), ::estd::equals<int32_t>(1));

    // Checks if all of the value in vector c0 are equal to 1.
    ::estd::all_of(c0.cbegin(), c0.cend(), ::estd::equals<int32_t>(1));

    // Checks if all of the value in deque c1 are equal to 1.
    ::estd::all_of(c1.cbegin(), c1.cend(), ::estd::equals<int32_t>(1));
    c1.push_back(1); // appending value to the container c1

    // Returns the minimum value of two integers which is 10U.
    ::estd::min(15U, 10U);
    // Returns the maximum value of two integers which is 15U.
    ::estd::max(15U, 10U);
    // Swaps the values of two variables.
    uint16_t a = 1, b = 2;
    ::estd::swap(a, b);
    // Check all of the elements in container equals to 1.
    ::estd::all_of(c.cbegin(), c.cend(), ::estd::equals<int32_t>(1));
    // Check if all of the elements in container not equals to 1.
    ::estd::all_of(c.cbegin(), c.cend(), ::estd::not_equals<int32_t>(1));
    // Check if all of the elements in container are greater than a certain value(0).
    ::estd::all_of(c.cbegin(), c.cend(), ::estd::greater1<int32_t>(0));
    // Check if all of the elements in container are less than a certain value(0).
    ::estd::all_of(c.cbegin(), c.cend(), ::estd::less1<int32_t>(10));
    ::estd::any_of(c.cbegin(), c.cend(), ::estd::equals<int32_t>(1));
    ::estd::none_of(c.cbegin(), c.cend(), ::estd::equals<int32_t>(-1));
    // EXAMPLE_END container
}
} // namespace

TEST(algorithm_test, run_examples) { Usage(); }
