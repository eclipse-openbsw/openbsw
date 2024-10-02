// Copyright 2024 Accenture.

#include "estd/priority_queue.h"

#include "internal/TestClassCalls.h"

#include <gtest/gtest.h>

#include <type_traits>

using namespace ::testing;

// explicit instantiation to get correct test coverage
template class ::estd::declare::priority_queue<int32_t, 10>;
template class ::estd::priority_queue<int32_t>;

using IntQueue10 = ::estd::declare::priority_queue<int32_t, 10>;
using IntQueue   = ::estd::priority_queue<int32_t>;

static_assert(std::is_same<int32_t, IntQueue10::value_type>::value, "");
static_assert(std::is_same<int32_t&, IntQueue10::reference>::value, "");
static_assert(std::is_same<int32_t const&, IntQueue10::const_reference>::value, "");

namespace
{
#define MAKE_QUEUE(T, N, Name)                     \
    ::estd::declare::priority_queue<T, N> Name##_; \
    ::estd::priority_queue<T>& Name(Name##_)

template<size_t total>
void validateSize(size_t overhead32, size_t overhead64)
{
    ::estd::declare::priority_queue<int32_t, total> container;

    size_t itemSize = sizeof(int32_t);

    ASSERT_EQ(sizeof(size_t) * 2, sizeof(::estd::priority_queue<int32_t>));
    if (sizeof(size_t) == 4)
    {
        ASSERT_EQ(total * itemSize + overhead32, sizeof(container)) << total << ":" << overhead32;
    }
    else
    {
        ASSERT_EQ(total * itemSize + overhead64, sizeof(container)) << total << ":" << overhead64;
    }
}

TEST(PriorityQueue, TestSizeOf)
{
    validateSize<5>(16, 36);
    validateSize<10>(16, 32);
    validateSize<1>(16, 36);
    validateSize<20>(16, 32);
    validateSize<40>(16, 32);
}

TEST(PriorityQueue, TestEmpty)
{
    ::estd::declare::priority_queue<int32_t, 1> arr;
    ASSERT_TRUE(arr.empty());
    ASSERT_FALSE(arr.full());
    ASSERT_EQ(0U, arr.size());
}

TEST(PriorityQueue, TestSizeAndMaxSize)
{
    IntQueue10 a;
    ASSERT_EQ(0U, a.size());
    ASSERT_EQ(10U, a.max_size());
}

TEST(PriorityQueue, TestQueue)
{
    MAKE_QUEUE(int32_t, 10, q);

    q.push(10);
    ASSERT_EQ(10, q.top());
    q.push(5);
    ASSERT_EQ(10, q.top());
    q.push(11);

    ASSERT_EQ(3U, q.size());
    ASSERT_EQ(11, q.top());

    q.pop();
    ASSERT_EQ(10, q.top());
    ASSERT_EQ(2U, q.size());
}

TEST(PriorityQueue, TestAsserts)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ::estd::declare::priority_queue<int32_t, 2> q;

    ASSERT_THROW(q.top(), ::estd::assert_exception);
    ASSERT_THROW(q.pop(), ::estd::assert_exception);

    q.push(10);
    q.push(5);

    ASSERT_THROW(q.push(12), ::estd::assert_exception);
}

TEST(PriorityQueue, TestPop)
{
    ::estd::declare::priority_queue<int32_t, 2> q;

    q.push(1);

    ASSERT_EQ(1U, q.size());

    ASSERT_EQ(1, q.top());
    q.pop();
    ASSERT_EQ(0U, q.size());
}

TEST(PriorityQueue, TestDestructorCalls)
{
    {
        MAKE_QUEUE(::internal::TestClassCalls, 10, a);

        // make sure that the destructors are called when we assign
        // a new value.
        for (size_t i = 0; i < a.max_size(); ++i)
        {
            a.push(::internal::TestClassCalls(i));
            ASSERT_EQ(i + 1, a.size());
        }
        ASSERT_EQ(10U, a.size());
        ::internal::TestClassCalls::reset();
    }

    // verify that the destructors were called when the array was destroyed
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 10, 0, 0));
}

TEST(PriorityQueue, TestCopyConstructorCalls)
{
    MAKE_QUEUE(::internal::TestClassCalls, 10, a);

    // initialize array
    for (size_t i = 0; i < a.max_size() - 2; ++i)
    {
        a.push(::internal::TestClassCalls(i));
    }

    // now create new deque from other deque
    ::internal::TestClassCalls::reset();
    ::estd::declare::priority_queue<::internal::TestClassCalls, 10> b(a);

    // verify that the number of calls was correct
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));

    ASSERT_EQ(8U, a.size());

    // and verify that the values were copied.
    int32_t i = 7;
    while (!b.empty())
    {
        ASSERT_EQ(i--, b.top().tag());
        b.pop();
    }
}

TEST(PriorityQueue, TestCopyConstructorDeclareCalls)
{
    ::estd::declare::priority_queue<::internal::TestClassCalls, 10> a;

    // initialize array
    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.push(::internal::TestClassCalls(i));
    }

    // now create new deque from other deque
    ::internal::TestClassCalls::reset();
    ::estd::declare::priority_queue<::internal::TestClassCalls, 10> b(a);

    // verify that the number of calls was correct
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 10, 0, 0, 0, 0));

    ASSERT_EQ(10U, a.size());

    // and verify that the values were copied.
    int32_t i = 9;
    while (!b.empty())
    {
        ASSERT_EQ(i--, b.top().tag());
        b.pop();
    }
}

void copy(::estd::declare::priority_queue<::internal::TestClassCalls, 10> copy)
{
    // make sure that our copies were done properly.
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));
    ASSERT_EQ(8U, copy.size());
    // and verify that the values were copied.
    int32_t i = 7;
    while (!copy.empty())
    {
        ASSERT_EQ(i--, copy.top().tag());
        copy.pop();
    }
}

TEST(PriorityQueue, TestCopyConstructorCallsToFunction)
{
    MAKE_QUEUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (size_t i = 0; i < a.max_size() - 2; ++i)
    {
        a.push(::internal::TestClassCalls(i));
    }

    ::internal::TestClassCalls::reset();
    copy(a);
}

TEST(PriorityQueue, TestSimpleAssignmentCalls)
{
    MAKE_QUEUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (size_t i = 0; i < a.max_size() - 2; ++i)
    {
        a.push(::internal::TestClassCalls(i));
    }

    ::estd::declare::priority_queue<::internal::TestClassCalls, 10> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));

    ASSERT_EQ(8U, b.size());
    int32_t i = 7;
    while (!b.empty())
    {
        ASSERT_EQ(i--, b.top().tag());
        b.pop();
    }
}

TEST(PriorityQueue, TestAssignmentIdentical)
{
    ::estd::declare::priority_queue<int32_t, 10> a;

    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.push(i);
    }

    ::estd::declare::priority_queue<int32_t, 10> b;

    b = a;

    ASSERT_EQ(10U, b.size());

    int32_t count = 9;
    size_t size   = b.size();

    // and that the values are correct
    for (size_t i = 0; i < size; ++i)
    {
        ASSERT_EQ(count, b.top());
        b.pop();
        count--;
    }
    ASSERT_EQ(-1, count);
}

TEST(PriorityQueue, TestSelfAssignmentCalls)
{
    MAKE_QUEUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (size_t i = 0; i < a.max_size() - 2; ++i)
    {
        a.push(::internal::TestClassCalls(i));
    }

    ::estd::declare::priority_queue<::internal::TestClassCalls, 10> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ::estd::priority_queue<::internal::TestClassCalls>& tmp = b;
    // test self assignment
    ::internal::TestClassCalls::reset();
    tmp = b;
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 0, 0, 0));
}

TEST(PriorityQueue, TestBaseAssignmentCalls)
{
    MAKE_QUEUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (size_t i = 0; i < a.max_size() - 2; ++i)
    {
        a.push(::internal::TestClassCalls(i));
    }

    ::estd::declare::priority_queue<::internal::TestClassCalls, 10> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ::estd::priority_queue<::internal::TestClassCalls>& tmp = b;

    MAKE_QUEUE(::internal::TestClassCalls, 10, c);

    // make sure we can assign into a value of the base class
    ::internal::TestClassCalls::reset();
    c = tmp;
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));

    ASSERT_EQ(8U, b.size());
    int32_t i = 7;
    while (!b.empty())
    {
        ASSERT_EQ(i--, b.top().tag());
        b.pop();
    }
}

} // namespace
