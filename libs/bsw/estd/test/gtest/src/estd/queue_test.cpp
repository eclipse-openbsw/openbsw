// Copyright 2024 Accenture.

#include "estd/queue.h"

#include "estd/assert.h"
#include "internal/NoDefaultConstructorObject.h"
#include "internal/TestClassCalls.h"
#include "internal/UncopyableObject.h"

#include <gtest/gtest.h>

using namespace ::testing;

// explicit instantiation to get correct test coverage
template class ::estd::declare::queue<int32_t, 10>;
template class ::estd::queue<int32_t>;

namespace
{
using IntQueue = ::estd::queue<int32_t>;

#define MAKE_QUEUE(T, N, Name)            \
    ::estd::declare::queue<T, N> Name##_; \
    ::estd::queue<T>& Name(Name##_)

template<size_t total>
void validateSize(size_t overhead32, size_t overhead64)
{
    ::estd::declare::queue<int32_t, total> container;

    size_t itemSize = sizeof(int32_t);

    if (sizeof(size_t) == 4)
    {
        ASSERT_EQ(4U, sizeof(::estd::queue<int32_t>));
        ASSERT_EQ((total + 1) * itemSize + overhead32, sizeof(container))
            << total << ":" << overhead32;
    }
    else
    {
        ASSERT_EQ(8U, sizeof(::estd::queue<int32_t>));
        ASSERT_EQ((total + 1) * itemSize + overhead64, sizeof(container))
            << total << ":" << overhead64;
    }
}

TEST(Queue, TestSizeOf)
{
    validateSize<5>(20, 40);
    validateSize<10>(20, 44);
    validateSize<1>(20, 40);
    validateSize<20>(20, 44);
    validateSize<40>(20, 44);
}

TEST(Queue, TestContructor)
{
    MAKE_QUEUE(int32_t, 10, q);
    ASSERT_TRUE(q.empty());
    ASSERT_FALSE(q.full());
    ASSERT_EQ(0U, q.size());
    ASSERT_EQ(10U, q.max_size());
}

TEST(Queue, TestEmpty)
{
    MAKE_QUEUE(int32_t, 10, q);
    ASSERT_TRUE(q.empty());
}

TEST(Queue, Full)
{
    MAKE_QUEUE(int32_t, 10, q);
    for (IntQueue::size_type i = 0; i < q.max_size(); ++i)
    {
        q.push(0);
    }
    ASSERT_TRUE(q.full());
}

TEST(Queue, Push)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_QUEUE(int32_t, 10, q);
    int32_t i = 1;
    while (!q.full())
    {
        q.push(i);
        ASSERT_EQ((size_t)i, q.size());
        ASSERT_EQ(i, q.back());
        ++i;
    }
    ASSERT_EQ(q.max_size(), q.size());
    ASSERT_TRUE(q.full());
    ASSERT_THROW({ q.push(10); }, ::estd::assert_exception);
}

TEST(Queue, PushAssign)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_QUEUE(int32_t, 10, q);
    int32_t i = 1;
    while (!q.full())
    {
        q.push() = i;
        ASSERT_EQ((size_t)i, q.size());
        ASSERT_EQ(i, q.back());
        ++i;
    }
    ASSERT_EQ(q.max_size(), q.size());
    ASSERT_TRUE(q.full());
    ASSERT_THROW({ q.push() = 10; }, ::estd::assert_exception);
}

TEST(Queue, Emplace)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_QUEUE(int32_t, 10, q);
    int32_t i = 1;
    while (!q.full())
    {
        q.emplace().construct(i);
        ASSERT_EQ((size_t)i, q.size());
        ASSERT_EQ(i, q.back());
        ++i;
    }
    ASSERT_EQ(q.max_size(), q.size());
    ASSERT_TRUE(q.full());
    ASSERT_THROW({ q.emplace().construct(10); }, ::estd::assert_exception);
}

TEST(Queue, Pop)
{
    MAKE_QUEUE(int32_t, 10, q);
    q.push(1);
    ASSERT_EQ(1U, q.size());
    q.push() = 2;
    ASSERT_EQ(2U, q.size());
    q.pop();
    ASSERT_EQ(1U, q.size());
    q.pop();
    ASSERT_EQ(0U, q.size());
}

TEST(Queue, TestBack)
{
    MAKE_QUEUE(int32_t, 10, q);
    ::estd::queue<int32_t> const& cq = q;

    q.push(1);
    ASSERT_EQ(1, q.back());
    ASSERT_EQ(1, cq.back());
    q.push() = 2;
    ASSERT_EQ(2, q.back());
}

TEST(Queue, TestFront)
{
    MAKE_QUEUE(int32_t, 10, q);
    ::estd::queue<int32_t> const& cq = q;

    q.push(1);
    ASSERT_EQ(1, q.front());
    ASSERT_EQ(1, cq.front());
    q.push() = 2;
    ASSERT_EQ(1, q.front());
}

TEST(Deque, BackEmpty)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_QUEUE(int32_t, 10, d);

    ASSERT_THROW(d.back(), ::estd::assert_exception);
}

TEST(Deque, FrontEmpty)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_QUEUE(int32_t, 10, d);

    ASSERT_THROW(d.front(), ::estd::assert_exception);
}

TEST(Queue, RelationalOperators)
{
    MAKE_QUEUE(int32_t, 10, d1);
    MAKE_QUEUE(int32_t, 11, d2);
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
    d1.push(1);
    ASSERT_NE(d1, d2);
    ASSERT_FALSE((d1 == d2));
    ASSERT_TRUE((d1 != d2));
    d2.push(1);
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
    d1.push(2);
    d2.push(0);
    ASSERT_GT(d1, d2);
    ASSERT_TRUE((d1 > d2));
    ASSERT_LT(d2, d1);
    ASSERT_TRUE((d2 < d1));
}

TEST(Queue, TestUncopyable)
{
    MAKE_QUEUE(::internal::UncopyableObject, 10, q);

    q.emplace().construct(1);
    q.emplace().construct(2);

    ASSERT_EQ(2U, q.size());
    ASSERT_EQ(1, q.front().i);
    ASSERT_EQ(2, q.back().i);
    q.pop();

    ASSERT_EQ(2, q.front().i);
}

TEST(Queue, TestNoDefaultConstructor)
{
    MAKE_QUEUE(::internal::NoDefaultConstructorObject, 10, q);

    q.emplace().construct(1);
    q.emplace().construct(2);

    ASSERT_EQ(2U, q.size());
    ASSERT_EQ(1, q.front().i);
    ASSERT_EQ(2, q.back().i);
    q.pop();

    ASSERT_EQ(2, q.front().i);
}

TEST(Queue, TestDestructorCalls)
{
    ::internal::TestClassCalls::reset();
    {
        MAKE_QUEUE(::internal::TestClassCalls, 10, a);

        ::internal::TestClassCalls::reset();

        // make sure that the destructors are called when we assign
        // a new value.
        for (int32_t i = 0; i < (int32_t)a.max_size(); ++i)
        {
            a.emplace().construct(i);
        }
        ASSERT_TRUE(::internal::TestClassCalls::verify(10, 0, 0, 0, 0, 0));

        ::internal::TestClassCalls::reset();
    }

    // verify that the destructors were called when the array was destroyed
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 10, 0, 0));
}

TEST(Queue, TestCopyConstructorCalls)
{
    MAKE_QUEUE(::internal::TestClassCalls, 10, a);

    // initialize array
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        ::internal::TestClassCalls::reset();
        a.emplace().construct(i);
        ASSERT_TRUE(::internal::TestClassCalls::verify(1, 0, 0, 0, 0, 0));
    }

    // now create new deque from other deque
    ::internal::TestClassCalls::reset();
    ::estd::declare::queue<::internal::TestClassCalls, 10> b(a);

    // verify that the number of calls was correct
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));

    ASSERT_EQ(8U, a.size());

    // and verify that the values were copied.
    int32_t i = 0;
    while (!b.empty())
    {
        ASSERT_EQ(i++, b.front().tag());
        b.pop();
    }
}

TEST(Queue, TestCopyConstructorDeclareCalls)
{
    ::estd::declare::queue<::internal::TestClassCalls, 10> a;

    // initialize array
    for (int32_t i = 0; i < (int32_t)a.max_size(); ++i)
    {
        a.emplace().construct(i);
    }

    // now create new deque from other deque
    ::internal::TestClassCalls::reset();
    ::estd::declare::queue<::internal::TestClassCalls, 10> b(a);

    // verify that the number of calls was correct
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 10, 0, 0, 0, 0));

    ASSERT_EQ(10U, a.size());

    // and verify that the values were copied.
    int32_t i = 0;
    while (!b.empty())
    {
        ASSERT_EQ(i++, b.front().tag());
        b.pop();
    }
}

void copy(::estd::declare::queue<::internal::TestClassCalls, 10> copy)
{
    // make sure that our copies were done properly.
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));
    ASSERT_EQ(8U, copy.size());
    // and verify that the values were copied.
    int32_t i = 0;
    while (!copy.empty())
    {
        ASSERT_EQ(i++, copy.front().tag());
        copy.pop();
    }
}

TEST(Queue, TestCopyConstructorCallsToFunction)
{
    MAKE_QUEUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        ::internal::TestClassCalls::reset();
        a.emplace().construct(i);
        ASSERT_TRUE(::internal::TestClassCalls::verify(1, 0, 0, 0, 0, 0));
    }

    ::internal::TestClassCalls::reset();
    copy(a);
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 8, 0, 0));
}

TEST(Queue, TestSimpleAssignmentCalls)
{
    MAKE_QUEUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        a.emplace().construct(i);
    }

    ::estd::declare::queue<::internal::TestClassCalls, 10> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));

    ASSERT_EQ(8U, b.size());
    int32_t i = 0;
    while (!b.empty())
    {
        ASSERT_EQ(i++, b.front().tag());
        b.pop();
    }
}

void fillQueue(::estd::queue<int32_t>& q)
{
    for (size_t i = 0; i < q.max_size(); ++i)
    {
        q.push(i);
    }
}

TEST(Queue, TestAssignmentIdentical)
{
    ::estd::declare::queue<int32_t, 10> a;
    fillQueue(a);

    ::estd::declare::queue<int32_t, 10> b;

    b = a;

    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b.front());
        b.pop();
    }
}

TEST(Queue, TestCopyConstructorBase)
{
    ::estd::declare::queue<int32_t, 10> a;
    fillQueue(a);

    ::estd::queue<int32_t>& base = a;
    ::estd::declare::queue<int32_t, 10> b(a);

    ASSERT_EQ(10U, b.size());
    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b.front());
        b.pop();
    }

    b = base;
    ASSERT_EQ(10U, b.size());
    // and that the values are correct
    for (size_t i = 0; i < b.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, b.front());
        b.pop();
    }

    ::estd::declare::queue<int32_t, 10> c(a);
    ASSERT_EQ(10U, c.size());
    // and that the values are correct
    for (size_t i = 0; i < c.size(); ++i)
    {
        ASSERT_EQ((int32_t)i, c.front());
        c.pop();
    }
}

TEST(Queue, TestSelfAssignmentCalls)
{
    MAKE_QUEUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        a.emplace().construct(i);
    }

    ::estd::declare::queue<::internal::TestClassCalls, 10> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ::estd::queue<::internal::TestClassCalls>& tmp = b;
    // test self assignment
    ::internal::TestClassCalls::reset();
    tmp = b;
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 0, 0, 0));
}

TEST(Queue, TestBaseAssignmentCalls)
{
    MAKE_QUEUE(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        a.emplace().construct(i);
    }

    ::estd::declare::queue<::internal::TestClassCalls, 10> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ::estd::queue<::internal::TestClassCalls>& tmp = b;

    MAKE_QUEUE(::internal::TestClassCalls, 10, c);

    // make sure we can assign into a value of the base class
    ::internal::TestClassCalls::reset();
    c = tmp;
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 8, 0, 0, 0, 0));

    ASSERT_EQ(8U, b.size());
    int32_t i = 0;
    while (!b.empty())
    {
        ASSERT_EQ(i++, b.front().tag());
        b.pop();
    }
}

} // namespace
