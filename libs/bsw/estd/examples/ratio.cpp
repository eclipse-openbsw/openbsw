// Copyright 2024 Accenture.

#include <estd/ratio.h>

#include <gtest/gtest.h>

TEST(RatioExample, basic_functionality)
{
    // [EXAMPLE_RATIO_BASIC_FUNCTIONALITY_START]
    // num is used to get the numerator of a ratio.
    static_assert((3 == ::estd::ratio<3, 5>::num), "");

    // den is used to get the denominator of a ratio.
    static_assert((5 == ::estd::ratio<3, 5>::den), "");

    static_assert(std::is_same<::estd::ratio<1, 5>::type, ::estd::ratio<1, 5>>::value, "");

    static_assert(std::is_same<::estd::ratio<10, 50>::type, ::estd::ratio<1, 5>>::value, "");

    static_assert(std::is_same<::estd::ratio<-1, -5>::type, ::estd::ratio<1, 5>>::value, "");

    static_assert(std::is_same<::estd::ratio<-10, 50>::type, ::estd::ratio<-1, 5>>::value, "");

    // [EXAMPLE_RATIO_BASIC_FUNCTIONALITY_END]
}

TEST(RatioExample, comparison_operators)
{
    // [EXAMPLE_RATIO_COMPARISON_OPERATORS_START]
    static_assert(::estd::ratio_equal<::estd::ratio<1, 5>, ::estd::ratio<2, 10>>::value, "");

    static_assert(!::estd::ratio_not_equal<::estd::ratio<1, 5>, ::estd::ratio<2, 10>>::value, "");

    static_assert(!::estd::ratio_less<::estd::ratio<1, 5>, ::estd::ratio<2, 10>>::value, "");

    static_assert(!::estd::ratio_greater<::estd::ratio<1, 5>, ::estd::ratio<2, 10>>::value, "");

    static_assert(::estd::ratio_less_equal<::estd::ratio<1, 5>, ::estd::ratio<2, 10>>::value, "");

    static_assert(
        ::estd::ratio_greater_equal<::estd::ratio<1, 5>, ::estd::ratio<2, 10>>::value, "");
    // [EXAMPLE_RATIO_COMPARISON_OPERATORS_END]
}

TEST(RatioExample, arithmetic_operators)
{
    // [EXAMPLE_RATIO_ARITHMETIC_OPERATORS_START]
    {
        static_assert(
            std::is_same<
                ::estd::ratio<11, 30>,
                ::estd::ratio_add<::estd::ratio<1, 5>, ::estd::ratio<1, 6>>::type>::value,
            "");

        static_assert(
            std::is_same<
                ::estd::ratio<1, 30>,
                ::estd::ratio_subtract<::estd::ratio<1, 5>, ::estd::ratio<1, 6>>::type>::value,
            "");

        static_assert(
            std::is_same<
                ::estd::ratio<7, 15>,
                ::estd::ratio_multiply<::estd::ratio<2, 5>, ::estd::ratio<7, 6>>::type>::value,
            "");

        static_assert(
            std::is_same<
                ::estd::ratio<4, 3>,
                ::estd::ratio_divide<::estd::ratio<2, 4>, ::estd::ratio<3, 8>>::type>::value,
            "");
    }
    // [EXAMPLE_RATIO_ARITHMETIC_OPERATORS_END]
}

TEST(RatioExample, predefined_types)
{
    // [EXAMPLE_RATIO_PREDEFINED_TYPES_START]
    static_assert(std::is_same<::estd::ratio<1000000000000000000, 1>, ::estd::exa>::value, "");

    static_assert(std::is_same<::estd::ratio<1000000000000000, 1>, ::estd::peta>::value, "");

    // [EXAMPLE_RATIO_PREDEFINED_TYPES_END]
}
