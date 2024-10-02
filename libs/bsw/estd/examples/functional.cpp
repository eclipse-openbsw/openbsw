// Copyright 2024 Accenture.

#include "estd/functional.h"

#include "estd/none.h"
#include "estd/type_traits.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

// [EXAMPLE_FUNCTION_ZERO_PARAM_START]
/* Define a zero parameter function */
int32_t foo0() { return 2; }

TEST(Function, TestFreeFunctionZeroParams)
{
    ::estd::function<int32_t()> f = ::estd::function<int32_t()>::create<&foo0>();
    ASSERT_EQ(2, f());
}

// [EXAMPLE_FUNCTION_ZERO_PARAM_END]

// [EXAMPLE_FUNCTION_FUNCTOR_ZERO_PARAM_START]
/* Define functor */
struct Bar0
{
    int32_t operator()() const { return 7; }
};

TEST(Function, TestFunctorZeroParams)
{
    Bar0 bar;
    ::estd::function<int32_t()> f(::estd::make_function(bar));
    ASSERT_EQ(7, f());
}

// [EXAMPLE_FUNCTION_FUNCTOR_ZERO_PARAM_END]

TEST(Function, TestLambdaParameter)
{
    // [EXAMPLE_FUNCTION_LAMBDA_PARAM_START]
    auto callme = [](::estd::function<int32_t()> f) { return f(); };
    auto f      = []() { return 88; };
    ASSERT_EQ(88, callme(::estd::make_function(f)));
    // [EXAMPLE_FUNCTION_LAMBDA_PARAM_END]
}

// [EXAMPLE_CLOSURE_THREE_PARAM_START]
int32_t foo3(float, int32_t, int32_t h) { return h; }

TEST(Function, TestFreeFunctionClosureThreeParam)
{
    ::estd::function<int32_t(float, int32_t, int32_t)> f
        = ::estd::function<int32_t(float, int32_t, int32_t)>::create<&foo3>();
    ::estd::closure<int32_t(float, int32_t, int32_t)> d
        = ::estd::bind_all(f, 2.14f, int32_t(3), int32_t(4));
    ASSERT_EQ(4, d());
}

// [EXAMPLE_CLOSURE_THREE_PARAM_END]

// [EXAMPLE_CLOSURE_BIND1ST_THREE_PARAM_START]
int32_t foobar3(int32_t const, int32_t const, int32_t const h) { return h; }

TEST(Function, TestBind1stThreeParams)
{
    using fct = ::estd::function<int32_t(int32_t const, int32_t const, int32_t const)>;
    fct f     = fct::create<&foobar3>();
    using bnd = ::estd::binder1st<int32_t(int32_t const, int32_t const, int32_t const)>;
    bnd g     = ::estd::bind1st(f, int32_t(23));
    ASSERT_EQ(4, g(2, 4));
}

// [EXAMPLE_CLOSURE_BIND1ST_THREE_PARAM_END]
