// Copyright 2024 Accenture.

#include "concepts/iterable.h"
#include "estd/slice.h"
#include "estd/vec.h"
#include "internal/TestClassCalls.h"

#include <platform/estdint.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{

using namespace ::testing;
using namespace ::estd::test;
using ::internal::TestClassCalls;

/**
 * Test type vec of at most 10 int32_t values.
 */
using IntVec10 = ::estd::vec<int32_t, 10>;

/**
 * Tests for the typedefs of vec.
 */
static_assert(std::is_same<int32_t, IntVec10::value_type>::value, "");
static_assert(std::is_same<int32_t&, IntVec10::reference>::value, "");
static_assert(std::is_same<int32_t const&, IntVec10::const_reference>::value, "");
static_assert(std::is_same<int32_t*, IntVec10::pointer>::value, "");
static_assert(std::is_same<int32_t const*, IntVec10::const_pointer>::value, "");
static_assert(std::is_same<std::size_t, IntVec10::size_type>::value, "");
static_assert(std::is_same<std::ptrdiff_t, IntVec10::difference_type>::value, "");
static_assert(std::is_same<int32_t*, IntVec10::iterator>::value, "");
static_assert(std::is_same<int32_t const*, IntVec10::const_iterator>::value, "");
static_assert(std::is_same<std::reverse_iterator<int32_t*>, IntVec10::reverse_iterator>::value, "");
static_assert(
    std::is_same<std::reverse_iterator<int32_t const*>, IntVec10::const_reverse_iterator>::value,
    "");

/**
 * A test type without a default constructor
 */
struct Bar
{
    Bar(Bar const&) = delete;

    Bar& operator=(Bar const&) = delete;

    explicit Bar(int32_t v) : _v1(v), _v2(v) {}

    Bar(int32_t v1, int64_t v2) : _v1(v1), _v2(v2) {}

    int32_t _v1;
    int64_t _v2;
};

/**
 * A test type with a default constructor
 */
struct Foo
{
    Foo() = default;

    Foo(Foo const&) = delete;

    Foo& operator=(Foo const&) = delete;

    int32_t _v = 88;
};

/**
 * A test type aligned to 256 bytes with a lot of padding.
 */
struct alignas(256) Aligned256Type
{
    uint32_t a;
};

/**
 * Test type for ForwardIterableConcept and ReverseIterableConcept.
 *
 * It needs a LENGTH constant and an instance of Subject named subject which is filled
 * with LENGTH incrementing values starting with the value 1.
 */
struct IntVector3
{
    static constexpr size_t LENGTH = 3;

    using Subject = ::estd::vec<size_t, LENGTH>;

    Subject subject;

    IntVector3()
    {
        for (size_t i = 0; i < LENGTH; ++i)
        {
            subject.push_back(i + 1);
        }
    }
};

INSTANTIATE_TYPED_TEST_SUITE_P(vec_test, ForwardIterableConcept, IntVector3);
INSTANTIATE_TYPED_TEST_SUITE_P(vec_test, ReverseIterableConcept, IntVector3);

template<class T, std::size_t N>
struct VecTestParams
{
    using VecType = ::estd::vec<T, N>;
};

template<typename TestParams>
struct vec_test : ::testing::Test
{
    using VecType = typename TestParams::VecType;
};

using VecTypes = ::testing::Types<
    VecTestParams<uint32_t, 17>,
    VecTestParams<uint32_t, 13456>,
    VecTestParams<uint8_t, 1>,
    VecTestParams<bool, 13>,
    VecTestParams<uint64_t, 10000>,
    VecTestParams<Bar, 5>,
    VecTestParams<Aligned256Type, 1>>;

TYPED_TEST_SUITE(vec_test, VecTypes);

/**
 * \desc
 * Verifies the a default constructed vec is empty and all elements are free.
 */
TYPED_TEST(vec_test, default_constructor)
{
    typename TestFixture::VecType v;
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(0U, v.size());
    EXPECT_EQ(v.max_size, v.free());
}

/**
 * \desc
 * Test for the constructor creating n copies of one given value.
 */
TEST(vec_test, constructor_n_values)
{
    IntVec10 v(10, 42);
    EXPECT_TRUE(v.full());
    EXPECT_EQ(10U, v.size());
    EXPECT_EQ(0U, v.free());
    EXPECT_THAT(v, Each(Eq(42)));
}

/**
 * \desc
 * Test for the constructor creating n copies of one given value. This test assures that
 * at most N copies are created even if n > N and no assert is done.
 */
TEST(vec_test, constructor_more_than_N_values)
{
    // Uniform initialization
    IntVec10 v{15, 42};
    EXPECT_TRUE(v.full());
    EXPECT_EQ(10U, v.size());
    EXPECT_EQ(0U, v.free());
    EXPECT_THAT(v, Each(Eq(42)));
}

/**
 * \desc
 * Test for the constructor creating n default values in vec.
 */
TEST(vec_test, constructor_n_default_values)
{
    IntVec10 v(9);
    EXPECT_FALSE(v.full());
    EXPECT_EQ(9U, v.size());
    EXPECT_EQ(1U, v.free());
    EXPECT_THAT(v, Each(Eq(0)));
}

/**
 * \desc
 * Test for the constructor creating n default values in vec. This test assures that
 * at most N values are created even if n > N and no assert is done.
 */
TEST(vec_test, constructor_more_than_N_default_values)
{
    // Uniform initialization
    IntVec10 v{15};
    EXPECT_TRUE(v.full());
    EXPECT_EQ(10U, v.size());
    EXPECT_EQ(0U, v.free());
    EXPECT_THAT(v, Each(Eq(0)));
}

/**
 * \desc
 * Verifies that after a call to assign exactly n copies of the given value are in the vec.
 */
TEST(vec_test, assign_n_values)
{
    IntVec10 v;
    // assign 5 values.
    v.assign(5, 123456789);
    EXPECT_EQ(5U, v.size());
    EXPECT_EQ(5U, v.free());
    EXPECT_THAT(v, Each(Eq(123456789)));

    // now assign 7 values
    v.assign(7, 123);
    EXPECT_EQ(7U, v.size());
    EXPECT_EQ(3U, v.free());
    EXPECT_THAT(v, Each(Eq(123)));

    // now assign 0 values
    v.assign(0, 123);
    EXPECT_TRUE(v.empty());
}

/**
 * \desc
 * Verifies that assigning more than N values doesn't assert and n copies are stored in the vec.
 */
TEST(vec_test, assign_more_than_N_values)
{
    IntVec10 v;
    // n will be limited to N in the call to assign.
    v.assign(15, 123456789);
    EXPECT_EQ(10U, v.size());
    EXPECT_EQ(0U, v.free());
    EXPECT_THAT(v, Each(Eq(123456789)));
}

/**
 * \desc
 * Verifies that assign from a source iterator range, which fits into the target vector copies
 * the complete range into that target.
 */
TEST(vec_test, assign_range)
{
    std::vector<int32_t> source = {1, 2, 3, 4, 5};

    ::estd::vec<int32_t, 10> target;
    target.assign(source.begin(), source.end());

    EXPECT_EQ(5U, target.size());
    EXPECT_EQ(5U, target.free());
    EXPECT_THAT(target, ElementsAreArray(source));

    // now assign from a different source replacing all previous elements
    std::vector<int32_t> source2 = {2, 3, 4, 5, 6, 7, 8, 9, 10};
    target.assign(source2.begin(), source2.end());
    EXPECT_EQ(9U, target.size());
    EXPECT_EQ(1U, target.free());
    EXPECT_THAT(target, ElementsAreArray(source2));
}

/**
 * \desc
 * Verifies that assign from a source iterator range, which exceeds the maximum size of the target
 * vec, only assign max_size values.
 */
TEST(vec_test, assign_range_exceeding_max_size)
{
    std::vector<int32_t> source = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    ::estd::vec<int32_t, 10> target;
    ASSERT_GT(source.size(), target.max_size);
    target.assign(source.begin(), source.end());

    EXPECT_EQ(10U, target.size());
    EXPECT_EQ(0U, target.free());
    EXPECT_THAT(target, ElementsAreArray(source.begin(), (source.begin() + target.max_size)));
}

/**
 * \desc
 * Verifies behaviour of push_back(value):
 * - Elements are inserted at the end
 * - Asserts when called on a full vec
 */
TEST(vec_test, push_back)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::vec<int32_t, 10> v;

    for (int32_t i = 0; i < static_cast<int32_t>(::estd::vec<int32_t, 10>::max_size); ++i)
    {
        v.push_back(i);
        EXPECT_EQ(i, v.back());
        EXPECT_EQ(0, v.front());
    }
    EXPECT_EQ(10U, v.size());
    EXPECT_EQ(0U, v.free());
    EXPECT_TRUE(v.full());

    EXPECT_THROW({ v.push_back(10); }, ::estd::assert_exception);
}

/**
 * \desc
 * Verifies that push_back works for a type T, which is not copyable.
 */
TEST(vec_test, push_back_non_copyable)
{
    static_assert(!std::is_copy_constructible<Foo>::value, "");
    static_assert(!std::is_copy_assignable<Foo>::value, "");
    ::estd::vec<Foo, 10> v;
    v.push_back()._v = 17;
    EXPECT_EQ(1U, v.size());
    EXPECT_EQ(9U, v.free());
    EXPECT_EQ(17, v[0]._v);
}

/**
 * \desc
 * Verifies behaviour of push_back():
 * - Elements are inserted at the end
 * - Asserts when called on a full vec
 */
TEST(vec_test, push_back_assign)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::vec<int32_t, 10> v;
    for (int32_t i = 0; i < 10; ++i)
    {
        v.push_back() = i;
        EXPECT_EQ(i, v.back());
        EXPECT_EQ(0, v.front());
    }
    EXPECT_EQ(10U, v.size());
    EXPECT_EQ(0U, v.free());
    EXPECT_TRUE(v.full());

    EXPECT_THROW(v.push_back() = 10, ::estd::assert_exception);
}

/**
 * \desc
 * Test case for verifying that
 * - calling pop_back() on a vec removes the last element and decreases the size of the vec.
 * - calling pop_back() on an empty vec asserts.
 */
TEST(vec_test, pop_back)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::vec<int32_t, 10> v;
    v.push_back(1);
    EXPECT_EQ(1, v.front());
    EXPECT_EQ(1, v.back());

    v.pop_back();
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(0U, v.size());
    EXPECT_EQ(10U, v.free());

    v.push_back(2);
    v.push_back(3);
    EXPECT_EQ(2, v.front());
    EXPECT_EQ(3, v.back());
    EXPECT_EQ(2U, v.size());
    EXPECT_EQ(8U, v.free());

    v.pop_back();
    EXPECT_EQ(2, v.front());
    EXPECT_EQ(2, v.back());
    EXPECT_EQ(1U, v.size());
    EXPECT_EQ(9U, v.free());

    v.pop_back();
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(10U, v.free());

    EXPECT_THROW(v.pop_back(), ::estd::assert_exception);
}

/**
 * \desc
 * Verifies behaviour of emplace_back():
 * - Elements are inserted at the end
 * - Asserts when called on a full vec
 */
TEST(vec_test, emplace_back)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::vec<int32_t, 10> v;

    for (int32_t i = 0; i < static_cast<int32_t>(::estd::vec<int32_t, 10>::max_size); ++i)
    {
        v.emplace_back(i);
        EXPECT_EQ(i, v.back());
        EXPECT_EQ(0, v.front());
    }
    EXPECT_EQ(10U, v.size());
    EXPECT_EQ(0U, v.free());
    EXPECT_TRUE(v.full());

    EXPECT_THROW({ v.emplace_back(10); }, ::estd::assert_exception);
}

/**
 * \desc
 * Verifies behaviour of emplace_back():
 * - Elements are inserted at the end
 * - Asserts when called on a full vec
 */
TEST(vec_test, emplace_back_assign)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    ::estd::vec<int32_t, 10> v;
    for (int32_t i = 0; i < 10; ++i)
    {
        v.emplace_back() = i;
        EXPECT_EQ(i, v.back());
        EXPECT_EQ(0, v.front());
    }
    EXPECT_EQ(10U, v.size());
    EXPECT_EQ(0U, v.free());
    EXPECT_TRUE(v.full());

    EXPECT_THROW(v.emplace_back() = 10, ::estd::assert_exception);
}

/**
 * \desc
 * Verifies that emplace_back works for POD data types.
 */
TEST(vec_test, emplace_back_pod)
{
    struct PodData
    {
        uint8_t x;
        uint64_t y;
    };

    ::estd::vec<PodData, 7> v;

    v.emplace_back() = {3U, 8U};
    EXPECT_EQ(3U, v[0].x);
    EXPECT_EQ(8U, v[0].y);
}

/**
 * \desc
 * Verifies that push_back works for a type T, which is not copyable.
 */
TEST(vec_test, emplace_back_uncopyable)
{
    static_assert(!std::is_copy_constructible<Bar>::value, "");
    static_assert(!std::is_copy_assignable<Bar>::value, "");

    ::estd::vec<Bar, 15> v;

    Bar& b(v.emplace_back(10));
    EXPECT_EQ(10, b._v1);
    EXPECT_EQ(10, b._v2);
    EXPECT_EQ(1U, v.size());
    EXPECT_EQ(14U, v.free());

    Bar* pB = &v.emplace_back(10, 17);
    EXPECT_EQ(10, pB->_v1);
    EXPECT_EQ(17, pB->_v2);
    EXPECT_EQ(2U, v.size());
    EXPECT_EQ(13U, v.free());
}

/**
 * \desc
 * Verifies that begin() returns an iterator to the first element of a vec.
 */
TEST(vec_test, begin)
{
    ::estd::vec<int32_t, 10> v;
    v.push_back(17);
    EXPECT_EQ(17, *v.begin());

    // change value via the iterator
    auto itr = v.begin();
    *itr     = 42;
    EXPECT_EQ(42, v[0]);
}

/**
 * \desc
 * Verifies that rbegin() returns an iterator to the last element of a vec.
 */
TEST(vec_test, rbegin)
{
    ::estd::vec<size_t, 10> v;
    for (size_t i = 0; i < 10; ++i)
    {
        v.push_back(i);
    }
    EXPECT_EQ(9, *v.rbegin());
    EXPECT_EQ(&v[9], &(*(v.rbegin())));

    // change value via the iterator
    auto itr = v.rbegin();
    *itr     = 42;
    EXPECT_EQ(42, v[9]);
}

/**
 * \desc
 * Testcases for various iterator scenarios.
 */
TEST(vec_test, iterators)
{
    ::estd::vec<int32_t, 10> v(10, 42);

    // std::count returns number of elements in range [begin(), end()[ matching a value.
    EXPECT_EQ(10, std::count(v.begin(), v.end(), 42));

    for (size_t i = 0; i < v.size(); ++i)
    {
        v[i] = static_cast<int32_t>(i);
    }

    int32_t i = 0;
    // iterator over content of vec using iterator
    for (::estd::vec<int32_t, 10>::iterator itr = v.begin(); itr != v.end(); ++itr, ++i)
    {
        EXPECT_EQ(i, *itr);
    }
    i = 0;
    // iterator over content of vec using const_iterator
    for (::estd::vec<int32_t, 10>::const_iterator itr = v.begin(); itr != v.end(); ++itr, ++i)
    {
        EXPECT_EQ(i, *itr);
    }
    i = 0;
    // iterator over content of vec using const_iterator
    for (::estd::vec<int32_t, 10>::const_iterator itr = v.cbegin(); itr != v.cend(); ++itr, ++i)
    {
        EXPECT_EQ(i, *itr);
    }
    i = 9;
    // iterator over content of vec using reverse_iterator
    for (::estd::vec<int32_t, 10>::reverse_iterator itr = v.rbegin(); itr != v.rend(); ++itr, --i)
    {
        EXPECT_EQ(i, *itr);
    }
    i = 9;
    // iterator over content of vec using const_reverse_iterator
    for (::estd::vec<int32_t, 10>::const_reverse_iterator itr = v.rbegin(); itr != v.rend();
         ++itr, --i)
    {
        EXPECT_EQ(i, *itr);
    }
    i = 9;
    // iterator over content of vec using const_reverse_iterator
    for (::estd::vec<int32_t, 10>::const_reverse_iterator itr = v.crbegin(); itr != v.crend();
         ++itr, --i)
    {
        EXPECT_EQ(i, *itr);
    }
}

/**
 * \desc
 * Verifies that the random access operator returns access to the element at a given position.
 */
TEST(vec_test, random_access_operator)
{
    constexpr size_t MaxSize = 10;
    ::estd::vec<int32_t, MaxSize> v;
    ::estd::vec<int32_t, MaxSize> const& cv = v;

    // fill vec with ascending values and access them
    for (size_t i = 0; i < MaxSize; ++i)
    {
        v.push_back(static_cast<int32_t>(i));
        EXPECT_EQ(int32_t(i), v[i]) << "with i: " << i;
        EXPECT_EQ(int32_t(i), cv[i]) << "with i: " << i;
    }

    // reverse the order of the elements in the vec using std::swap
    auto const size = v.size();
    for (size_t i = 0; i < size / 2; ++i)
    {
        std::swap(v[size - 1 - i], v[i]);
    }
    for (size_t i = 0; i < MaxSize; ++i)
    {
        EXPECT_EQ(int32_t(MaxSize - 1 - i), v[i]) << "with i: " << i;
        EXPECT_EQ(int32_t(MaxSize - 1 - i), cv[i]) << "with i: " << i;
    }
}

/**
 * \desc
 * Verifies that at() returns access to an element at a given index and asserts when accessing
 * an element at an index out of bounds.
 */
TEST(vec_test, at)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    constexpr size_t MaxSize = 10;
    ::estd::vec<int32_t, MaxSize> v;
    auto const& cv = v;

    // fill vec with ascending values and access them
    for (size_t i = 0; i < MaxSize; ++i)
    {
        v.push_back(static_cast<int32_t>(i));
        EXPECT_EQ(int32_t(i), v.at(i)) << "with i: " << i;
        EXPECT_EQ(int32_t(i), cv.at(i)) << "with i: " << i;
    }

    // reverse the order of the elements in the vec using std::swap
    auto const size = v.size();
    for (size_t i = 0; i < size / 2; ++i)
    {
        std::swap(v.at(size - 1 - i), v.at(i));
    }
    for (size_t i = 0; i < MaxSize; ++i)
    {
        EXPECT_EQ(int32_t(MaxSize - 1 - i), v.at(i)) << "with i: " << i;
        EXPECT_EQ(int32_t(MaxSize - 1 - i), cv.at(i)) << "with i: " << i;
    }
    EXPECT_THROW(v.at(MaxSize), ::estd::assert_exception);
    EXPECT_THROW(cv.at(MaxSize);, ::estd::assert_exception);
}

/**
 * \desc
 * Tests that calling front()
 * - asserts when called on an empty vec
 * - returns access to the first element of a vec
 */
TEST(vec_test, front)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::vec<int32_t, 10> v;
    auto const& cv = v;

    EXPECT_THROW(v.front(), ::estd::assert_exception);
    EXPECT_THROW(cv.front(), ::estd::assert_exception);

    v.push_back(42);
    v.push_back(43);

    EXPECT_EQ(42, cv.front());
    v.front() = 99;
    EXPECT_EQ(99, v.front());
}

/**
 * \desc
 * Tests that calling back()
 * - asserts when called on an empty vec
 * - returns access to the last element of a vec
 */
TEST(vec_test, back)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::vec<int64_t, 10> v;
    auto const& cv = v;

    EXPECT_THROW(v.back(), ::estd::assert_exception);
    EXPECT_THROW(cv.back(), ::estd::assert_exception);

    v.push_back(42);
    v.push_back(43);

    EXPECT_EQ(43, cv.back());
    v.back() = 99;
    EXPECT_EQ(99, v.back());
}

/**
 * \desc
 * Verifies that data() returns a pointer to the internal data and that this pointer can be used
 * to iterate over the data.
 */
TEST(vec_test, data)
{
    ::estd::vec<int32_t, 20> v;
    for (int32_t i = 0; i < 10; ++i)
    {
        v.emplace_back(i);
    }
    // Access through pointer
    std::vector<int32_t>::pointer p = v.data();
    for (int32_t i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i, *p++);
    }
    // Access through const_pointer
    std::vector<int32_t>::const_pointer c = v.data();
    for (int32_t i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i, *c++);
    }
}

/**
 * \desc
 * Verifies that data() returns a read pointer to the internal data when called on a const reference
 * to a vec and that this pointer can be used to iterate over the data.
 */
TEST(vec_test, const_data)
{
    ::estd::vec<int32_t, 20> v;

    for (int32_t i = 0; i < 10; ++i)
    {
        v.push_back(i);
    }

    // Create const reference to vec
    ::estd::vec<int32_t, 20> const& v_ref(v);

    int32_t const* p = v_ref.data();
    for (int32_t i = 0; i < 10; ++i)
    {
        EXPECT_EQ(i, *p++);
    }
}

/**
 * \desc
 * Verifies that clear removes all items from a vec which is empty after the call.
 */
TEST(vec_test, clear)
{
    ::estd::vec<int32_t, 10> v;
    EXPECT_TRUE(v.empty());
    // Clear on an empty vec doesn't change it
    v.clear();
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(10U, v.free());
    v.assign(4, 99);
    ASSERT_TRUE(!v.empty());
    EXPECT_EQ(6U, v.free());
    // Clear on a non-empty vec removes all content
    v.clear();
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(10U, v.free());
}

/**
 * \desc
 * Example for a vec of vecs. Verifies that nested vecs are possible.
 */
TEST(vec_test, vec_of_vecs)
{
    ::estd::vec<::estd::vec<int32_t, 10>, 10> vecOfVecs;

    // Extra scope to avoid shadowing of the name item
    {
        auto& item = vecOfVecs.emplace_back();
        EXPECT_EQ(0U, item.size());
        EXPECT_EQ(1U, vecOfVecs.size());
        EXPECT_EQ(9U, vecOfVecs.free());

        item.push_back(10);
        EXPECT_EQ(1U, item.size());
        EXPECT_EQ(9U, item.free());
    }

    EXPECT_EQ(1, vecOfVecs.size());
    for (auto& item : vecOfVecs)
    {
        EXPECT_EQ(1U, item.size());
        EXPECT_EQ(9U, item.free());
    }
}

/**
 * \desc
 * Verifies that a copy constructed vec contains the same data as the source vec.
 */
TEST(vec_test, copy_constructor)
{
    ::estd::vec<int32_t, 10> a(10);
    ASSERT_EQ(10, a.size());
    for (size_t i = 0; i < a.size(); ++i)
    {
        a[i] = static_cast<int32_t>(i);
    }

    // Create copy of vec a
    ::estd::vec<int32_t, 10> b(a);
    EXPECT_EQ(10, b.size());
    for (size_t i = 0; i < b.size(); ++i)
    {
        EXPECT_EQ((int32_t)i, b[i]);
    }
}

/**
 * \desc
 * Verifies that a copy created by the assignment operator contains the same data as the source vec.
 */
TEST(vec_test, assignment)
{
    ::estd::vec<int32_t, 10> a;
    for (size_t i = 0; i < 10; ++i)
    {
        a.emplace_back(i);
    }

    ::estd::vec<int32_t, 10> b;
    // Create a copy of vec by assignment
    b = a;

    EXPECT_THAT(b, ElementsAreArray(a));
}

/**
 * Test fixture for lifecycle tests for vec. These test make use of the helper class
 * TestClassCalls which counts all the calls to constructor, destructor, assignment and so on.
 */
struct vec_test_lifecycle : ::testing::Test
{
    using VecType = ::estd::vec<TestClassCalls, 5>;

    vec_test_lifecycle() { TestClassCalls::reset(); }
};

/**
 * \desc
 * This test verifies that no constructor of any data of a vec is called, when the vec is default
 * constructed.
 */
TEST_F(vec_test_lifecycle, default_constructor)
{
    ::estd::vec<TestClassCalls, 10> a;
    TestClassCalls::verify(0, 0, 0, 0, 0, 0);
}

/**
 * \desc
 * Verifies that, when calling the size constructor of vec, n elements are default constructed.
 */
TEST_F(vec_test_lifecycle, constructor_size)
{
    TestClassCalls::reset();
    ::estd::vec<TestClassCalls, 10> v(2);
    EXPECT_TRUE(TestClassCalls::verify(2, 0, 0, 0, 0, 0));
}

/**
 * \desc
 * Tests that emplace_back calls the constructor of the element directly.
 */
TEST_F(vec_test_lifecycle, emplace_back)
{
    {
        ::estd::vec<TestClassCalls, 10> v;
        v.emplace_back(88, 99, 90);
        TestClassCalls::verify(1, 0, 0, 0, 0, 0);

        v.emplace_back(9);
        TestClassCalls::verify(2, 0, 0, 0, 0, 0);
    }
    // When v goes out of scope, the elements are destroyed
    TestClassCalls::verify(2, 0, 0, 2, 0, 0);
}

/**
 * \desc
 * Tests that
 * - push_back() constructs an element at the end of the vec.
 * - push_back(T const&) copy constructs an element at the end of the vec.
 */
TEST_F(vec_test_lifecycle, push_back)
{
    {
        ::estd::vec<TestClassCalls, 10> v;
        // Create one element at the end of v (will be default constructed)
        (void)v.push_back();
        ASSERT_TRUE(TestClassCalls::verify(1, 0, 0, 0, 0, 0));
        // Create one element as a copy of a temporary
        v.push_back(TestClassCalls(88, 99, 90));
        ASSERT_TRUE(TestClassCalls::verify(2, 1, 0, 1, 0, 0));
    }
    // When v goes out of scope, the elements are destroyed
    TestClassCalls::verify(2, 1, 0, 3, 0, 0);
}

/**
 * \desc
 * Verifies that calling clear() destroys all elements of a vec.
 */
TEST_F(vec_test_lifecycle, clear)
{
    ::estd::vec<TestClassCalls, 10> v;
    v.emplace_back(88, 99, 90);
    v.emplace_back(44, 22);
    EXPECT_TRUE(TestClassCalls::verify(2, 0, 0, 0, 0, 0));
    v.clear();
    EXPECT_TRUE(TestClassCalls::verify(2, 0, 0, 2, 0, 0));
}

/**
 * \desc
 * Verifies that when assign(n, v) is called on a vec that already contains some elements, they will
 * be destroyed.
 */
TEST_F(vec_test_lifecycle, assign)
{
    TestClassCalls const value{10};
    EXPECT_TRUE(TestClassCalls::verify(1, 0, 0, 0, 0, 0));
    TestClassCalls::reset();

    // Create a vec and insert two values.
    ::estd::vec<TestClassCalls, 10> v;
    v.emplace_back(value);
    v.emplace_back(value);
    EXPECT_TRUE(TestClassCalls::verify(0, 2, 0, 0, 0, 0));
    TestClassCalls::reset();

    // Now assign 5 values to v --> existing elements are destroyed.
    v.assign(5, value);
    EXPECT_TRUE(TestClassCalls::verify(0, 5, 0, 2, 0, 0));

    EXPECT_EQ(5, v.size());
    EXPECT_EQ(value, v[0]);
    EXPECT_EQ(value, v[1]);
    EXPECT_EQ(value, v[2]);
    EXPECT_EQ(value, v[3]);
    EXPECT_EQ(value, v[4]);
}

/**
 * \desc
 * Verifies that when assign(begin, end) is called on a vec that already contains some elements,
 * they will be destroyed.
 */
TEST_F(vec_test_lifecycle, assign_iterator_range)
{
    TestClassCalls const value{10};
    EXPECT_TRUE(TestClassCalls::verify(1, 0, 0, 0, 0, 0));
    TestClassCalls::reset();

    // Create a vec and insert two values.
    ::estd::vec<TestClassCalls, 10> target;
    target.emplace_back(value);
    target.emplace_back(value);
    EXPECT_TRUE(TestClassCalls::verify(0, 2, 0, 0, 0, 0));
    TestClassCalls::reset();

    // Create a second vec and insert five values.
    ::estd::vec<TestClassCalls, 10> source;
    source.emplace_back(value);
    source.emplace_back(value);
    source.emplace_back(value);
    source.emplace_back(value);
    source.emplace_back(value);
    EXPECT_TRUE(TestClassCalls::verify(0, 5, 0, 0, 0, 0));
    TestClassCalls::reset();

    // Now assign 5 values to target via iterators --> existing elements are destroyed.
    target.assign(source.begin(), source.end());
    EXPECT_TRUE(TestClassCalls::verify(0, 5, 0, 2, 0, 0));

    EXPECT_EQ(5, target.size());
    EXPECT_EQ(value, target[0]);
    EXPECT_EQ(value, target[1]);
    EXPECT_EQ(value, target[2]);
    EXPECT_EQ(value, target[3]);
    EXPECT_EQ(value, target[4]);
}

/**
 * \desc
 * Verifies that
 * - resizing an empty vec to 0 has no effect.
 * - resizing an empty vec to n constructs n elements.
 * - resizing a non-empty vec of size n destroys n elements.
 * - resizing a vec to n > N creates at most N elements and doesn't assert.
 */
TEST_F(vec_test_lifecycle, resize)
{
    ::estd::vec<TestClassCalls, 10> vec;
    EXPECT_EQ(0U, vec.size());
    EXPECT_EQ(10U, vec.free());

    // no resize
    vec.resize(0U);
    EXPECT_TRUE(TestClassCalls::verify(0, 0, 0, 0, 0, 0));
    EXPECT_EQ(0U, vec.size());
    EXPECT_EQ(10U, vec.free());

    // resize bigger
    TestClassCalls::reset();
    vec.resize(10);
    EXPECT_EQ(10U, vec.size());
    EXPECT_EQ(0U, vec.free());
    EXPECT_TRUE(TestClassCalls::verify(10, 0, 0, 0, 0, 0));

    // resize smaller
    TestClassCalls::reset();
    vec.resize(0);
    EXPECT_EQ(0U, vec.size());
    EXPECT_EQ(10U, vec.free());
    EXPECT_TRUE(TestClassCalls::verify(0, 0, 0, 10, 0, 0));

    // resize to larger than max size of vec
    TestClassCalls::reset();
    vec.resize(11);
    EXPECT_EQ(10U, vec.size());
    EXPECT_EQ(0U, vec.free());
    EXPECT_TRUE(TestClassCalls::verify(10, 0, 0, 0, 0, 0));
}

/**
 * \desc
 * Verifies that when a vec is destroyed it'll destroy all its elements.
 */
TEST_F(vec_test_lifecycle, destructor)
{
    {
        ::estd::vec<TestClassCalls, 10> v(4);
        EXPECT_TRUE(TestClassCalls::verify(4, 0, 0, 0, 0, 0));
    }
    EXPECT_TRUE(TestClassCalls::verify(4, 0, 0, 4, 0, 0));
}

// clang warns about self assignment, which is good and we want to keep, but we also want to keep
// (and test!) safe handling of self assignment, in the case that someone does it anyway.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
/**
 * \desc
 * Verifies that assigning a vec to another vec calls the assignment operator of all elements.
 */
TEST_F(vec_test_lifecycle, assignment)
{
    ::estd::vec<TestClassCalls, 10> source(5);
    EXPECT_TRUE(TestClassCalls::verify(5, 0, 0, 0, 0, 0));
    ::estd::vec<TestClassCalls, 10> target;
    EXPECT_TRUE(TestClassCalls::verify(5, 0, 0, 0, 0, 0));
    // assign to an empty vec will not call any destructors but just make copies
    target = source;
    EXPECT_TRUE(TestClassCalls::verify(5, 5, 0, 0, 0, 0));

    TestClassCalls::reset();
    // assign to a non-empty vec will first clear it and then assign
    target = source;
    EXPECT_TRUE(TestClassCalls::verify(0, 5, 0, 5, 0, 0));

    TestClassCalls::reset();
    ASSERT_EQ(5, target.size());
    // self assignment must not alter the content of a vec
    target = target;
    EXPECT_TRUE(TestClassCalls::verify(0, 0, 0, 0, 0, 0));
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

/**
 * \desc
 * Tests that the copy constructor calls the copy constructor for all elements.
 */
TEST_F(vec_test_lifecycle, copy_constructor)
{
    ::estd::vec<TestClassCalls, 10> source(5);
    EXPECT_EQ(5, source.size());
    EXPECT_TRUE(TestClassCalls::verify(5, 0, 0, 0, 0, 0));
    ::estd::vec<TestClassCalls, 10> target(source);
    EXPECT_EQ(5, target.size());
    EXPECT_TRUE(TestClassCalls::verify(5, 5, 0, 0, 0, 0));
}

/**
 * \desc
 * Verifies that the memory management of a vec preserves the alignment requirements of the
 * type T it holds. This is important if sizeof(T) > sizeof(vec::size_type) because the first
 * member of a vec is the size followed by the memory to store N instances of T. The naive
 * approach of just uint8_t _data[sizeof(T) * N] would violate T's alignment requirements if
 * sizeof(T) > sizeof(vec::size_type).
 */
TEST(vec_test, alignment_preservation)
{
    ::estd::vec<Aligned256Type, 5> v{};

    EXPECT_EQ(0U, reinterpret_cast<uintptr_t>(&v[0]) % 256);
    EXPECT_EQ(0U, reinterpret_cast<uintptr_t>(&v[1]) % 256);
}

/**
 * \desc
 * This test makes sure, that a vec is convertible to a slice.
 */
TEST(vec_test, conversion_to_slice)
{
    estd::vec<uint8_t, 5> source(5, 0xFF);
    estd::slice<uint8_t> target(source);

    EXPECT_THAT(target, ElementsAreArray(source));
}

/**
 * \desc
 * Verifies that a vec can be constructed from a slice.
 */
TEST(vec_test, constructor_from_slice)
{
    // Make sure that not another one argument constructor is called.
    {
        uint8_t const initValues[] = {17};
        estd::vec<uint8_t, 5> data{initValues};

        EXPECT_EQ(1, data.size());
        EXPECT_EQ(17, data[0]);
    }
    // Make sure that not another two argument constructor is called.
    {
        uint8_t const initValues[] = {17, 19};
        estd::vec<uint8_t, 5> data{initValues};

        EXPECT_EQ(2, data.size());
        EXPECT_EQ(17, data[0]);
        EXPECT_EQ(19, data[1]);
    }
    // Construct a vec with three elements.
    {
        uint8_t const initValues[] = {17, 19, 23};
        estd::vec<uint8_t, 5> data{initValues};

        EXPECT_EQ(3, data.size());
        EXPECT_EQ(17, data[0]);
        EXPECT_EQ(19, data[1]);
        EXPECT_EQ(23, data[2]);
    }
    // Construct a vec with N elements
    {
        // Test that also non const init data works
        uint8_t initValues[] = {17, 19, 23, 29, 31};
        estd::vec<uint8_t, 5> data{initValues};

        EXPECT_EQ(5, data.size());
        EXPECT_EQ(17, data[0]);
        EXPECT_EQ(19, data[1]);
        EXPECT_EQ(23, data[2]);
        EXPECT_EQ(29, data[3]);
        EXPECT_EQ(31, data[4]);
    }
}

/**
 * \desc
 * Verifies that if a vec is constructed from a slice with more than N entries,
 * no assert is done and only N elements are copied to the vec.
 */
TEST(vec_test, constructor_from_slice_more_than_capacity)
{
    // Test that also non const init data works
    uint8_t initValues[] = {17, 19, 23, 29, 31, 37};
    estd::vec<uint8_t, 5> data{initValues};
    ASSERT_GT(sizeof(initValues), data.max_size);
    EXPECT_EQ(5, data.size());
    EXPECT_EQ(17, data[0]);
    EXPECT_EQ(19, data[1]);
    EXPECT_EQ(23, data[2]);
    EXPECT_EQ(29, data[3]);
    EXPECT_EQ(31, data[4]);
}

/**
 * \desc
 * Verifies that a vec constructed from an empty slice is empty.
 */
TEST(vec_test, constructor_from_empty_slice)
{
    estd::vec<uint8_t, 5> data{estd::slice<uint8_t const>()};
    EXPECT_EQ(0, data.size());
    EXPECT_TRUE(data.empty());
}

} // namespace
