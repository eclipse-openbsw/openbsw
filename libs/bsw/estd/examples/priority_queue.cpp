// Copyright 2024 Accenture.

#include <estd/priority_queue.h>

#include <gtest/gtest.h>

TEST(PriorityQueueExample, Construction_and_operations)
{
    // [EXAMPLE_START]
    // construction of a default estd::declare::priority_queue that stores up to 10 elements
    ::estd::declare::priority_queue<int32_t, 4> q;

    // top() & pop() operations will result in an error as priority_queue is empty
    // q.top(); q.pop();

    // push() elements in priority_queue
    q.push(20);

    // top() shows the highest priority element on priority_queue
    ASSERT_EQ(20, q.top());
    q.push(4);

    // size() shows the number of elements present in priority_queue
    ASSERT_EQ(2U, q.size());

    // max_size() shows the maximum number of elements priority_queue can hold
    ASSERT_EQ(4U, q.max_size());
    q.push(11);
    ASSERT_EQ(20, q.top());
    q.push(5);
    ASSERT_EQ(4U, q.size());
    ASSERT_EQ(4U, q.max_size());

    // push() operation will throw an error as priority_queue is full
    // q.push(12);

    q.pop();
    ASSERT_EQ(11, q.top());
    ASSERT_EQ(3U, q.size());
    ASSERT_EQ(4U, q.max_size());
    // [EXAMPLE_END]
}
