// Copyright 2024 Accenture.

#include "estd/vector.h"

#include "concepts/iterable.h"
#include "estd/deque.h"
#include "internal/NoDefaultConstructorObject.h"
#include "internal/TestClassCalls.h"
#include "internal/UncopyableButMovableObject.h"

#include <platform/estdint.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

using namespace ::testing;
using namespace ::estd::test;

// explicit instantiation to get correct test coverage
template class ::estd::declare::vector<int32_t, 10>;
template class ::estd::vector<int32_t>;

using IntVector10 = ::estd::declare::vector<int32_t, 10>;
using IntVector   = ::estd::vector<int32_t>;

static_assert(std::is_same<int32_t, IntVector10::value_type>::value, "");
static_assert(std::is_same<int32_t&, IntVector10::reference>::value, "");
static_assert(std::is_same<int32_t const&, IntVector10::const_reference>::value, "");
static_assert(std::is_same<int32_t*, IntVector10::pointer>::value, "");
static_assert(std::is_same<int32_t const*, IntVector10::const_pointer>::value, "");
static_assert(std::is_same<std::size_t, IntVector10::size_type>::value, "");
static_assert(std::is_same<std::ptrdiff_t, IntVector10::difference_type>::value, "");

static_assert(std::is_same<int32_t, IntVector::value_type>::value, "");
static_assert(std::is_same<int32_t&, IntVector::reference>::value, "");
static_assert(std::is_same<int32_t const&, IntVector::const_reference>::value, "");
static_assert(std::is_same<int32_t*, IntVector::pointer>::value, "");
static_assert(std::is_same<int32_t const*, IntVector::const_pointer>::value, "");
static_assert(std::is_same<std::size_t, IntVector::size_type>::value, "");
static_assert(std::is_same<std::ptrdiff_t, IntVector::difference_type>::value, "");

namespace
{
struct IntVector3_P
{
    enum Constants
    {
        LENGTH = 3
    };

    using Subject = ::estd::vector<int32_t>;
    ::estd::declare::vector<int32_t, LENGTH> subject;

    IntVector3_P()
    {
        for (size_t i = 0; i < LENGTH; ++i)
        {
            subject.push_back(i + 1);
        }
    }
};

INSTANTIATE_TYPED_TEST_SUITE_P(Vector, ForwardIterableConcept, IntVector3_P);
INSTANTIATE_TYPED_TEST_SUITE_P(Vector, ReverseIterableConcept, IntVector3_P);

#define MAKE_VECTOR(T, N, Name)            \
    ::estd::declare::vector<T, N> Name##_; \
    ::estd::vector<T>& Name(Name##_)

template<size_t total>
void validateVectorSize()
{
    ::estd::declare::vector<int32_t, total> container;

    size_t bucketSize = sizeof(int32_t);

    if (sizeof(size_t) == 4)
    {
        ASSERT_EQ(8U, sizeof(IntVector));
        ASSERT_EQ(total * bucketSize + 8, sizeof(container)) << "32bit:" << total;
    }
    else
    {
        size_t padding = (total * bucketSize) % 8;

        ASSERT_EQ(16U, sizeof(IntVector));
        ASSERT_EQ(total * bucketSize + 16 + padding, sizeof(container)) << "64bit:" << total;
    }
}

TEST(Vector, TestSizeOf)
{
    validateVectorSize<10>();
    validateVectorSize<5>();
    validateVectorSize<1>();
}

struct Foo
{
    Foo() = default;

    Foo(Foo const&)            = delete;
    Foo& operator=(Foo const&) = delete;

    int32_t _v = 0;
};

struct Bar
{
    Bar(int32_t v) : _v1(v), _v2(v) {}

    Bar(int32_t v1, int32_t v2) : _v1(v1), _v2(v2) {}

    Bar(Bar const&)            = delete;
    Bar& operator=(Bar const&) = delete;

    int32_t _v1;
    int32_t _v2;
};

struct Crazy
{
    Crazy(int32_t v1, int32_t v2, int32_t v3, int32_t v4, int32_t v5, int32_t v6)
    : _v1(v1), _v2(v2), _v3(v3), _v4(v4), _v5(v5), _v6(v6)
    {}

    Crazy(Crazy const&)            = delete;
    Crazy& operator=(Crazy const&) = delete;

    int32_t _v1;
    int32_t _v2;
    int32_t _v3;
    int32_t _v4;
    int32_t _v5;
    int32_t _v6;
};

TEST(Vector, TestAllocateBack)
{
    ::estd::declare::vector<Bar, 10> v;
    ::estd::vector<Bar>& v2(v);

    Bar& b(v.emplace_back().construct(10));
    ASSERT_EQ(10, b._v1);
    ASSERT_EQ(10, b._v2);
    ASSERT_EQ(1U, v.size());
    Bar* pB = &v.emplace_back().construct(10, 17);
    ASSERT_EQ(10, pB->_v1);
    ASSERT_EQ(17, pB->_v2);
    ASSERT_EQ(2U, v2.size());
}

TEST(Vector, TestAllocateBackCrazy)
{
    ::estd::declare::vector<Crazy, 10> v;
    Crazy* pC = new (v.emplace_back().release()) Crazy(1, 2, 3, 4, 5, 6);
    ASSERT_EQ(1U, v.size());
    ASSERT_EQ(1, pC->_v1);
    ASSERT_EQ(2, pC->_v2);
    ASSERT_EQ(3, pC->_v3);
    ASSERT_EQ(4, pC->_v4);
    ASSERT_EQ(5, pC->_v5);
    ASSERT_EQ(6, pC->_v6);
}

TEST(Vector, TestEmplaceIterator)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::declare::vector<int32_t, 4> v;
    v.emplace(v.cbegin()).construct(3);
    ASSERT_EQ(1U, v.size());
    ASSERT_THAT(v, ElementsAre(3));
    v.emplace(v.cbegin()).construct(1);
    ASSERT_EQ(2U, v.size());
    ASSERT_THAT(v, ElementsAre(1, 3));
    v.emplace(v.cbegin() + 1U).construct(2);
    ASSERT_EQ(3U, v.size());
    ASSERT_THAT(v, ElementsAre(1, 2, 3));
    v.emplace(v.cend()).construct(4);
    ASSERT_EQ(4U, v.size());
    ASSERT_THAT(v, ElementsAre(1, 2, 3, 4));

    // vector is full
    ASSERT_THROW({ v.emplace(v.begin()); }, ::estd::assert_exception);
}

TEST(Vector, TestNonCopyable)
{
    ::estd::declare::vector<Foo, 10> v;
    Foo& r = v.push_back();
    r._v   = 17;
}

TEST(Vector, TestConstructors)
{
    IntVector10 v1;
    ASSERT_TRUE(v1.empty());
    ASSERT_EQ(0U, v1.size());

    IntVector10 v2(10, 42);
    ASSERT_TRUE(v2.full());
    ASSERT_EQ(10U, v2.size());
    for (IntVector::size_type i = 0; i < v2.max_size(); ++i)
    {
        ASSERT_EQ(42, v2[i]);
    }
}

TEST(Vector, TestAssign)
{
    MAKE_VECTOR(int32_t, 10, v);
    v.assign(10, 42);
    ASSERT_EQ(10U, v.size());
    for (IntVector::size_type i = 0; i < v.max_size(); ++i)
    {
        ASSERT_EQ(42, v[i]);
    }
}

TEST(Vector, AssignCopies)
{
    MAKE_VECTOR(::internal::TestClassCalls, 10, d);

    ::internal::TestClassCalls item(17);

    ::internal::TestClassCalls::reset();
    d.assign(5, item);
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 5, 0, 0, 0, 0));
    ASSERT_EQ(5U, d.size());
    for (::estd::vector<::internal::TestClassCalls>::const_iterator itr = d.cbegin();
         itr != d.cend();
         ++itr)
    {
        ASSERT_EQ(17, itr->tag());
    }
}

TEST(Vector, AssignIteratorCopies)
{
    MAKE_VECTOR(::internal::TestClassCalls, 10, d);

    for (int32_t i = 0; i < 5; ++i)
    {
        d.emplace_back().construct(i);
    }

    MAKE_VECTOR(::internal::TestClassCalls, 10, a);

    ::internal::TestClassCalls::reset();
    a.assign(d.begin(), d.end());

    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 5, 0, 0, 0, 0));
    ASSERT_EQ(5U, a.size());
    int32_t count = 0;
    for (::estd::vector<::internal::TestClassCalls>::const_iterator itr = a.cbegin();
         itr != a.cend();
         ++itr)
    {
        ASSERT_EQ(count++, itr->tag());
    }
}

TEST(Vector, TestAssignRange)
{
    MAKE_VECTOR(int32_t, 10, source);
    for (IntVector::size_type i = 0; i < source.max_size(); ++i)
    {
        source.push_back(i);
    }
    MAKE_VECTOR(int32_t, 10, target);
    target.assign(source.begin(), source.end());

    ASSERT_EQ(10U, target.size());
    for (IntVector::size_type i = 0; i < target.size(); ++i)
    {
        ASSERT_EQ(source[i], target[i]);
    }
}

TEST(Vector, TestBegin)
{
    MAKE_VECTOR(int32_t, 10, v);
    v.push_back(17);
    ASSERT_EQ(17, *v.begin());
    IntVector::iterator itr = v.begin();
    *itr                    = 42;
    ASSERT_EQ(42, v[0]);
}

TEST(Vector, TestRBegin)
{
    MAKE_VECTOR(int32_t, 10, v);
    for (IntVector::size_type i = 0; i < v.max_size(); ++i)
    {
        v.push_back(i);
    }
    ASSERT_EQ(9, *v.rbegin());
    IntVector::reverse_iterator itr = v.rbegin();
    *itr                            = 42;
    ASSERT_EQ(42, v[9]);
}

TEST(Vector, TestIterators)
{
    MAKE_VECTOR(int32_t, 10, v);
    for (IntVector::size_type i = 0; i < v.max_size(); ++i)
    {
        v.push_back(42);
        ASSERT_EQ(42, v[i]);
    }
    ASSERT_EQ(10, std::count(v.begin(), v.end(), 42));
    for (IntVector::size_type i = 0; i < v.size(); ++i)
    {
        v[i] = i;
    }
    int32_t i = 0;
    for (IntVector::iterator itr = v.begin(); itr != v.end(); ++itr, ++i)
    {
        ASSERT_EQ(i, *itr);
    }
    i = 0;
    for (IntVector::const_iterator itr = v.cbegin(); itr != v.cend(); ++itr, ++i)
    {
        ASSERT_EQ(i, *itr);
    }
    i = 9;
    for (IntVector::reverse_iterator itr = v.rbegin(); itr != v.rend(); ++itr, --i)
    {
        ASSERT_EQ(i, *itr);
    }
    i = 9;
    for (IntVector::const_reverse_iterator itr = v.crbegin(); itr != v.crend(); ++itr, --i)
    {
        ASSERT_EQ(i, *itr);
    }
}

TEST(Vector, RelationalOperators)
{
    MAKE_VECTOR(int32_t, 10, d1);
    MAKE_VECTOR(int32_t, 11, d2);
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

TEST(Vector, RelationalSameSizeDifferentItems)
{
    MAKE_VECTOR(int32_t, 10, v);
    MAKE_VECTOR(int32_t, 10, d);

    v.push_back(1);
    d.push_back(2);

    ASSERT_NE(v, d);
    v.push_back(2);
    ASSERT_FALSE(v < d);
}

TEST(Vector, TestMaxSize)
{
    MAKE_VECTOR(int32_t, 10, v);
    ASSERT_EQ(IntVector::size_type(10), v.max_size());
}

TEST(Vector, TestEmpty)
{
    MAKE_VECTOR(int32_t, 10, v);
    ASSERT_TRUE(v.empty());
}
#ifndef __GNUC__
TEST(Vector, TestFull)
{
    MAKE_VECTOR(int32_t, 0, v);
    ASSERT_TRUE(v.empty());
    ASSERT_TRUE(v.full());
}
#endif
TEST(Vector, TestSize)
{
    MAKE_VECTOR(int32_t, 10, v);
    IntVector::size_type s = 0;
    while (!v.full())
    {
        ASSERT_EQ(s, v.size());
        v.push_back(1);
        ++s;
    }
    ASSERT_EQ(v.max_size(), v.size());
}

TEST(Vector, TestPushBack)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    MAKE_VECTOR(int32_t, 10, v);
    ASSERT_THROW({ v.back(); }, ::estd::assert_exception);
    ASSERT_THROW({ v.front(); }, ::estd::assert_exception);

    for (IntVector::size_type i = 0; i < v.max_size(); ++i)
    {
        v.push_back(i);
        ASSERT_EQ(IntVector::value_type(i), v.back());
        ASSERT_EQ(0, v.front());
    }
    ASSERT_EQ(v.max_size(), v.size());
    ASSERT_TRUE(v.full());
    ASSERT_THROW({ v.push_back(10); }, ::estd::assert_exception);
}

TEST(Vector, TestPushBackAssign)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_VECTOR(int32_t, 10, v);
    for (IntVector::size_type i = 0; i < v.max_size(); ++i)
    {
        v.push_back() = i;
        ASSERT_EQ(IntVector::value_type(i), v.back());
    }
    ASSERT_EQ(v.max_size(), v.size());
    ASSERT_EQ(v.max_size(), v.size());
    ASSERT_TRUE(v.full());
    ASSERT_THROW(v.push_back() = 10, ::estd::assert_exception);
}

TEST(Vector, TestPopBack)
{
    MAKE_VECTOR(int32_t, 10, v);
    v.push_back(1);
    ASSERT_EQ(IntVector::size_type(1), v.size());
    ASSERT_EQ(1, v.front());
    ASSERT_EQ(1, v.back());
    v.pop_back();
    ASSERT_TRUE(v.empty());
    ASSERT_EQ(IntVector::size_type(0), v.size());
    v.push_back(2);
    v.push_back(3);
    ASSERT_EQ(2, v.front());
    ASSERT_EQ(3, v.back());
    ASSERT_EQ(IntVector::size_type(2), v.size());
    v.pop_back();
    ASSERT_EQ(2, v.front());
    ASSERT_EQ(2, v.back());
    ASSERT_EQ(IntVector::size_type(1), v.size());
    v.pop_back();
    ASSERT_TRUE(v.empty());
}

TEST(Vector, TestPopBackTestClassCalls)
{
    ::internal::TestClassCalls::reset();
    ::estd::declare::vector<::internal::TestClassCalls, 2> vec;
    ::internal::TestClassCalls::verify(0, 0, 0, 0, 0, 0);
    vec.push_back();
    ::internal::TestClassCalls::verify(1, 0, 0, 0, 0, 0);
    vec.push_back();
    ::internal::TestClassCalls::verify(2, 0, 0, 0, 0, 0);

    vec.pop_back();
    ::internal::TestClassCalls::verify(2, 0, 0, 1, 0, 0);
    vec.pop_back();
    ::internal::TestClassCalls::verify(2, 0, 0, 2, 0, 0);
}

TEST(Vector, TestRandomAccessOperator)
{
    MAKE_VECTOR(int32_t, 10, v);
    for (IntVector::size_type i = 0; i < v.max_size(); ++i)
    {
        v.push_back(i);
        ASSERT_EQ(IntVector::value_type(i), v[i]) << "with i: " << i;
    }
    IntVector::size_type sz = v.size();
    for (IntVector::size_type i = 0; i < sz / 2; ++i)
    {
        std::swap(v[sz - 1 - i], v[i]);
    }
    for (IntVector::size_type i = 0; i < v.max_size(); ++i)
    {
        ASSERT_EQ(IntVector::value_type(v.max_size() - 1 - i), v[i]) << "with i: " << i;
    }
}

TEST(Vector, TestConstRandomAccessOperator)
{
    MAKE_VECTOR(int32_t, 10, v);
    ::estd::vector<int32_t> const& cv = v;

    for (IntVector::size_type i = 0; i < v.max_size(); ++i)
    {
        v.push_back(i);
        IntVector::const_reference r = cv[i];
        ASSERT_EQ(IntVector::value_type(i), r) << "with i: " << i;
    }
}

TEST(Vector, TestAt)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_VECTOR(int32_t, 10, v);
    for (IntVector::size_type i = 0; i < v.max_size(); ++i)
    {
        v.push_back(i);
        ASSERT_EQ(IntVector::value_type(i), v.at(i)) << "with i: " << i;
    }
    IntVector::size_type sz = v.size();
    for (IntVector::size_type i = 0; i < sz / 2; ++i)
    {
        std::swap(v.at(sz - 1 - i), v.at(i));
    }
    for (IntVector::size_type i = 0; i < v.max_size(); ++i)
    {
        ASSERT_EQ(IntVector::value_type(v.max_size() - 1 - i), v.at(i)) << "with i: " << i;
    }
    ASSERT_THROW(v.at(v.max_size()), ::estd::assert_exception);
}

TEST(Vector, TestConstAt)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_VECTOR(int32_t, 10, v);

    ::estd::vector<int32_t> const& cv = v;

    for (IntVector::size_type i = 0; i < v.max_size(); ++i)
    {
        v.push_back(i);
        IntVector::const_reference r = cv.at(i);
        ASSERT_EQ(IntVector::value_type(i), r) << "with i: " << i;
    }
    ASSERT_THROW(cv.at(v.max_size());, ::estd::assert_exception);
}

TEST(Vector, TestConstFront)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_VECTOR(int32_t, 10, v);
    ::estd::vector<int32_t> const& cv = v;

    ASSERT_THROW(cv.front(), ::estd::assert_exception);

    v.push_back(42);
    v.push_back(43);
    IntVector::const_reference r = cv.front();
    ASSERT_EQ(42, r);
}

TEST(Vector, TestConstBack)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_VECTOR(int32_t, 10, v);
    ::estd::vector<int32_t> const& cv = v;

    ASSERT_THROW(cv.back(), ::estd::assert_exception);

    v.push_back(42);
    v.push_back(43);
    IntVector::const_reference r = cv.back();
    ASSERT_EQ(43, r);
}

TEST(Vector, TestClear)
{
    MAKE_VECTOR(int32_t, 10, v);
    v.clear();
    ASSERT_TRUE(v.empty());
    v.push_back(1);
    ASSERT_TRUE(!v.empty());
    v.clear();
    ASSERT_TRUE(v.empty());
}

TEST(Vector, TestInsertAtFront)
{
    MAKE_VECTOR(int32_t, 4, v);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    IntVector::iterator i = v.insert(v.begin(), 1);

    ASSERT_EQ(v.begin(), i);
    ASSERT_THAT(v, ElementsAre(1, 2, 3, 4));
    ASSERT_EQ(IntVector::size_type(4), v.size());
}

TEST(Vector, TestInsertInMiddle)
{
    MAKE_VECTOR(int32_t, 4, v);
    v.push_back(0);
    v.push_back(1);
    v.push_back(3);
    IntVector::iterator i = v.insert(v.begin() + 2, 2);

    ASSERT_EQ(v.begin() + 2, i);
    ASSERT_THAT(v, ElementsAre(0, 1, 2, 3));
    ASSERT_EQ(IntVector::size_type(4), v.size());
}

TEST(Vector, TestInsertInMiddle2)
{
    MAKE_VECTOR(int32_t, 4, v);
    v.push_back(0);
    v.push_back(3);
    v.insert(v.begin() + 1, 2, 1);
    ASSERT_THAT(v, ElementsAre(0, 1, 1, 3));
    ASSERT_EQ(IntVector::size_type(4), v.size());
}

TEST(Vector, TestInsertFails)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_VECTOR(int32_t, 2, v);
    v.push_back(2);
    v.push_back(3);
    ASSERT_THROW(v.insert(v.begin(), 1), ::estd::assert_exception);
}

TEST(Vector, TestInsertFillAtFront)
{
    MAKE_VECTOR(int32_t, 6, v);
    v.push_back(4);
    v.push_back(5);
    v.push_back(6);
    v.insert(v.begin(), 3, 1);

    ASSERT_THAT(v, ElementsAre(1, 1, 1, 4, 5, 6));
    ASSERT_EQ(IntVector::size_type(6), v.size());
}

TEST(Vector, TestInsertFillInMiddle)
{
    MAKE_VECTOR(int32_t, 6, v);
    v.push_back(0);
    v.push_back(1);
    v.push_back(4);
    v.push_back(5);
    v.insert(v.begin() + 2, 2, 2);

    ASSERT_THAT(v, ElementsAre(0, 1, 2, 2, 4, 5));
    ASSERT_EQ(IntVector::size_type(6), v.size());
}

TEST(Vector, TestInsertFillFails)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_VECTOR(int32_t, 4, v);
    v.push_back(2);
    v.push_back(3);
    ASSERT_THROW(v.insert(v.begin(), 3, 1), ::estd::assert_exception);
}

TEST(Vector, TestInsertRangeAtFront)
{
    MAKE_VECTOR(int32_t, 3, source);
    source.push_back(1);
    source.push_back(2);
    source.push_back(3);
    MAKE_VECTOR(int32_t, 6, target);
    target.assign(IntVector::size_type(3), 42);

    target.insert(target.begin(), source.begin(), source.end());

    ASSERT_THAT(target, ElementsAre(1, 2, 3, 42, 42, 42));
    ASSERT_EQ(IntVector::size_type(6), target.size());
}

TEST(Vector, TestInsertRangeInMiddle)
{
    MAKE_VECTOR(int32_t, 3, source);
    source.push_back(1);
    source.push_back(2);
    source.push_back(3);
    MAKE_VECTOR(int32_t, 6, target);
    target.assign(IntVector::size_type(3), 42);
    ASSERT_THAT(target, ElementsAre(42, 42, 42));

    target.insert(target.begin() + 2, source.begin(), source.end());

    ASSERT_THAT(target, ElementsAre(42, 42, 1, 2, 3, 42));
    ASSERT_EQ(IntVector::size_type(6), target.size());
}

TEST(Vector, TestEraseAtFront)
{
    MAKE_VECTOR(int32_t, 4, v);
    v.push_back(0);
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.erase(v.begin());

    ASSERT_THAT(v, ElementsAre(1, 2, 3));
    ASSERT_EQ(IntVector::size_type(3), v.size());
}

TEST(Vector, TestEraseAssert)
{
    MAKE_VECTOR(int32_t, 2, v);
    v.push_back(0);
    v.push_back(1);
    ASSERT_EQ(2U, v.size());
    ASSERT_EQ(1, *v.erase(v.cbegin()));
    ASSERT_EQ(1U, v.size());
    ::estd::vector<int32_t>::iterator iter;

    iter = v.erase(v.cbegin());
    ASSERT_EQ(v.end(), iter);
    ASSERT_EQ(0U, v.size());

    iter = v.erase(v.cbegin());
    ASSERT_EQ(v.end(), iter);
}

TEST(Vector, TestEraseAtEnd)
{
    MAKE_VECTOR(int32_t, 4, v);
    v.push_back(0);
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.erase(v.end() - 1);

    ASSERT_THAT(v, ElementsAre(0, 1, 2));
    ASSERT_EQ(IntVector::size_type(3), v.size());
}

TEST(Vector, TestEraseFails)
{
    MAKE_VECTOR(int32_t, 4, v);
    ASSERT_EQ(v.end(), v.erase(v.begin()));
}

TEST(Vector, TestEraseRangeAtFront)
{
    MAKE_VECTOR(int32_t, 4, v);
    v.push_back(0);
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.erase(v.begin(), v.begin() + 2);

    ASSERT_THAT(v, ElementsAre(2, 3));
    ASSERT_EQ(IntVector::size_type(2), v.size());
}

TEST(Vector, TestEraseRangeAtEnd)
{
    MAKE_VECTOR(int32_t, 4, v);
    v.push_back(0);
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.erase(v.end() - 2, v.end());

    ASSERT_THAT(v, ElementsAre(0, 1));
    ASSERT_EQ(IntVector::size_type(2), v.size());
}

TEST(Vector, TestEraseRangeFails)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_VECTOR(int32_t, 4, v);
    v.push_back(0);
    v.push_back(1);
    ASSERT_THROW(v.erase(v.begin(), v.begin() + 3), ::estd::assert_exception);
}

TEST(Vector, TestEraseReverseRange)
{
    MAKE_VECTOR(int32_t, 4, v);
    v.push_back(0);
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.erase(v.begin() + 3, v.begin());
    ASSERT_EQ(IntVector::size_type(4), v.size());
}

TEST(Vector, UncopyableButMovable)
{
    MAKE_VECTOR(::internal::UncopyableButMovableObject, 10, v);

    v.push_back().i = 10;
    v.emplace_back().construct(12);
    v.emplace_back().construct(14);
    v.emplace(v.cbegin() + 1U).construct(11);
    v.emplace(v.cbegin() + 3U).construct(13);

    ASSERT_EQ(5U, v.size());
    ASSERT_EQ(10, v.at(0).i);
    ASSERT_EQ(12, v.at(2).i);
    v.pop_back();
    ASSERT_EQ(4U, v.size());
    ASSERT_EQ(10, v.at(0).i);
    v.erase(v.begin());
    ASSERT_EQ(3U, v.size());
    ASSERT_EQ(11, v.at(0).i);
    v.erase(v.begin(), v.end());
    ASSERT_EQ(0U, v.size());
}

TEST(Vector, TestNoDefaultConstructor)
{
    MAKE_VECTOR(::internal::NoDefaultConstructorObject, 10, v);

    v.emplace_back().construct(12);
    v.emplace_back().construct(13);
    v.emplace_back().construct(14);

    ASSERT_EQ(3U, v.size());
    ASSERT_EQ(12, v.at(0).i);
    ASSERT_EQ(13, v.at(1).i);
    v.pop_back();
    ASSERT_EQ(2U, v.size());
    ASSERT_EQ(12, v.at(0).i);
    v.erase(v.begin());
    ASSERT_EQ(1U, v.size());
    ASSERT_EQ(13, v.at(0).i);
    v.erase(v.begin(), v.end());
    ASSERT_EQ(0U, v.size());
}

class Nested
{
public:
    ::estd::declare::vector<int32_t, 10> data_;
    Nested() = default;
};

TEST(Vector, TestNested)
{
    ::estd::declare::vector<Nested, 10> v;

    Nested& item = v.push_back();
    ASSERT_EQ(0U, item.data_.size());
    ASSERT_EQ(1U, v.size());
    item.data_.push_back(10);
    ASSERT_EQ(1U, item.data_.size());
}

TEST(Vector, TestVectorOfVector)
{
    ::estd::declare::vector<::estd::declare::vector<int32_t, 10>, 10> v;

    ::estd::vector<int32_t>& item = v.push_back();
    ASSERT_EQ(0U, item.size());
    ASSERT_EQ(1U, v.size());
    item.push_back(10);
    ASSERT_EQ(1U, item.size());

    for (auto const& item : v)
    {
        ASSERT_EQ(1U, item.size());
    }
}

TEST(Vector, TestCreate)
{
    ::internal::TestClassCalls::reset();
    ::estd::declare::vector<::internal::TestClassCalls, 5> vec;
    ::internal::TestClassCalls::verify(0, 0, 0, 0, 0, 0);
    vec.push_back();
    ::internal::TestClassCalls::verify(1, 0, 0, 0, 0, 0);

    vec.pop_back();
    ::internal::TestClassCalls::verify(1, 0, 0, 1, 0, 0);
}

TEST(Vector, TestErase)
{
    ::internal::TestClassCalls::reset();
    ::estd::declare::vector<::internal::TestClassCalls, 5> vec;
    vec.push_back();
    vec.erase(vec.cbegin());
    ::internal::TestClassCalls::verify(1, 0, 0, 1, 0, 0);
}

TEST(Vector, TestEraseMultiple)
{
    ::internal::TestClassCalls::reset();
    ::estd::declare::vector<::internal::TestClassCalls, 5> vec;
    vec.push_back();
    vec.push_back();
    vec.erase(vec.cbegin(), vec.cend());
    ::internal::TestClassCalls::verify(2, 0, 0, 2, 0, 0);
}

TEST(Vector, TestClearDestructors)
{
    ::internal::TestClassCalls::reset();
    ::estd::declare::vector<::internal::TestClassCalls, 5> vec;
    vec.push_back();
    vec.push_back();
    vec.clear();
    ::internal::TestClassCalls::verify(2, 0, 0, 2, 0, 0);
}

TEST(Vector, TestInsertMoves)
{
    ::internal::TestClassCalls::reset();
    ::estd::declare::vector<::internal::TestClassCalls, 5> vec;
    vec.push_back(::internal::TestClassCalls(0));
    vec.push_back(::internal::TestClassCalls(3));

    vec.insert(vec.begin() + 1, 2, ::internal::TestClassCalls(1));
    ::internal::TestClassCalls::verify(3, 3, 1, 3, 1, 0);
}

TEST(Vector, TestInsertMoves2)
{
    ::internal::TestClassCalls::reset();
    ::estd::declare::vector<::internal::TestClassCalls, 6> vec;
    vec.push_back(::internal::TestClassCalls(1));
    vec.push_back(::internal::TestClassCalls(2));
    vec.push_back(::internal::TestClassCalls(3));
    vec.push_back(::internal::TestClassCalls(4));

    vec.insert(vec.begin(), 2, ::internal::TestClassCalls(0));
    ::internal::TestClassCalls::verify(5, 4, 2, 5, 2, 2);
}

TEST(Vector, TestResizeDefault)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    using ::internal::TestClassCalls;

    ::internal::TestClassCalls::reset();
    ::estd::declare::vector<TestClassCalls, 10> vec;
    ASSERT_EQ(0U, vec.size());

    // no resize
    vec.resize(0U);
    ASSERT_TRUE(TestClassCalls::verify(0, 0, 0, 0, 0, 0));
    ASSERT_EQ(0U, vec.size());

    // resize bigger
    ::internal::TestClassCalls::reset();
    vec.resize(10);
    ASSERT_EQ(10U, vec.size());
    ASSERT_TRUE(TestClassCalls::verify(10, 0, 0, 0, 0, 0));

    // resize smaller
    ::internal::TestClassCalls::reset();
    vec.resize(0);
    ASSERT_EQ(0U, vec.size());
    ASSERT_TRUE(TestClassCalls::verify(0, 0, 0, 10, 0, 0));

    // resize out of bounds
    ASSERT_THROW({ vec.resize(11); }, ::estd::assert_exception);
}

TEST(Vector, TestResizeUserObject)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    using ::internal::TestClassCalls;

    ::internal::TestClassCalls::reset();
    ::estd::declare::vector<TestClassCalls, 10> vec;
    ASSERT_EQ(0U, vec.size());

    // no resize
    TestClassCalls item;
    ::internal::TestClassCalls::reset();
    vec.resize(0U, item);
    ASSERT_TRUE(TestClassCalls::verify(0, 0, 0, 0, 0, 0));
    ASSERT_EQ(0U, vec.size());

    // resize bigger
    ::internal::TestClassCalls::reset();
    vec.resize(10, item);
    ASSERT_EQ(10U, vec.size());
    ASSERT_TRUE(TestClassCalls::verify(0, 10, 0, 0, 0, 0));

    // resize smaller
    ::internal::TestClassCalls::reset();
    vec.resize(0, item);
    ASSERT_EQ(0U, vec.size());
    ASSERT_TRUE(TestClassCalls::verify(0, 0, 0, 10, 0, 0));

    // resize out of bounds
    ASSERT_THROW({ vec.resize(11, item); }, ::estd::assert_exception);
}

TEST(Vector, IteratorToConstIterator)
{
    MAKE_VECTOR(int32_t, 10, v);

    IntVector::iterator i        = v.begin();
    IntVector::const_iterator ci = i;
    ASSERT_TRUE(i == ci);
}

TEST(Vector, TestCopyConstructor)
{
    ::estd::declare::vector<int32_t, 10> a;
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = i;
    }

    ::estd::declare::vector<int32_t, 10> b = a;

    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i]);
    }
}

TEST(Vector, TestDestructorCalls)
{
    ::internal::TestClassCalls::reset();
    {
        MAKE_VECTOR(::internal::TestClassCalls, 10, a);

        ::internal::TestClassCalls::reset();

        for (size_t i = 0; i < a.max_size(); ++i)
        {
            a.push_back(::internal::TestClassCalls(i));
        }
        ASSERT_TRUE(::internal::TestClassCalls::verify(10, 10, 0, 10, 0, 0));

        ::internal::TestClassCalls::reset();
    }

    // verify that the destructors were called when the array was destroyed
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 10, 0, 0));
}

TEST(Vector, TestConstructorSizeDefaultCalls)
{
    ::internal::TestClassCalls::reset();
    ::estd::declare::vector<::internal::TestClassCalls, 10> a(2);
    ASSERT_TRUE(::internal::TestClassCalls::verify(2, 0, 0, 0, 0, 0));
}

TEST(Vector, TestCopyConstructorCalls)
{
    MAKE_VECTOR(::internal::TestClassCalls, 10, a);

    // initialize array
    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.push_back(::internal::TestClassCalls(i));
    }

    // now create new array from other array
    ::internal::TestClassCalls::reset();
    ::estd::declare::vector<::internal::TestClassCalls, 10> b = a;

    // verify that the number of calls was correct
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 10, 0, 0, 0, 0));

    ASSERT_EQ(10U, b.size());
    // and verify that the values were copied.
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i].tag());
    }
}

void arrayCopy(::estd::declare::vector<::internal::TestClassCalls, 10> copy)
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

TEST(Vector, TestCopyConstructorCallsToFunction)
{
    MAKE_VECTOR(::internal::TestClassCalls, 10, a);

    // initialize array
    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.push_back(::internal::TestClassCalls(i));
    }

    ::internal::TestClassCalls::reset();
    arrayCopy(a);
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 10, 0, 10, 0, 0));
}

TEST(Vector, TestAssignmentCalls)
{
    MAKE_VECTOR(::internal::TestClassCalls, 10, a);

    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.push_back(::internal::TestClassCalls(i));
    }

    ::estd::declare::vector<::internal::TestClassCalls, 10> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 10, 0, 0, 0, 0));

    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i].tag());
    }

    MAKE_VECTOR(::internal::TestClassCalls, 10, c);

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

TEST(Vector, TestAssignmentIdentical)
{
    ::estd::declare::vector<int32_t, 10> a;

    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.push_back(i);
    }

    ::estd::declare::vector<int32_t, 10> b;

    b = a;

    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i]);
    }
}

TEST(Vector, TestAssignmentIncorrectSize)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_VECTOR(::internal::TestClassCalls, 10, a);

    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.push_back(::internal::TestClassCalls(i));
    }

    ::estd::declare::vector<::internal::TestClassCalls, 5> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    ASSERT_THROW(b = a, ::estd::assert_exception);
}

TEST(Vector, TestCopyIncorrectSize)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_VECTOR(::internal::TestClassCalls, 10, a);

    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.push_back(::internal::TestClassCalls(i));
    }

    try
    {
        ::estd::declare::vector<::internal::TestClassCalls, 5> b(a);
        ASSERT_TRUE(0);
    }
    catch (::estd::assert_exception const& e)
    {}
}

TEST(Vector, IteratorErase)
{
    ::estd::declare::vector<int32_t, 10> v;

    for (size_t i = 0; i < v.max_size(); ++i)
    {
        v.push_back(i);
    }

    int32_t count = 0;
    for (::estd::vector<int32_t>::const_iterator iter = v.begin(); iter != v.end();)
    {
        count++;
        if (*iter % 2 == 0)
        {
            iter = v.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    ASSERT_EQ(10, count);

    size_t pos = 0;
    ASSERT_EQ(5U, v.size());
    ASSERT_EQ(1, v[pos++]);
    ASSERT_EQ(3, v[pos++]);
    ASSERT_EQ(5, v[pos++]);
    ASSERT_EQ(7, v[pos++]);
    ASSERT_EQ(9, v[pos++]);
}

bool is_even(int32_t i) { return i % 2 == 0; }

TEST(Vector, IteratorEraseAlgorithm)
{
    ::estd::declare::vector<int32_t, 10> v;

    for (size_t i = 0; i < v.max_size(); ++i)
    {
        v.push_back(i);
    }

    v.erase(std::remove_if(v.begin(), v.end(), is_even), v.cend());

    size_t pos = 0;
    ASSERT_EQ(5U, v.size());
    ASSERT_EQ(1, v[pos++]);
    ASSERT_EQ(3, v[pos++]);
    ASSERT_EQ(5, v[pos++]);
    ASSERT_EQ(7, v[pos++]);
    ASSERT_EQ(9, v[pos++]);
}

TEST(Vector, InsertFromDeque)
{
    ::estd::declare::vector<int32_t, 10> v;
    ::estd::declare::deque<int32_t, 10> d;

    d.push_back(1);
    d.push_back(2);

    v.insert(v.begin(), d.begin(), d.end());

    ASSERT_EQ(2U, v.size());
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(v[1], 2);
}

TEST(Vector, TestData)
{
    ::estd::declare::vector<int32_t, 20> v;
    ::estd::vector<int32_t>& v2(v);

    ASSERT_EQ(0U, v2.size());

    for (int32_t i = 0; i < 10; ++i)
    {
        v2.push_back(i);
    }

    ASSERT_EQ(10U, v2.size());

    std::vector<int32_t>::pointer p = v2.data();
    for (int32_t i = 0; i < 10; ++i)
    {
        ASSERT_EQ(i, *p++);
    }

    std::vector<int32_t>::const_pointer c = v2.data();
    for (int32_t i = 0; i < 10; ++i)
    {
        ASSERT_EQ(i, *c++);
    }
}

TEST(Vector, TestConstData)
{
    ::estd::declare::vector<int32_t, 20> v;
    ::estd::vector<int32_t>& v2(v);

    ASSERT_EQ(0U, v2.size());

    for (int32_t i = 0; i < 10; ++i)
    {
        v2.push_back(i);
    }

    ::estd::vector<int32_t> const& v3(v2);

    ASSERT_EQ(10U, v3.size());

    std::vector<int32_t>::const_pointer p = v3.data();
    for (int32_t i = 0; i < 10; ++i)
    {
        ASSERT_EQ(i, *p++);
    }
}

struct alignas(256) Aligned_256
{
    uint32_t a;
};

TEST(Vector, alignment_preservation)
{
    ::estd::declare::vector<Aligned_256, 5> dv;
    ::estd::vector<Aligned_256>& v = dv;

    EXPECT_EQ(0U, reinterpret_cast<uintptr_t>(&dv[0]) % 256);
    EXPECT_EQ(0U, reinterpret_cast<uintptr_t>(&dv[1]) % 256);

    EXPECT_EQ(0U, reinterpret_cast<uintptr_t>(&v[0]) % 256);
    EXPECT_EQ(0U, reinterpret_cast<uintptr_t>(&v[1]) % 256);
}

TEST(Vector, TestZeroSized)
{
    ::estd::declare::vector<int32_t, 0> v;

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    // general checks
    ASSERT_TRUE(v.empty());
    ASSERT_TRUE(v.full());
    ASSERT_EQ(0, v.max_size());
    ASSERT_EQ(0, v.size());

    // read accesses unallowed
    ASSERT_THROW({ v.at(0); }, ::estd::assert_exception);
    ASSERT_THROW({ v.front(); }, ::estd::assert_exception);
    ASSERT_THROW({ v.back(); }, ::estd::assert_exception);

    // write accesses unallowed
    ASSERT_THROW({ v.push_back(); }, ::estd::assert_exception);
    ASSERT_THROW({ v.push_back(123); }, ::estd::assert_exception);
    ASSERT_THROW({ v.pop_back(); }, ::estd::assert_exception);
    ASSERT_THROW({ v.insert(v.begin(), 123); }, ::estd::assert_exception);
    ASSERT_THROW({ v.insert(v.begin(), 1, 123); }, ::estd::assert_exception);
    ASSERT_THROW({ v.emplace(v.begin()); }, ::estd::assert_exception);
    ASSERT_THROW({ v.emplace_back(); }, ::estd::assert_exception);
    ASSERT_THROW({ v.resize(1); }, ::estd::assert_exception);
    ASSERT_THROW({ v.resize(1, 123); }, ::estd::assert_exception);

    ::estd::declare::vector<int32_t, 0> w;
    MAKE_VECTOR(int32_t, 3, source);
    source.push_back(123);
    source.push_back(459);
    source.push_back(789);

    // should have no changes and/or exceptions
    ASSERT_NO_THROW(v.insert(v.begin(), source.begin(), source.end()));
    ASSERT_EQ(v, w);

    ASSERT_NO_THROW(v.erase(v.begin()));
    ASSERT_EQ(v, w);

    ASSERT_NO_THROW(v.erase(v.begin(), v.end()));
    ASSERT_EQ(v, w);

    ASSERT_NO_THROW(v.clear());
    ASSERT_EQ(v, w);

    ASSERT_NO_THROW(v.assign(1, 123));
    ASSERT_EQ(v, w);

    ASSERT_NO_THROW(v.assign(source.begin(), source.end()));
    ASSERT_EQ(v, w);

    ASSERT_NO_THROW(v.resize(0));
    ASSERT_EQ(v, w);

    ASSERT_NO_THROW(v.resize(0, 123));
    ASSERT_EQ(v, w);
}

} // namespace
