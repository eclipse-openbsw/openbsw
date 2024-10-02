// Copyright 2024 Accenture.

#include "estd/functional.h"
#include "estd/none.h"
#include "estd/type_traits.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

using namespace ::testing;

namespace
{
void fooRef(int32_t& result) { result = 2; }

int32_t foo0() { return 2; }

int32_t foo1(float f) { return f; }

int32_t foo2(float, int32_t g) { return g; }

int32_t foobar2(int32_t const f, int32_t const) { return f; }

int32_t foo3(float, int32_t, int32_t h) { return h; }

int32_t foo4(float, int32_t, int32_t, int32_t i) { return i; }

int32_t foo5(float, int32_t, int32_t, int32_t, int32_t j) { return j; }

int32_t foobar3(int32_t const, int32_t const, int32_t const h) { return h; }

int32_t foobar4(int32_t const, int32_t const, int32_t const, int32_t const i) { return i; }

int32_t foobar5(int32_t const, int32_t const, int32_t const, int32_t const, int32_t const j)
{
    return j;
}

struct Foo
{
    int32_t foo0() { return 3; }

    int32_t foo1(float f) { return f; }

    int32_t foo2(float, int32_t g) { return g; }

    int32_t foo3(float, int32_t, int32_t h) { return h; }

    int32_t foo4(float, int32_t, int32_t, int32_t i) { return i; }

    int32_t foo5(float, int32_t, int32_t, int32_t, int32_t j) { return j; }

    static int32_t bar0() { return 5; }

    static int32_t bar1(float f) { return f; }

    static int32_t bar2(float, int32_t g) { return g; }

    static int32_t bar3(float, int32_t, int32_t h) { return h; }

    static int32_t bar4(float, int32_t, int32_t, int32_t i) { return i; }

    static int32_t bar5(float, int32_t, int32_t, int32_t, int32_t j) { return j; }
};

struct Bar0
{
    int32_t operator()() const { return 7; }
};

struct Bar1
{
    int32_t operator()(float f) const { return f; }
};

struct Bar2
{
    int32_t operator()(float, int32_t g) const { return g; }
};

struct Bar3
{
    int32_t operator()(float, int32_t, int32_t h) const { return h; }
};

struct Bar4
{
    int32_t operator()(float, int32_t, int32_t, int32_t i) const { return i; }
};

struct Bar5
{
    int32_t operator()(float, int32_t, int32_t, int32_t, int32_t j) const { return j; }
};

struct BarV
{
    int32_t _m;

    int32_t operator()()
    {
        _m++;
        return _m + 7;
    }

    int32_t operator()(int32_t f) const { return _m + f; }

    int32_t operator()(int32_t, int32_t g) const { return _m + g; }

    int32_t operator()(int32_t, int32_t, int32_t h) const { return _m + h; }

    int32_t operator()(int32_t, int32_t, int32_t, int32_t i) const { return _m + i; }

    int32_t operator()(int32_t, int32_t, int32_t, int32_t, int32_t j) const { return _m + j; }
};

struct IntClass
{
    IntClass() : i(10) {}

    int32_t i;
};

static_assert(::estd::is_callable<Bar0>::value, "");
static_assert(::estd::is_callable<Bar1>::value, "");
static_assert(::estd::is_callable<Bar2>::value, "");
static_assert(::estd::is_callable<Bar3>::value, "");
static_assert(::estd::is_callable<Bar4>::value, "");
static_assert(::estd::is_callable<Bar5>::value, "");
static_assert(::estd::is_callable<BarV>::value, "");

static_assert(::estd::is_callable<::estd::function<void()>>::value, "");
static_assert(::estd::is_callable<::estd::function<int(float)>>::value, "");
static_assert(
    ::estd::is_callable<::estd::function<int(void*, bool, float*, IntClass&)>>::value, "");

TEST(Function, TestNoFunctionDefined)
{
    ::estd::function<IntClass()> f;
    ASSERT_EQ(10, f().i);

    ::estd::function<IntClass(IntClass)> f1;
    IntClass a1;
    ASSERT_EQ(10, f1(a1).i);

    ::estd::function<IntClass(IntClass, IntClass)> f2;
    IntClass a2;
    ASSERT_EQ(10, f2(a1, a2).i);

    ::estd::function<IntClass(IntClass, IntClass, IntClass)> f3;
    IntClass a3;
    ASSERT_EQ(10, f3(a1, a2, a3).i);

    ::estd::function<IntClass(IntClass, IntClass, IntClass, IntClass)> f4;
    IntClass a4;
    ASSERT_EQ(10, f4(a1, a2, a3, a4).i);

    ::estd::function<IntClass(IntClass, IntClass, IntClass, IntClass, IntClass)> f5;
    IntClass a5;
    ASSERT_EQ(10, f5(a1, a2, a3, a4, a5).i);
}

TEST(Function, OperatorBool)
{
    ::estd::function<int32_t()> f0;
    ::estd::function<int32_t(float)> f1;
    ::estd::function<int32_t(float, int32_t)> f2;
    ::estd::function<int32_t(float, int32_t, int32_t)> f3;
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t)> f4;
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t, int32_t)> f5;

    EXPECT_FALSE(f0);
    EXPECT_FALSE(f1);
    EXPECT_FALSE(f2);
    EXPECT_FALSE(f3);
    EXPECT_FALSE(f4);
    EXPECT_FALSE(f5);

    f0 = ::estd::function<int32_t()>::create<&Foo::bar0>();
    f1 = ::estd::function<int32_t(float)>::create<&Foo::bar1>();
    f2 = ::estd::function<int32_t(float, int32_t)>::create<&Foo::bar2>();
    f3 = ::estd::function<int32_t(float, int32_t, int32_t)>::create<&Foo::bar3>();
    f4 = ::estd::function<int32_t(float, int32_t, int32_t, int32_t)>::create<&Foo::bar4>();
    f5 = ::estd::function<int32_t(float, int32_t, int32_t, int32_t, int32_t)>::create<&Foo::bar5>();

    EXPECT_TRUE(f0);
    EXPECT_TRUE(f1);
    EXPECT_TRUE(f2);
    EXPECT_TRUE(f3);
    EXPECT_TRUE(f4);
    EXPECT_TRUE(f5);

    f0 = ::estd::none;
    f1 = ::estd::none;
    f2 = ::estd::none;
    f3 = ::estd::none;
    f4 = ::estd::none;
    f5 = ::estd::none;

    EXPECT_FALSE(f0);
    EXPECT_FALSE(f1);
    EXPECT_FALSE(f2);
    EXPECT_FALSE(f3);
    EXPECT_FALSE(f4);
    EXPECT_FALSE(f5);
}

TEST(Function, HasValue)
{
    ::estd::function<int32_t()> f0;
    ::estd::function<int32_t(float)> f1;
    ::estd::function<int32_t(float, int32_t)> f2;
    ::estd::function<int32_t(float, int32_t, int32_t)> f3;
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t)> f4;
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t, int32_t)> f5;

    EXPECT_FALSE(f0.has_value());
    EXPECT_FALSE(f1.has_value());
    EXPECT_FALSE(f2.has_value());
    EXPECT_FALSE(f3.has_value());
    EXPECT_FALSE(f4.has_value());
    EXPECT_FALSE(f5.has_value());

    f0 = ::estd::function<int32_t()>::create<&Foo::bar0>();
    f1 = ::estd::function<int32_t(float)>::create<&Foo::bar1>();
    f2 = ::estd::function<int32_t(float, int32_t)>::create<&Foo::bar2>();
    f3 = ::estd::function<int32_t(float, int32_t, int32_t)>::create<&Foo::bar3>();
    f4 = ::estd::function<int32_t(float, int32_t, int32_t, int32_t)>::create<&Foo::bar4>();
    f5 = ::estd::function<int32_t(float, int32_t, int32_t, int32_t, int32_t)>::create<&Foo::bar5>();

    EXPECT_TRUE(f0.has_value());
    EXPECT_TRUE(f1.has_value());
    EXPECT_TRUE(f2.has_value());
    EXPECT_TRUE(f3.has_value());
    EXPECT_TRUE(f4.has_value());
    EXPECT_TRUE(f5.has_value());

    f0 = ::estd::none;
    f1 = ::estd::none;
    f2 = ::estd::none;
    f3 = ::estd::none;
    f4 = ::estd::none;
    f5 = ::estd::none;

    EXPECT_FALSE(f0.has_value());
    EXPECT_FALSE(f1.has_value());
    EXPECT_FALSE(f2.has_value());
    EXPECT_FALSE(f3.has_value());
    EXPECT_FALSE(f4.has_value());
    EXPECT_FALSE(f5.has_value());
}

/*
 * Tests for 0 parameters
 */
TEST(Function, TestFreeFunctionZeroParams)
{
    ::estd::function<int32_t()> f = ::estd::function<int32_t()>::create<&foo0>();
    ASSERT_EQ(2, f());
}

#ifndef _MSC_VER
TEST(Function, TestMemberFunctionZeroParams)
{
    Foo foo;
    ::estd::function<int32_t()> f = ::estd::function<int32_t()>::create<Foo, &Foo::foo0>(foo);
    ASSERT_EQ(3, f());
}
#endif

TEST(Function, TestStaticMemberFunctionZeroParams)
{
    ::estd::function<int32_t()> f = ::estd::function<int32_t()>::create<&Foo::bar0>();
    ASSERT_EQ(5, f());
}

TEST(Function, TestFunctorZeroParams)
{
    Bar0 bar;
    ::estd::function<int32_t()> f(::estd::make_function(bar));
    ASSERT_EQ(7, f());
}

TEST(Function, TestLambdaParameter)
{
    auto callme = [](::estd::function<int32_t()> f) { return f(); };
    auto f      = []() { return 88; };
    ASSERT_EQ(88, callme(::estd::make_function(f)));
}

/*
 * Tests for 1 parameter
 */
TEST(Function, TestFreeFunctionOneParam)
{
    ::estd::function<int32_t(float)> f = ::estd::function<int32_t(float)>::create<&foo1>();
    ASSERT_EQ(2, f(2.14));
}

TEST(Function, TestFreeFunctionOneParamVoid)
{
    ::estd::function<void(int32_t&)> f = ::estd::function<void(int32_t&)>::create<&fooRef>();
    int32_t result                     = 0;
    f(result);
    ASSERT_EQ(2, result);
}

TEST(Function, TestFreeFunctionClosureOneParam)
{
    ::estd::function<int32_t(float)> f = ::estd::function<int32_t(float)>::create<&foo1>();
    ::estd::closure<int32_t(float)> d  = ::estd::bind_all(f, 2.14f);
    ASSERT_EQ(2, d());
}

TEST(Function, TestMemberFunctionOneParam)
{
    Foo foo;
    ::estd::function<int32_t(float)> f
        = ::estd::function<int32_t(float)>::create<Foo, &Foo::foo1>(foo);
    ASSERT_EQ(3, f(3.14));
}

TEST(Function, TestStaticMemberFunctionOneParam)
{
    ::estd::function<int32_t(float)> f = ::estd::function<int32_t(float)>::create<&Foo::bar1>();
    ASSERT_EQ(5, f(5.75));
}

TEST(Function, TestFunctorOneParam)
{
    Bar1 bar;
    ::estd::function<int32_t(float)> f = ::estd::make_function(bar);
    ASSERT_EQ(7, f(7.65));
}

/*
 * Tests for 2 parameters
 */
TEST(Function, TestFreeFunctionClosureTwoParam)
{
    ::estd::function<int32_t(float, int32_t)> f
        = ::estd::function<int32_t(float, int32_t)>::create<&foo2>();
    ::estd::closure<int32_t(float, int32_t)> d = ::estd::bind_all(f, 2.14f, int32_t(3));
    ASSERT_EQ(3, d());
}

TEST(Function, TestFreeFunctionTwoParams)
{
    ::estd::function<int32_t(float, int32_t)> f
        = ::estd::function<int32_t(float, int32_t)>::create<&foo2>();
    ASSERT_EQ(2, f(2.14, 2));
}

TEST(Function, TestBind1stTwoParams)
{
    using fct = ::estd::function<int32_t(int32_t const, int32_t const)>;
    fct f     = fct::create<&foobar2>();
    using bnd = ::estd::binder1st<int32_t(int32_t const, int32_t const)>;
    bnd g     = ::estd::bind1st(f, int32_t(23));
    ASSERT_EQ(23, g(2));
}

TEST(Function, TestMemberFunctionTwoParams)
{
    Foo foo;
    ::estd::function<int32_t(float, int32_t)> f
        = ::estd::function<int32_t(float, int32_t)>::create<Foo, &Foo::foo2>(foo);
    ASSERT_EQ(3, f(3.14, 3));
}

TEST(Function, TestStaticMemberFunctionTwoParams)
{
    ::estd::function<int32_t(float, int32_t)> f
        = ::estd::function<int32_t(float, int32_t)>::create<&Foo::bar2>();
    ASSERT_EQ(5, f(5.75, 5));
}

TEST(Function, TestFunctorTwoParams)
{
    Bar2 bar;
    ::estd::function<int32_t(float, int32_t)> f(::estd::make_function(bar));
    ASSERT_EQ(7, f(7.65, 7));
}

/*
 * Tests for 3 parameters
 */
TEST(Function, TestFreeFunctionClosureThreeParam)
{
    ::estd::function<int32_t(float, int32_t, int32_t)> f
        = ::estd::function<int32_t(float, int32_t, int32_t)>::create<&foo3>();
    ::estd::closure<int32_t(float, int32_t, int32_t)> d
        = ::estd::bind_all(f, 2.14f, int32_t(3), int32_t(4));
    ASSERT_EQ(4, d());
}

TEST(Function, TestFreeFunctionThreeParams)
{
    ::estd::function<int32_t(float, int32_t, int32_t)> f
        = ::estd::function<int32_t(float, int32_t, int32_t)>::create<&foo3>();
    ASSERT_EQ(4, f(2.14, 2, 4));
}

TEST(Function, TestBind1stThreeParams)
{
    using fct = ::estd::function<int32_t(int32_t const, int32_t const, int32_t const)>;
    fct f     = fct::create<&foobar3>();
    using bnd = ::estd::binder1st<int32_t(int32_t const, int32_t const, int32_t const)>;
    bnd g     = ::estd::bind1st(f, int32_t(23));
    ASSERT_EQ(4, g(2, 4));
}

TEST(Function, TestMemberFunctionThreeParams)
{
    Foo foo;
    ::estd::function<int32_t(float, int32_t, int32_t)> f
        = ::estd::function<int32_t(float, int32_t, int32_t)>::create<Foo, &Foo::foo3>(foo);
    ASSERT_EQ(4, f(3.14, 3, 4));
    using bnd = ::estd::binder1st<int32_t(float, int32_t, int32_t)>;
    bnd g     = ::estd::bind1st(f, 3.14F);
    ASSERT_EQ(4, g(3, 4));
}

TEST(Function, TestStaticMemberFunctionThreeParams)
{
    ::estd::function<int32_t(float, int32_t, int32_t)> f
        = ::estd::function<int32_t(float, int32_t, int32_t)>::create<&Foo::bar3>();
    ASSERT_EQ(4, f(5.75, 5, 4));
}

TEST(Function, TestFunctorThreeParams)
{
    Bar3 bar;
    ::estd::function<int32_t(float, int32_t, int32_t)> f(::estd::make_function(bar));
    ASSERT_EQ(4, f(7.65, 7, 4));
}

/*
 * Tests for 4 parameters
 */
TEST(Function, TestFreeFunctionClosureFourParam)
{
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t)> f
        = ::estd::function<int32_t(float, int32_t, int32_t, int32_t)>::create<&foo4>();
    ::estd::closure<int32_t(float, int32_t, int32_t, int32_t)> d
        = ::estd::bind_all(f, 2.14f, int32_t(3), int32_t(4), int32_t(5));
    ASSERT_EQ(5, d());
}

TEST(Function, TestFreeFunctionFourParams)
{
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t)> f
        = ::estd::function<int32_t(float, int32_t, int32_t, int32_t)>::create<&foo4>();
    ASSERT_EQ(5, f(2.14, 2, 4, 5));
}

TEST(Function, TestBind1stFourParams)
{
    using fct
        = ::estd::function<int32_t(int32_t const, int32_t const, int32_t const, int32_t const)>;
    fct f = fct::create<&foobar4>();
    using bnd
        = ::estd::binder1st<int32_t(int32_t const, int32_t const, int32_t const, int32_t const)>;
    bnd g = ::estd::bind1st(f, int32_t(23));
    ASSERT_EQ(5, g(2, 4, 5));
}

TEST(Function, TestMemberFunctionFourParams)
{
    Foo foo;
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t)> f
        = ::estd::function<int32_t(float, int32_t, int32_t, int32_t)>::create<Foo, &Foo::foo4>(foo);
    ASSERT_EQ(5, f(3.14, 3, 4, 5));
    using bnd = ::estd::binder1st<int32_t(float, int32_t, int32_t, int32_t)>;
    bnd g     = ::estd::bind1st(f, 3.14F);
    ASSERT_EQ(5, g(3, 4, 5));
}

TEST(Function, TestStaticMemberFunctionFourParams)
{
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t)> f
        = ::estd::function<int32_t(float, int32_t, int32_t, int32_t)>::create<&Foo::bar4>();
    ASSERT_EQ(5, f(5.75, 5, 4, 5));
}

TEST(Function, TestFunctorFourParams)
{
    Bar4 bar;
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t)> f(::estd::make_function(bar));
    ASSERT_EQ(5, f(7.65, 7, 4, 5));
}

/*
 * Tests for 5 parameters
 */
TEST(Function, TestFreeFunctionClosureFiveParam)
{
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t, int32_t)> f
        = ::estd::function<int32_t(float, int32_t, int32_t, int32_t, int32_t)>::create<&foo5>();
    ::estd::closure<int32_t(float, int32_t, int32_t, int32_t, int32_t)> d
        = ::estd::bind_all(f, 2.14f, int32_t(3), int32_t(4), int32_t(5), int32_t(6));
    ASSERT_EQ(6, d());
}

TEST(Function, TestFreeFunctionFiveParams)
{
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t, int32_t)> f
        = ::estd::function<int32_t(float, int32_t, int32_t, int32_t, int32_t)>::create<&foo5>();
    ASSERT_EQ(7, f(2.14, 2, 4, 5, 7));
}

TEST(Function, TestBind1stFiveParams)
{
    using fct = ::estd::function<int32_t(
        int32_t const, int32_t const, int32_t const, int32_t const, int32_t const)>;
    fct f     = fct::create<&foobar5>();
    using bnd = ::estd::binder1st<int32_t(
        int32_t const, int32_t const, int32_t const, int32_t const, int32_t const)>;
    bnd g     = ::estd::bind1st(f, int32_t(23));
    ASSERT_EQ(7, g(2, 4, 5, 7));
}

TEST(Function, TestMemberFunctionFiveParams)
{
    Foo foo;
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t, int32_t)> f
        = ::estd::function<int32_t(
            float, int32_t, int32_t, int32_t, int32_t)>::create<Foo, &Foo::foo5>(foo);
    ASSERT_EQ(7, f(3.14, 3, 4, 5, 7));
    using bnd = ::estd::binder1st<int32_t(float, int32_t, int32_t, int32_t, int32_t)>;
    bnd g     = ::estd::bind1st(f, 3.14F);
    ASSERT_EQ(7, g(3, 4, 5, 7));
}

TEST(Function, TestStaticMemberFunctionFiveParams)
{
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t, int32_t)> f
        = ::estd::function<int32_t(
            float, int32_t, int32_t, int32_t, int32_t)>::create<&Foo::bar5>();
    ASSERT_EQ(8, f(5.75, 5, 4, 5, 8));
}

TEST(Function, TestFunctorFiveParams)
{
    Bar5 bar;
    ::estd::function<int32_t(float, int32_t, int32_t, int32_t, int32_t)> f(
        ::estd::make_function(bar));
    ASSERT_EQ(8, f(7.65, 7, 4, 5, 8));
}

/*
 * Others
 */

#ifndef _MSC_VER
TEST(Function, CopyConstructorZeroParams)
{
    {
        ::estd::function<int32_t()> f = ::estd::function<int32_t()>::create<&foo0>();
        ::estd::function<int32_t()> g(f);
        ASSERT_EQ(2, f());
        ASSERT_EQ(2, g());
    }
    {
        Foo foo;
        ::estd::function<int32_t()> f = ::estd::function<int32_t()>::create<Foo, &Foo::foo0>(foo);
        ::estd::function<int32_t()> g(f);
        ASSERT_EQ(3, f());
        ASSERT_EQ(3, g());
    }
    {
        ::estd::function<int32_t()> f = ::estd::function<int32_t()>::create<&Foo::bar0>();
        ::estd::function<int32_t()> g(f);
        ASSERT_EQ(5, f());
        ASSERT_EQ(5, g());
    }
    {
        Bar0 bar;
        ::estd::function<int32_t()> f = ::estd::make_function(bar);
        ::estd::function<int32_t()> g = ::estd::make_function(f);
        ASSERT_EQ(7, f());
        ASSERT_EQ(7, g());
    }
}

TEST(Function, AssignmentOperatorZeroParams)
{
    {
        ::estd::function<int32_t()> f = ::estd::function<int32_t()>::create<&foo0>(), g;
        g                             = f;
        ASSERT_EQ(2, f());
        ASSERT_EQ(2, g());
    }
    {
        Foo foo;
        ::estd::function<int32_t()> f = ::estd::function<int32_t()>::create<Foo, &Foo::foo0>(foo),
                                    g;
        g = f;
        ASSERT_EQ(3, f());
        ASSERT_EQ(3, g());
    }
    {
        ::estd::function<int32_t()> f = ::estd::function<int32_t()>::create<&Foo::bar0>(), g;
        g                             = f;
        ASSERT_EQ(5, f());
        ASSERT_EQ(5, g());
    }
    {
        Bar0 bar;
        ::estd::function<int32_t()> f = ::estd::make_function(bar), g;
        g                             = f;
        ASSERT_EQ(7, f());
        ASSERT_EQ(7, g());
    }
}

#endif

class Const
{
    int32_t _i;

public:
    Const(int32_t i) : _i(i) {}

    int32_t get0() const { return _i; }

    int32_t get0_b() const { return _i * 2; }

    int32_t get1(Const&) const { return _i; }

    int32_t get1NotConst(Const const&) const { return _i; }

    int32_t get2(Const const&, Const const&) const { return _i; }

    int32_t get3(Const const&, Const const&, Const const&) const { return _i; }

    int32_t get4(Const const&, Const const&, Const const&, Const const&) const { return _i; }

    int32_t get5(Const const&, Const const&, Const const&, Const const&, Const const&) const
    {
        return _i;
    }
};

TEST(Function, ConstMemberFunction)
{
    Const i(17);
    ::estd::function<int32_t()> f = ::estd::function<int32_t()>::create<Const, &Const::get0>(i);
    ASSERT_EQ(17, f());

    Const const& c                 = i;
    ::estd::function<int32_t()> f2 = ::estd::function<int32_t()>::create<Const, &Const::get0>(c);
    ASSERT_EQ(17, f2());

    ::estd::function<int32_t(Const&)> f3
        = ::estd::function<int32_t(Const&)>::create<Const, &Const::get1>(c);
    ASSERT_EQ(17, f3(i));

    ::estd::function<int32_t(Const const&)> f3NotConst
        = ::estd::function<int32_t(Const const&)>::create<Const, &Const::get1NotConst>(i);
    ASSERT_EQ(17, f3NotConst(i));

    ::estd::function<int32_t(Const const&, Const const&)> f4
        = ::estd::function<int32_t(Const const&, Const const&)>::create<Const, &Const::get2>(c);
    ASSERT_EQ(17, f4(i, i));
    ASSERT_EQ(17, f4(c, c));

    ::estd::function<int32_t(Const const&, Const const&, Const const&)> f5
        = ::estd::function<int32_t(
            Const const&, Const const&, Const const&)>::create<Const, &Const::get3>(c);
    ASSERT_EQ(17, f5(i, i, i));
    ASSERT_EQ(17, f5(c, c, c));

    ::estd::function<int32_t(Const const&, Const const&, Const const&, Const const&)> f6
        = ::estd::function<int32_t(Const const&, Const const&, Const const&, Const const&)>::
            create<Const, &Const::get4>(c);
    ASSERT_EQ(17, f6(i, i, i, i));
    ASSERT_EQ(17, f6(c, c, c, c));

    ::estd::function<int32_t(Const const&, Const const&, Const const&, Const const&, Const const&)>
        f7 = ::estd::function<int32_t(
            Const const&, Const const&, Const const&, Const const&, Const const&)>::
            create<Const, &Const::get5>(c);
    ASSERT_EQ(17, f7(i, i, i, i, i));
    ASSERT_EQ(17, f7(c, c, c, c, c));
}

TEST(Function, Comparison)
{
    Const a(1);
    Const b(2);

    // Same target
    auto const f  = ::estd::function<int32_t()>::create<Const, &Const::get0>(a);
    auto const f2 = ::estd::function<int32_t()>::create<Const, &Const::get0>(a);
    EXPECT_TRUE(f == f2);

    // Same object, different member function
    auto const f3 = ::estd::function<int32_t()>::create<Const, &Const::get0_b>(a);
    EXPECT_FALSE(f == f3);

    // Different object, same member function
    auto const f4 = ::estd::function<int32_t()>::create<Const, &Const::get0>(b);
    EXPECT_FALSE(f == f4);
}

} // namespace
