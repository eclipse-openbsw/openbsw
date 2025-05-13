// Copyright 2024 Accenture.

#include "estd/deque.h"

#include "concepts/iterable.h"
#include "internal/NoDefaultConstructorObject.h"
#include "internal/TestClassCalls.h"
#include "internal/UncopyableButMovableObject.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <type_traits>

using namespace ::testing;
using namespace ::estd::test;

// explicit instantiation to get correct test coverage
template class ::estd::declare::deque<int32_t, 10>;
template class ::estd::deque<int32_t>;
template class ::estd::deque_iterator<int32_t>;
template class ::estd::deque_const_iterator<int32_t>;

using IntDeque10 = ::estd::declare::deque<int32_t, 10>;
using IntDeque   = ::estd::deque<int32_t>;

static_assert(std::is_same<int32_t, IntDeque10::value_type>::value, "");
static_assert(std::is_same<int32_t&, IntDeque10::reference>::value, "");
static_assert(std::is_same<int32_t const&, IntDeque10::const_reference>::value, "");
static_assert(std::is_same<int32_t*, IntDeque10::pointer>::value, "");
static_assert(std::is_same<int32_t const*, IntDeque10::const_pointer>::value, "");
static_assert(std::is_same<std::size_t, IntDeque10::size_type>::value, "");
static_assert(std::is_same<std::ptrdiff_t, IntDeque10::difference_type>::value, "");

static_assert(std::is_same<int32_t, IntDeque::value_type>::value, "");
static_assert(std::is_same<int32_t&, IntDeque::reference>::value, "");
static_assert(std::is_same<int32_t const&, IntDeque::const_reference>::value, "");
static_assert(std::is_same<int32_t*, IntDeque::pointer>::value, "");
static_assert(std::is_same<int32_t const*, IntDeque::const_pointer>::value, "");
static_assert(std::is_same<std::size_t, IntDeque::size_type>::value, "");
static_assert(std::is_same<std::ptrdiff_t, IntDeque::difference_type>::value, "");

namespace
{
struct IntDeque10_P
{
    enum Constants
    {
        LENGTH = 10
    };

    using Subject = ::estd::deque<int32_t>;
    ::estd::declare::deque<int32_t, LENGTH> subject;

    IntDeque10_P()
    {
        for (size_t i = 0; i < LENGTH; ++i)
        {
            subject.push_back(i + 1);
        }
    }
};

INSTANTIATE_TYPED_TEST_SUITE_P(Deque, ForwardIterableConcept, IntDeque10_P);
INSTANTIATE_TYPED_TEST_SUITE_P(Deque, ReverseIterableConcept, IntDeque10_P);

// deque instances used
// deque<int32_t, 10>
// deque<internal::TestClassCalls, 10>
// deque<internal::TestClassCalls, 15>
// deque<internal::TestClassCalls, 5>
// deque<internal::NoDefaultConstructorObject, 15>
// deque<internal::UncopyableObject, 10>
// deque<int32_t, 11>
// deque<int32_t, 20>
// deque<int32_t, 15>
// deque<int32_t, 9>

#define MAKE_DEQUE(T, N, Name)            \
    ::estd::declare::deque<T, N> Name##_; \
    ::estd::deque<T>& Name = Name##_

template<size_t total>
void validateDequeSize()
{
    ::estd::declare::deque<int32_t, total> container;

    size_t bucketSize = sizeof(int32_t);

    if (sizeof(size_t) == 4)
    {
        ASSERT_EQ(16U, sizeof(::estd::deque<int32_t>));
        ASSERT_EQ((total + 1) * bucketSize + 16, sizeof(container)) << "32bit:" << total;
    }
    else
    {
        size_t padding = ((total + 1) * bucketSize) % 8;

        ASSERT_EQ(32U, sizeof(::estd::deque<int32_t>));
        ASSERT_EQ((total + 1) * bucketSize + 32 + padding, sizeof(container)) << "64bit:" << total;
    }
}

TEST(Deque, TestSizeOf)
{
    validateDequeSize<10>();
    validateDequeSize<5>();
    validateDequeSize<1>();
}

/**
 * \desc
 * Tests that max_size() returns the value passed as template parameter N.
 */
TEST(Deque, MaxSize)
{
    MAKE_DEQUE(int32_t, 10, d);
    ASSERT_EQ(10U, d.max_size());
}

TEST(Deque, Empty)
{
    MAKE_DEQUE(int32_t, 10, d);
    ASSERT_TRUE(d.empty());
}

TEST(Deque, Full)
{
    MAKE_DEQUE(int32_t, 0, d);
    ASSERT_TRUE(d.empty());
    ASSERT_TRUE(d.full());
}

TEST(Deque, Size)
{
    MAKE_DEQUE(int32_t, 10, d);
    IntDeque::size_type s = 0;
    while (!d.full())
    {
        ASSERT_EQ(s, d.size());
        d.push_back(1);
        ++s;
    }
    ASSERT_EQ(d.max_size(), d.size());
}

TEST(Deque, Clear)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.clear();
    ASSERT_TRUE(d.empty());
    d.push_back(1);
    ASSERT_TRUE(!d.empty());
    d.clear();
    ASSERT_TRUE(d.empty());
}

/**
 * \desc
 * Testcase to verify that clear will call the destructor on all deleted elements.
 */
TEST(Deque, ClearDestructs)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, d);
    d.assign(10, ::internal::TestClassCalls());
    ::internal::TestClassCalls::reset();
    d.clear();
    ::internal::TestClassCalls::verify(0, 0, 0, 10, 0, 0);
}

TEST(Deque, Constructor)
{
    MAKE_DEQUE(int32_t, 10, d);
    ASSERT_EQ(0U, d.size());
    ASSERT_EQ(10U, d.max_size());
    ASSERT_TRUE(d.empty());
}

TEST(Deque, FillConstructor)
{
    ::internal::TestClassCalls::reset();
    ::estd::declare::deque<::internal::TestClassCalls, 10> d(5);

    ASSERT_TRUE(::internal::TestClassCalls::verify(1, 5, 0, 1, 0, 0));
    ASSERT_EQ(5U, d.size());
}

TEST(Deque, BackEmpty)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(int32_t, 10, d);

    ASSERT_THROW(d.back(), ::estd::assert_exception);
}

TEST(Deque, BackConstEmpty)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(int32_t, 10, d1);

    ::estd::deque<int32_t> const& d = d1;
    ASSERT_THROW(d.back(), ::estd::assert_exception);
    d1.push_back(1);
    ASSERT_EQ(1, d.back());
}

TEST(Deque, FrontEmpty)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(int32_t, 10, d);

    ASSERT_THROW(d.front(), ::estd::assert_exception);
}

TEST(Deque, FrontConstEmpty)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(int32_t, 10, d1);

    ::estd::deque<int32_t> const& d = d1;
    ASSERT_THROW(d.front(), ::estd::assert_exception);
    d1.push_back(1);
    ASSERT_EQ(1, d.back());
    ASSERT_EQ(1, d.front());
}

TEST(Deque, PushBack)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(int32_t, 10, d);
    for (IntDeque::size_type v = 0; v < d.max_size(); ++v)
    {
        d.push_back(v);
        ASSERT_EQ((int32_t)v, d.back());
        ASSERT_EQ(0, d.front());
    }
    ASSERT_EQ(d.max_size(), d.size());
    ASSERT_TRUE(d.full());
    ASSERT_THROW({ d.push_back(10); }, ::estd::assert_exception);
}

TEST(Deque, PushBackAssign)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(int32_t, 10, d);
    for (IntDeque::size_type v = 0; v < d.max_size(); ++v)
    {
        d.push_back() = v;
        ASSERT_EQ((int32_t)v, d.back());
    }
    ASSERT_EQ(d.max_size(), d.size());
    ASSERT_TRUE(d.full());
    ASSERT_THROW({ d.push_back() = 10; }, ::estd::assert_exception);
}

TEST(Deque, EmplaceBack)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(int32_t, 10, d);
    for (int32_t v = 0; v < (int32_t)d.max_size(); ++v)
    {
        d.emplace_back().construct(v);
        ASSERT_EQ((int32_t)v, d.back());
    }
    ASSERT_EQ(d.max_size(), d.size());
    ASSERT_TRUE(d.full());
    ASSERT_THROW({ d.emplace_back().construct(10); }, ::estd::assert_exception);
}

TEST(Deque, EmplaceIteratorFull)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::declare::deque<int32_t, 1> d;

    d.emplace(d.cbegin()).construct(1);
    ASSERT_EQ(1U, d.size());
    ASSERT_EQ(1, d[0]);

    ASSERT_THROW({ d.emplace(d.cbegin()); }, ::estd::assert_exception);
}

TEST(Deque, PushFront)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(int32_t, 10, d);
    for (IntDeque::size_type v = 0; v < d.max_size(); ++v)
    {
        d.push_front(v);
        ASSERT_EQ((int32_t)v, d.front());
        ASSERT_EQ(0, d.back());
    }
    ASSERT_EQ(d.max_size(), d.size());
    ASSERT_TRUE(d.full());
    ASSERT_THROW({ d.push_front(10); }, ::estd::assert_exception);
}

TEST(Deque, PushFrontAssign)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(int32_t, 10, d);
    for (IntDeque::size_type v = 0; v < d.max_size(); ++v)
    {
        d.push_front() = v;
        ASSERT_EQ((int32_t)v, d.front());
    }
    ASSERT_EQ(d.max_size(), d.size());
    ASSERT_TRUE(d.full());
    ASSERT_THROW({ d.push_front() = 10; }, ::estd::assert_exception);
}

TEST(Deque, EmplaceFront)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(int32_t, 10, d);
    for (int32_t v = 0; v < (int32_t)d.max_size(); ++v)
    {
        d.emplace_front().construct(v);
        ASSERT_EQ((int32_t)v, d.front());
    }
    ASSERT_EQ(d.max_size(), d.size());
    ASSERT_TRUE(d.full());
    ASSERT_THROW({ d.emplace_front().construct(10); }, ::estd::assert_exception);
}

TEST(Deque, RandomAccessOperator)
{
    MAKE_DEQUE(int32_t, 10, d);
    for (IntDeque::size_type v = 0; v < d.max_size(); ++v)
    {
        d.push_back(v);
        ASSERT_EQ((int32_t)v, d[v]) << "with v: " << v;
    }
    IntDeque::size_type sz = d.size();
    for (IntDeque::size_type v = 0; v < sz / 2; ++v)
    {
        std::swap(d[sz - 1 - v], d[v]);
    }
    for (IntDeque::size_type v = 0; v < d.max_size(); ++v)
    {
        ASSERT_EQ((int32_t)(d.max_size() - 1 - v), d[v]) << "with v: " << v;
    }
}

TEST(Deque, At)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(int32_t, 10, d);
    for (IntDeque::size_type v = 0; v < d.max_size(); ++v)
    {
        d.push_back(v);
        ASSERT_EQ((int32_t)v, d.at(v)) << "with v: " << v;
    }
    IntDeque::size_type sz = d.size();
    for (IntDeque::size_type v = 0; v < sz / 2; ++v)
    {
        std::swap(d.at(sz - 1 - v), d.at(v));
    }
    for (IntDeque::size_type v = 0; v < d.max_size(); ++v)
    {
        ASSERT_EQ((int32_t)(d.max_size() - 1 - v), d.at(v)) << "with v: " << v;
    }
    ASSERT_THROW(d.at(d.max_size()), ::estd::assert_exception);
}

TEST(Deque, AtConst)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(int32_t, 10, d1);

    ::estd::deque<int32_t> const& d = d1;

    for (IntDeque::size_type v = 0; v < d.max_size(); ++v)
    {
        d1.push_back(v);
        ASSERT_EQ((int32_t)v, d.at(v)) << "with v: " << v;
    }
    ASSERT_THROW(d.at(d.max_size()), ::estd::assert_exception);
}

TEST(Deque, PopFront)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(1);
    ASSERT_EQ(1U, d.size());
    ASSERT_EQ(1, d.front());
    ASSERT_EQ(1, d.back());
    d.pop_front();
    ASSERT_TRUE(d.empty());

    // pop front on empty. Shouldn't crash!
    d.pop_front();
    ASSERT_TRUE(d.empty());

    ASSERT_EQ(0U, d.size());
    d.push_back(2);
    d.push_back(3);
    ASSERT_EQ(2, d.front());
    ASSERT_EQ(3, d.back());
    ASSERT_EQ(2U, d.size());
    d.pop_front();
    ASSERT_EQ(3, d.front());
    ASSERT_EQ(3, d.back());
    ASSERT_EQ(1U, d.size());
    d.pop_front();
    ASSERT_TRUE(d.empty());
}

/**
 * \desc
 * Verifies that pop_front will call the destructor of removed elements.
 */
TEST(Deque, PopFrontDestructs)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, d);
    d.assign(10, ::internal::TestClassCalls());
    ::internal::TestClassCalls::reset();
    for (size_t i = 1; i <= d.max_size(); ++i)
    {
        d.pop_front();
        ::internal::TestClassCalls::verify(0, 0, 0, i, 0, 0);
    }
}

TEST(Deque, PopBack)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(1);
    ASSERT_EQ(1U, d.size());
    ASSERT_EQ(1, d.front());
    ASSERT_EQ(1, d.back());
    d.pop_back();
    ASSERT_TRUE(d.empty());
    // pop back on empty. Shouldn't crash!
    d.pop_back();
    ASSERT_TRUE(d.empty());

    ASSERT_EQ(0U, d.size());
    d.push_back(2);
    d.push_back(3);
    ASSERT_EQ(2, d.front());
    ASSERT_EQ(3, d.back());
    ASSERT_EQ(2U, d.size());
    d.pop_back();
    ASSERT_EQ(2, d.front());
    ASSERT_EQ(2, d.back());
    ASSERT_EQ(1U, d.size());
    d.pop_back();
    ASSERT_TRUE(d.empty());
}

/**
 * \desc
 * Verifies that pop_back will call the destructor of removed elements.
 */
TEST(Deque, PopBackDestructs)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, d);
    d.assign(10, ::internal::TestClassCalls());
    ::internal::TestClassCalls::reset();
    for (size_t i = 1; i <= d.max_size(); ++i)
    {
        d.pop_back();
        ::internal::TestClassCalls::verify(0, 0, 0, i, 0, 0);
    }
}

TEST(Deque, PushPopFrontBack)
{
    MAKE_DEQUE(int32_t, 10, d);
    int32_t i = 0;
    while (!d.full())
    {
        if (i % 2 == 0)
        {
            d.push_back(i);
        }
        else
        {
            d.push_front(i);
        }
        ++i;
    }
    // 9 7 5 3 1 0 2 4 6 8
    ASSERT_EQ(d.max_size(), (size_t)i);
    ASSERT_EQ(9, d.front());
    ASSERT_EQ(8, d.back());
    ASSERT_EQ(9, d[0]);
    ASSERT_EQ(7, d[1]);
    ASSERT_EQ(5, d[2]);
    ASSERT_EQ(3, d[3]);
    ASSERT_EQ(1, d[4]);
    ASSERT_EQ(0, d[5]);
    ASSERT_EQ(2, d[6]);
    ASSERT_EQ(4, d[7]);
    ASSERT_EQ(6, d[8]);
    ASSERT_EQ(8, d[9]);
    d.pop_back();
    ASSERT_EQ(9, d.front());
    ASSERT_EQ(6, d.back());
    d.pop_front();
    ASSERT_EQ(7, d.front());
    ASSERT_EQ(6, d.back());
    d.pop_back();
    ASSERT_EQ(7, d.front());
    ASSERT_EQ(4, d.back());
    d.pop_front();
    ASSERT_EQ(5, d.front());
    ASSERT_EQ(4, d.back());
    d.pop_back();
    ASSERT_EQ(5, d.front());
    ASSERT_EQ(2, d.back());
    d.pop_front();
    ASSERT_EQ(3, d.front());
    ASSERT_EQ(2, d.back());
    d.pop_back();
    ASSERT_EQ(3, d.front());
    ASSERT_EQ(0, d.back());
    d.pop_front();
    ASSERT_EQ(1, d.front());
    ASSERT_EQ(0, d.back());
    d.pop_back();
    ASSERT_EQ(1, d.front());
    ASSERT_EQ(1, d.back());
    d.pop_front();
    ASSERT_TRUE(d.empty());
}

TEST(Deque, Begin)
{
    MAKE_DEQUE(int32_t, 10, d);
    ASSERT_EQ(d.end(), d.begin());
    d.push_back(1);
    ASSERT_EQ(1, *d.begin());
    d.push_back(2);
    ASSERT_EQ(1, *d.begin());
    d.push_front(3);
    ASSERT_EQ(3, *d.begin());
}

TEST(Deque, ReverseIterator)
{
    MAKE_DEQUE(int32_t, 9, d);
    int32_t i = 1;
    while (!d.full())
    {
        d.push_back(i++);
    }
    IntDeque::reverse_iterator rit = d.rbegin();
    ASSERT_EQ(9, *rit);
    ASSERT_EQ(8, *++rit);
    ASSERT_EQ(8, *rit++);
    ASSERT_EQ(7, *rit);
    ASSERT_EQ(5, *(rit + 2));
    ASSERT_EQ(8, *(rit - 1));
    rit += 1;
    ASSERT_EQ(6, *rit);
    rit -= 2;
    ASSERT_EQ(8, *rit);
    ASSERT_EQ(6, rit[2]);
    ASSERT_EQ(9, d.rbegin() - d.rend());
}

TEST(Deque, ConstReverseIterator)
{
    MAKE_DEQUE(int32_t, 9, d);
    int32_t i = 1;
    while (!d.full())
    {
        d.push_back(i++);
    }
    ::estd::deque<int32_t> const& cd = d;

    IntDeque::const_reverse_iterator rit = cd.rbegin();
    ASSERT_EQ(9, *rit);
    ASSERT_EQ(8, *++rit);
    ASSERT_EQ(8, *rit++);
    ASSERT_EQ(7, *rit);
    ASSERT_EQ(5, *(rit + 2));
    ASSERT_EQ(8, *(rit - 1));
    rit += 1;
    ASSERT_EQ(6, *rit);
    rit -= 2;
    ASSERT_EQ(8, *rit);
    ASSERT_EQ(6, rit[2]);
    ASSERT_EQ(9, cd.rbegin() - cd.rend());
}

TEST(Deque, ConstReverseIteratorConstMethods)
{
    MAKE_DEQUE(int32_t, 9, d);
    int32_t i = 1;
    while (!d.full())
    {
        d.push_back(i++);
    }
    ::estd::deque<int32_t> const& cd = d;

    IntDeque::const_reverse_iterator rit = cd.crbegin();
    ASSERT_EQ(9, *rit);
    ASSERT_EQ(8, *++rit);
    ASSERT_EQ(8, *rit++);
    ASSERT_EQ(7, *rit);
    ASSERT_EQ(5, *(rit + 2));
    ASSERT_EQ(8, *(rit - 1));
    rit += 1;
    ASSERT_EQ(6, *rit);
    rit -= 2;
    ASSERT_EQ(8, *rit);
    ASSERT_EQ(6, rit[2]);
    ASSERT_EQ(9, cd.crbegin() - cd.crend());
}

TEST(Deque, Assign)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.assign(5, 17);
    ASSERT_EQ(5U, d.size());
    int32_t count = 0;
    for (IntDeque::const_iterator itr = d.cbegin(); itr != d.cend(); ++itr, ++count)
    {
        ASSERT_EQ(17, *itr);
    }
    ASSERT_EQ(5, count);
    d.assign(10, 19);
    ASSERT_EQ(10U, d.size());
    count = 0;
    for (IntDeque::const_iterator itr = d.cbegin(); itr != d.cend(); ++itr, ++count)
    {
        ASSERT_EQ(19, *itr);
    }
    ASSERT_TRUE(d.full());
    ASSERT_EQ(10, count);
    d.assign(IntDeque::size_type(0), 1);
    ASSERT_EQ(0U, d.size());
    ASSERT_TRUE(d.empty());

    int32_t a[] = {1, 2, 3};
    d.assign(&a[0], &a[3]);
    ASSERT_EQ(3U, d.size());
    IntDeque::const_iterator itr = d.cbegin();
    ASSERT_EQ(1, *itr++);
    ASSERT_EQ(2, *itr++);
    ASSERT_EQ(3, *itr++);
    ASSERT_EQ(d.cend(), itr);
}

TEST(Deque, AssignCopies)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, d);

    ::internal::TestClassCalls item(17);

    ::internal::TestClassCalls::reset();
    d.assign(5, item);
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 5, 0, 0, 0, 0));
    ASSERT_EQ(5U, d.size());
    for (::estd::deque<::internal::TestClassCalls>::const_iterator itr = d.cbegin();
         itr != d.cend();
         ++itr)
    {
        ASSERT_EQ(17, itr->tag());
    }
}

TEST(Deque, AssignIteratorFull)
{
    MAKE_DEQUE(int32_t, 10, d1);
    MAKE_DEQUE(int32_t, 1, d2);

    d1.push_back(1);
    d1.push_back(2);

    d2.assign(d1.cbegin(), d1.cend());
    ASSERT_EQ(1U, d2.size());
    ASSERT_EQ(1, d2[0]);
}

TEST(Deque, InsertIteratorFull)
{
    MAKE_DEQUE(int32_t, 1, d2);

    d2.insert(d2.cbegin(), 2, 5);
    ASSERT_EQ(1U, d2.size());
    ASSERT_EQ(5, d2[0]);
}

TEST(Deque, AssignIteratorCopies)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, d);

    for (int32_t i = 0; i < 5; ++i)
    {
        d.emplace_back().construct(i);
    }

    MAKE_DEQUE(::internal::TestClassCalls, 10, a);

    ::internal::TestClassCalls::reset();
    a.assign(d.cbegin(), d.cend());

    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 5, 0, 0, 0, 0));
    ASSERT_EQ(5U, a.size());
    int32_t count = 0;
    for (::estd::deque<::internal::TestClassCalls>::const_iterator itr = a.cbegin();
         itr != a.cend();
         ++itr)
    {
        ASSERT_EQ(count++, itr->tag());
    }
}

TEST(Deque, ResizeOneParameterCopies)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, d);

    d.emplace_back().construct(1);

    ::internal::TestClassCalls::reset();

    d.resize(3);

    ASSERT_TRUE(::internal::TestClassCalls::verify(2, 0, 0, 0, 0, 0));
    ASSERT_EQ(1, d[0].tag());
    ASSERT_EQ(0, d[1].tag());
    ASSERT_EQ(0, d[2].tag());

    ::internal::TestClassCalls::reset();

    d.resize(20);
    ASSERT_TRUE(::internal::TestClassCalls::verify(7, 0, 0, 0, 0, 0));

    for (std::size_t i = 3; i < 10; ++i)
    {
        ASSERT_EQ(0, d[i].tag());
    }
    ASSERT_TRUE(d.full());
    ASSERT_EQ(10U, d.size());

    ::internal::TestClassCalls::reset();
    d.resize(0);
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 10, 0, 0));

    ASSERT_TRUE(d.empty());
    ASSERT_EQ(0U, d.size());
}

/**
 * \desc
 * Verifies that resize will call the destructor for removed elements.
 */
TEST(Deque, ResizeOneParameterDestructs)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, d);
    d.assign(10, ::internal::TestClassCalls());
    ASSERT_TRUE(d.full());
    ::internal::TestClassCalls::reset();
    d.resize(0);
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 10, 0, 0));
}

TEST(Deque, ResizeTwoParameter)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back() = 1;
    d.resize(3, 4);
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(4, d[1]);
    ASSERT_EQ(4, d[2]);
    d.resize(20, 10);
    for (std::size_t i = 3; i < 10; ++i)
    {
        ASSERT_EQ(10, d[i]);
    }
    ASSERT_TRUE(d.full());
    ASSERT_EQ(10U, d.size());
    d.resize(0, 10);
    ASSERT_TRUE(d.empty());
    ASSERT_EQ(0U, d.size());
}

TEST(Deque, ResizeTwoParameterCopies)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, d);

    d.emplace_back().construct(1);

    ::internal::TestClassCalls item(4);

    ::internal::TestClassCalls::reset();

    d.resize(3, item);

    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 2, 0, 0, 0, 0));
    ASSERT_EQ(1, d[0].tag());
    ASSERT_EQ(4, d[1].tag());
    ASSERT_EQ(4, d[2].tag());

    ::internal::TestClassCalls item2(10);

    ::internal::TestClassCalls::reset();

    d.resize(20, item2);
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 7, 0, 0, 0, 0));

    for (std::size_t i = 3; i < 10; ++i)
    {
        ASSERT_EQ(10, d[i].tag());
    }
    ASSERT_TRUE(d.full());
    ASSERT_EQ(10U, d.size());

    ::internal::TestClassCalls::reset();
    d.resize(0, item2);
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 10, 0, 0));

    ASSERT_TRUE(d.empty());
    ASSERT_EQ(0U, d.size());
}

/**
 * \desc
 * Verifies that resize will call the destructor for removed elements.
 */
TEST(Deque, ResizeTwoParameterDestructs)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, d);
    d.assign(10, ::internal::TestClassCalls());
    ASSERT_TRUE(d.full());
    ::internal::TestClassCalls::reset();
    d.resize(0, ::internal::TestClassCalls());
    ::internal::TestClassCalls::verify(1, 0, 0, 11, 0, 0);
}

TEST(Deque, EraseSingleElementOfEmptyList)
{
    MAKE_DEQUE(int32_t, 10, d);
    ASSERT_EQ(d.end(), d.erase(d.cbegin()));
    ASSERT_EQ(d.end(), d.erase(d.cend()));
}

TEST(Deque, EraseSingleElementPositionBeforeTail)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    IntDeque::iterator itr = d.erase(d.cbegin() + 1);
    ASSERT_EQ(3, *itr);
    ASSERT_EQ(3U, d.size());
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(3, d[1]);
    ASSERT_EQ(4, d[2]);
}

TEST(Deque, EraseSingleElementPositionAfterHead)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_front(0);
    d.push_front(-1);
    d.push_front(-2);
    IntDeque::iterator itr = d.erase(d.cbegin() + 2);
    ASSERT_EQ(6U, d.size());
    ASSERT_EQ(-2, d[0]);
    ASSERT_EQ(-1, d[1]);
    ASSERT_EQ(1, d[2]);
    ASSERT_EQ(2, d[3]);
    ASSERT_EQ(3, d[4]);
    ASSERT_EQ(4, d[5]);
    ASSERT_EQ(1, *itr);
}

TEST(Deque, EraseSingleElement)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    IntDeque::iterator itr = d.erase(d.cbegin());
    ASSERT_EQ(d.end(), itr);
    ASSERT_TRUE(d.empty());
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    itr = d.end() - 1;
    ASSERT_EQ(3, *itr);
    itr = d.erase(IntDeque::const_iterator(itr));
    ASSERT_EQ(d.end(), itr);
    ASSERT_EQ(2U, d.size());
    itr = d.erase(d.cbegin());
    ASSERT_EQ(2, *itr);
    ASSERT_EQ(1U, d.size());
    d.push_back(1);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);
    ASSERT_EQ(2, d[0]);
    ASSERT_EQ(1, d[1]);
    ASSERT_EQ(3, d[2]);
    ASSERT_EQ(4, d[3]);
    ASSERT_EQ(5, d[4]);
    ASSERT_EQ(5U, d.size());
    itr = d.erase(d.cbegin() + 1);
    ASSERT_EQ(3, *itr);
    ASSERT_EQ(2, d[0]);
    ASSERT_EQ(3, d[1]);
    ASSERT_EQ(4, d[2]);
    ASSERT_EQ(5, d[3]);
    ASSERT_EQ(4U, d.size());
    d.push_front(1);
    d.push_front(0);
    d.push_front(9);
    ASSERT_EQ(9, d[0]);
    ASSERT_EQ(0, d[1]);
    ASSERT_EQ(1, d[2]);
    ASSERT_EQ(2, d[3]);
    ASSERT_EQ(3, d[4]);
    ASSERT_EQ(4, d[5]);
    ASSERT_EQ(5, d[6]);
    ASSERT_EQ(7U, d.size());
    itr = d.erase(d.cbegin() + 1);
    ASSERT_EQ(1, *itr);
    ASSERT_EQ(9, d[0]);
    ASSERT_EQ(1, d[1]);
    ASSERT_EQ(2, d[2]);
    ASSERT_EQ(3, d[3]);
    ASSERT_EQ(4, d[4]);
    ASSERT_EQ(5, d[5]);
    ASSERT_EQ(6U, d.size());
}

TEST(Deque, EraseRangeNoOps)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);

    ASSERT_EQ(2U, d.size());
    d.erase(d.cend(), d.cend());
    ASSERT_EQ(2U, d.size());

    IntDeque::const_iterator a = d.cbegin();
    ++a;
    d.erase(a, a);
    ASSERT_EQ(2U, d.size());
}

TEST(Deque, EraseRange)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);
    IntDeque::iterator itr = d.erase(d.cbegin() + 1, d.cbegin() + 3);
    ASSERT_EQ(4, *itr);
    ASSERT_EQ(3U, d.size());
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(4, d[1]);
    ASSERT_EQ(5, d[2]);
    itr = d.erase(IntDeque::const_iterator(itr), d.cend());
    ASSERT_EQ(1U, d.size());
    ASSERT_TRUE(itr == d.end());
    ASSERT_EQ(1, d[0]);
    itr = d.erase(d.cbegin(), d.cend());
    ASSERT_EQ(0U, d.size());
    ASSERT_TRUE(itr == d.end());
}

TEST(Deque, EraseRangeWrappedContiguousRange)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);
    d.push_front(0);
    d.push_front(-1);
    d.push_front(-2);
    IntDeque::iterator itr = d.erase(d.cbegin() + 1, d.cbegin() + 2);
    ASSERT_EQ(0, *itr);
    ASSERT_EQ(7U, d.size());
    ASSERT_EQ(-2, d[0]);
    ASSERT_EQ(0, d[1]);
    ASSERT_EQ(1, d[2]);
    ASSERT_EQ(2, d[3]);
    ASSERT_EQ(3, d[4]);
    ASSERT_EQ(4, d[5]);
    ASSERT_EQ(5, d[6]);
}

TEST(Deque, EraseRangeWrappedContiguousRange2)
{
    // clang-format off
                               //  h           t  ==> h=0 t=6
    MAKE_DEQUE(int32_t, 6, d); // |* * * * * * *|

                    //  h              ==> h=0 t=0
                    //  t
    d.push_back(1); // |1 * * * * * *|

    d.push_back(2); //  h t            ==> h=0 t=1
                    // |1 2 * * * * *|

    d.push_back(3); //  h   t          ==> h=0 t=2
                    // |1 2 3 * * * *|

    d.push_back(4); //  h     t        ==> h=0 t=3
                    // |1 2 3 4 * * *|

    d.push_back(5); //  h       t      ==> h=0 t=4
                    // |1 2 3 4 5 * *|

    d.push_back(6); //  h         t    ==> h=0 t=5
                    // |1 2 3 4 5 6 *|

    d.pop_front();  //    h       t    ==> h=1 t=5
                    // |* 2 3 4 5 6 *|

    d.push_back(7); //    h         t  ==> h=1 t=6
                    // |* 2 3 4 5 6 7|

    d.pop_front();  //      h       t  ==> h=2 t=6
                    // |* * 3 4 5 6 7|

    d.push_back(8); //  t   h          ==> h=2 t=0
                    // |8 * 3 4 5 6 7|
                    //          ^ ^

    IntDeque::iterator itr = d.erase(d.cbegin() + 2, d.cbegin() + 3);
                    //  t     h        ==> h=3 t=0
                    // |8 * * 3 4 6 7|
                    //            ^

    ASSERT_EQ(6, *itr);
    ASSERT_EQ(5U, d.size());
    ASSERT_EQ(3, d[0]);
    ASSERT_EQ(4, d[1]);
    ASSERT_EQ(6, d[2]);
    ASSERT_EQ(7, d[3]);
    ASSERT_EQ(8, d[4]);
}

TEST(Deque, EraseRangeWrappedContiguousRange3)
{
                               //  h           t  ==> h=0 t=6
    MAKE_DEQUE(int32_t, 6, d); // |* * * * * * *|

                    //  h              ==> h=0 t=0
                    //  t
    d.push_back(1); // |1 * * * * * *|

    d.push_back(2); //  h t            ==> h=0 t=1
                    // |1 2 * * * * *|

    d.push_back(3); //  h   t          ==> h=0 t=2
                    // |1 2 3 * * * *|

    d.push_back(4); //  h     t        ==> h=0 t=3
                    // |1 2 3 4 * * *|

    d.push_back(5); //  h       t      ==> h=0 t=4
                    // |1 2 3 4 5 * *|

    d.push_back(6); //  h         t    ==> h=0 t=5
                    // |1 2 3 4 5 6 *|

    d.pop_front();  //    h       t    ==> h=1 t=5
                    // |* 2 3 4 5 6 *|

    d.push_back(7); //    h         t  ==> h=1 t=6
                    // |* 2 3 4 5 6 7|

    d.pop_front();  //      h       t  ==> h=2 t=6
                    // |* * 3 4 5 6 7|

    d.push_back(8); //  t   h          ==> h=2 t=0
                    // |8 * 3 4 5 6 7|
                    //          ^

    IntDeque::iterator itr = d.begin();
    while (itr != d.end())
    {
        if (*itr == 5)
        {
            itr = d.erase(itr);  //  t     h        ==> h=3 t=0
            break;               // |8 * * 3 4 6 7|
        }                        //            ^
        itr++;
    }
    // clang-format on

    ASSERT_EQ(6, *itr);
    ASSERT_EQ(5U, d.size());
    ASSERT_EQ(3, d[0]);
    ASSERT_EQ(4, d[1]);
    ASSERT_EQ(6, d[2]);
    ASSERT_EQ(7, d[3]);
    ASSERT_EQ(8, d[4]);
}

TEST(Deque, EraseRangeWrappedRangeWrapped)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);
    d.push_front(0);
    d.push_front(-1);
    d.push_front(-2);
    IntDeque::iterator itr = d.erase(d.cbegin() + 1, d.cbegin() + 3);
    ASSERT_EQ(1, *itr);
    ASSERT_EQ(6U, d.size());
    ASSERT_EQ(-2, d[0]);
    ASSERT_EQ(1, d[1]);
    ASSERT_EQ(2, d[2]);
    ASSERT_EQ(3, d[3]);
    ASSERT_EQ(4, d[4]);
    ASSERT_EQ(5, d[5]);
}

TEST(Deque, EraseRangeWrappedWiderRangeWrapped)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);
    d.push_front(0);
    d.push_front(-1);
    d.push_front(-2);
    IntDeque::iterator itr = d.erase(d.cbegin() + 1, d.cbegin() + 5);
    ASSERT_EQ(3, *itr);
    ASSERT_EQ(4U, d.size());
    ASSERT_EQ(-2, d[0]);
    ASSERT_EQ(3, d[1]);
    ASSERT_EQ(4, d[2]);
    ASSERT_EQ(5, d[3]);
}

TEST(Deque, EraseRangeWrappedAll)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);
    d.push_front(0);
    d.push_front(-1);
    d.push_front(-2);
    IntDeque::iterator itr = d.erase(d.cbegin(), d.cend());
    ASSERT_TRUE(itr == d.end());
    ASSERT_EQ(0U, d.size());
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |* * 1 2 3 4 5 * * * * |
 * |    h       t         |
 * |      ^               |
 */
TEST(Deque, InsertSingleElementMoveHeadLeft)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(0);
    d.push_back(0);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);
    d.pop_front();
    d.pop_front();

    d.insert(d.cbegin() + 1, -1);
    ASSERT_EQ(6U, d.size());
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(-1, d[1]);
    ASSERT_EQ(2, d[2]);
    ASSERT_EQ(3, d[3]);
    ASSERT_EQ(4, d[4]);
    ASSERT_EQ(5, d[5]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |1 2 3 4 5 * * * * * * |
 * |h       t             |
 * |  ^                   |
 */
TEST(Deque, InsertSingleElementMoveTailRight)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);

    d.insert(d.cbegin() + 1, -1);
    ASSERT_EQ(6U, d.size());
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(-1, d[1]);
    ASSERT_EQ(2, d[2]);
    ASSERT_EQ(3, d[3]);
    ASSERT_EQ(4, d[4]);
    ASSERT_EQ(5, d[5]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |1 2 3 4 5 * * * *-1 0 |
 * |        t         h   |
 * |  ^                   |
 */
TEST(Deque, InsertSingleElementWrappedMoveTailRight)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);
    d.push_front(0);
    d.push_front(-1);

    d.insert(d.cbegin() + 3, -2);
    ASSERT_EQ(8U, d.size());
    ASSERT_EQ(-1, d[0]);
    ASSERT_EQ(0, d[1]);
    ASSERT_EQ(1, d[2]);
    ASSERT_EQ(-2, d[3]);
    ASSERT_EQ(2, d[4]);
    ASSERT_EQ(3, d[5]);
    ASSERT_EQ(4, d[6]);
    ASSERT_EQ(5, d[7]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |1 2 3 4 5 * * * *-1 0 |
 * |        t         h   |
 * |                    ^ |
 */
TEST(Deque, InsertSingleElementWrappedMoveHeadLeft)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);
    d.push_front(0);
    d.push_front(-1);

    d.insert(d.cbegin() + 1, -2);
    ASSERT_EQ(8U, d.size());
    ASSERT_EQ(-1, d[0]);
    ASSERT_EQ(-2, d[1]);
    ASSERT_EQ(0, d[2]);
    ASSERT_EQ(1, d[3]);
    ASSERT_EQ(2, d[4]);
    ASSERT_EQ(3, d[5]);
    ASSERT_EQ(4, d[6]);
    ASSERT_EQ(5, d[7]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |* * * * * * * * * * * |
 * |h                   t |
 * |                      |
 */
TEST(Deque, InsertSingleElementInEmptyDeque)
{
    MAKE_DEQUE(int32_t, 10, d);
    IntDeque::iterator itr = d.insert(d.cbegin(), 1);
    ASSERT_EQ(1U, d.size());
    ASSERT_EQ(itr, d.begin());
    ASSERT_EQ(1, *d.begin());
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |1 * * * * * * * * * * |
 * |h                     |
 * |t                     |
 * |^                     |
 */
TEST(Deque, InsertSingleElementInDequeOfSizeOneWithHeadAndTailAtZeroBeforeBegin)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    IntDeque::iterator itr = d.insert(d.cbegin(), -1);
    ASSERT_EQ(2U, d.size());
    ASSERT_EQ(-1, *itr);
    ASSERT_EQ(-1, d[0]);
    ASSERT_EQ(1, d[1]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |1 * * * * * * * * * * |
 * |h                     |
 * |t                     |
 * |^                     |
 */
TEST(Deque, InsertSingleElementInDequeOfSizeOneWithHeadAndTailAtZeroBeforeEnd)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    IntDeque::iterator itr = d.insert(d.cend(), -1);
    ASSERT_EQ(2U, d.size());
    ASSERT_EQ(-1, *itr);
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(-1, d[1]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |* * * * * * * * * * 1 |
 * |                    h |
 * |                    t |
 * |                    ^ |
 */
TEST(Deque, InsertSingleElementInDequeOfSizeOneWithHeadAndTailAtMaxSizeBeforeBegin)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(1);
    IntDeque::iterator itr = d.insert(d.cbegin(), -1);
    ASSERT_EQ(2U, d.size());
    ASSERT_EQ(-1, *itr);
    ASSERT_EQ(-1, d[0]);
    ASSERT_EQ(1, d[1]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |* * * * * * * * * * 1 |
 * |                    h |
 * |                    t |
 * |                    ^ |
 */
TEST(Deque, InsertSingleElementInDequeOfSizeOneWithHeadAndTailAtMaxSizeBeforeEnd)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(1);
    IntDeque::iterator itr = d.insert(d.cend(), -1);
    ASSERT_EQ(2U, d.size());
    ASSERT_EQ(-1, *itr);
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(-1, d[1]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |* * * * * * * * 1 * * |
 * |                h     |
 * |                t     |
 * |                ^     |
 */
TEST(Deque, InsertSingleElementInDequeOfSizeOneWithHeadAndTailEqualBeforeBegin)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(1);
    d.push_front(1);
    d.push_front(1);
    d.pop_back();
    d.pop_back();
    IntDeque::iterator itr = d.insert(d.cbegin(), -1);
    ASSERT_EQ(2U, d.size());
    ASSERT_EQ(-1, *itr);
    ASSERT_EQ(-1, d[0]);
    ASSERT_EQ(1, d[1]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |* * * * * * * * 1 * * |
 * |                h     |
 * |                t     |
 * |                ^     |
 */
TEST(Deque, InsertSingleElementInDequeOfSizeOneWithHeadAndTailEqualBeforeEnd)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(1);
    d.push_front(1);
    d.push_front(1);
    d.pop_back();
    d.pop_back();
    IntDeque::iterator itr = d.insert(d.cend(), -1);
    ASSERT_EQ(2U, d.size());
    ASSERT_EQ(-1, *itr);
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(-1, d[1]);
}

TEST(Deque, InsertSingleElementIntoFullDeque)
{
    ::estd::declare::deque<int32_t, 10> d(10, 0);
    ASSERT_TRUE(d.full());
    ASSERT_EQ(d.end(), d.insert(d.cbegin(), 1));
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |* * 1 2 3 4 5 * * * * |
 * |    h       t         |
 * |      ^               |
 */
TEST(Deque, EmplaceSingleElementMoveHeadLeft)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(0);
    d.push_back(0);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);
    d.pop_front();
    d.pop_front();

    d.emplace(d.cbegin() + 1).construct(-1);

    ASSERT_EQ(6U, d.size());
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(-1, d[1]);
    ASSERT_EQ(2, d[2]);
    ASSERT_EQ(3, d[3]);
    ASSERT_EQ(4, d[4]);
    ASSERT_EQ(5, d[5]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |1 2 3 4 5 * * * * * * |
 * |h       t             |
 * |  ^                   |
 */
TEST(Deque, EmplaceSingleElementMoveTailRight)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);

    d.emplace(d.cbegin() + 1).construct(-1);
    ASSERT_EQ(6U, d.size());
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(-1, d[1]);
    ASSERT_EQ(2, d[2]);
    ASSERT_EQ(3, d[3]);
    ASSERT_EQ(4, d[4]);
    ASSERT_EQ(5, d[5]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |1 2 3 4 5 * * * *-1 0 |
 * |        t         h   |
 * |  ^                   |
 */
TEST(Deque, EmplaceSingleElementWrappedMoveTailRight)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);
    d.push_front(0);
    d.push_front(-1);

    d.emplace(d.cbegin() + 3).construct(-2);
    ASSERT_EQ(8U, d.size());
    ASSERT_EQ(-1, d[0]);
    ASSERT_EQ(0, d[1]);
    ASSERT_EQ(1, d[2]);
    ASSERT_EQ(-2, d[3]);
    ASSERT_EQ(2, d[4]);
    ASSERT_EQ(3, d[5]);
    ASSERT_EQ(4, d[6]);
    ASSERT_EQ(5, d[7]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |1 2 3 4 5 * * * *-1 0 |
 * |        t         h   |
 * |                    ^ |
 */
TEST(Deque, EmplaceSingleElementWrappedMoveHeadLeft)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.push_back(2);
    d.push_back(3);
    d.push_back(4);
    d.push_back(5);
    d.push_front(0);
    d.push_front(-1);

    d.emplace(d.cbegin() + 1).construct(-2);
    ASSERT_EQ(8U, d.size());
    ASSERT_EQ(-1, d[0]);
    ASSERT_EQ(-2, d[1]);
    ASSERT_EQ(0, d[2]);
    ASSERT_EQ(1, d[3]);
    ASSERT_EQ(2, d[4]);
    ASSERT_EQ(3, d[5]);
    ASSERT_EQ(4, d[6]);
    ASSERT_EQ(5, d[7]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |* * * * * * * * * * * |
 * |h                   t |
 * |                      |
 */
TEST(Deque, EmplaceSingleElementInEmptyDeque)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.emplace(d.cbegin()).construct(1);
    ASSERT_EQ(1U, d.size());
    ASSERT_EQ(1, *d.begin());
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |1 * * * * * * * * * * |
 * |h                     |
 * |t                     |
 * |^                     |
 */
TEST(Deque, EmplaceSingleElementInDequeOfSizeOneWithHeadAndTailAtZeroBeforeBegin)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.emplace(d.cbegin()).construct(-1);
    ASSERT_EQ(2U, d.size());
    ASSERT_EQ(-1, d[0]);
    ASSERT_EQ(1, d[1]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |1 * * * * * * * * * * |
 * |h                     |
 * |t                     |
 * |^                     |
 */
TEST(Deque, EmplaceSingleElementInDequeOfSizeOneWithHeadAndTailAtZeroBeforeEnd)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_back(1);
    d.emplace(d.cend()).construct(-1);
    ASSERT_EQ(2U, d.size());
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(-1, d[1]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |* * * * * * * * * * 1 |
 * |                    h |
 * |                    t |
 * |                    ^ |
 */
TEST(Deque, EmplaceSingleElementInDequeOfSizeOneWithHeadAndTailAtMaxSizeBeforeBegin)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(1);
    d.emplace(d.cbegin()).construct(-1);
    ASSERT_EQ(2U, d.size());
    ASSERT_EQ(-1, d[0]);
    ASSERT_EQ(1, d[1]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |* * * * * * * * * * 1 |
 * |                    h |
 * |                    t |
 * |                    ^ |
 */
TEST(Deque, EmplaceSingleElementInDequeOfSizeOneWithHeadAndTailAtMaxSizeBeforeEnd)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(1);
    d.emplace(d.cend()).construct(-1);
    ASSERT_EQ(2U, d.size());
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(-1, d[1]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |* * * * * * * * 1 * * |
 * |                h     |
 * |                t     |
 * |                ^     |
 */
TEST(Deque, EmplaceSingleElementInDequeOfSizeOneWithHeadAndTailEqualBeforeBegin)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(1);
    d.push_front(1);
    d.push_front(1);
    d.pop_back();
    d.pop_back();
    d.emplace(d.cbegin()).construct(-1);
    ASSERT_EQ(2U, d.size());
    ASSERT_EQ(-1, d[0]);
    ASSERT_EQ(1, d[1]);
}

/**
 * \desc
 * |0 1 2 3 4 5 6 7 8 9 10|
 * |* * * * * * * * 1 * * |
 * |                h     |
 * |                t     |
 * |                ^     |
 */
TEST(Deque, EmplaceSingleElementInDequeOfSizeOneWithHeadAndTailEqualBeforeEnd)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(1);
    d.push_front(1);
    d.push_front(1);
    d.pop_back();
    d.pop_back();
    d.emplace(d.cend()).construct(-1);
    ASSERT_EQ(2U, d.size());
    ASSERT_EQ(1, d[0]);
    ASSERT_EQ(-1, d[1]);
}

TEST(Deque, InsertFill)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.insert(d.cbegin(), 5, 17);
    ASSERT_EQ(5U, d.size());
    int32_t i = 0;
    for (int& itr : d)
    {
        ASSERT_EQ(17, itr) << "@ " << i;
        ++i;
    }
}

TEST(Deque, InsertRange)
{
    MAKE_DEQUE(int32_t, 10, d1);
    MAKE_DEQUE(int32_t, 10, d2);
    // insert empty deque
    ASSERT_EQ(d2.end(), d2.insert(d2.cbegin(), d1.begin(), d1.end()));
    ASSERT_TRUE(d2.empty());
    // insert queue with one element
    d1.push_back(1);
    ::estd::deque<int32_t>::iterator iter = d2.insert(d2.cbegin(), d1.begin(), d1.end());
    ASSERT_EQ(1, *iter);
    ASSERT_EQ(1U, d2.size());
    ASSERT_EQ(1, d2[0]);
    d1.push_front(2);
    d2.insert(d2.cbegin(), d1.begin(), d1.end());
    ASSERT_EQ(3U, d2.size());
    ASSERT_EQ(2, d2[0]);
    ASSERT_EQ(1, d2[1]);
    ASSERT_EQ(1, d2[2]);
    d1.clear();
    d1.push_back(3);
    d1.push_back(4);
    d1.push_back(5);
    d1.push_back(6);
    d1.push_back(7);
    d1.push_back(8);
    d1.push_back(9);
    d1.push_back(10);
    d2.insert(d2.cbegin() + 1, d1.begin(), d1.end());
    ASSERT_EQ(2, d2[0]);
    ASSERT_EQ(3, d2[1]);
    ASSERT_EQ(4, d2[2]);
    ASSERT_EQ(5, d2[3]);
    ASSERT_EQ(6, d2[4]);
    ASSERT_EQ(7, d2[5]);
    ASSERT_EQ(8, d2[6]);
    ASSERT_EQ(9, d2[7]);
    ASSERT_EQ(1, d2[8]);
    ASSERT_EQ(1, d2[9]);
}

TEST(Deque, RelationalOperators)
{
    MAKE_DEQUE(int32_t, 10, d1);
    MAKE_DEQUE(int32_t, 11, d2);
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
    d1.push_back(1);
    ASSERT_NE(d1, d2);
    ASSERT_FALSE((d1 == d2));
    ASSERT_TRUE((d1 != d2));
    d2.push_back(1);
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
    d1.push_back(2);
    d2.push_back(0);
    ASSERT_GT(d1, d2);
    ASSERT_TRUE((d1 > d2));
    ASSERT_LT(d2, d1);
    ASSERT_TRUE((d2 < d1));
}

TEST(Deque, RelationalSameSizeDifferentItems)
{
    MAKE_DEQUE(int32_t, 10, v);
    MAKE_DEQUE(int32_t, 10, d);

    v.push_back(1);
    d.push_back(2);

    ASSERT_NE(v, d);
    v.push_back(2);
    ASSERT_FALSE(v < d);
}

TEST(Deque, UncopyableButMovable)
{
    MAKE_DEQUE(::internal::UncopyableButMovableObject, 10, q);

    q.emplace_back().construct(1);
    q.emplace_back().construct(2);
    q.emplace_back().construct(3);
    q.emplace_front().construct(4);

    ASSERT_EQ(4U, q.size());
    ASSERT_EQ(4, q.front().i);
    ASSERT_EQ(3, q.back().i);
    q.erase(q.cbegin());
    ASSERT_EQ(3U, q.size());
    q.pop_back();
    ASSERT_EQ(2U, q.size());
    q.pop_front();
    ASSERT_EQ(1U, q.size());
    q.erase(q.cbegin(), q.cend());
    ASSERT_EQ(0U, q.size());
}

TEST(Deque, TestNoDefaultConstructor)
{
    MAKE_DEQUE(::internal::NoDefaultConstructorObject, 10, q);

    q.emplace_back().construct(1);
    q.emplace_back().construct(2);
    q.emplace_back().construct(3);
    q.emplace_front().construct(4);

    ASSERT_EQ(4U, q.size());
    ASSERT_EQ(4, q.front().i);
    ASSERT_EQ(3, q.back().i);
    q.erase(q.cbegin());
    ASSERT_EQ(3U, q.size());
    q.pop_back();
    ASSERT_EQ(2U, q.size());
    q.pop_front();
    ASSERT_EQ(1U, q.size());
    q.erase(q.cbegin(), q.cend());
    ASSERT_EQ(0U, q.size());
}

TEST(Deque, TestDestructorCalls)
{
    ::internal::TestClassCalls::reset();
    {
        MAKE_DEQUE(::internal::TestClassCalls, 10, a);

        ::internal::TestClassCalls::reset();

        // make sure that the destructors are called when we assign
        // a new value.
        for (int32_t i = 0; i < (int32_t)a.max_size(); ++i)
        {
            a.emplace_back().construct(i);
        }
        ASSERT_TRUE(::internal::TestClassCalls::verify(10, 0, 0, 0, 0, 0));

        ::internal::TestClassCalls::reset();
    }

    // verify that the destructors were called when the array was destroyed
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 10, 0, 0));
}

TEST(Deque, TestEmplaceBackCopy)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, a);

    ::internal::TestClassCalls tc(5);

    ::internal::TestClassCalls::reset();

    a.emplace_back().construct(tc);

    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 1, 0, 0, 0, 0));
}

TEST(Deque, TestCopyConstructorCalls)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, a);

    // initialize array
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        ::internal::TestClassCalls::reset();
        a.emplace_back().construct(static_cast<int32_t>(i));
        ASSERT_TRUE(::internal::TestClassCalls::verify(1, 0, 0, 0, 0, 0));
    }

    // now create new deque from other deque
    ::internal::TestClassCalls::reset();
    ::estd::declare::deque<::internal::TestClassCalls, 10> b(a);

    // verify that the number of calls was correct
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));

    ASSERT_EQ(8U, a.size());

    // and verify that the values were copied.
    int32_t i = 0;
    for (::estd::deque<::internal::TestClassCalls>::const_iterator iter = b.cbegin();
         iter != b.cend();
         ++iter)
    {
        ASSERT_EQ(i++, iter->tag());
    }
}

TEST(Deque, TestCopyConstructorDeclareCalls)
{
    ::estd::declare::deque<::internal::TestClassCalls, 10> a;

    // initialize array
    for (int32_t i = 0; i < 5; ++i)
    {
        a.emplace_back().construct(i);
    }

    // now create new deque from other deque
    ::internal::TestClassCalls::reset();
    ::estd::declare::deque<::internal::TestClassCalls, 10> b(a);

    // verify that the number of calls was correct
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 5, 0, 0, 0, 0));

    ASSERT_EQ(5U, a.size());

    // and verify that the values were copied.
    int32_t i = 0;
    for (::estd::deque<::internal::TestClassCalls>::const_iterator iter = b.cbegin();
         iter != b.cend();
         ++iter)
    {
        ASSERT_EQ(i++, iter->tag());
    }
}

void copy(::estd::declare::deque<::internal::TestClassCalls, 10> copy)
{
    // make sure that our copies were done properly.
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));
    ASSERT_EQ(8U, copy.size());
    // and verify that the values were copied.
    int32_t i = 0;
    for (::estd::deque<::internal::TestClassCalls>::const_iterator iter = copy.cbegin();
         iter != copy.cend();
         ++iter)
    {
        ASSERT_EQ(i++, iter->tag());
    }
}

TEST(Deque, TestCopyConstructorCallsToFunction)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        ::internal::TestClassCalls::reset();
        a.emplace_back().construct(i);
        ASSERT_TRUE(::internal::TestClassCalls::verify(1, 0, 0, 0, 0, 0));
    }

    ::internal::TestClassCalls::reset();
    copy(a);
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 8, 0, 0));
}

TEST(Deque, TestAssignmentCalls)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        a.emplace_back().construct(i);
    }

    ::estd::declare::deque<::internal::TestClassCalls, 10> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));

    int32_t i = 0;
    for (::estd::deque<::internal::TestClassCalls>::const_iterator iter = b.cbegin();
         iter != b.cend();
         ++iter)
    {
        ASSERT_EQ(i++, iter->tag());
    }

    ::estd::deque<::internal::TestClassCalls>& tmp = b;
    // test self assignment
    ::internal::TestClassCalls::reset();
    tmp = b;
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 0, 0, 0));

    MAKE_DEQUE(::internal::TestClassCalls, 10, c);

    // make sure we can assign into a value of the base class
    ::internal::TestClassCalls::reset();
    c = tmp;
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));

    i = 0;
    for (::estd::deque<::internal::TestClassCalls>::const_iterator iter = c.cbegin();
         iter != c.cend();
         ++iter)
    {
        ASSERT_EQ(i++, iter->tag());
    }
}

void fillDeque(::estd::deque<int32_t>& d)
{
    for (size_t i = 0; i < d.max_size(); ++i)
    {
        d.push_back(i);
    }
}

TEST(Deque, TestAssignmentIdentical)
{
    ::estd::declare::deque<int32_t, 10> a;
    fillDeque(a);

    ::estd::declare::deque<int32_t, 10> b;

    b = a;
    ASSERT_EQ(10U, b.size());

    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b.front());
        b.pop_front();
    }
}

TEST(Deque, TestAssignmentMore)
{
    ::estd::declare::deque<int32_t, 10> a;
    fillDeque(a);

    ::estd::declare::deque<int32_t, 10> b;
    b.push_back(0);
    b.push_back(1);

    a = b;
    ASSERT_EQ(2U, a.size());

    // and that the values are correct
    for (size_t i = 0; i < a.size(); ++i)
    {
        ASSERT_EQ((int)i, b.front());
        b.pop_front();
    }
}

TEST(Deque, TestCopyConstructorBase)
{
    ::estd::declare::deque<int32_t, 10> a;
    fillDeque(a);

    ::estd::deque<int32_t>& base = a;
    ::estd::declare::deque<int32_t, 10> b(base);

    ASSERT_EQ(10U, b.size());
    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b.front());
        b.pop_front();
    }

    b = base;
    ASSERT_EQ(10U, b.size());
    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b.front());
        b.pop_front();
    }
}

TEST(Deque, TestAssignmentCallsAndDestructor)
{
    ::internal::TestClassCalls::reset();

    MAKE_DEQUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        a.emplace_back().construct(i);
    }

    ::estd::declare::deque<::internal::TestClassCalls, 10> b;
    for (int32_t i = 0; i < 5; ++i)
    {
        b.emplace_back().construct(i);
    }

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();
    b = a;
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 3, 0, 0, 5, 0));

    ASSERT_EQ(8U, b.size());
    int32_t i = 0;
    for (::estd::deque<::internal::TestClassCalls>::const_iterator iter = b.cbegin();
         iter != b.cend();
         ++iter)
    {
        ASSERT_EQ(i++, iter->tag());
    }
}

TEST(Deque, TestAssignmentDestructorCalls)
{
    {
        MAKE_DEQUE(::internal::TestClassCalls, 10, a);

        // initialize deque
        for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
        {
            a.emplace_back().construct(i);
        }

        ::estd::declare::deque<::internal::TestClassCalls, 10> b;
        for (int32_t i = 0; i < 5; ++i)
        {
            b.emplace_back().construct(i);
        }

        // verify that the assignment operator works
        b = a;
        ::internal::TestClassCalls::reset();
    }

    // make sure that we destroyed the objects we created.
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 16, 0, 0));
}

TEST(Deque, TestAssignmentIncorrectSize)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        a.emplace_back().construct(i);
    }

    ::estd::declare::deque<::internal::TestClassCalls, 5> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    ASSERT_THROW(b = a, ::estd::assert_exception);
}

TEST(Deque, TestAssignmentBiggerSize)
{
    MAKE_DEQUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        a.emplace_back().construct(i);
    }

    ::estd::declare::deque<::internal::TestClassCalls, 15> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));

    ASSERT_EQ(8U, b.size());
    int32_t i = 0;
    for (::estd::deque<::internal::TestClassCalls>::const_iterator iter = b.cbegin();
         iter != b.cend();
         ++iter)
    {
        ASSERT_EQ(i++, iter->tag());
    }
}

TEST(Deque, TestCopyIncorrectSize)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_DEQUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        a.emplace_back().construct(i);
    }

    try
    {
        ::estd::declare::deque<::internal::TestClassCalls, 5> b(a);
        ASSERT_TRUE(0);
    }
    catch (::estd::assert_exception const& e)
    {}
}

/*
 *
 * DequeIterator tests
 *
 */
TEST(DequeIterator, DefaultConstructible) { IntDeque::iterator itr; }

TEST(DequeIterator, CopyConstructibleAndCopyAssingable)
{
    IntDeque::iterator itr;
    IntDeque::iterator itr2(itr);
    ASSERT_EQ(itr, itr2);
    itr = itr2;
    ASSERT_EQ(itr, itr2);
}

TEST(DequeIterator, Comparable)
{
    IntDeque::iterator itr;
    IntDeque::iterator itr2(itr);
    ASSERT_TRUE(itr == itr2);
    ASSERT_FALSE(itr != itr2);
}

TEST(DequeIterator, DereferenceableRValue)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(1);
    IntDeque::iterator itr = d.begin();
    ASSERT_EQ(d.front(), *itr);
    ASSERT_EQ(&d.front(), itr.operator->());
}

TEST(DequeIterator, DereferenceableLValue)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(1);
    IntDeque::iterator itr = d.begin();
    ASSERT_EQ(1, *itr);
    *itr = 10;
    ASSERT_EQ(10, *itr);
    ASSERT_EQ(10, d.front());
}

TEST(DequeIterator, Equality)
{
    MAKE_DEQUE(int32_t, 10, d);
    IntDeque const& c = d;
    ASSERT_EQ(d.end(), d.begin());
    ASSERT_EQ(d.cend(), d.cbegin());
    ASSERT_EQ(c.begin(), d.cbegin());
    d.push_back(1);
    ASSERT_NE(d.end(), d.begin());
    ASSERT_NE(d.cend(), d.cbegin());
    ASSERT_NE(c.end(), d.cbegin());
}

TEST(DequeIterator, Increment)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(2);
    d.push_back(3);
    d.push_back(4);
    d.push_front(1);
    d.push_back(5);
    IntDeque::const_iterator itr = d.cbegin();
    ASSERT_EQ(1, *itr);
    ++itr;
    ASSERT_EQ(2, *itr);
    itr++;
    ASSERT_EQ(3, *itr);
    ASSERT_EQ(3, *itr++);
    ASSERT_EQ(4, *itr);
    ASSERT_EQ(5, *++itr);
}

TEST(DequeIterator, Decrement)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(2);
    d.push_back(3);
    d.push_back(4);
    d.push_front(1);
    d.push_back(5);
    IntDeque::const_iterator citr = d.cend();
    --citr;
    ASSERT_EQ(5, *citr);
    --citr;
    ASSERT_EQ(4, *citr);
    citr--;
    ASSERT_EQ(3, *citr);
    ASSERT_EQ(3, *citr--);
    ASSERT_EQ(2, *citr);
    ASSERT_EQ(1, *--citr);

    IntDeque::iterator itr = d.end();
    --itr;
    ASSERT_EQ(5, *itr);
    --itr;
    ASSERT_EQ(4, *itr);
    itr--;
    ASSERT_EQ(3, *itr);
    ASSERT_EQ(3, *itr--);
    ASSERT_EQ(2, *itr);
    ASSERT_EQ(1, *--itr);
}

TEST(DequeIterator, ArithmeticOperators)
{
    MAKE_DEQUE(int32_t, 10, d);
    d.push_front(2);
    d.push_back(3);
    d.push_back(4);
    d.push_front(1);
    d.push_back(5);
    d.push_back(6);
    d.push_back(7);
    d.push_back(8);
    d.push_back(9);
    d.push_back(10);
    IntDeque::iterator itr = d.begin();
    ASSERT_EQ(1, *itr);
    ASSERT_EQ(10, *(itr + 9));
    ASSERT_EQ(1, *(itr + 0));
    ASSERT_EQ(9, *(itr + 8));
    ASSERT_EQ(9, *(8 + itr));
    itr = d.end();
    ASSERT_EQ(8, (*--itr - 2));

    IntDeque::const_iterator citr = d.cbegin();
    ASSERT_EQ(1, *citr);
    ASSERT_EQ(10, *(citr + 9));
    ASSERT_EQ(1, *(citr + 0));
    ASSERT_EQ(9, *(citr + 8));
    ASSERT_EQ(9, *(8 + citr));
    citr = d.cend();
    ASSERT_EQ(8, (*--citr - 2));

    ASSERT_EQ(10, (d.cend() - d.cbegin()));
    ASSERT_EQ(10, (d.cbegin() - d.cend()));
    ASSERT_EQ(10, (d.end() - d.begin()));
    ASSERT_EQ(10, (d.begin() - d.end()));
}

TEST(DequeIterator, ArithmeticOperatorsDifferentDeques)
{
    MAKE_DEQUE(int32_t, 10, d1);
    MAKE_DEQUE(int32_t, 10, d2);

    d1.push_back(1);
    d2.push_back(1);

    ASSERT_EQ(0, d1.begin() - d2.begin());
    ASSERT_EQ(0, d1.cbegin() - d2.cbegin());
}

TEST(DequeIterator, ArithmeticOperatorsWithDefaultConstructedIterators)
{
    IntDeque::iterator itr1, itr2;
    ASSERT_EQ(0, (itr1 - itr2));
    ASSERT_EQ(0, (itr2 - itr1));
}

TEST(DequeIterator, RelationalOperators)
{
    MAKE_DEQUE(int32_t, 10, d);
    for (int32_t i = 0; i < 10; ++i)
    {
        d.push_back() = i;
    }
    IntDeque::iterator itr1 = d.begin();
    IntDeque::iterator itr2 = d.begin();
    ASSERT_TRUE(itr1 >= itr2);
    ASSERT_TRUE(itr2 >= itr1);
    ASSERT_TRUE(itr1 <= itr2);
    ASSERT_TRUE(itr2 <= itr1);
    ASSERT_TRUE(++itr2 > itr1);
    ASSERT_FALSE(itr1 > itr2);
    ASSERT_FALSE(itr1 >= itr2);

    ASSERT_FALSE(itr2 < itr2);
    ASSERT_TRUE(++itr2 > itr1);
    itr1 += 3;
    ASSERT_TRUE(itr2 < itr1);

    IntDeque::const_iterator citr1 = d.cbegin();
    IntDeque::const_iterator citr2 = d.cbegin();
    ASSERT_TRUE(citr1 >= citr2);
    ASSERT_TRUE(citr2 >= citr1);
    ASSERT_TRUE(citr1 <= citr2);
    ASSERT_TRUE(citr2 <= citr1);
    ASSERT_TRUE(++citr2 > citr1);
    ASSERT_FALSE(citr1 > citr2);
    ASSERT_FALSE(citr1 >= citr2);
    ASSERT_FALSE(citr2 < citr1);

    ASSERT_TRUE(++citr2 > citr1);
    citr1 += 3;
    ASSERT_TRUE(citr2 < citr1);
}

TEST(DequeIterator, ComparisonLessEqual)
{
    MAKE_DEQUE(int32_t, 10, d);

    d.push_back(1);
    d.push_back(2);

    IntDeque::iterator a = d.begin();
    IntDeque::iterator b = d.begin();
    ASSERT_TRUE(b <= a);

    ++b;

    ASSERT_FALSE(b <= a);

    IntDeque::const_iterator ca = d.cbegin();
    IntDeque::const_iterator cb = d.cbegin();
    ASSERT_TRUE(cb <= ca);

    ++cb;

    ASSERT_FALSE(cb <= ca);
}

TEST(DequeIterator, ComparisonDifferentDeques)
{
    MAKE_DEQUE(int32_t, 10, left);
    MAKE_DEQUE(int32_t, 10, right);

    IntDeque::iterator left1  = left.begin();
    IntDeque::iterator right1 = right.begin();

    EXPECT_FALSE(left1 == right1);
    EXPECT_FALSE(left1 < right1);
    EXPECT_FALSE(left1 <= right1);
    EXPECT_FALSE(left1 > right1);
    EXPECT_FALSE(left1 >= right1);
}

TEST(DequeIterator, ConstComparisonDifferentDeques)
{
    MAKE_DEQUE(int32_t, 10, left);
    MAKE_DEQUE(int32_t, 10, right);

    IntDeque::const_iterator left1  = left.cbegin();
    IntDeque::const_iterator right1 = right.cbegin();

    EXPECT_FALSE(left1 == right1);
    EXPECT_FALSE(left1 < right1);
    EXPECT_FALSE(left1 <= right1);
    EXPECT_FALSE(left1 > right1);
    EXPECT_FALSE(left1 >= right1);
}

TEST(DequeIterator, CompoundAssignmentOperations)
{
    MAKE_DEQUE(int32_t, 10, d);
    for (int32_t i = 0; i < 10; ++i)
    {
        d.push_back() = i;
    }
    IntDeque::iterator itr = d.begin();
    ASSERT_EQ(0, *itr);
    itr += 1;
    ASSERT_EQ(1, *itr);
    itr += 2;
    ASSERT_EQ(3, *itr);
    itr += 3;
    ASSERT_EQ(6, *itr);
    itr -= 4;
    ASSERT_EQ(2, *itr);
}

TEST(DequeIterator, OffsetDereferenceOperator)
{
    MAKE_DEQUE(int32_t, 10, d);
    for (int32_t i = 0; i < 10; ++i)
    {
        d.push_back() = i;
    }
    IntDeque::iterator itr = d.begin();
    ASSERT_EQ(0, itr[0]);
    ASSERT_EQ(1, itr[1]);
    ASSERT_EQ(9, itr[9]);

    IntDeque::const_iterator citr = d.cbegin();
    ASSERT_EQ(0, citr[0]);
    ASSERT_EQ(1, citr[1]);
    ASSERT_EQ(9, citr[9]);
}

TEST(DequeIterator, IteratorToConstIterator)
{
    MAKE_DEQUE(int32_t, 10, d);
    IntDeque::iterator i = d.begin();
    IntDeque::const_iterator ci(i);
    ASSERT_TRUE(IntDeque::const_iterator(i) == ci);
}

TEST(DequeIterator, DefaultConstructors)
{
    IntDeque::iterator i1, i2;

    ASSERT_THAT(i1.operator->(), IsNull());
    ASSERT_TRUE(i1 <= i2);
    ASSERT_EQ(i1, i2);

    ++i1;
    ASSERT_EQ(i1, i2);
    i1++;
    ASSERT_EQ(i1, i2);
    ASSERT_EQ(0, i1 - i2);

    IntDeque::const_iterator ci1, ci2;
    ASSERT_THAT(i1.operator->(), IsNull());
    ASSERT_TRUE(ci1 <= ci2);
    ASSERT_EQ(ci1, ci2);

    ++ci1;
    ASSERT_EQ(ci1, ci2);
    ci1++;
    ASSERT_EQ(ci1, ci2);
    ASSERT_EQ(0, ci1 - ci2);
}
} // namespace
