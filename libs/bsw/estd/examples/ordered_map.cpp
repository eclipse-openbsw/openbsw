// Copyright 2024 Accenture.

#include "estd/ordered_map.h"

#include <gtest/gtest.h>

namespace
{
struct Value
{
    Value() : _v() {}

    Value(int32_t v) : _v(v) {}

    int32_t _v;
};

using IntValueMap = ::estd::ordered_map<int32_t, Value>;
#define MAKE_MAP(K, T, N, Name)                    \
    ::estd::declare::ordered_map<K, T, N> Name##_; \
    ::estd::ordered_map<K, T>& Name = Name##_

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

void example_construction()
{
    // [EXAMPLE_ORDERED_MAP_DECLARATION_START]
    // instantiation of the map that stores up to 10 elements in ascending order
    ::estd::declare::ordered_map<int32_t, int32_t, 10> m;
    // check the lower and upper bound
    ASSERT_EQ(m.end(), m.lower_bound(0));
    ASSERT_EQ(m.end(), m.upper_bound(0));
    // [EXAMPLE_ORDERED_MAP_DECLARATION_END]
    // [EXAMPLE_ORDERED_MAP_MEMBER_FUNCTION_START]
    MAKE_MAP(int32_t, Value, 10, testMap);
    // check whether the key is in the testMap or not
    ASSERT_EQ(0U, testMap.count(1));
    // assign value of 100 to key 1 for the ordered map named testMap
    testMap[1] = 100;
    // check whether the key is in the testMap or not
    ASSERT_EQ(1U, testMap.count(1));
    // clear the ordered map
    testMap.clear();
    // assign key-value pair to testMap
    testMap[1] = 100;
    testMap[2] = 200;
    testMap.insert(std::make_pair(3, 300));
    testMap.emplace(4); // key is 4 and value is 0
    // erase options in testMap with position
    testMap.erase(testMap.cend());
    // erase options in testMap with key
    testMap.erase(1);
    // erase options in testMap for range of positions
    testMap.erase(testMap.cbegin(), testMap.cend());
    // check the capacity of testMap
    testMap.empty();    // returns TRUE
    testMap.size();     // size is 0
    testMap.max_size(); // max_size is 10
    // [EXAMPLE_ORDERED_MAP_MEMBER_FUNCTION_END]
    // [EXAMPLE_ORDERED_MAP_OPERATIONS_START]
    std::pair<
        ::estd::ordered_map<int32_t, int32_t>::iterator,
        ::estd::ordered_map<int32_t, int32_t>::iterator>
        r = m.equal_range(0);
    ASSERT_EQ(m.end(), r.first);
    m[1]                                                      = 2;
    m[3]                                                      = 4;
    ::estd::ordered_map<int32_t, int32_t>::value_compare comp = m.value_comp();
    ASSERT_TRUE(comp(*m.find(1), *m.find(3)));
    MAKE_MAP(int32_t, Value, 10, m1);
    m1[1]                 = 100;
    IntValueMap const& cm = m1;
    // Find whether the key is in the testMap or not
    ASSERT_EQ(m1.begin(), m1.find(1));
    ASSERT_EQ(m1.cbegin(), cm.find(1));
    ASSERT_EQ(1, m1.find(1)->first);
    ASSERT_EQ(1, cm.find(1)->first);
    ASSERT_EQ(1, (*m1.find(1)).first);
    ASSERT_EQ(1, (*cm.find(1)).first);
    ASSERT_EQ(100, m1.find(1)->second._v);
    ASSERT_EQ(100, cm.find(1)->second._v);
    ASSERT_EQ(100, (*m1.find(1)).second._v);
    ASSERT_EQ(100, (*cm.find(1)).second._v);
    // check whether testMap is full or not
    ASSERT_FALSE(m1.full());
    // fill the map with some data
    for (auto i = 0U; i < 5; ++i)
    {
        testMap[2 * i] = 10 * i;
    }
    // iterator to the end of the map
    auto const endIter = testMap.end();
    // key *3* does not exist in the ordered_map, so it performs insertion.
    (void)testMap[3];
    // insertion invalidates all the references and iterators referring
    // to the reallocated elements after an added item.
    (void)(endIter == testMap.end()); // undefined behavior, endIter is invalidated
    // use key_comp to compare the temp value assigned
    static_assert(
        std::is_same<IntCompare, ::estd::ordered_map<int32_t, int32_t, IntCompare>::key_compare>::
            value,
        "");
    ::estd::declare::ordered_map<int32_t, int32_t, 10, IntCompare> _m(IntCompare(17));
    ::estd::ordered_map<int32_t, int32_t, IntCompare>& m2 = _m;
    ASSERT_EQ(17, m2.key_comp()._i);
    // [EXAMPLE_ORDERED_MAP_OPERATIONS_END]
}
} // namespace

TEST(OrderedMapExample, run_examples) { example_construction(); }
