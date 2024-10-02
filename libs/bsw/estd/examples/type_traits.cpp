// Copyright 2024 Accenture.

#include "estd/type_traits.h"

#include "estd/static_assert.h"
#include "estd/type_utils.h"

#include <platform/estdint.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <type_traits>

using namespace ::testing;

namespace
{
// [EXAMPLE_TYPE_TRAITS_SUPPORT_START]
union tempU
{};

struct tempA
{};

struct tempB
{};

struct tempC : public tempB
{};

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

// [EXAMPLE_TYPE_TRAITS_SUPPORT_END]
} // namespace

TEST(TypeTraitsExample, TestIsClass)
{
    // [EXAMPLE_TYPE_TRAITS_IS_CLASS_START]
    ESTD_STATIC_ASSERT(::estd::is_class<Call0>::value);
    ESTD_STATIC_ASSERT(::estd::is_class<Call1>::value);
    ESTD_STATIC_ASSERT(::estd::is_class<tempA>::value);
    ESTD_STATIC_ASSERT(::estd::is_class<tempC>::value);
    // [EXAMPLE_TYPE_TRAITS_IS_CLASS_END]
}

TEST(TypeTraitsExample, TestIsCallable)
{
    // [EXAMPLE_TYPE_TRAITS_IS_CALLABLE_START]
    ESTD_STATIC_ASSERT(::estd::is_callable<Call0>::value);
    ESTD_STATIC_ASSERT(::estd::is_callable<Call1>::value);
    ESTD_STATIC_ASSERT(::estd::is_callable<void (*)()>::value);
    ESTD_STATIC_ASSERT(!::estd::is_callable<tempA>::value);
    ESTD_STATIC_ASSERT(!::estd::is_callable<tempC>::value);
    ESTD_STATIC_ASSERT(!::estd::is_callable<int>::value);
    // [EXAMPLE_TYPE_TRAITS_IS_CALLABLE__END]
}

TEST(TypeTraitsExample, TestIsSigned)
{
    // [EXAMPLE_TYPE_TRAITS_IS_SIGNED_START]
    ESTD_STATIC_ASSERT((::estd::is_same<bool, ::estd::make_signed<bool>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int8_t, ::estd::make_signed<int8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int8_t, ::estd::make_signed<uint8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int16_t, ::estd::make_signed<int16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int16_t, ::estd::make_signed<uint16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int32_t, ::estd::make_signed<int32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int32_t, ::estd::make_signed<uint32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::make_signed<int64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::make_signed<uint64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<tempA, ::estd::make_signed<tempA>::type>::value));
    // [EXAMPLE_TYPE_TRAITS_IS_SIGNED_END]
}

TEST(TypeTraitsExample, is_base_of)
{
    // [EXAMPLE_TYPE_TRAITS_IS_BASE_OF_START]
    ESTD_STATIC_ASSERT((::estd::is_base_of<tempB, tempC>::value));
    ESTD_STATIC_ASSERT(!(::estd::is_base_of<tempC, tempB>::value));
    ESTD_STATIC_ASSERT((::estd::is_base_of<tempB, tempB>::value));
    ESTD_STATIC_ASSERT(!(::estd::is_base_of<tempB, tempA>::value));
    // [EXAMPLE_TYPE_TRAITS_IS_BASE_OF_END]
}

TEST(TypeTraitsExample, TestMakeSigned)
{
    // [EXAMPLE_TYPE_TRAITS_MAKE_SIGNED_START]
    ESTD_STATIC_ASSERT((::estd::is_same<bool, ::estd::make_signed<bool>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int8_t, ::estd::make_signed<int8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int8_t, ::estd::make_signed<uint8_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int16_t, ::estd::make_signed<int16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int16_t, ::estd::make_signed<uint16_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int32_t, ::estd::make_signed<int32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int32_t, ::estd::make_signed<uint32_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::make_signed<int64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<int64_t, ::estd::make_signed<uint64_t>::type>::value));
    ESTD_STATIC_ASSERT((::estd::is_same<tempA, ::estd::make_signed<tempA>::type>::value));
    // [EXAMPLE_TYPE_TRAITS_MAKE_SIGNED_END]
}
