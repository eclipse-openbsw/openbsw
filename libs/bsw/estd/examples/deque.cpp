// Copyright 2024 Accenture.

#include "estd/deque.h"

#include "estd/algorithm.h"
#include "estd/constructor.h"

#include <gtest/gtest.h>

#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunused-private-field"

namespace
{
void example_construction()
{
    // EXAMPLE_START declare
    // construction of a defaulted estd::declare::deque that stores up to 10 elements
    ::estd::declare::deque<uint8_t, 10> deq1;

    // construction of deques prefilled with a given number of elements
    ::estd::declare::deque<uint8_t, 10> deq2(8);
    ::estd::declare::deque<uint8_t, 10> deq3(8, 255);

    // construction of a deque from another deque
    ::estd::declare::deque<uint8_t, 20> deq4(deq3);

    // function taking an estd::deque parameter
    auto size = [](::estd::deque<uint8_t> const& d) { return d.size(); };
    size(deq4);
    // EXAMPLE_END declare
}

using IntDeque = ::estd::deque<int32_t>;

// EXAMPLE_START iterate
TEST(DequeIterator, Comparable)
{
    IntDeque::iterator itr;
    IntDeque::iterator itr2(itr);
    ASSERT_TRUE(itr == itr2);
    ASSERT_FALSE(itr != itr2);
}

// EXAMPLE_END iterate

void example_capacity()
{
    // EXAMPLE_START capacity
    ::estd::declare::deque<uint8_t, 10> deq6;
    deq6.push_back(1);
    deq6.push_back(2);

    assert(deq6.size() == 2);        // returns true if the size of deq6 is 2
    ASSERT_EQ(10U, deq6.max_size()); // returns true if value matches maximum size of deq6

    deq6.clear();
    ASSERT_TRUE(deq6.empty()); // returns true if the deq6 is empty or not
    // EXAMPLE_END capacity
}

class Foo
{
public:
    Foo() = default;

    Foo(int x, int y) : x_(x), y_(y) {}

    Foo(Foo const&)            = delete; // Copy constructor
    Foo& operator=(Foo const&) = delete; // Copy assignment operator

private:
    int x_;
    int y_;
};

void example_construct()
{
    // EXAMPLE_START modify
    using std::begin;
    using std::end;

    ::estd::declare::deque<uint8_t, 10> deq7;

    // resize deque from size 0 to size 2, growing two elements with value 255
    deq7.resize(2, 255);

    ::estd::declare::deque<uint8_t, 5> deq8;

    // assign range to deque
    uint8_t testData[] = {0, 1, 2, 3, 4};
    deq8.assign(begin(testData), end(testData));

    // pushing beyond the limit will result in a failed assert.
    ::estd::declare::deque<uint8_t, 10> deq9;
    deq9.push_back(1);
    deq9.push_front(0);
    deq9.insert(deq9.cbegin(), 3, 5);
    deq9.insert(deq9.cbegin(), -1);

    deq9.pop_front(); // removes one element in the front()
    deq9.pop_back();  // removes one element in the back()

    deq9.erase(deq9.cbegin());              // erase using position
    deq9.erase(deq9.cbegin(), deq9.cend()); // erase using range of position

    // instantiation of a deque of a noncopyable non default constructible type
    ::estd::declare::deque<Foo, 10> deq10;
    deq10.emplace_back().construct(13, 17);
    // EXAMPLE_END modify
}

void example_compare()
{
    // EXAMPLE_START compare
    // create deques and push elements to them
    ::estd::declare::deque<uint8_t, 10> deq11;
    ::estd::declare::deque<uint8_t, 10> deq12;

    deq11.push_back(1);
    deq12.push_back(1);
    deq11.push_back(2);

    // deques with different sizes are compared below

    ASSERT_NE(deq11, deq12);
    ASSERT_FALSE(deq11 < deq12);

    deq12.push_back(2);

    // deques with same sizes are compared below
    ASSERT_EQ(deq11, deq12);
    ASSERT_TRUE((deq11 == deq12));
    ASSERT_GE(deq11, deq12);
    ASSERT_TRUE((deq11 >= deq12));
    ASSERT_GE(deq12, deq11);
    ASSERT_TRUE((deq12 >= deq11));
    ASSERT_LE(deq11, deq12);
    ASSERT_TRUE((deq11 <= deq12));
    ASSERT_LE(deq12, deq11);
    ASSERT_TRUE((deq12 <= deq11));
    // EXAMPLE_END compare
}
} // namespace

TEST(Deque, run_examples)
{
    example_construction();
    example_capacity();
    example_construct();
    example_compare();
}
