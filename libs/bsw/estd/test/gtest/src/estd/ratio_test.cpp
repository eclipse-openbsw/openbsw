// Copyright 2024 Accenture.

#include "estd/ratio.h"

#ifndef ESTD_NO_CHRONO

#include <platform/estdint.h>

#include <gtest/gtest.h>

using namespace ::testing;

// explicit template instantiation

namespace estd
{
namespace internal
{
template struct _static_sign<10>;
template struct _static_abs<10>;
template struct _static_gcd<10, 20>;
template struct _static_csb<10>;
template struct _static_clz<10>;
template struct _safe_multiply<10, 20>;
template struct _big_less<10ULL, 20ULL, 30ULL, 40ULL>;
template struct _big_add<10ULL, 20ULL, 30ULL, 40ULL>;
template struct _big_sub<40ULL, 30ULL, 20ULL, 10ULL>;
template struct _big_mul<10ULL, 20ULL>;
template struct _big_div<10ULL, 20ULL, 30ULL>;

} // namespace internal

template class ratio<10, 20>;
} // namespace estd

#define static_assert_eq(EXPECTED, TESTED) \
    static_assert((EXPECTED == TESTED), #TESTED " should equal " #EXPECTED)

TEST(Ratio, TestBasics)
{
    static_assert_eq(3, (::estd::ratio<3, 5>::num));
    static_assert_eq(5, (::estd::ratio<3, 5>::den));
    static_assert(std::is_same<::estd::ratio<1, 5>::type, ::estd::ratio<1, 5>>::value, "");
    static_assert(std::is_same<::estd::ratio<10, 50>::type, ::estd::ratio<1, 5>>::value, "");
    static_assert(std::is_same<::estd::ratio<-1, -5>::type, ::estd::ratio<1, 5>>::value, "");
    static_assert(std::is_same<::estd::ratio<-10, 50>::type, ::estd::ratio<-1, 5>>::value, "");
    static_assert(std::is_same<::estd::ratio<10, -50>::type, ::estd::ratio<-1, 5>>::value, "");
    static_assert(std::is_same<::estd::ratio<50, 10>::type, ::estd::ratio<5, 1>>::value, "");
}

TEST(Ratio, TestComparisonOperators)
{
    static_assert(::estd::ratio_equal<::estd::ratio<1, 5>, ::estd::ratio<2, 10>>::value, "");
    static_assert(!::estd::ratio_equal<::estd::ratio<1, 5>, ::estd::ratio<2, 5>>::value, "");
    static_assert(::estd::ratio_equal<::estd::ratio<3, 28>, ::estd::ratio<3, 28>>::value, "");
    static_assert(!::estd::ratio_equal<::estd::ratio<3, 28>, ::estd::ratio<3, 27>>::value, "");

    static_assert(!::estd::ratio_not_equal<::estd::ratio<1, 5>, ::estd::ratio<2, 10>>::value, "");
    static_assert(::estd::ratio_not_equal<::estd::ratio<1, 5>, ::estd::ratio<2, 5>>::value, "");
    static_assert(!::estd::ratio_not_equal<::estd::ratio<3, 28>, ::estd::ratio<3, 28>>::value, "");
    static_assert(::estd::ratio_not_equal<::estd::ratio<3, 28>, ::estd::ratio<3, 27>>::value, "");

    static_assert(!::estd::ratio_less<::estd::ratio<1, 5>, ::estd::ratio<2, 10>>::value, "");
    static_assert(::estd::ratio_less<::estd::ratio<1, 5>, ::estd::ratio<2, 5>>::value, "");
    static_assert(!::estd::ratio_less<::estd::ratio<3, 28>, ::estd::ratio<3, 28>>::value, "");
    static_assert(::estd::ratio_less<::estd::ratio<3, 28>, ::estd::ratio<3, 27>>::value, "");

    static_assert(!::estd::ratio_greater<::estd::ratio<1, 5>, ::estd::ratio<2, 10>>::value, "");
    static_assert(!::estd::ratio_greater<::estd::ratio<1, 5>, ::estd::ratio<2, 5>>::value, "");
    static_assert(!::estd::ratio_greater<::estd::ratio<3, 28>, ::estd::ratio<3, 28>>::value, "");
    static_assert(!::estd::ratio_greater<::estd::ratio<3, 28>, ::estd::ratio<3, 27>>::value, "");

    static_assert(::estd::ratio_less_equal<::estd::ratio<1, 5>, ::estd::ratio<2, 10>>::value, "");
    static_assert(::estd::ratio_less_equal<::estd::ratio<1, 5>, ::estd::ratio<2, 5>>::value, "");
    static_assert(!::estd::ratio_less_equal<::estd::ratio<4, 28>, ::estd::ratio<3, 28>>::value, "");
    static_assert(::estd::ratio_less_equal<::estd::ratio<3, 28>, ::estd::ratio<3, 27>>::value, "");

    static_assert(
        ::estd::ratio_greater_equal<::estd::ratio<1, 5>, ::estd::ratio<2, 10>>::value, "");
    static_assert(
        !::estd::ratio_greater_equal<::estd::ratio<1, 5>, ::estd::ratio<2, 5>>::value, "");
    static_assert(
        ::estd::ratio_greater_equal<::estd::ratio<4, 28>, ::estd::ratio<3, 28>>::value, "");
    static_assert(
        !::estd::ratio_greater_equal<::estd::ratio<3, 28>, ::estd::ratio<3, 27>>::value, "");
}

TEST(Ratio, TestArithmeticOperators)
{
    static_assert(
        std::is_same<
            ::estd::ratio<11, 30>,
            ::estd::ratio_add<::estd::ratio<1, 5>, ::estd::ratio<1, 6>>::type>::value,
        "");
    static_assert(
        std::is_same<
            ::estd::ratio<-1, 30>,
            ::estd::ratio_add<::estd::ratio<-1, 5>, ::estd::ratio<1, 6>>::type>::value,
        "");
    static_assert(
        std::is_same<
            ::estd::ratio<12, 17>,
            ::estd::ratio_add<::estd::ratio<4, 17>, ::estd::ratio<8, 17>>::type>::value,
        "");

    static_assert(
        std::is_same<
            ::estd::ratio<1, 30>,
            ::estd::ratio_subtract<::estd::ratio<1, 5>, ::estd::ratio<1, 6>>::type>::value,
        "");
    static_assert(
        std::is_same<
            ::estd::ratio<-11, 30>,
            ::estd::ratio_subtract<::estd::ratio<-1, 5>, ::estd::ratio<1, 6>>::type>::value,
        "");
    static_assert(
        std::is_same<
            ::estd::ratio<-4, 17>,
            ::estd::ratio_subtract<::estd::ratio<4, 17>, ::estd::ratio<8, 17>>::type>::value,
        "");

    static_assert(
        std::is_same<
            ::estd::ratio<7, 15>,
            ::estd::ratio_multiply<::estd::ratio<2, 5>, ::estd::ratio<7, 6>>::type>::value,
        "");
    static_assert(
        std::is_same<
            ::estd::ratio<-7, 15>,
            ::estd::ratio_multiply<::estd::ratio<-2, 5>, ::estd::ratio<7, 6>>::type>::value,
        "");
    static_assert(
        std::is_same<
            ::estd::ratio<3, 16>,
            ::estd::ratio_multiply<::estd::ratio<2, 4>, ::estd::ratio<3, 8>>::type>::value,
        "");

    static_assert(
        std::is_same<
            ::estd::ratio<12, 35>,
            ::estd::ratio_divide<::estd::ratio<2, 5>, ::estd::ratio<7, 6>>::type>::value,
        "");
    static_assert(
        std::is_same<
            ::estd::ratio<-12, 35>,
            ::estd::ratio_divide<::estd::ratio<-2, 5>, ::estd::ratio<7, 6>>::type>::value,
        "");
    static_assert(
        std::is_same<
            ::estd::ratio<4, 3>,
            ::estd::ratio_divide<::estd::ratio<2, 4>, ::estd::ratio<3, 8>>::type>::value,
        "");
}

TEST(Ratio, TestPredefinedTypes)
{
    static_assert(std::is_same<::estd::ratio<1000000000000000000, 1>, ::estd::exa>::value, "");
    static_assert(std::is_same<::estd::ratio<1000000000000000, 1>, ::estd::peta>::value, "");
    static_assert(std::is_same<::estd::ratio<1000000000000, 1>, ::estd::tera>::value, "");
    static_assert(std::is_same<::estd::ratio<1000000000, 1>, ::estd::giga>::value, "");
    static_assert(std::is_same<::estd::ratio<1000000, 1>, ::estd::mega>::value, "");
    static_assert(std::is_same<::estd::ratio<1000, 1>, ::estd::kilo>::value, "");
    static_assert(std::is_same<::estd::ratio<100, 1>, ::estd::hecto>::value, "");
    static_assert(std::is_same<::estd::ratio<10, 1>, ::estd::deca>::value, "");
    static_assert(std::is_same<::estd::ratio<1, 10>, ::estd::deci>::value, "");
    static_assert(std::is_same<::estd::ratio<1, 100>, ::estd::centi>::value, "");
    static_assert(std::is_same<::estd::ratio<1, 1000>, ::estd::milli>::value, "");
    static_assert(std::is_same<::estd::ratio<1, 1000000>, ::estd::micro>::value, "");
    static_assert(std::is_same<::estd::ratio<1, 1000000000>, ::estd::nano>::value, "");
    static_assert(std::is_same<::estd::ratio<1, 1000000000000>, ::estd::pico>::value, "");
    static_assert(std::is_same<::estd::ratio<1, 1000000000000000>, ::estd::femto>::value, "");
    static_assert(std::is_same<::estd::ratio<1, 1000000000000000000>, ::estd::atto>::value, "");
}

TEST(Ratio, Limits)
{
    static_assert_eq(INTMAX_MAX, (::estd::ratio<INTMAX_MAX, 1>::num));
    static_assert_eq(1, (::estd::ratio<INTMAX_MAX, 1>::den));
    static_assert_eq(
        1,
        (::estd::ratio_multiply<::estd::ratio<INTMAX_MAX, 1>, ::estd::ratio<1, INTMAX_MAX>>::num));
    static_assert_eq(
        1,
        (::estd::ratio_multiply<::estd::ratio<INTMAX_MAX, 1>, ::estd::ratio<1, INTMAX_MAX>>::den));
    static_assert_eq(
        INTMAX_MAX, (::estd::ratio_divide<::estd::ratio<INTMAX_MAX, 1>, ::estd::ratio<1, 1>>::num));
    static_assert_eq(
        1, (::estd::ratio_divide<::estd::ratio<INTMAX_MAX, 1>, ::estd::ratio<1, 1>>::den));
}

#endif // ifndef ESTD_NO_CHRONO
