// Copyright 2024 Accenture.

#include "estd/ordered_vector.h"

#include <gmock/gmock.h>

namespace
{
void example_construction()
{
    // [EXAMPLE_ORDERED_VECTOR_START]
    using IntVector10 = ::estd::declare::ordered_vector<int32_t, 10, std::less<int32_t>>;
    // declare the ordered vector of size 10
    ::estd::declare::ordered_vector<int32_t, 10> a;
    // inserting elements to the ordered vector created
    a.insert(3);
    EXPECT_THAT(a, testing::ElementsAre(3));
    a.insert(2);
    EXPECT_THAT(a, testing::ElementsAre(2, 3));
    a.insert(1);
    // ordered elements in the vector
    EXPECT_THAT(a, testing::ElementsAre(1, 2, 3));
    a.resize(2);
    // element size reduced to 2
    EXPECT_THAT(a, testing::ElementsAre(1, 2));
    // makes the vector empty
    a.clear();
    a.find_or_insert(5);
    // Since element 5 is not found, it is inserted otherwise returns position.
    EXPECT_THAT(a, testing::ElementsAre(5));
    // Check whether the value is in the vector
    ASSERT_TRUE(a.contains(5));
    // finds and remove particular element
    // a.remove_if(comp); removes element based on predicate
    a.remove(5);
    a.insert(1);
    // erase element using iterator
    a.erase(a.begin());
    a.insert(1);
    a.insert(2);
    // erase element using range of iterators
    a.erase(a.begin(), a.end());
    a.insert(5);
    IntVector10::const_iterator itr = a.begin();
    itr++;
    // returns the position of the value mentioned
    ASSERT_EQ(*itr, 2);
    // [EXAMPLE_ORDERED_VECTOR_END]
}
} // namespace

TEST(OrderedVectorExample, run_examples) { example_construction(); }
