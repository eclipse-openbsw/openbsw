// Copyright 2024 Accenture.

#include "estd/type_traits.h"

#include "estd/static_assert.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

using namespace ::testing;

namespace
{
// clang-format off
struct A {};
struct B {};
struct C : public B {};

// clang-format on

struct Call0
{
    void operator()() {}
};

struct Call1
{
    int operator()() { return 9; }

    int operator()(int) { return 9; }

    int operator()(int, int) { return 9; }
};

} // namespace

int callable_test_function() { return 9; }

TEST(TypeTraits, TestIsCallable)
{
    static_assert(::estd::is_callable<decltype(callable_test_function)>::value, "");
    static_assert(::estd::is_callable<Call0>::value, "");
    static_assert(::estd::is_callable<Call1>::value, "");
    static_assert(::estd::is_callable<void (*)()>::value, "");

    auto l = [](int a, int b) { return a + b; };
    void(l(1, 2));
    static_assert(::estd::is_callable<decltype(l)>::value, "");

    int temp       = 0;
    auto l_capture = [&](int a) { return a + temp; };
    void(l_capture(1));
    static_assert(::estd::is_callable<decltype(l_capture)>::value, "");

    static_assert(!::estd::is_callable<A>::value, "");
    static_assert(!::estd::is_callable<C>::value, "");
    static_assert(!::estd::is_callable<int>::value, "");
}

TEST(TypeTraits, TestIsConst)
{
    static_assert(!::estd::is_const<int>::value, "");
    static_assert(::estd::is_const<int const>::value, "");

    int i        = 0;
    int const ci = 0;

    ASSERT_FALSE(::estd::value_trait<::estd::is_const>(i));
    ASSERT_TRUE(::estd::value_trait<::estd::is_const>(ci));
}

TEST(TypeTraits, TestMakeSigned)
{
    ESTD_STATIC_ASSERT((::estd::is_same<bool, ::estd::make_signed<bool>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int8_t, ::estd::make_signed<int8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int8_t, ::estd::make_signed<uint8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int16_t, ::estd::make_signed<int16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int16_t, ::estd::make_signed<uint16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int32_t, ::estd::make_signed<int32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int32_t, ::estd::make_signed<uint32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::make_signed<int64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::make_signed<uint64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<A, ::estd::make_signed<A>::type>::value));
}

TEST(TypeTraits, TestMakeUnsigned)
{
    ESTD_STATIC_ASSERT((::estd::is_same<bool, ::estd::make_unsigned<bool>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint8_t, ::estd::make_unsigned<int8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint8_t, ::estd::make_unsigned<uint8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint16_t, ::estd::make_unsigned<int16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint16_t, ::estd::make_unsigned<uint16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint32_t, ::estd::make_unsigned<int32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint32_t, ::estd::make_unsigned<uint32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::make_unsigned<int64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::make_unsigned<uint64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<A, ::estd::make_unsigned<A>::type>::value));
}

TEST(TypeTraits, TestCommonIntegralType)
{
    // clang-format off
    ESTD_STATIC_ASSERT((::estd::is_same<uint8_t, ::estd::internal::_common_integral_type<uint8_t, uint8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint8_t, ::estd::internal::_common_integral_type<uint8_t, int8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint16_t, ::estd::internal::_common_integral_type<uint8_t, uint16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int16_t, ::estd::internal::_common_integral_type<uint8_t, int16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint32_t, ::estd::internal::_common_integral_type<uint8_t, uint32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int32_t, ::estd::internal::_common_integral_type<uint8_t, int32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<uint8_t, uint64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::internal::_common_integral_type<uint8_t, int64_t>::type>::value));

    ESTD_STATIC_ASSERT((::estd::is_same<uint8_t, ::estd::internal::_common_integral_type<int8_t, uint8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int8_t, ::estd::internal::_common_integral_type<int8_t, int8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint16_t, ::estd::internal::_common_integral_type<int8_t, uint16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int16_t, ::estd::internal::_common_integral_type<int8_t, int16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint32_t, ::estd::internal::_common_integral_type<int8_t, uint32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int32_t, ::estd::internal::_common_integral_type<int8_t, int32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<int8_t, uint64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::internal::_common_integral_type<int8_t, int64_t>::type>::value));

    ESTD_STATIC_ASSERT((::estd::is_same<uint16_t, ::estd::internal::_common_integral_type<uint16_t, uint8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint16_t, ::estd::internal::_common_integral_type<uint16_t, int8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint16_t, ::estd::internal::_common_integral_type<uint16_t, uint16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint16_t, ::estd::internal::_common_integral_type<uint16_t, int16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint32_t, ::estd::internal::_common_integral_type<uint16_t, uint32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int32_t, ::estd::internal::_common_integral_type<uint16_t, int32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<uint16_t, uint64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::internal::_common_integral_type<uint16_t, int64_t>::type>::value));

    ESTD_STATIC_ASSERT((::estd::is_same<int16_t, ::estd::internal::_common_integral_type<int16_t, uint8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int16_t, ::estd::internal::_common_integral_type<int16_t, int8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint16_t, ::estd::internal::_common_integral_type<int16_t, uint16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int16_t, ::estd::internal::_common_integral_type<int16_t, int16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint32_t, ::estd::internal::_common_integral_type<int16_t, uint32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int32_t, ::estd::internal::_common_integral_type<int16_t, int32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<int16_t, uint64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::internal::_common_integral_type<int16_t, int64_t>::type>::value));

    ESTD_STATIC_ASSERT((::estd::is_same<uint32_t, ::estd::internal::_common_integral_type<uint32_t, uint8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint32_t, ::estd::internal::_common_integral_type<uint32_t, int8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint32_t, ::estd::internal::_common_integral_type<uint32_t, uint16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint32_t, ::estd::internal::_common_integral_type<uint32_t, int16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint32_t, ::estd::internal::_common_integral_type<uint32_t, uint32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint32_t, ::estd::internal::_common_integral_type<uint32_t, int32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<uint32_t, uint64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::internal::_common_integral_type<uint32_t, int64_t>::type>::value));

    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::internal::_common_integral_type<int64_t, uint8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::internal::_common_integral_type<int64_t, int8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::internal::_common_integral_type<int64_t, uint16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::internal::_common_integral_type<int64_t, int16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::internal::_common_integral_type<int64_t, uint32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::internal::_common_integral_type<int64_t, int32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<int64_t, uint64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::internal::_common_integral_type<int64_t, int64_t>::type>::value));

    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<uint64_t, uint8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<uint64_t, int8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<uint64_t, uint16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<uint64_t, int16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<uint64_t, uint32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<uint64_t, int32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<uint64_t, uint64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<uint64_t, ::estd::internal::_common_integral_type<uint64_t, int64_t>::type>::value));

    ESTD_STATIC_ASSERT((::estd::is_same<unsigned long, ::estd::internal::_common_integral_type<unsigned long, int>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<unsigned long, ::estd::internal::_common_integral_type<int, unsigned long>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<long, ::estd::internal::_common_integral_type<long, int>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<long, ::estd::internal::_common_integral_type<int, long>::type>::value));
    // clang-format on
}

TEST(TypeTraits, is_base_of)
{
    ESTD_STATIC_ASSERT((::estd::is_base_of<B, C>::value));
    ESTD_STATIC_ASSERT(!(::estd::is_base_of<C, B>::value));
    ESTD_STATIC_ASSERT((::estd::is_base_of<B, B>::value));
    ESTD_STATIC_ASSERT(!(::estd::is_base_of<B, A>::value));
}
