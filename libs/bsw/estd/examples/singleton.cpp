// Copyright 2024 Accenture.

#include "estd/singleton.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

using namespace ::testing;

// [EXAMPLE_SINGLETON_STRUCTURE_START]

struct Foo : ::estd::singleton<Foo>
{
    // initialize singleton instance
    Foo() : ::estd::singleton<Foo>(*this), _v(0) {}

    int32_t _v; // internal variable
};

TEST(Singleton, TestInstance)
{
    // check Foo is not instantiated
    ASSERT_FALSE(Foo::instantiated());
    Foo f;                            // instance
    ASSERT_EQ(&f, &Foo::instance());  // check the singleton instance's match
    ASSERT_TRUE(Foo::instantiated()); // verify Foo is instantiated
}

// [EXAMPLE_SINGLETON_STRUCTURE_END]
