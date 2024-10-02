// Copyright 2024 Accenture.

#include "estd/result.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

using namespace ::testing;

enum class Problem
{
    Fail,
    Flaw,
    Oversight,
};

namespace estd
{
template<>
struct result_traits<Problem>
{
    static constexpr bool LIMITED = true;
};
} // namespace estd

namespace
{
struct Error
{
    uint32_t code;

    explicit Error(uint32_t code) : code(code) {}
};

struct Foo
{
    size_t bar;

    Foo(size_t bar = 42) : bar(bar) {}
};

struct Pod
{
    uint8_t t[10];
};
} // namespace

static_assert(
    !std::is_default_constructible<::estd::result<std::pair<int, bool>, Error>>::value,
    "::estd::result<T, E> should not be default-constructible");

static_assert(
    !std::is_default_constructible<::estd::result<Pod, int>>::value,
    "::estd::result<T, E> should not be default-constructible");

TEST(result, access_value)
{
    ::estd::result<size_t, Error> r1(size_t(1));

    ASSERT_TRUE(r1.has_value());
    EXPECT_EQ(1U, r1.get());

    r1.get() = 5;
    ASSERT_TRUE(r1.has_value());
    EXPECT_EQ(5U, r1.get());

    ::estd::result<size_t, Error> const r2(size_t(2));

    ASSERT_TRUE(r2.has_value());
    EXPECT_EQ(2U, r2.get());
}

TEST(result, access_error)
{
    ::estd::result<size_t, Error> err1((Error(2U)));

    ASSERT_FALSE(err1.has_value());
    EXPECT_EQ(2U, err1.error().code);

    ::estd::result<size_t, Error> const err2((Error(76U)));

    ASSERT_FALSE(err2.has_value());
    EXPECT_EQ(76U, err2.error().code);
}

TEST(result, value_or)
{
    ::estd::result<size_t, Error> const r1(1);
    EXPECT_EQ(1U, r1.value_or(2U));

    ::estd::result<size_t, Error> const r2((Error(1U)));
    EXPECT_EQ(2U, r2.value_or(2U));
}

TEST(result, dereference)
{
    Foo f1(1U);
    ::estd::result<Foo, Error> r1(f1);

    Foo& value1 = r1.get();

    r1->bar = 3;
    EXPECT_EQ(3U, (*r1).bar);
    EXPECT_EQ(&value1, r1.operator->());

    (*r1).bar = 8;
    EXPECT_EQ(8U, (*r1).bar);
    EXPECT_EQ(&value1, &r1.operator*());

    Foo const f2(2U);
    ::estd::result<Foo, Error> const r2(f2);

    Foo const& value2 = r2.get();

    EXPECT_EQ(&value2, r2.operator->());
    EXPECT_EQ(&value2, &r2.operator*());
}

TEST(result, result_void)
{
    ::estd::result<void, int> r{};

    EXPECT_TRUE(r.has_value());
    r = 1;

    EXPECT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), 1);
}

TEST(result, optimized_storage)
{
    ::estd::result<void, Problem> r{};

    EXPECT_EQ(sizeof(::estd::result<void, Problem>), sizeof(Problem));
    EXPECT_TRUE(r.has_value());

    r = Problem::Flaw;

    EXPECT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), Problem::Flaw);
}

TEST(result, optimized_nonvoid)
{
    ::estd::result<size_t, Problem> r1{11U};

    EXPECT_TRUE(r1.has_value());
    EXPECT_EQ(r1.get(), 11U);

    ::estd::result<size_t, Problem> r2{Problem::Oversight};

    EXPECT_FALSE(r2.has_value());
    EXPECT_EQ(r2.error(), Problem::Oversight);
}

TEST(result, check_implicit_bool_conversion)
{
    ::estd::result<int, Problem> r{99U};
    EXPECT_TRUE(r);
    ::estd::result<bool, Problem> e{Problem::Fail};
    EXPECT_FALSE(e);
}
