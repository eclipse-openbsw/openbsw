// Copyright 2024 Accenture.

#include <estd/assert.h>
#include <estd/indestructible.h>
#include <estd/static_assert.h>

#include <gtest/gtest.h>

#include <type_traits>

using estd::indestructible;

TEST(IndestructibleExample, CanBeConstructed)
{
    // [EXAMPLE_INDESTRUCTIBLE_START]
    // Creating a structure 'Constructible'.
    struct Constructible
    {
        uint8_t _data;

        explicit Constructible(uint8_t data) : _data(data) {}
    };

    // Creating an instance of indestructible class.
    indestructible<Constructible> instance(0xAA);

    // Dereferencing the value using 'operator->()'.
    ASSERT_EQ(instance->_data, 0xAA);

    // Dereferencing the value using 'get()'
    ASSERT_EQ(instance.get()._data, 0xAA);
    // Dereferencing the value using 'operator T&()'
    Constructible& ref = instance;
    EXPECT_EQ(&ref, &instance.get());
    // [EXAMPLE_INDESTRUCTIBLE_END]
}
