// Copyright 2024 Accenture.

#include "estd/uncopyable.h"

#include <estd/optional.h>
#include <platform/estdint.h>

#include <gtest/gtest.h>

TEST(OptionalExample, Creation)
{
    // [EXAMPLE_OPTIONAL_CREATION_START]
    // Creating empty optional instance of the class optional final.
    ::estd::optional<int32_t> a;
    // Returns false, since 'a' is empty.
    EXPECT_FALSE(a.has_value());

    // Creating an optional instance with value being passed to it.
    ::estd::optional<int32_t> b = 11;
    ::estd::optional<int32_t> c(5);
    // Creating and assigning an optional instance from another optional instance.
    ::estd::optional<int32_t> d(b);

    EXPECT_TRUE(b.has_value());
    EXPECT_TRUE(c.has_value());
    EXPECT_TRUE(d.has_value());

    // Creating constant optional instance of the class optional final.
    ::estd::optional<int32_t> const e(10);

    // Creating empty optional instance of the class optional<T&>
    ::estd::optional<int32_t&> ir;
    // Returns false, since 'ir' is empty.
    EXPECT_FALSE(ir.has_value());
    // Creating constant empty optional instance of the class optional<T&>
    ::estd::optional<int32_t&> const irr;

    // Creating an optional instance of the class optional<T&> with reference being passed to it.
    int32_t i = 0;
    ::estd::optional<int32_t&> ir2(i);
    ASSERT_TRUE(ir2.has_value());
    // [EXAMPLE_OPTIONAL_CREATION_END]
}

TEST(OptionalExample, Dereferencing)
{
    // [EXAMPLE_OPTIONAL_DEREFERENCING_START]
    // Defining a class 'SomeClass' with parameterized constructor
    class SomeClass
    {
    public:
        int i;

        SomeClass(int i) : i(i) {}
    };

    ::estd::optional<int32_t> a(5);
    ASSERT_TRUE(a.has_value());
    // dereferencing the value using operator*().
    EXPECT_EQ(5, *a);
    // Changing the value of 'a'.
    a = 10;
    EXPECT_EQ(10, *a);
    ::estd::optional<SomeClass> optionalObject = SomeClass(42);
    // dereferencing the value using operator->().
    EXPECT_EQ(optionalObject->i, 42);

    int32_t b = 0;
    ::estd::optional<int32_t&> ir(b);
    ASSERT_TRUE(ir.has_value());
    // dereferencing the value using operator*()
    EXPECT_EQ(b, *ir);
    // dereferencing the value using operator->().
    EXPECT_EQ(&b, ir.operator->());
    // [EXAMPLE_OPTIONAL_DEREFERENCING_END]
}

TEST(OptionalExample, Can_be_defaulted_to_a_value)
{
    // [EXAMPLE_OPTIONAL_VALUE_OR_START]
    ::estd::optional<int32_t> i;

    // i.value_or(5) returns 5, since 'i' is empty.
    EXPECT_EQ(5, i.value_or(5));

    i = 6;
    // i.value_or(5) returns 6, since 'i' contains value 6.
    EXPECT_EQ(6, i.value_or(5));
    // [EXAMPLE_OPTIONAL_VALUE_OR_END]
}

TEST(OptionalExample, Comparison_operators)
{
    // [EXAMPLE_OPTIONAL_COMPARISON_OPERATORS_START]
    ::estd::optional<int32_t> i0 = 7;
    ::estd::optional<int32_t> i1 = 5;

    EXPECT_TRUE(i0 == i0);
    EXPECT_TRUE(i0 != i1);

    i0.reset();
    i1.reset();
    EXPECT_TRUE(i0 == i1);
    EXPECT_FALSE(i0 != i1);
    // [EXAMPLE_OPTIONAL_COMPARISON_OPERATORS_END]
}
