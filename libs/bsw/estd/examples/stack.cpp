// Copyright 2024 Accenture.

#include "estd/stack.h"

#include "estd/assert.h"

#include <gtest/gtest.h>

TEST(StackExample, construction)
{
    // [EXAMPLE_STACK_CONSTRUCT_START]
    // Construction of a default estd::declare::stack that stores up to 10 elements
    ::estd::declare::stack<int32_t, 10> a;
    // Construction of a stack from another stack
    ::estd::declare::stack<int32_t, 10> b(a);
    // [EXAMPLE_STACK_CONSTRUCT_END]
}

TEST(StackExample, access)
{
    // [EXAMPLE_STACK_ACCESS_START]
    // Declare an empty stack
    ::estd::declare::stack<int32_t, 5> s;
    ::estd::stack<int32_t>& cs = s;
    s.push(1);
    ASSERT_EQ(1, s.top());
    ASSERT_EQ(1, cs.top());
    s.push() = 2;
    ASSERT_EQ(2, s.top());
    // [EXAMPLE_STACK_ACCESS_END]
}

TEST(StackExample, capacity)
{
    // [EXAMPLE_STACK_CAPACITY_START]
    // Declare an empty stack
    ::estd::declare::stack<int32_t, 10> s;
    s.push(1);
    ASSERT_EQ(1U, s.size());
    s.push() = 2;
    ASSERT_EQ(2U, s.size());
    s.pop();
    ASSERT_EQ(1U, s.size());
    s.pop();
    ASSERT_EQ(0U, s.size());
    ASSERT_EQ(10U, s.max_size());
    ASSERT_TRUE(s.empty());
    for (estd::stack<int32_t>::size_type i = 0; i < s.max_size(); ++i)
    {
        s.push(0);
    }
    ASSERT_TRUE(s.full());
    // [EXAMPLE_STACK_CAPACITY_END]
}

TEST(StackExample, modifiers)
{
    // [EXAMPLE_STACK_MODIFIERS_START]
    // Declare an empty stack
    ::estd::declare::stack<int32_t, 10> s;
    int32_t i = 1;
    while (!s.full())
    {
        s.emplace().construct(i);
        ASSERT_EQ((size_t)i, s.size());
        ASSERT_EQ(i, s.top());
        ++i;
    }
    ASSERT_EQ(s.max_size(), s.size());
    ASSERT_TRUE(s.full());
    // [EXAMPLE_STACK_MODIFIERS_END]
}
