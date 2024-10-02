// Copyright 2024 Accenture.

#include "estd/constructor.h"

#include "estd/vector.h"

#include <gtest/gtest.h>

void emplaceExample(::estd::vector<int>& v)
{
    // EXAMPLE_BEGIN emplaceExample
    for (int i = 0; i < 5; ++i)
    {
        v.emplace_back().construct(i);
    }

    // Creating another vector
    estd::declare::vector<int, 5> v2;

    // Use construct to emplace_back element to the constructor v.
    v2.emplace_back().construct(1);

    // Use emplace to append elements
    v2.emplace(v2.cend()).construct(6);
    // EXAMPLE_END emplaceExample
}

TEST(constructor_test, run_examples)
{
    estd::declare::vector<int, 5> a;
    emplaceExample(a);
}
