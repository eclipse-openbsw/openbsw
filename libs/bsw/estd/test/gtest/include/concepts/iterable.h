// Copyright 2024 Accenture.

#pragma once

#include "estd/type_traits.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <type_traits>

using namespace ::testing;

namespace estd
{
namespace test
{
/**
 * Test templates for types that provide iteration facilities.
 * Must be instantiated with a struct of the form:
 *
 *  struct $Name$_P
 *  {
 *      enum { LENGTH = $number of elements$ };
 *      typedef $type$ Subject;
 *      $declaration type$ subject;
 *  };
 *
 * Where $type$ is the type under test, and 'subject' an instance of it.
 * Iterating over 'subject' must yield exactly 'LENGTH' iterations,
 * with ascending integer values starting from 1.
 * If additional setup is required to prepare 'subject' for the test
 * it should be performed in the constructor of the above struct.
 */

template<typename T>
struct ForwardIterableConcept : ::testing::Test
{
    T t;
};

template<typename T>
struct ReverseIterableConcept : ::testing::Test
{
    T t;
};

TYPED_TEST_SUITE_P(ForwardIterableConcept);
TYPED_TEST_SUITE_P(ReverseIterableConcept);

TYPED_TEST_P(ForwardIterableConcept, has_begin_and_end_methods_yielding_forward_iterators)
{
    using iterator = typename TypeParam::Subject::iterator;
    int32_t i      = 0;
    for (iterator iter = this->t.subject.begin(); iter != this->t.subject.end(); ++iter)
    {
        ++i;
        EXPECT_FALSE(::estd::value_trait<std::is_const>(*iter));
        EXPECT_EQ(i, *iter);
    }
    EXPECT_THAT(i, Eq(this->t.LENGTH));
}

TYPED_TEST_P(ForwardIterableConcept, has_begin_and_end_methods_yielding_const_forward_iterators)
{
    using iterator = typename TypeParam::Subject::const_iterator;

    int32_t i = 0;
    for (iterator iter = this->t.subject.cbegin(); iter != this->t.subject.cend(); ++iter)
    {
        ++i;
        EXPECT_TRUE(::estd::value_trait<std::is_const>(*iter));
        EXPECT_EQ(i, *iter);
    }
    EXPECT_THAT(i, Eq(this->t.LENGTH));
}

TYPED_TEST_P(ForwardIterableConcept, has_cbegin_and_cend_methods_yielding_const_forward_iterators)
{
    using iterator = typename TypeParam::Subject::const_iterator;
    int32_t i      = 0;
    for (iterator iter = this->t.subject.cbegin(); iter != this->t.subject.cend(); ++iter)
    {
        ++i;
        EXPECT_TRUE(::estd::value_trait<std::is_const>(*iter));
        EXPECT_EQ(i, *iter);
    }
    EXPECT_THAT(i, Eq(this->t.LENGTH));
}

TYPED_TEST_P(ReverseIterableConcept, has_rbegin_and_rend_methods_yielding_reverse_iterators)
{
    using iterator = typename TypeParam::Subject::reverse_iterator;
    int32_t i      = this->t.LENGTH;
    for (iterator iter = this->t.subject.rbegin(); iter != this->t.subject.rend(); ++iter)
    {
        EXPECT_FALSE(::estd::value_trait<std::is_const>(*iter));
        EXPECT_EQ(i, *iter);
        --i;
    }
    EXPECT_EQ(0, i);
}

TYPED_TEST_P(ReverseIterableConcept, has_crbegin_and_crend_methods_yielding_const_reverse_iterators)
{
    using iterator = typename TypeParam::Subject::const_reverse_iterator;
    int32_t i      = this->t.LENGTH;
    for (iterator iter = this->t.subject.crbegin(); iter != this->t.subject.crend(); ++iter)
    {
        EXPECT_TRUE(::estd::value_trait<std::is_const>(*iter));
        EXPECT_EQ(i, *iter);
        --i;
    }
    EXPECT_EQ(0, i);
}

TYPED_TEST_P(ReverseIterableConcept, has_rbegin_and_rend_methods_yielding_const_reverse_iterators)
{
    using iterator = typename TypeParam::Subject::const_reverse_iterator;
    int32_t i      = this->t.LENGTH;
    for (iterator iter = this->t.subject.crbegin(); iter != this->t.subject.crend(); ++iter)
    {
        EXPECT_TRUE(::estd::value_trait<std::is_const>(*iter));
        EXPECT_EQ(i, *iter);
        --i;
    }
    EXPECT_EQ(0, i);
}

REGISTER_TYPED_TEST_SUITE_P(
    ForwardIterableConcept,
    has_begin_and_end_methods_yielding_forward_iterators,
    has_begin_and_end_methods_yielding_const_forward_iterators,
    has_cbegin_and_cend_methods_yielding_const_forward_iterators);

REGISTER_TYPED_TEST_SUITE_P(
    ReverseIterableConcept,
    has_rbegin_and_rend_methods_yielding_reverse_iterators,
    has_crbegin_and_crend_methods_yielding_const_reverse_iterators,
    has_rbegin_and_rend_methods_yielding_const_reverse_iterators);

} // namespace test
} // namespace estd

