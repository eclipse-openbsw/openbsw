// Copyright 2024 Accenture.

#include "estd/array.h"

#include "concepts/iterable.h"
#include "internal/TestClassCalls.h"

#include <gtest/gtest.h>

#include <array>
#include <type_traits>

using namespace ::testing;
using namespace ::estd::test;

// explicit instantiation to get correct test coverage
template class ::estd::array<int32_t, 10>;

using IntArray10 = ::estd::array<int32_t, 10>;

static_assert(std::is_same<int32_t, IntArray10::value_type>::value, "");
static_assert(std::is_same<int32_t*, IntArray10::pointer>::value, "");
static_assert(std::is_same<int32_t&, IntArray10::reference>::value, "");
static_assert(std::is_same<int32_t const*, IntArray10::const_pointer>::value, "");
static_assert(std::is_same<int32_t const&, IntArray10::const_reference>::value, "");
static_assert(std::is_same<int32_t*, IntArray10::iterator>::value, "");
static_assert(std::is_same<int32_t const*, IntArray10::const_iterator>::value, "");

using IntArrayData0  = ::estd::array_data<int32_t, 0>;
using IntArrayData10 = ::estd::array_data<int32_t, 10>;

static_assert(std::is_same<int32_t*, IntArrayData0::type>::value, "");
static_assert(std::is_same<int32_t[10], IntArrayData10::type>::value, "");

static_assert(sizeof(::estd::array<int32_t, 1>) == sizeof(int32_t), "");
static_assert(sizeof(::estd::array<int64_t, 11>) == sizeof(int64_t) * 11, "");
static_assert(sizeof(::estd::array<int8_t, 10005>) == 10005, "");

namespace
{
struct IntArray3_P
{
    enum Constants
    {
        LENGTH = 3
    };

    using Subject = ::estd::array<int32_t, LENGTH>;
    Subject subject;

    IntArray3_P()
    {
        for (size_t i = 0; i < LENGTH; ++i)
        {
            subject[i] = i + 1;
        }
    }
};

INSTANTIATE_TYPED_TEST_SUITE_P(Array, ForwardIterableConcept, IntArray3_P);
INSTANTIATE_TYPED_TEST_SUITE_P(Array, ReverseIterableConcept, IntArray3_P);

#define MAKE_ARRAY_FROM(T, N, Name) \
    ::estd::array<T, N> Name##_;    \
    ::estd::array<T, N>& Name = Name##_

/**
 * \refs:    SMD_estd_array
 * \desc
 * This test assures that aggregate initialization of an array initializes the array members
 * as expected with the values of the initializer list.
 */
TEST(Array, TestAggregateInitialization)
{
    ::estd::array<int32_t, 3> a = {{1, 2, 3}};
    ASSERT_EQ(1, a[0]);
    ASSERT_EQ(2, a[1]);
    ASSERT_EQ(3, a[2]);
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * This test assures that make_array() returns an array of size one.
 */
TEST(Array, Test_make_array)
{
    ::estd::array<int32_t, 1> a = ::estd::make_array<int32_t>(123);
    EXPECT_EQ(123, a[0]);

    ::estd::array<uint8_t, 1> b = ::estd::make_array<uint8_t>(222);
    EXPECT_EQ(222, b[0]);
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * This test that zero length arrays can exist and their iterators work, i.e. begin and end are the
 * same.
 */
TEST(Array, ZeroLength)
{
    ::estd::array<int32_t, 0> zeroLengthArray{};

    EXPECT_EQ(zeroLengthArray.begin(), zeroLengthArray.end());
    EXPECT_EQ(zeroLengthArray.rbegin(), zeroLengthArray.rend());
    EXPECT_EQ(zeroLengthArray.cbegin(), zeroLengthArray.cend());
    EXPECT_EQ(zeroLengthArray.crbegin(), zeroLengthArray.crend());
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Assures that the random access operator accesses elements at the given index.
 */
TEST(Array, TestRandomAccessOperator)
{
    // Create an uninitialized array.
    IntArray10 a;
    for (int32_t i = 0; i < 10; ++i)
    {
        // Write to position i.
        a[i] = i;
    }
    for (int32_t i = 0; i < 10; ++i)
    {
        // Check that write worked.
        ASSERT_EQ(i, a[i]);
    }
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Tests that front() returns a reference to the first element of an array.
 */
TEST(Array, TestFront)
{
    IntArray10 a;
    a[0] = 17;
    // Read using front()
    ASSERT_EQ(17, a.front());
    // Write using front()
    a.front() = 18;
    ASSERT_EQ(18, a[0]);
    ASSERT_EQ(a[0], a.front());

    IntArray10 const& ca = a;
    ASSERT_EQ(ca[0], ca.front());
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Tests that back() returns a reference to the last element of an array.
 */
TEST(Array, TestBack)
{
    IntArray10 a;
    a[9] = 17;
    // Read using back()
    ASSERT_EQ(17, a.back());
    // Write using back()
    a.back() = 18;
    ASSERT_EQ(18, a[9]);
    ASSERT_EQ(a[9], a.back());

    IntArray10 const& ca = a;
    ASSERT_EQ(ca[9], ca.back());
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Tests that zero length arrays are empty and every array else is not.
 */
TEST(Array, TestEmpty)
{
#ifndef __GNUC__
    ::estd::array<int32_t, 0> emptyArray;
    ASSERT_TRUE(emptyArray.empty());
#endif
    ::estd::array<int32_t, 1> nonEmptyArray{};
    ASSERT_FALSE(nonEmptyArray.empty());
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Tests that data() returns a pointer to the underlying data of the array.
 */
TEST(Array, TestData)
{
    IntArray10 a;
    IntArray10::pointer p = a.data();
    ASSERT_EQ(&a[0], p);

    IntArray10 const& ca        = a;
    IntArray10::const_pointer c = ca.data();
    ASSERT_EQ(&a[0], c);
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Tests that size() and max_size() return the defined size of an array.
 */
TEST(Array, TestSizeAndMaxSize)
{
    IntArray10 a{};
    ASSERT_EQ(10U, a.size());
    ASSERT_EQ(10U, a.max_size());
    ASSERT_EQ(a.size(), a.max_size());
#ifndef __GNUC__
    ::estd::array<int32_t, 0> b;
    ASSERT_EQ(0, b.size());
    ASSERT_EQ(0, b.max_size());
    ASSERT_EQ(b.size(), b.max_size());
#endif
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Assures that calling fill() with a given value initializes the entire array with that value.
 */
TEST(Array, TestFill)
{
    IntArray10 a;
    a.fill(17);
    for (int& i : a)
    {
        ASSERT_EQ(17, i);
    }
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Tests that calling the swap() function will exchange the content of two arrays of the same size.
 */
TEST(Array, SwapMethod)
{
    IntArray10 a1, a2;
    for (IntArray10::size_type i = 0; i < a1.size(); ++i)
    {
        a1[i] = int32_t(i);
        a2[i] = int32_t(10 + i);
    }
    a1.swap(a2);
    for (IntArray10::size_type i = 0; i < a1.size(); ++i)
    {
        ASSERT_EQ(a1[i], int32_t(10 + i));
        ASSERT_EQ(a2[i], int32_t(i));
    }
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Tests that calling std::swap() will exchange the content of two arrays of the same size.
 */
TEST(Array, Swap)
{
    IntArray10 a1, a2;
    for (IntArray10::size_type i = 0; i < a1.size(); ++i)
    {
        a1[i] = int32_t(i);
        a2[i] = int32_t(10 + i);
    }
    std::swap(a1, a2);
    for (IntArray10::size_type i = 0; i < a1.size(); ++i)
    {
        ASSERT_EQ(a1[i], int32_t(10 + i));
        ASSERT_EQ(a2[i], int32_t(i));
    }
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Tests that at() can be used to access elements at a given index. If index is out of bounds,
 * at() will assert.
 */
TEST(Array, TestAt)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    IntArray10 a;
    for (int32_t i = 0; i < 10; ++i)
    {
        a.at(i) = i;
    }
    for (int32_t i = 0; i < 10; ++i)
    {
        ASSERT_EQ(i, a.at(i));
    }

    IntArray10 const& ca = a;
    for (int32_t i = 0; i < 10; ++i)
    {
        ASSERT_EQ(i, ca.at(i));
    }

    ASSERT_THROW(a.at(11), ::estd::assert_exception);
    ASSERT_THROW(ca.at(11), ::estd::assert_exception);
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Tests that begin() iterator points to the first element of an array.
 */
TEST(Array, TestBegin)
{
    IntArray10 a;
    a.front() = 17;
    ASSERT_EQ(17, *a.begin());
    IntArray10::iterator itr = a.begin();

    *itr = 42;
    ASSERT_EQ(42, a[0]);
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Tests that rbegin() iterator points to the last element of an array.
 */
TEST(Array, TestRBegin)
{
    IntArray10 a;
    a.back() = 17;
    ASSERT_EQ(17, *a.rbegin());
    IntArray10::reverse_iterator itr = a.rbegin();

    *itr = 42;
    ASSERT_EQ(42, a[9]);
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * This test evaluates some general use cases for iterators of an array.
 */
TEST(Array, TestIterators)
{
    IntArray10 a{};
    // Fill array completely with value 42
    std::fill_n(a.begin(), a.size(), 42);
    for (int& i : a)
    {
        ASSERT_EQ(42, i);
    }

    // Count occurrences of 42 using iterator and const_iterator.
    ASSERT_EQ(10, std::count(a.begin(), a.end(), 42));
    ASSERT_EQ(10, std::count(a.cbegin(), a.cend(), 42));

    // Fill array with ascending numbers.
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = i;
    }

    // Check that iterator can be used to validate the sequence.
    int32_t i = 0;
    for (IntArray10::iterator itr = a.begin(); itr != a.end(); ++itr, ++i)
    {
        ASSERT_EQ(i, *itr);
    }

    // Check that reverse_iterator can be used to validate the sequence in reverse order.
    i = 9;
    for (IntArray10::reverse_iterator itr = a.rbegin(); itr != a.rend(); ++itr, --i)
    {
        ASSERT_EQ(i, *itr);
    }
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Assures that different use cases that involve the comparison operators of an array do work as
 * specified. It tests ==, >=, >, <, <= and !=.
 */
TEST(Array, TestComparisonOperators)
{
    IntArray10 a1, a2;
    for (size_t i = 0; i < a1.max_size(); ++i)
    {
        a1[i] = (a2[i] = i);
    }
    ASSERT_EQ(a1, a2);
    ASSERT_TRUE(a1 == a2);
    ASSERT_GE(a1, a2);
    ASSERT_TRUE(a1 >= a2);
    ASSERT_LE(a1, a2);
    ASSERT_TRUE(a1 <= a2);
    ++a1[0];
    ASSERT_NE(a1, a2);
    ASSERT_TRUE(a1 != a2);
    ASSERT_LT(a2, a1);
    ASSERT_TRUE(a2 < a1);
    ASSERT_LE(a2, a1);
    ASSERT_TRUE(a2 <= a1);
    ++a2[0];
    ++a2[1];
    ASSERT_GT(a2, a1);
    ASSERT_TRUE(a2 > a1);
    ASSERT_GE(a2, a1);
    ASSERT_TRUE(a2 >= a1);
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Test for reverse_iterator. Traverse through an array using reverse_iterator and
 * const_reverse_iterator.
 */
TEST(Array, TestReverseIterators)
{
    ::estd::array<int32_t, 3> a{1, 2, 3};

    ASSERT_EQ(3, *(a.rbegin()));
    ASSERT_EQ(3, *(a.crbegin()));

    // Iterate using const_reverse_iterator
    {
        ::estd::array<int32_t, 3>::const_reverse_iterator iter = a.crbegin();
        ASSERT_EQ(3, *iter);
        iter++;
        ASSERT_EQ(2, *iter);
        iter++;
        ASSERT_EQ(1, *iter);
        iter++;
        ASSERT_EQ(a.crend(), iter);
    }
    // Iterate using reverse_iterator
    {
        ::estd::array<int32_t, 3>::reverse_iterator iter = a.rbegin();
        ASSERT_EQ(3, *iter);
        iter++;
        ASSERT_EQ(2, *iter);
        iter++;
        ASSERT_EQ(1, *iter);
        iter++;
        ASSERT_EQ(a.rend(), iter);
    }
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Verifies that a const_iterator can be created from an iterator.
 */
TEST(Array, IteratorToConstIterator)
{
    MAKE_ARRAY_FROM(int32_t, 10, a);
    IntArray10::iterator i = a.begin();

    IntArray10::const_iterator ci = i;
    ASSERT_TRUE(i == ci);
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Assures that the copy constructor initializes an array from a given array.
 */
TEST(Array, TestCopyConstructor)
{
    ::estd::array<int32_t, 10> a{};
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = i;
    }

    // Create copy of array a into b.
    ::estd::array<int32_t, 10> b = a;

    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i]);
    }
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Test case to verify that estd::array behaves like std:array considering constructor calls.
 * An array of size 10 is created, so 10 constructor calls are expected.
 */
TEST(Array, TestConstructorCalls)
{
    using ::internal::TestClassCalls;
    {
        // check how STL behaves
        TestClassCalls::reset();
        std::array<TestClassCalls, 10> a;
        ASSERT_TRUE(TestClassCalls::verify(10, 0, 0, 0, 0, 0));
    }
    {
        // compare to estd
        TestClassCalls::reset();
        ::estd::array<TestClassCalls, 10> a;
        ASSERT_TRUE(TestClassCalls::verify(10, 0, 0, 0, 0, 0));
    }
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Test case to verify that estd::array behaves like std:array considering destructor calls.
 * An array of size 10 is created, so 10 destructor calls are expected when the array goes
 * out of scope.
 */
TEST(Array, TestDestructorCalls)
{
    using ::internal::TestClassCalls;
    {
        // check how STL behaves
        {
            std::array<TestClassCalls, 10> a;
            TestClassCalls::reset();
        }
        ASSERT_TRUE(TestClassCalls::verify(0, 0, 0, 10, 0, 0));
    }
    {
        // compare to estd
        {
            ::estd::array<TestClassCalls, 10> a;
            TestClassCalls::reset();
        }
        ASSERT_TRUE(TestClassCalls::verify(0, 0, 0, 10, 0, 0));
    }
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Test case to verify that estd::array behaves like std:array considering copy constructor calls.
 * An array of size 10 is created, so 10 copy constructor calls are expected when copy initializing.
 */
TEST(Array, TestCopyConstructorCalls)
{
    using ::internal::TestClassCalls;
    {
        // Check how STL behaves.
        TestClassCalls::reset();
        std::array<TestClassCalls, 10> a;
        TestClassCalls::reset();
        std::array<TestClassCalls, 10> b = a;
        ASSERT_TRUE(TestClassCalls::verify(0, 10, 0, 0, 0, 0));
        TestClassCalls::reset();
        std::array<TestClassCalls, 10> c(a);
        ASSERT_TRUE(TestClassCalls::verify(0, 10, 0, 0, 0, 0));
    }
    {
        // Compare to estd::array.
        TestClassCalls::reset();
        ::estd::array<TestClassCalls, 10> a;
        TestClassCalls::reset();
        ::estd::array<TestClassCalls, 10> b = a;
        ASSERT_TRUE(TestClassCalls::verify(0, 10, 0, 0, 0, 0));
        TestClassCalls::reset();
        ::estd::array<TestClassCalls, 10> c(a);
        ASSERT_TRUE(TestClassCalls::verify(0, 10, 0, 0, 0, 0));
    }
}

void arrayCopy(::estd::array<::internal::TestClassCalls, 10> copy)
{
    // Make sure that our copies were done properly.
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 10, 0, 0, 0, 0));
    ASSERT_EQ(10U, copy.size());
    // Verify that the values were copied.
    for (size_t i = 0; i < copy.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, copy[i].tag());
    }
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Tests that passing an array py value to a function copies each element.
 */
TEST(Array, TestCopyConstructorCallsToFunction)
{
    MAKE_ARRAY_FROM(::internal::TestClassCalls, 10, a);

    // Initialize array with ascending values.
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = ::internal::TestClassCalls(i);
    }

    ::internal::TestClassCalls::reset();
    arrayCopy(a);
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 10, 0, 10, 0, 0));
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Verify that the assignment operator is called for each element of an array when an array is
 * assigned to another.
 */
TEST(Array, TestAssignmentCalls)
{
    MAKE_ARRAY_FROM(::internal::TestClassCalls, 10, a);

    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = ::internal::TestClassCalls(i);
    }

    ::estd::array<::internal::TestClassCalls, 10> b;
    ::internal::TestClassCalls::reset();

    // Verify that the assignment operator works
    b = a;

    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 0, 10, 0));

    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i].tag());
    }
}

/**
 * \refs:    SMD_estd_array
 * \desc
 * Test that verifies that the assignment of an array to another copies all values.
 */
TEST(Array, TestAssignmentIdentical)
{
    // Create an array a of size 10
    ::estd::array<int32_t, 10> a{};
    // and fill it with ascending numbers.
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = i;
    }
    // Create another array b of size 10.
    ::estd::array<int32_t, 10> b{};
    // Assign array a to b
    b = a;
    // and check that the values are correct.
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i]);
    }
    // self assignment
    ::estd::array<int32_t, 10>* other = &b;
    b                                 = *other;
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i]);
    }
}

} // namespace
