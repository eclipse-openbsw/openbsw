// Copyright 2024 Accenture.

#include "estd/indestructible.h"

#include "estd/assert.h"

#include <gtest/gtest.h>

#include <type_traits>

using ::estd::indestructible;

namespace
{
struct Constructible
{
    uint8_t _data;

    explicit Constructible(uint8_t data) : _data(data) {}
};

struct HasDestructor
{
    ~HasDestructor() { estd_assert(0); }
};

} // namespace

static_assert(
    std::is_trivially_destructible<indestructible<HasDestructor>>::value,
    "Indestructible: TriviallyDestructible");

TEST(Indestructible, DoesNotCallDestructor)
{
    ASSERT_NO_THROW({ ESR_UNUSED indestructible<HasDestructor> instance; });
}

TEST(Indestructible, CanBeConstructed)
{
    indestructible<Constructible> instance(0xAA);

    ASSERT_EQ(instance->_data, 0xAA);
    ASSERT_EQ(instance.get()._data, 0xAA);

    Constructible& ref = instance;
    EXPECT_EQ(&ref, &instance.get());
}
