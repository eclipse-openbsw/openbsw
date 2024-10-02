// Copyright 2024 Accenture.

#include "estd/none.h"

#include <estd/functional.h>
#include <estd/optional.h>

#include <gtest/gtest.h>

// [EXAMPLE_NONE_INITIALIZATION_TO_OBJECT_STRUCT_START]
// Creating a structure Foo.
struct Foo
{
    // Static method that returns a constant integer value of 5
    static int32_t bar0() { return 5; }

    // Static method that takes a float value and returns the integer value of the float.
    static int32_t bar1(float f) { return f; }
};

// [EXAMPLE_NONE_INITIALIZATION_TO_OBJECT_STRUCT_END]

TEST(NoneExample, Usage_of_none_in_function)
{
    // [EXAMPLE_NONE_INITIALIZATION_TO_OBJECT_START]
    // Declare a function object of type.
    ::estd::function<int32_t()> f0;      // estd::function<int32_t()> named f0.
    ::estd::function<int32_t(float)> f1; // estd::function<int32_t(float)> named f1.

    // Check if f0 and f1 has been set to a target.
    EXPECT_FALSE(f0.has_value()); // Expecting it to be false initially.
    EXPECT_FALSE(f1.has_value());

    // Set f0 to a target function Foo::bar0 and f1 to a target function Foo::bar1.
    f0 = ::estd::function<int32_t()>::create<&Foo::bar0>();
    f1 = ::estd::function<int32_t(float)>::create<&Foo::bar1>();

    // Check if f0 and f1 has been set to a target.
    EXPECT_TRUE(f0.has_value()); // Expecting it to be true now.
    EXPECT_TRUE(f1.has_value());

    // Assign estd::none to f0 and f1 .
    f0 = ::estd::none; // Making it an empty or null function object.
    f1 = ::estd::none;

    // Check if f0 and f1 has been set to a target.
    EXPECT_FALSE(f0.has_value()); // Expecting it to be false after assignment to estd::none.
    EXPECT_FALSE(f1.has_value());

    // [EXAMPLE_NONE_INITIALIZATION_TO_OBJECT_END]
}

TEST(NoneExample, conversion_operator_of_none)
{
    // [EXAMPLE_NONE_CONVERSION_OPERATOR_OF_NONE_START]
    estd::none_t noneInstance;

    // Test the conversion to int
    int resultInt = static_cast<int>(noneInstance);

    // Check if resultInt is equal to 0
    EXPECT_EQ(resultInt, 0); // Assuming the default-constructed int value is 0

    // Test the conversion to double
    double resultDouble = static_cast<double>(noneInstance);

    // Check if resultDouble is equal to 0.0.
    EXPECT_EQ(resultDouble, 0.0); // Assuming the default-constructed double value is 0.0
    // [EXAMPLE_NONE_CONVERSION_OPERATOR_OF_NONE_END]
}
