// Copyright 2024 Accenture.

#include "platform/atomic.h"

#include <gtest/gtest.h>

#include <limits>

// sanity tests

template<typename T>
class AtomicSanityTest : public ::testing::Test
{};

typedef ::testing::Types<
    char,
    signed char,
    unsigned char,
    wchar_t,

    short,
    short int,
    signed short,
    signed short int,

    unsigned short,
    unsigned short int,

    int,
    signed,
    signed int,

    unsigned,
    unsigned int,

    long,
    long int,
    signed long,
    signed long int,

    unsigned long,
    unsigned long int,

    long long,
    long long int,
    signed long long,
    signed long long int,

    unsigned long long,
    unsigned long long int,

    float,
    double>
    TestTypes;

TYPED_TEST_SUITE(AtomicSanityTest, TestTypes);

/**
 * \refs:    SMD_platform_atomic
 * \desc
 * This test assures that atomics can be constructed with a given value.
 */
TYPED_TEST(AtomicSanityTest, init)
{
    ::platform::atomic<TypeParam> value(5);
    EXPECT_EQ(5, value.load());
}

/**
 * \refs:    SMD_platform_atomic
 * \desc
 * This test assures that atomics can store a given value.
 */
TYPED_TEST(AtomicSanityTest, store)
{
    ::platform::atomic<TypeParam> value{};
    value.store(7);
    EXPECT_EQ(7, value.load());
}

/**
 * \refs:    SMD_platform_atomic
 * \desc
 * This test assures that atomics can store the maximum value of the underlying type.
 */
TYPED_TEST(AtomicSanityTest, storeMax)
{
    ::platform::atomic<TypeParam> value{};
    value.store(::std::numeric_limits<TypeParam>::max());
    EXPECT_EQ(::std::numeric_limits<TypeParam>::max(), value.load());
}

struct PodTest
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
};

/**
 * \refs:    SMD_platform_atomic
 * \desc
 * This test assures that atomics can store also POD types that fit into the supported size
 * of atomic types.
 */
TEST(AtomicSanityTest, podStruct)
{
    PodTest initValue = {15U, 77U, 255U};
    ::platform::atomic<PodTest> value{};

    value.store(initValue);

    PodTest result = value.load();

    EXPECT_EQ(15U, result.a);
    EXPECT_EQ(77U, result.b);
    EXPECT_EQ(255U, result.c);
}

/**
 * \refs:    SMD_platform_atomic
 * \desc
 * This test assures that atomics can store values of type bool.
 */
TEST(AtomicSanityTest, boolean)
{
    ::platform::atomic<bool> value{};

    value.store(false);
    EXPECT_FALSE(value.load());

    value.store(true);
    EXPECT_TRUE(value.load());

    value.store(false);
    EXPECT_FALSE(value.load());
}

/**
 * \refs:    SMD_platform_atomic
 * \desc
 * This test assures that atomics can store values of type pointer.
 */
TEST(AtomicSanityTest, ptr)
{
    uint8_t tmp = 5U;
    ::platform::atomic<uint8_t*> value{};

    value.store(&tmp);
    EXPECT_EQ(5U, *value.load());
}

/**
 * \refs:    SMD_platform_SupportedSize
 * \desc
 * This test assures that atomic_base can be instantiated with up to 8 bytes.
 */
TEST(AtomicSanityTest, compileTest)
{
    ::platform::internal::atomic_type_selector<1U>::type t1;
    ::platform::internal::atomic_type_selector<2U>::type t2;
    ::platform::internal::atomic_type_selector<3U>::type t3;
    ::platform::internal::atomic_type_selector<4U>::type t4;
    ::platform::internal::atomic_type_selector<5U>::type t5;
    ::platform::internal::atomic_type_selector<6U>::type t6;
    ::platform::internal::atomic_type_selector<7U>::type t7;
    ::platform::internal::atomic_type_selector<8U>::type t8;

    ::platform::internal::atomic_base<uint8_t[1U]> v1{};
    ::platform::internal::atomic_base<uint8_t[2U]> v2{};
    ::platform::internal::atomic_base<uint8_t[3U]> v3{};
    ::platform::internal::atomic_base<uint8_t[4U]> v4{};
    ::platform::internal::atomic_base<uint8_t[5U]> v5{};
    ::platform::internal::atomic_base<uint8_t[6U]> v6{};
    ::platform::internal::atomic_base<uint8_t[7U]> v7{};
    ::platform::internal::atomic_base<uint8_t[8U]> v8{};

    // clang-format off
    (void)t1; (void)t5; (void)v1; (void)v5;
    (void)t2; (void)t6; (void)v2; (void)v6;
    (void)t3; (void)t7; (void)v3; (void)v7;
    (void)t4; (void)t8; (void)v4; (void)v8;
    // clang-format on
}
