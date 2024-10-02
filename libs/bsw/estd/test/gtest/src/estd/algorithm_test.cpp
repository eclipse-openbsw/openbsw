// Copyright 2024 Accenture.

#include "estd/algorithm.h"

#include "estd/array.h"
#include "estd/deque.h"
#include "estd/vector.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

#include <algorithm>

namespace
{
using namespace ::testing;

template<class T>
class AlgorithmTest : public ::testing::Test
{
public:
    T _container;
};

static_assert(::estd::min(1, 2) == 1, "::estd::min test");
static_assert(::estd::min(1, 2, 3, 4) == 1, "::estd::min (variadic) test");
static_assert(::estd::min(2, 3, 1, 4) == 1, "::estd::min (variadic) test");
static_assert(::estd::max(10, 12) == 12, "::estd::max test");
static_assert(::estd::max(10, 11, 12, 13) == 13, "::estd::max (variadic) test");
static_assert(::estd::max(10, 11, 13, 12) == 13, "::estd::max (variadic) test");

static_assert((::estd::maximum<int, 99, 2>::value == 99), "::estd::maximum<int> test");
static_assert((::estd::maximum<uint32_t, 21, 133>::value == 133), "::estd::maximum<uint32_t> test");

static_assert((::estd::minimum<int8_t, 56, 2>::value == 2), "::estd::minimum<int8_t> test");
static_assert(
    (::estd::minimum<long long, 998, 999>::value == 998), "::estd::minimum<long long> test");

TEST(AlgorithmTest, min)
{
    ASSERT_EQ(10U, ::estd::min(15U, 10U));
    ASSERT_EQ(10U, ::estd::min(10U, 15U));
}

TEST(AlgorithmTest, max)
{
    ASSERT_EQ(15U, ::estd::max(15U, 10U));
    ASSERT_EQ(15U, ::estd::max(10U, 15U));
}

TEST(AlgorithmTest, swap)
{
    uint16_t a = 1, b = 2;
    ::estd::swap(a, b);
    EXPECT_EQ(1U, b);
    EXPECT_EQ(2U, a);
}

// ********** all_of **********

TEST(AlgorithmTest, all_of_vector)
{
    ::estd::declare::vector<int32_t, 10U> container;

    container.push_back(1);
    container.push_back(1);
    ASSERT_TRUE(::estd::all_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(1)));
    ASSERT_FALSE(
        ::estd::all_of(container.cbegin(), container.cend(), ::estd::not_equals<int32_t>(1)));

    container.push_back(2);
    ASSERT_FALSE(::estd::all_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(1)));
    ASSERT_TRUE(::estd::all_of(container.cbegin(), container.cend(), ::estd::greater1<int32_t>(0)));
    ASSERT_TRUE(::estd::all_of(container.cbegin(), container.cend(), ::estd::less1<int32_t>(10)));
}

TEST(AlgorithmTest, all_of_deque)
{
    ::estd::declare::deque<int32_t, 10> container;
    container.push_back(1);
    container.push_back(1);
    ASSERT_TRUE(::estd::all_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(1)));
    ASSERT_FALSE(
        ::estd::all_of(container.cbegin(), container.cend(), ::estd::not_equals<int32_t>(1)));

    container.push_back(2);
    ASSERT_FALSE(::estd::all_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(1)));
    ASSERT_TRUE(::estd::all_of(container.cbegin(), container.cend(), ::estd::greater1<int32_t>(0)));
    ASSERT_TRUE(::estd::all_of(container.cbegin(), container.cend(), ::estd::less1<int32_t>(10)));
}

TEST(AlgorithmTest, all_of_array)
{
    ::estd::array<int32_t, 10> container;
    for (int& i : container)
    {
        i = 1;
    }
    ASSERT_TRUE(::estd::all_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(1)));

    container[2] = 2;
    ASSERT_FALSE(::estd::all_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(1)));
}

// ********** any_of **********

TEST(AlgorithmTest, any_of_vector)
{
    ::estd::declare::vector<int32_t, 10> container;
    container.push_back(1);
    container.push_back(2);
    container.push_back(3);
    ASSERT_TRUE(::estd::any_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(1)));
    ASSERT_FALSE(::estd::any_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(4)));
}

TEST(AlgorithmTest, any_of_deque)
{
    ::estd::declare::deque<int32_t, 10> container;
    container.push_back(1);
    container.push_back(2);
    container.push_back(3);
    ASSERT_TRUE(::estd::any_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(1)));
    ASSERT_FALSE(::estd::any_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(4)));
}

TEST(AlgorithmTest, any_of_array)
{
    ::estd::array<int32_t, 10> container;
    for (size_t i = 0; i < container.size(); ++i)
    {
        container[i] = i;
    }
    ASSERT_TRUE(::estd::any_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(1)));
    ASSERT_FALSE(::estd::any_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(-1)));
}

// ********** none_of **********

TEST(AlgorithmTest, none_of_vector)
{
    ::estd::declare::vector<int32_t, 10> container;
    container.push_back(1);
    container.push_back(2);
    container.push_back(3);
    ASSERT_TRUE(::estd::none_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(4)));
    ASSERT_FALSE(::estd::none_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(1)));
}

TEST(AlgorithmTest, none_of_deque)
{
    ::estd::declare::deque<int32_t, 10> container;
    container.push_back(1);
    container.push_back(2);
    container.push_back(3);
    ASSERT_TRUE(::estd::none_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(4)));
    ASSERT_FALSE(::estd::none_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(1)));
}

TEST(AlgorithmTest, none_of_array)
{
    ::estd::array<int32_t, 10> const container = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ASSERT_TRUE(::estd::none_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(-1)));
    ASSERT_FALSE(::estd::none_of(container.cbegin(), container.cend(), ::estd::equals<int32_t>(1)));
}

} // anonymous namespace

#ifndef __DCC__
using Containers
    = ::testing::Types<::estd::declare::vector<int32_t, 10U>, ::estd::declare::deque<int32_t, 10U>>;

TYPED_TEST_SUITE(AlgorithmTest, Containers);

TYPED_TEST(AlgorithmTest, std_count)
{
    TypeParam c;
    c.push_back(0);
    c.push_back(0);
    c.push_back(1);
    c.push_back(2);
    c.push_back(0);
    ASSERT_EQ(3, std::count(c.begin(), c.end(), 0));
}
#endif
