// Copyright 2024 Accenture.

#include "estd/ordered_map.h"

#include "internal/TestClassCalls.h"

#include <gtest/gtest.h>

#include <functional>
#include <type_traits>

using namespace ::testing;

template class ::estd::declare::ordered_map<int32_t, int32_t, 10>;
template class ::estd::ordered_map<int32_t, int32_t>;

namespace
{
struct Value
{
    Value() : _v() {}

    Value(int32_t v) : _v(v) {}

    int32_t _v;
};

#define MAKE_MAP(K, T, N, Name)                    \
    ::estd::declare::ordered_map<K, T, N> Name##_; \
    ::estd::ordered_map<K, T>& Name = Name##_

template<size_t total>
void validateMapSize(size_t overhead32, size_t overhead64)
{
    ::estd::declare::ordered_map<int32_t, int32_t, total> container;

    size_t itemSize = sizeof(int32_t) + sizeof(int32_t);

    if (sizeof(size_t) == 4)
    {
        ASSERT_EQ(8U, sizeof(::estd::ordered_map<int32_t, int32_t>));
        ASSERT_EQ(total * itemSize + overhead32, sizeof(container)) << total << ":" << overhead32;
    }
    else
    {
        ASSERT_EQ(16U, sizeof(::estd::ordered_map<int32_t, int32_t>));
        ASSERT_EQ(total * itemSize + overhead64, sizeof(container)) << total << ":" << overhead64;
    }
}
} // namespace

using IntValueMap = ::estd::ordered_map<int32_t, Value>;

static_assert(std::is_same<int32_t, IntValueMap::key_type>::value, "");
static_assert(std::is_same<Value, IntValueMap::mapped_type>::value, "");
static_assert(std::is_same<std::pair<int32_t const, Value>, IntValueMap::value_type>::value, "");
static_assert(std::is_same<std::pair<int32_t const, Value>&, IntValueMap::reference>::value, "");
static_assert(
    std::is_same<std::pair<int32_t const, Value> const&, IntValueMap::const_reference>::value, "");
static_assert(std::is_same<std::pair<int32_t const, Value>*, IntValueMap::pointer>::value, "");
static_assert(
    std::is_same<std::pair<int32_t const, Value> const*, IntValueMap::const_pointer>::value, "");
static_assert(std::is_same<std::less<int32_t>, IntValueMap::key_compare>::value, "");
static_assert(std::is_same<bool, IntValueMap::value_compare::result_type>::value, "");
static_assert(
    std::is_same<std::pair<int32_t const, Value>, IntValueMap::value_compare::first_argument_type>::
        value,
    "");
static_assert(
    std::is_same<
        std::pair<int32_t const, Value>,
        IntValueMap::value_compare::second_argument_type>::value,
    "");
static_assert(std::is_same<std::size_t, IntValueMap::size_type>::value, "");
static_assert(std::is_same<std::ptrdiff_t, IntValueMap::difference_type>::value, "");

TEST(OrderedMap, TestSizeOf)
{
    validateMapSize<5>(16, 32);
    validateMapSize<10>(16, 32);
    validateMapSize<1>(16, 32);
    validateMapSize<20>(16, 32);
    validateMapSize<40>(16, 32);
}

TEST(OrderedMap, TestConstructor)
{
    MAKE_MAP(int32_t, Value, 10, m);
    ASSERT_EQ(0U, m.size());
    ASSERT_EQ(10U, m.max_size());
    ASSERT_TRUE(m.empty());
}

TEST(OrderedMap, TestEmpty)
{
    MAKE_MAP(int32_t, Value, 10, m);
    ASSERT_TRUE(m.empty());
}

TEST(OrderedMap, TestFull)
{
    MAKE_MAP(int32_t, Value, 10, m);
    for (size_t k = 0; k < m.max_size(); ++k)
    {
        m[k]._v = k;
    }
    ASSERT_TRUE(m.full());
}

TEST(OrderedMap, TestMaxSize)
{
    MAKE_MAP(int32_t, Value, 10, m);
    ASSERT_EQ(10U, m.max_size());
}

TEST(OrderedMap, TestInsert)
{
    MAKE_MAP(int32_t, Value, 10, m);
    std::pair<IntValueMap::iterator, bool> r = m.insert(std::make_pair(1, 100));
    ASSERT_TRUE(r.second);
    ASSERT_EQ(100, r.first->second._v);
    ASSERT_EQ(100, m[1]._v);
    ASSERT_EQ(1U, m.size());
    // insert different value with same key
    r = m.insert(std::make_pair(1, 101));
    ASSERT_FALSE(r.second);
    ASSERT_EQ(100, r.first->second._v);
    ASSERT_EQ(100, m[1]._v);
    ASSERT_EQ(1U, m.size());
    // insert same value with different key
    r = m.insert(std::make_pair(2, 100));
    ASSERT_TRUE(r.second);
    ASSERT_EQ(100, r.first->second._v);
    ASSERT_EQ(100, m[2]._v);
    ASSERT_EQ(2U, m.size());

    // insert values until map is full
    int32_t key = 3;
    while (!m.full())
    {
        int32_t value = 100 + key;
        r             = m.insert(std::make_pair(key, value));
        ASSERT_TRUE(r.second) << "with [" << key << ", " << value << "]";
        ASSERT_EQ(value, r.first->second._v);
        ASSERT_EQ(value, m[key]._v);
        ASSERT_EQ((size_t)key, m.size());
        ++key;
    }
}

TEST(OrderedMap, TestEmplace)
{
    using ::internal::TestClassCalls;
    using MapType = ::estd::ordered_map<int32_t, TestClassCalls>;
    ::estd::declare::ordered_map<int32_t, TestClassCalls, 10> m;

    TestClassCalls::reset();

    std::pair<MapType::iterator, bool> r = m.emplace(1);
    ASSERT_TRUE(r.second);

    ASSERT_TRUE(TestClassCalls::verify(1, 0, 0, 0, 0, 0));
    TestClassCalls::reset();

    ASSERT_EQ(1U, m.size());

    // emplace different value with same key
    r = m.emplace(1);
    ASSERT_FALSE(r.second);
    ASSERT_EQ(1U, m.size());
    ASSERT_TRUE(TestClassCalls::verify(0, 0, 0, 0, 0, 0));
    TestClassCalls::reset();

    // emplace different key
    r = m.emplace(2);
    ASSERT_TRUE(TestClassCalls::verify(1, 0, 0, 0, 0, 0));
    TestClassCalls::reset();
    ASSERT_EQ(2U, m.size());

    // emplace different key with same index
    r = m.emplace(10);
    ASSERT_TRUE(TestClassCalls::verify(1, 0, 0, 0, 0, 0));
    TestClassCalls::reset();
    ASSERT_EQ(3U, m.size());

    r = m.emplace(7); // causes one move
    ASSERT_TRUE(TestClassCalls::verify(1, 0, 1, 1, 0, 0));
    TestClassCalls::reset();
    ASSERT_EQ(4U, m.size());

    r = m.emplace(6); // causes two moves
    ASSERT_TRUE(TestClassCalls::verify(1, 0, 2, 2, 0, 0));
    TestClassCalls::reset();
    ASSERT_EQ(5U, m.size());
}

TEST(OrderedMap, TestInsertSameIndexDifferentKey)
{
    MAKE_MAP(int32_t, Value, 10, m);
    m[1]  = 100;
    m[10] = 110;
    m[7]  = 107;

    std::pair<IntValueMap::iterator, bool> r = m.insert(std::make_pair(2, 105));
    ASSERT_TRUE(r.second);
}

TEST(OrderedMap, TestRandomAccess)
{
    MAKE_MAP(int32_t, Value, 10, m);
    m[2] = 200;
    ASSERT_EQ(1U, m.size());
    ASSERT_EQ(200, m[2]._v);
    m[2] = 201;
    ASSERT_EQ(1U, m.size());
    ASSERT_EQ(201, m[2]._v);
    m[1] = 100;
    ASSERT_EQ(2U, m.size());
    ASSERT_EQ(100, m[1]._v);
}

TEST(OrderedMap, RandomAccessEfficiency)
{
    using ::internal::TestClassCalls;
    ::estd::declare::ordered_map<int32_t, TestClassCalls, 10> m;
    TestClassCalls::reset();
    m[0];
    TestClassCalls::verify(1, 0, 0, 0, 0, 0);
}

TEST(OrderedMap, TestAt)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_MAP(int32_t, Value, 10, m);
    m[1] = 100;
    ASSERT_EQ(100, m.at(1)._v);
    ASSERT_THROW(m.at(2), ::estd::assert_exception);
    m.at(1) = 1000;
    ASSERT_EQ(1000, m[1]._v);
}

TEST(OrderedMap, TestAtBounds)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_MAP(int32_t, Value, 10, m);
    IntValueMap const& cm = m;

    m[1] = 100;
    ASSERT_EQ(100, m.at(1)._v);
    ASSERT_EQ(100, cm.at(1)._v);
    ASSERT_THROW({ m.at(2); }, ::estd::assert_exception);
    ASSERT_THROW({ cm.at(2); }, ::estd::assert_exception);

    m[10] = 110;
    ASSERT_EQ(110, m.at(10)._v);
    ASSERT_EQ(110, cm.at(10)._v);
    ASSERT_THROW({ m.at(2); }, ::estd::assert_exception);
    ASSERT_THROW({ cm.at(2); }, ::estd::assert_exception);
}

TEST(OrderedMap, TestFind)
{
    MAKE_MAP(int32_t, Value, 10, m);
    IntValueMap const& cm = m;
    ASSERT_EQ(m.end(), m.find(1));
    ASSERT_EQ(m.cend(), cm.find(1));
    m[1] = 100;
    ASSERT_EQ(m.begin(), m.find(1));
    ASSERT_EQ(m.cbegin(), cm.find(1));
    ASSERT_EQ(1, m.find(1)->first);
    ASSERT_EQ(1, cm.find(1)->first);
    ASSERT_EQ(1, (*m.find(1)).first);
    ASSERT_EQ(1, (*cm.find(1)).first);
    ASSERT_EQ(100, m.find(1)->second._v);
    ASSERT_EQ(100, cm.find(1)->second._v);
    ASSERT_EQ(100, (*m.find(1)).second._v);
    ASSERT_EQ(100, (*cm.find(1)).second._v);
    m[10] = 110;
    m[7]  = 107;
    ASSERT_EQ(1, m.find(1)->first);
    ASSERT_EQ(100, m.find(1)->second._v);
    ASSERT_EQ(7, m.find(7)->first);
    ASSERT_EQ(107, m.find(7)->second._v);
    ASSERT_EQ(10, m.find(10)->first);
    ASSERT_EQ(110, m.find(10)->second._v);
    ASSERT_EQ(m.end(), m.find(2));
    ASSERT_EQ(cm.cend(), cm.find(2));
}

TEST(OrderedMap, CountReturnsZeroOnEmptyMap)
{
    MAKE_MAP(int32_t, Value, 10, m);
    ASSERT_EQ(0U, m.count(0));
}

TEST(OrderedMap, TestCount)
{
    MAKE_MAP(int32_t, Value, 10, m);
    ASSERT_EQ(0U, m.count(1));
    m[1] = 100;
    ASSERT_EQ(1U, m.count(1));
    m[1] = 101;
    ASSERT_EQ(1U, m.count(1));

    m.clear();
    for (int32_t i = 0; i < 8; ++i)
    {
        ASSERT_EQ(0U, m.count(i));
        m[i] = i;
        ASSERT_EQ(1U, m.count(i));
    }
}

TEST(OrderedMap, TestCountFullDoesntExist)
{
    MAKE_MAP(int32_t, Value, 10, m);
    for (int32_t i = 0; i < 10; ++i)
    {
        m[i] = i;
    }
    ASSERT_EQ(0U, m.count(100));
    ASSERT_EQ(1U, m.count(1));
}

TEST(OrderedMap, EraseConstIteratorPosition)
{
    using ::internal::TestClassCalls;
    TestClassCalls::reset();
    using Map = ::estd::declare::ordered_map<int32_t, TestClassCalls, 10>;
    Map m;
    m[0]  = TestClassCalls(10);
    m[1]  = TestClassCalls(11);
    m[11] = TestClassCalls(21);
    TestClassCalls::verify(6, 0, 0, 3, 0, 3);
    ASSERT_EQ(3U, m.size());
    Map::const_iterator i = m.find(0);
    ASSERT_NE(m.cend(), i);
    Map::iterator j = m.erase(i);
    ASSERT_EQ(2U, m.size());
    ASSERT_EQ(1, j->first);
    ASSERT_EQ(11, j->second.tag());
    TestClassCalls::verify(6, 0, 2, 6, 0, 3);
    j = m.erase(j);
    ASSERT_EQ(1U, m.size());
    ASSERT_EQ(11, j->first);
    ASSERT_EQ(21, j->second.tag());
    TestClassCalls::verify(6, 0, 3, 8, 0, 3);
    j = m.erase(j);
    ASSERT_EQ(0U, m.size());
    ASSERT_EQ(m.end(), j);
    TestClassCalls::verify(6, 0, 3, 9, 0, 3);
}

TEST(OrderedMap, EraseCEndIterator)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10> m;
    ASSERT_EQ(m.end(), m.erase(m.cend()));
    m[0] = 10;
    ASSERT_EQ(m.end(), m.erase(m.cend()));
}

TEST(OrderedMap, EraseKey)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10> m;
    int32_t key = 0;
    ASSERT_EQ(0U, m.erase(key));
    m[key] = 10;
    ASSERT_EQ(1U, m.size());
    ASSERT_EQ(1U, m.erase(key));
    ASSERT_EQ(0U, m.size());
}

TEST(OrderedMap, Equality)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10> m1;
    ::estd::declare::ordered_map<int32_t, int32_t, 20> m2;
    ASSERT_EQ(m1, m2);
    ASSERT_TRUE(m1 == m2);
    m1[0] = 10;
    ASSERT_NE(m1, m2);
    ASSERT_TRUE(m1 != m2);
    m2[0] = 10;
    ASSERT_EQ(m1, m2);
    ASSERT_TRUE(m1 == m2);
    m1[20] = 21;
    m1[40] = 41;
    m2[40] = 41;
    m2[20] = 21;
    ASSERT_EQ(m1, m2);
    ASSERT_TRUE(m1 == m2);
    m1[20] = 20;
    ASSERT_NE(m1, m2);
    ASSERT_FALSE(m1 == m2);
}

TEST(OrderedMap, EqualitySameSizeButDifferent)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10> m1;
    ::estd::declare::ordered_map<int32_t, int32_t, 20> m2;

    m1[0] = 10;
    m2[1] = 11;

    ASSERT_NE(m1, m2);
}

TEST(OrderedMap, ReverseIterators)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10> m;
    for (size_t i = 0; i < m.max_size(); ++i)
    {
        m[i] = i;
    }

    int32_t count = 9;
    for (::estd::ordered_map<int32_t, int32_t>::reverse_iterator iter = m.rbegin();
         iter != m.rend();
         ++iter)
    {
        ASSERT_EQ(count--, iter->first);
    }

    count = 9;
    for (::estd::ordered_map<int32_t, int32_t>::const_reverse_iterator iter = m.crbegin();
         iter != m.crend();
         ++iter)
    {
        ASSERT_EQ(count--, iter->first);
    }

    ::estd::ordered_map<int32_t, int32_t> const& cm = m;

    count = 9;
    for (::estd::ordered_map<int32_t, int32_t>::const_reverse_iterator iter = cm.rbegin();
         iter != cm.rend();
         ++iter)
    {
        ASSERT_EQ(count--, iter->first);
    }
}

TEST(OrderedMap, EraseConstIteratorRangeCBeginToCEnd)
{
    using ::internal::TestClassCalls;
    using Map = ::estd::declare::ordered_map<int32_t, TestClassCalls, 10>;
    Map m;
    TestClassCalls::reset();
    m[0]  = TestClassCalls(10);
    m[1]  = TestClassCalls(11);
    m[11] = TestClassCalls(21);
    TestClassCalls::verify(6, 0, 0, 3, 0, 3);
    ASSERT_EQ(3U, m.size());
    Map::iterator j = m.erase(m.cbegin(), m.cend());
    ASSERT_EQ(0U, m.size());
    ASSERT_EQ(m.end(), j);
    TestClassCalls::verify(6, 0, 0, 6, 0, 3);
}

TEST(OrderedMap, EraseConstIteratorRange)
{
    using ::internal::TestClassCalls;
    using Map = ::estd::declare::ordered_map<int32_t, TestClassCalls, 10>;
    Map m;
    TestClassCalls::reset();
    m[0]  = TestClassCalls(10);
    m[1]  = TestClassCalls(11);
    m[11] = TestClassCalls(21);
    TestClassCalls::verify(6, 0, 0, 3, 0, 3);
    ASSERT_EQ(3U, m.size());
    Map::iterator i1 = m.begin();
    Map::iterator i2 = m.find(0);
    Map::iterator j  = m.erase(i1, i2);
    // deleting an empty range has no effect
    ASSERT_EQ(3U, m.size());
    TestClassCalls::verify(6, 0, 0, 3, 0, 3);
    i2 = m.find(11);
    j  = m.erase(i1, i2);
    ASSERT_EQ(11, j->first);
    ASSERT_EQ(1U, m.size());
    TestClassCalls::verify(6, 0, 1, 6, 0, 3);
    j = m.erase(j, m.end());
    ASSERT_EQ(0U, m.size());
    ASSERT_EQ(m.end(), j);
    TestClassCalls::verify(6, 0, 1, 7, 0, 3);
}

TEST(OrderedMap, LowerBoundEmptyMap)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10> m;
    ASSERT_EQ(m.end(), m.lower_bound(0));
    ASSERT_EQ(m.end(), m.lower_bound(10));
}

TEST(OrderedMap, LowerBoundFullMap)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10> m;
    for (int32_t i = 0; i < 10; ++i)
    {
        m[i] = i;
    }
    ASSERT_EQ(m.end(), m.lower_bound(50));

    ::estd::ordered_map<int32_t, int32_t> const& cm = m;
    ASSERT_EQ(cm.cend(), cm.lower_bound(50));
}

TEST(OrderedMap, UpperBoundEmptyMap)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10> m;
    ASSERT_EQ(m.end(), m.upper_bound(0));
    ASSERT_EQ(m.end(), m.upper_bound(10));
}

TEST(OrderedMap, UpperBoundFullMap)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10> m;
    for (int32_t i = 0; i < 10; ++i)
    {
        m[i] = i;
    }
    ASSERT_EQ(m.end(), m.upper_bound(50));

    ::estd::ordered_map<int32_t, int32_t> const& cm = m;
    ASSERT_EQ(cm.cend(), cm.upper_bound(50));
}

TEST(OrderedMap, EqualRangeEmptyMap)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10> m;
    std::pair<
        ::estd::ordered_map<int32_t, int32_t>::iterator,
        ::estd::ordered_map<int32_t, int32_t>::iterator>
        r = m.equal_range(0);
    ASSERT_EQ(m.end(), r.first);
    ASSERT_EQ(m.end(), r.second);
    r = m.equal_range(10);
    ASSERT_EQ(m.end(), r.first);
    ASSERT_EQ(m.end(), r.second);

    ::estd::ordered_map<int32_t, int32_t> const& base = m;
    std::pair<
        ::estd::ordered_map<int32_t, int32_t>::const_iterator,
        ::estd::ordered_map<int32_t, int32_t>::const_iterator>
        cr = base.equal_range(0);
    ASSERT_EQ(base.end(), cr.first);
    ASSERT_EQ(base.end(), cr.second);
}

TEST(OrderedMap, LowerBound)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 5> m;
    m[0] = 0;
    ASSERT_EQ(m.begin(), m.lower_bound(0));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(0, 0)), *m.lower_bound(0));
    m[1] = 1;
    ASSERT_EQ(m.begin(), m.lower_bound(0));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(0, 0)), *m.lower_bound(0));
    m[2] = 2;
    ASSERT_NE(m.begin(), m.lower_bound(1));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(1, 1)), *m.lower_bound(1));
    m[9] = 9;
    ASSERT_EQ((std::pair<int32_t const, int32_t>(1, 1)), *m.lower_bound(1));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(9, 9)), *m.lower_bound(9));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(9, 9)), *m.lower_bound(8));
    m[17] = 17;
    ASSERT_EQ((std::pair<int32_t const, int32_t>(0, 0)), *m.lower_bound(0));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(1, 1)), *m.lower_bound(1));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(2, 2)), *m.lower_bound(2));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(9, 9)), *m.lower_bound(3));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(9, 9)), *m.lower_bound(8));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(9, 9)), *m.lower_bound(9));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(17, 17)), *m.lower_bound(10));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(17, 17)), *m.lower_bound(17));
    ASSERT_EQ(m.end(), m.lower_bound(18));
}

TEST(OrderedMap, UpperBound)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 5> m;
    m[0] = 0;
    ASSERT_EQ(m.end(), m.upper_bound(0));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(0, 0)), *m.upper_bound(-1));
    m[1] = 1;
    ASSERT_NE(m.end(), m.upper_bound(0));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(1, 1)), *m.upper_bound(0));
    m[2] = 2;
    ASSERT_EQ((std::pair<int32_t const, int32_t>(2, 2)), *m.upper_bound(1));
    m[9] = 9;
    ASSERT_EQ((std::pair<int32_t const, int32_t>(2, 2)), *m.upper_bound(1));
    ASSERT_EQ(m.end(), m.upper_bound(9));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(9, 9)), *m.upper_bound(8));
    m[17] = 17;
    ASSERT_EQ((std::pair<int32_t const, int32_t>(0, 0)), *m.upper_bound(-1));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(1, 1)), *m.upper_bound(0));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(2, 2)), *m.upper_bound(1));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(9, 9)), *m.upper_bound(2));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(9, 9)), *m.upper_bound(3));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(9, 9)), *m.upper_bound(8));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(17, 17)), *m.upper_bound(9));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(17, 17)), *m.upper_bound(10));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(17, 17)), *m.upper_bound(16));
    ASSERT_EQ(m.end(), m.upper_bound(17));
    ASSERT_EQ(m.end(), m.upper_bound(18));

    ::estd::ordered_map<int32_t, int32_t> const& base = m;
    ASSERT_EQ(base.cend(), base.upper_bound(17));
    ASSERT_EQ((std::pair<int32_t const, int32_t>(17, 17)), *base.upper_bound(16));
}

struct IntCompare
{
    IntCompare(int32_t i) : _i(i) {}

#if 0 /* not used */
        bool operator()(int32_t const& x, int32_t const& y) const
        {
            return x < _i;
        }
#endif
    int32_t _i;
};

/**
 * \desc
 * Make sure a copy of the comparator is returned by passing in a special
 * Comparator object with a dummy value.
 */
static_assert(
    std::is_same<IntCompare, ::estd::ordered_map<int32_t, int32_t, IntCompare>::key_compare>::value,
    "");

TEST(OrderedMap, TestKeyComp)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10, IntCompare> _m(IntCompare(17));
    ::estd::ordered_map<int32_t, int32_t, IntCompare>& m = _m;
    ASSERT_EQ(17, m.key_comp()._i);
}

TEST(OrderedMap, IteratorToConstIterator)
{
    MAKE_MAP(int32_t, Value, 10, a);

    IntValueMap::iterator i        = a.begin();
    IntValueMap::const_iterator ci = i;
    ASSERT_TRUE(i == ci);
}

TEST(OrderedMap, TestCopyConstructorCalls)
{
    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 5> a_;
    ::estd::ordered_map<int32_t, ::internal::TestClassCalls>& a = a_;

    ::internal::TestClassCalls::reset();
    a[0] = ::internal::TestClassCalls(1);
    ASSERT_TRUE(::internal::TestClassCalls::verify(2, 0, 0, 1, 0, 1));

    a[1] = ::internal::TestClassCalls(2);
    a[2] = ::internal::TestClassCalls(3);
    a[3] = ::internal::TestClassCalls(4);
    a[4] = ::internal::TestClassCalls(5);

    // now create new deque from other deque
    ::internal::TestClassCalls::reset();
    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 5> b(a);

    // verify that the number of calls was correct
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 5, 0, 0, 0, 0));

    ASSERT_EQ(5U, a.size());

    // and verify that the values were copied.
    ASSERT_EQ(1, b[0].tag());
    ASSERT_EQ(2, b[1].tag());
    ASSERT_EQ(3, b[2].tag());
    ASSERT_EQ(4, b[3].tag());
    ASSERT_EQ(5, b[4].tag());
}

TEST(OrderedMap, TestCopyConstructorDeclareCalls)
{
    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 5> a;

    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a[i] = ::internal::TestClassCalls(i);
    }

    // now create new deque from other deque
    ::internal::TestClassCalls::reset();
    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 10> b(a);

    // verify that the number of calls was correct
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 5, 0, 0, 0, 0));

    ASSERT_EQ(5U, a.size());

    // and verify that the values were copied.
    ASSERT_EQ(0, b[0].tag());
    ASSERT_EQ(1, b[1].tag());
    ASSERT_EQ(2, b[2].tag());
    ASSERT_EQ(3, b[3].tag());
    ASSERT_EQ(4, b[4].tag());
}

void copy(::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 10> copy)
{
    // make sure that our copies were done properly.
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 3, 0, 0, 0, 0));
    ASSERT_EQ(3U, copy.size());
    // and verify that the values were copied.
    ASSERT_EQ(0, copy[0].tag());
    ASSERT_EQ(1, copy[1].tag());
    ASSERT_EQ(2, copy[2].tag());
}

TEST(OrderedMap, TestCopyConstructorCallsToFunction)
{
    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 5> a;

    for (size_t i = 0; i < a.max_size() - 2; ++i)
    {
        a[i] = ::internal::TestClassCalls(i);
    }

    ::internal::TestClassCalls::reset();
    copy(a);
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 3, 0, 3, 0, 0));
}

TEST(OrderedMap, TestAssignmentCalls)
{
    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 5> a;

    for (size_t i = 0; i < a.max_size() - 2; ++i)
    {
        a[i] = ::internal::TestClassCalls(i);
    }

    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 10> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 3, 0, 0, 0, 0));

    ASSERT_EQ(0, b[0].tag());
    ASSERT_EQ(1, b[1].tag());
    ASSERT_EQ(2, b[2].tag());

    ::estd::ordered_map<int32_t, ::internal::TestClassCalls>& tmp = b;
    // test self assignment
    ::internal::TestClassCalls::reset();
    tmp = b;
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 0, 0, 0));

    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 10> c;

    // make sure we can assign into a value of the base class
    ::internal::TestClassCalls::reset();
    c = tmp;
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 3, 0, 0, 0, 0));

    ASSERT_EQ(0, c[0].tag());
    ASSERT_EQ(1, c[1].tag());
    ASSERT_EQ(2, c[2].tag());
}

TEST(OrderedMap, TestAssignmentIdentical)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10> a;

    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a[i] = i;
    }

    ::estd::declare::ordered_map<int32_t, int32_t, 10> b;

    b = a;

    ASSERT_EQ(10U, b.size());

    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i]);
    }

    ::estd::declare::ordered_map<int32_t, int32_t, 10> c(b);
    ASSERT_EQ(10U, c.size());

    // and that the values are correct
    for (size_t i = 0; i < c.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, c[i]);
    }

    ::estd::ordered_map<int32_t, int32_t>& base = c;
    ::estd::declare::ordered_map<int32_t, int32_t, 10> d(base);
    ASSERT_EQ(10U, d.size());

    // and that the values are correct
    for (size_t i = 0; i < d.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, d[i]);
    }
}

TEST(OrderedMap, TestAssignmentToBase)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10> a;

    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a[i] = i;
    }

    ::estd::ordered_map<int32_t, int32_t>& base = a;

    ::estd::declare::ordered_map<int32_t, int32_t, 10> b;

    b = base;

    ASSERT_EQ(10U, b.size());

    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b[i]);
    }
}

TEST(OrderedMap, TestAssignmentCallsAndDestructor)
{
    ::internal::TestClassCalls::reset();

    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 10> a;

    for (size_t i = 0; i < a.max_size() - 2; ++i)
    {
        a[i] = ::internal::TestClassCalls(i);
    }

    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 10> b;
    for (size_t i = 0; i < 5; ++i)
    {
        b[i] = ::internal::TestClassCalls(i);
    }

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();
    b = a;
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 5, 0, 0));

    ASSERT_EQ(8U, b.size());
    ASSERT_EQ(0, b[0].tag());
    ASSERT_EQ(1, b[1].tag());
    ASSERT_EQ(2, b[2].tag());
    ASSERT_EQ(3, b[3].tag());
    ASSERT_EQ(4, b[4].tag());
    ASSERT_EQ(5, b[5].tag());
    ASSERT_EQ(6, b[6].tag());
    ASSERT_EQ(7, b[7].tag());
}

TEST(OrderedMap, TestAssignmentDestructorCalls)
{
    {
        ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 10> a;

        for (size_t i = 0; i < a.max_size() - 2; ++i)
        {
            a[i] = ::internal::TestClassCalls(i);
        }

        ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 10> b;
        for (size_t i = 0; i < 5; ++i)
        {
            b[i] = ::internal::TestClassCalls(i);
        }

        // verify that the assignment operator works
        b = a;
        ::internal::TestClassCalls::reset();
    }

    // make sure that we destroyed the objects we created.
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 16, 0, 0));
}

TEST(OrderedMap, TestAssignmentIncorrectSize)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 10> a;

    for (size_t i = 0; i < a.max_size() - 2; ++i)
    {
        a[i] = ::internal::TestClassCalls(i);
    }

    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 5> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    ASSERT_THROW(b = a, ::estd::assert_exception);
}

TEST(OrderedMap, TestAssignmentBiggerSize)
{
    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 10> a;

    for (size_t i = 0; i < a.max_size() - 2; ++i)
    {
        a[i] = ::internal::TestClassCalls(i);
    }

    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 15> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));

    ASSERT_EQ(8U, b.size());
    ASSERT_EQ(0, b[0].tag());
    ASSERT_EQ(1, b[1].tag());
    ASSERT_EQ(2, b[2].tag());
    ASSERT_EQ(3, b[3].tag());
    ASSERT_EQ(4, b[4].tag());
    ASSERT_EQ(5, b[5].tag());
    ASSERT_EQ(6, b[6].tag());
    ASSERT_EQ(7, b[7].tag());
}

TEST(OrderedMap, TestCopyIncorrectSize)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 10> a;

    for (size_t i = 0; i < a.max_size() - 2; ++i)
    {
        a[i] = ::internal::TestClassCalls(i);
    }

    try
    {
        ::estd::declare::ordered_map<int32_t, ::internal::TestClassCalls, 5> b(a);
        ASSERT_TRUE(0);
    }
    catch (::estd::assert_exception const& e)
    {}
}

TEST(OrderedMap, TestValueCompare)
{
    ::estd::declare::ordered_map<int32_t, int32_t, 10> m;

    m[1] = 2;
    m[3] = 4;

    ::estd::ordered_map<int32_t, int32_t>::value_compare comp = m.value_comp();

    ASSERT_TRUE(comp(*m.find(1), *m.find(3)));
}
