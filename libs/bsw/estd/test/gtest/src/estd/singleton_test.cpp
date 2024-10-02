// Copyright 2024 Accenture.

#include "estd/singleton.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

using namespace ::testing;

namespace
{
struct Foo : ::estd::singleton<Foo>
{
    Foo() : ::estd::singleton<Foo>(*this), _v(0) {}

    int32_t _v;
};

TEST(Singleton, TestAssertNoInstance)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ASSERT_THROW(Foo::instance(), ::estd::assert_exception);
}

TEST(Singleton, TestInstance)
{
    ASSERT_FALSE(Foo::instantiated());
    Foo f;
    ASSERT_EQ(&f, &Foo::instance());
    ASSERT_TRUE(Foo::instantiated());
}

TEST(Singleton, TestConstructorDestructor)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ASSERT_THROW(Foo::instance(), ::estd::assert_exception);
    {
        Foo f;
        ASSERT_NO_THROW(Foo::instance());
    }
    ASSERT_THROW(Foo::instance(), ::estd::assert_exception);
}

TEST(Singleton, TestAssertOnMultipleInstantiation)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    Foo f;
    ASSERT_THROW({ Foo g; }, ::estd::assert_exception);
}
} // namespace
