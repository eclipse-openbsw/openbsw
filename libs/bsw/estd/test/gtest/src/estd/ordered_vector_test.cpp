// Copyright 2024 Accenture.

#include "estd/ordered_vector.h"

#include "estd/assert.h"
#include "internal/TestClassCalls.h"

#include <gmock/gmock.h>

#include <type_traits>

using namespace ::testing;

// explicit instantiation to get correct test coverage
template class ::estd::declare::ordered_vector<int32_t, 10, std::less<int32_t>>;
template class ::estd::ordered_vector<int32_t, std::less<int32_t>>;

using IntVector10 = ::estd::declare::ordered_vector<int32_t, 10, std::less<int32_t>>;
using IntVector   = ::estd::ordered_vector<int32_t, std::less<int32_t>>;

static_assert(std::is_same<int32_t const&, IntVector10::const_reference>::value, "");
static_assert(std::is_same<int32_t const*, IntVector10::const_iterator>::value, "");

namespace
{
#define MAKE_SORTED_LESS_VECTOR(T, N, Name)                      \
    ::estd::declare::ordered_vector<T, N, std::less<T>> Name##_; \
    ::estd::ordered_vector<T, std::less<T>>& Name = Name##_

// vector types we use in this test:
// ordered_vector<int32_t, 10, std::greater<int32_t> >
// ordered_vector<int32_t, 20, std::less<int32_t> >
// ordered_vector<int32_t, 10, std::less<int32_t> >
// ordered_vector< ::internal::TestClassCalls, 5, std::less<int32_t> >
// ordered_vector< ::internal::TestClassCalls, 10, std::less<int32_t> >

TEST(OrderedVector, TestSizeOf)
{
    ::estd::declare::ordered_vector<int32_t, 10> container;

    if (sizeof(size_t) == 4)
    {
        ASSERT_EQ(8U, sizeof(::estd::ordered_vector<int32_t>));
        ASSERT_EQ(10 * sizeof(int32_t) + 16, sizeof(container));
    }
    else
    {
        ASSERT_EQ(16U, sizeof(::estd::ordered_vector<int32_t>));
        ASSERT_EQ(10 * sizeof(int32_t) + 32, sizeof(container));
    }
}

TEST(OrderedVector, TestEmpty)
{
#ifndef __GNUC__
    ::estd::declare::ordered_vector<int32_t, 0, std::less<int32_t>> emptyArray;
    ASSERT_TRUE(emptyArray.empty());
#endif
    ::estd::declare::ordered_vector<int32_t, 10, std::less<int32_t>> nonEmptyArray;
    ASSERT_TRUE(nonEmptyArray.empty());
}

TEST(OrderedVector, TestSizeAndMaxSize)
{
    IntVector10 a;
    ASSERT_EQ(0U, a.size());
    ASSERT_EQ(10U, a.max_size());
#ifndef __GNUC__
    ::estd::declare::ordered_vector<int32_t, 0, std::less<int32_t>> b;
    ASSERT_EQ(0U, b.size());
    ASSERT_EQ(0U, b.max_size());
#endif
}

TEST(OrderedVector, TestData)
{
    IntVector10 a;
    a.insert(1);
    ASSERT_EQ(a.begin(), a.data());
}

TEST(OrderedVector, TestResize)
{
    IntVector10 a;
    a.insert(3);
    EXPECT_THAT(a, ElementsAre(3));
    a.insert(2);
    EXPECT_THAT(a, ElementsAre(2, 3));
    a.insert(1);
    EXPECT_THAT(a, ElementsAre(1, 2, 3));
    a.insert(8);
    EXPECT_THAT(a, ElementsAre(1, 2, 3, 8));

    a.resize(2);

    EXPECT_THAT(a, ElementsAre(1, 2));

    a.resize(3);

    EXPECT_THAT(a, ElementsAre(1, 2));
}

TEST(OrderedVector, TestInsert)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    IntVector10 a;
    ASSERT_TRUE(a.empty());
    ASSERT_EQ(0U, a.size());
    a.insert(10);
    ASSERT_EQ(1U, a.size());
    a.insert(5);
    ASSERT_EQ(2U, a.size());
    a.insert(7);
    ASSERT_EQ(3U, a.size());

    IntVector10::const_iterator iter = a.begin();
    ASSERT_EQ(5, *iter);
    iter++;
    ASSERT_EQ(7, *iter);
    iter++;
    ASSERT_EQ(10, *iter);
    iter++;
    ASSERT_EQ(a.end(), iter);

    ::estd::declare::ordered_vector<int32_t, 1> smallVec;
    smallVec.insert(1);
    ASSERT_THROW({ smallVec.insert(2); }, ::estd::assert_exception);
}

TEST(OrderedVector, TestFindOrInsert)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    IntVector10 a;
    EXPECT_THAT(a, ElementsAre());

    IntVector10::const_iterator it = a.find_or_insert(5);
    EXPECT_THAT(a, ElementsAre(5));
    EXPECT_EQ(it, a.begin());

    it = a.find_or_insert(5);
    EXPECT_THAT(a, ElementsAre(5));
    EXPECT_EQ(it, a.begin());

    it = a.find_or_insert(6);
    EXPECT_THAT(a, ElementsAre(5, 6));
    EXPECT_EQ(it, a.begin() + 1);

    it = a.find_or_insert(1);
    EXPECT_THAT(a, ElementsAre(1, 5, 6));
    EXPECT_EQ(it, a.begin());

    it = a.find_or_insert(4);
    EXPECT_THAT(a, ElementsAre(1, 4, 5, 6));
    EXPECT_EQ(it, a.begin() + 1);

    it = a.find_or_insert(2);
    EXPECT_THAT(a, ElementsAre(1, 2, 4, 5, 6));
    EXPECT_EQ(it, a.begin() + 1);

    it = a.find_or_insert(6);
    EXPECT_THAT(a, ElementsAre(1, 2, 4, 5, 6));
    EXPECT_EQ(it, a.begin() + 4);

    it = a.find_or_insert(5);
    EXPECT_THAT(a, ElementsAre(1, 2, 4, 5, 6));
    EXPECT_EQ(it, a.begin() + 3);

    it = a.find_or_insert(3);
    EXPECT_THAT(a, ElementsAre(1, 2, 3, 4, 5, 6));
    EXPECT_EQ(it, a.begin() + 2);

    it = a.find_or_insert(7);
    EXPECT_THAT(a, ElementsAre(1, 2, 3, 4, 5, 6, 7));
    EXPECT_EQ(it, a.begin() + 6);

    it = a.find_or_insert(10);
    EXPECT_THAT(a, ElementsAre(1, 2, 3, 4, 5, 6, 7, 10));
    EXPECT_EQ(it, a.begin() + 7);

    it = a.find_or_insert(20);
    EXPECT_THAT(a, ElementsAre(1, 2, 3, 4, 5, 6, 7, 10, 20));
    EXPECT_EQ(it, a.begin() + 8);

    it = a.find_or_insert(30);
    EXPECT_THAT(a, ElementsAre(1, 2, 3, 4, 5, 6, 7, 10, 20, 30));
    EXPECT_EQ(it, a.begin() + 9);

    // no more space
    it = a.find_or_insert(100);
    EXPECT_THAT(a, ElementsAre(1, 2, 3, 4, 5, 6, 7, 10, 20, 30));
    EXPECT_EQ(it, a.end());

    it = a.find_or_insert(5);
    EXPECT_THAT(a, ElementsAre(1, 2, 3, 4, 5, 6, 7, 10, 20, 30));
    EXPECT_EQ(it, a.begin() + 4);
}

TEST(OrderedVector, TestAt)
{
    IntVector10 a;

    a.insert(5);
    a.insert(10);
    a.insert(2);

    ASSERT_EQ(2, a.at(0));

    IntVector10 const& ca = a;

    ASSERT_EQ(5, ca.at(1));
    ASSERT_EQ(10, ca[2]);
}

TEST(OrderedVector, TestRemove)
{
    IntVector10 a;

    a.insert(1);
    a.insert(2);
    a.insert(3);

    a.remove(0);
    ASSERT_EQ(3U, a.size());

    a.remove(10);
    ASSERT_EQ(3U, a.size());

    a.remove(1);
    ASSERT_EQ(2U, a.size());

    IntVector10::const_iterator iter = a.begin();
    ASSERT_EQ(2, *iter);
    iter++;
    ASSERT_EQ(3, *iter);
    iter++;
    ASSERT_EQ(a.end(), iter);
}

TEST(OrderedVector, TestRemoveMultiple)
{
    IntVector10 a;

    a.insert(1);
    a.insert(1);
    a.insert(2);
    a.insert(3);

    ASSERT_EQ(4U, a.size());

    a.remove(1);
    ASSERT_EQ(2U, a.size());

    IntVector10::const_iterator iter = a.begin();
    ASSERT_EQ(2, *iter);
    iter++;
    ASSERT_EQ(3, *iter);
    iter++;
    ASSERT_EQ(a.end(), iter);
}

TEST(OrderedVector, TestContains)
{
    IntVector10 a;

    a.insert(1);
    a.insert(2);
    a.insert(3);

    ASSERT_TRUE(a.contains(1));
    ASSERT_TRUE(a.contains(2));
    ASSERT_TRUE(a.contains(3));

    ASSERT_FALSE(a.contains(0));
    ASSERT_FALSE(a.contains(4));
}

class EvenIf
{
public:
    EvenIf() = default;

    bool operator()(::internal::TestClassCalls const& value) { return (value.tag() % 2) == 0; }

    bool operator()(int32_t const& value) { return (value % 2) == 0; }
};

TEST(OrderedVector, TestRemoveIf)
{
    ::internal::TestClassCalls::reset();
    ::estd::declare::ordered_vector<::internal::TestClassCalls, 5> a;

    ::internal::TestClassCalls::reset();
    a.insert(::internal::TestClassCalls(1));
    a.insert(::internal::TestClassCalls(2));
    a.insert(::internal::TestClassCalls(3));
    a.insert(::internal::TestClassCalls(4));

    EvenIf comp;

    ::internal::TestClassCalls::reset();
    a.remove_if(comp);
    ::internal::TestClassCalls::verify(0, 0, 0, 2, 0, 2);

    ASSERT_EQ(2U, a.size());
    ASSERT_TRUE(a.contains(::internal::TestClassCalls(1)));
    ASSERT_FALSE(a.contains(::internal::TestClassCalls(2)));
    ASSERT_TRUE(a.contains(::internal::TestClassCalls(3)));
    ASSERT_FALSE(a.contains(::internal::TestClassCalls(4)));

    ::estd::ordered_vector<::internal::TestClassCalls>::const_iterator iter = a.cbegin();
    ASSERT_EQ(1, iter->tag());
    iter++;
    ASSERT_EQ(3, iter->tag());
}

TEST(OrderedVector, TestRemoveIf_SucceedingElements)
{
    ::estd::declare::ordered_vector<int32_t, 10> a;

    a.insert(1);
    a.insert(3);
    a.insert(5);
    a.insert(5);

    a.insert(2);
    a.insert(6);
    a.insert(6);
    a.insert(6);

    EvenIf comp;

    a.remove_if(comp);

    ASSERT_EQ(4U, a.size());
    EXPECT_EQ(1, a[0]);
    EXPECT_EQ(3, a[1]);
    EXPECT_EQ(5, a[2]);
    EXPECT_EQ(5, a[3]);
}

TEST(OrderedVector, TestErase)
{
    IntVector10 a;

    a.insert(1);
    a.insert(2);
    a.insert(3);
    a.insert(4);

    // delete 2
    IntVector10::const_iterator iter = a.begin();
    iter++;
    IntVector10::const_iterator iter1 = a.erase(iter);
    ASSERT_EQ(3, *iter1);
    ASSERT_EQ(3U, a.size());
}

TEST(OrderedVector, TestEraseRange)
{
    IntVector10 a;

    a.insert(1);
    a.insert(2);
    a.insert(3);
    a.insert(4);

    // delete 2 => end
    IntVector10::const_iterator iter = a.begin();
    iter++;
    IntVector10::const_iterator iter1 = a.erase(iter, a.end());
    ASSERT_EQ(a.end(), iter1);
    ASSERT_EQ(1U, a.size());
    ASSERT_TRUE(a.contains(1));
}

TEST(OrderedVector, TestClear)
{
    IntVector10 a;

    a.insert(1);
    a.insert(2);
    a.insert(3);
    a.insert(4);

    a.clear();
    ASSERT_EQ(0U, a.size());
    ASSERT_EQ(a.begin(), a.end());
}

TEST(OrderedVector, TestEqual)
{
    ::estd::declare::ordered_vector<int32_t, 10, std::less<int32_t>> _a1;
    ::estd::ordered_vector<int32_t, std::less<int32_t>>& a1 = _a1;
    ::estd::declare::ordered_vector<int32_t, 20, std::less<int32_t>> _a2;
    ::estd::ordered_vector<int32_t, std::less<int32_t>>& a2 = _a2;

    a1.insert(1);
    a1.insert(2);
    a1.insert(3);
    a1.insert(4);

    a2.insert(1);
    a2.insert(2);
    a2.insert(3);
    a2.insert(4);

    ASSERT_TRUE(a1 == a2);
    ASSERT_TRUE(a1 <= a2);

    a2.insert(5);
    ASSERT_TRUE(a1 != a2);
}

TEST(OrderedVector, TestIterators)
{
    IntVector10 a;
    a.insert(1);
    a.insert(2);
    a.insert(3);

    ASSERT_EQ(3, *(a.rbegin()));
    ASSERT_EQ(3, *(a.crbegin()));

    {
        IntVector10::const_reverse_iterator iter = a.crbegin();
        ASSERT_EQ(3, *iter);
        iter++;
        ASSERT_EQ(2, *iter);
        iter++;
        ASSERT_EQ(1, *iter);
        iter++;
        ASSERT_EQ(a.crend(), iter);
    }

    {
        IntVector10::const_reverse_iterator iter = a.rbegin();
        ASSERT_EQ(3, *iter);
        iter++;
        ASSERT_EQ(2, *iter);
        iter++;
        ASSERT_EQ(1, *iter);
        iter++;
        ASSERT_EQ(a.rend(), iter);
    }
}

TEST(OrderedVector, RelationalOperators)
{
    MAKE_SORTED_LESS_VECTOR(int32_t, 10, d1);
    MAKE_SORTED_LESS_VECTOR(int32_t, 20, d2);
    ASSERT_EQ(d1, d2);
    ASSERT_TRUE((d1 == d2));
    ASSERT_GE(d1, d2);
    ASSERT_TRUE((d1 >= d2));
    ASSERT_GE(d2, d1);
    ASSERT_TRUE((d2 >= d1));
    ASSERT_LE(d1, d2);
    ASSERT_TRUE((d1 <= d2));
    ASSERT_LE(d2, d1);
    ASSERT_TRUE((d2 <= d1));
    d1.insert(1);
    ASSERT_NE(d1, d2);
    ASSERT_FALSE((d1 == d2));
    ASSERT_TRUE((d1 != d2));
    d2.insert(1);
    ASSERT_EQ(d1, d2);
    ASSERT_TRUE((d1 == d2));
    ASSERT_GE(d1, d2);
    ASSERT_TRUE((d1 >= d2));
    ASSERT_GE(d2, d1);
    ASSERT_TRUE((d2 >= d1));
    ASSERT_LE(d1, d2);
    ASSERT_TRUE((d1 <= d2));
    ASSERT_LE(d2, d1);
    ASSERT_TRUE((d2 <= d1));
    d1.insert(2);
    d2.insert(0);
    ASSERT_GT(d1, d2);
    ASSERT_TRUE((d1 > d2));
    ASSERT_LT(d2, d1);
    ASSERT_TRUE((d2 < d1));
}

TEST(OrderedVector, TestComparisonOperatorsWithCompare)
{
    ::estd::declare::ordered_vector<int32_t, 10, std::greater<int32_t>> d1;
    ::estd::declare::ordered_vector<int32_t, 10, std::greater<int32_t>> d2;

    // since std::greater is the opposite of std::less
    // our checks need to be reversed
    d1.insert(1);
    d2.insert(2);

    ASSERT_GE(d1, d2);
    ASSERT_LE(d2, d1);
}

TEST(OrderedVector, TestCopyConstructor)
{
    ::estd::declare::ordered_vector<int32_t, 10> a;
    for (size_t i = 0; i < a.size(); ++i)
    {
        a.insert(i);
    }

    ::estd::declare::ordered_vector<int32_t, 10> b = a;

    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i]);
    }
}

TEST(OrderedVector, TestDestructorCalls)
{
    ::internal::TestClassCalls::reset();
    {
        MAKE_SORTED_LESS_VECTOR(::internal::TestClassCalls, 10, a);

        ::internal::TestClassCalls::reset();

        for (size_t i = 0; i < a.max_size(); ++i)
        {
            a.insert(::internal::TestClassCalls(i));
        }
        ASSERT_TRUE(::internal::TestClassCalls::verify(10, 10, 0, 10, 0, 0));

        ::internal::TestClassCalls::reset();
    }

    // verify that the destructors were called when the array was destroyed
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 10, 0, 0));
}

TEST(OrderedVector, TestCopyConstructorCalls)
{
    MAKE_SORTED_LESS_VECTOR(::internal::TestClassCalls, 10, a);

    // initialize array
    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.insert(::internal::TestClassCalls(i));
    }

    // now create new array from other array
    ::internal::TestClassCalls::reset();
    ::estd::declare::ordered_vector<::internal::TestClassCalls, 10> b = a;

    // verify that the number of calls was correct
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 10, 0, 0, 0, 0));

    ASSERT_EQ(10U, b.size());
    // and verify that the values were copied.
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i].tag());
    }
}

void arrayCopy(::estd::declare::ordered_vector<::internal::TestClassCalls, 10> copy)
{
    // make sure that our copies were done properly.
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 10, 0, 0, 0, 0));
    ASSERT_EQ(10U, copy.size());
    // and verify that the values were copied.
    for (size_t i = 0; i < copy.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, copy[i].tag());
    }
}

TEST(OrderedVector, TestCopyConstructorCallsToFunction)
{
    MAKE_SORTED_LESS_VECTOR(::internal::TestClassCalls, 10, a);

    // initialize array
    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.insert(::internal::TestClassCalls(i));
    }

    ::internal::TestClassCalls::reset();
    arrayCopy(a);
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 10, 0, 10, 0, 0));
}

TEST(OrderedVector, TestAssignmentCalls)
{
    MAKE_SORTED_LESS_VECTOR(::internal::TestClassCalls, 10, a);

    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.insert(::internal::TestClassCalls(i));
    }

    ::estd::declare::ordered_vector<::internal::TestClassCalls, 10> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 10, 0, 0, 0, 0));

    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i].tag());
    }

    MAKE_SORTED_LESS_VECTOR(::internal::TestClassCalls, 10, c);

    // make sure we can assign into a value of the base class
    ::internal::TestClassCalls::reset();
    c = b;
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 10, 0, 0, 0, 0));

    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i].tag());
    }
}

TEST(OrderedVector, TestAssignmentIdentical)
{
    ::estd::declare::ordered_vector<int32_t, 10> a;

    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.insert(i);
    }

    ::estd::declare::ordered_vector<int32_t, 10> b;

    b = a;

    ASSERT_EQ(10U, b.size());

    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i]);
    }
}

TEST(OrderedVector, TestAssignmentIncorrectSize)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_SORTED_LESS_VECTOR(::internal::TestClassCalls, 10, a);

    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.insert(::internal::TestClassCalls(i));
    }

    ::estd::declare::ordered_vector<::internal::TestClassCalls, 5> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    ASSERT_THROW(b = a, ::estd::assert_exception);
}

TEST(OrderedVector, TestCopyIncorrectSize)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_SORTED_LESS_VECTOR(::internal::TestClassCalls, 10, a);

    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.insert(::internal::TestClassCalls(i));
    }

    try
    {
        ::estd::declare::ordered_vector<::internal::TestClassCalls, 5> b(a);
        ASSERT_TRUE(0);
    }
    catch (::estd::assert_exception const& e)
    {}
}

} // namespace
