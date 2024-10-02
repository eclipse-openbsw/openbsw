// Copyright 2024 Accenture.

#include "estd/stack.h"

#include "internal/NoDefaultConstructorObject.h"
#include "internal/TestClassCalls.h"
#include "internal/UncopyableObject.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

using namespace ::testing;

// explicit instantiation to get correct test coverage
template class ::estd::stack<int32_t>;
template class ::estd::declare::stack<int32_t, 10>;

namespace
{
// stacks used:
// stack<int32_t, 10>
// stack<internal::TestClassCalls, 10>
// stack<int32_t, 20>
// stack<internal::NoDefaultConstructorObject, 10>
// stack<internal::UncopyableObject, 10>

using IntStack = ::estd::stack<int32_t>;

#define MAKE_STACK(T, N, Name)            \
    ::estd::declare::stack<T, N> Name##_; \
    ::estd::stack<T>& Name(Name##_)

template<size_t total>
void validateSize(size_t overhead32, size_t overhead64)
{
    ::estd::declare::stack<int32_t, total> container;

    size_t itemSize = sizeof(int32_t);

    if (sizeof(size_t) == 4)
    {
        ASSERT_EQ(4U, sizeof(::estd::stack<int32_t>));
        ASSERT_EQ(total * itemSize + overhead32, sizeof(container)) << total << ":" << overhead32;
    }
    else
    {
        ASSERT_EQ(8U, sizeof(::estd::stack<int32_t>));
        ASSERT_EQ(total * itemSize + overhead64, sizeof(container)) << total << ":" << overhead64;
    }
}

TEST(Stack, TestSizeOf)
{
    validateSize<5>(12, 28);
    validateSize<10>(12, 24);
    validateSize<1>(12, 28);
    validateSize<20>(12, 24);
    validateSize<40>(12, 24);
}

TEST(Stack, TestContructor)
{
    MAKE_STACK(int32_t, 10, s);
    ASSERT_TRUE(s.empty());
    ASSERT_FALSE(s.full());
    ASSERT_EQ(0U, s.size());
    ASSERT_EQ(10U, s.max_size());
}

TEST(Stack, TestEmpty)
{
    MAKE_STACK(int32_t, 10, s);
    ASSERT_TRUE(s.empty());
}

TEST(Stack, TestFull)
{
    MAKE_STACK(int32_t, 10, s);
    for (IntStack::size_type i = 0; i < s.max_size(); ++i)
    {
        s.push(0);
    }
    ASSERT_TRUE(s.full());
}

TEST(Stack, Push)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_STACK(int32_t, 10, s);
    int32_t i = 1;
    while (!s.full())
    {
        s.push(i);
        ASSERT_EQ((size_t)i, s.size());
        ASSERT_EQ(i, s.top());
        ++i;
    }
    ASSERT_EQ(s.max_size(), s.size());
    ASSERT_TRUE(s.full());
    ASSERT_THROW({ s.push(10); }, ::estd::assert_exception);
}

TEST(Stack, PushAssign)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_STACK(int32_t, 10, s);
    int32_t i = 1;
    while (!s.full())
    {
        s.push() = i;
        ASSERT_EQ((size_t)i, s.size());
        ASSERT_EQ(i, s.top());
        ++i;
    }
    ASSERT_EQ(s.max_size(), s.size());
    ASSERT_TRUE(s.full());
    ASSERT_THROW({ s.push() = 10; }, ::estd::assert_exception);
}

TEST(Stack, Emplace)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    MAKE_STACK(int32_t, 10, s);
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
    ASSERT_THROW({ s.emplace().construct(10); }, ::estd::assert_exception);
}

TEST(Stack, Pop)
{
    MAKE_STACK(int32_t, 10, s);
    s.push(1);
    ASSERT_EQ(1U, s.size());
    s.push() = 2;
    ASSERT_EQ(2U, s.size());
    s.pop();
    ASSERT_EQ(1U, s.size());
    s.pop();
    ASSERT_EQ(0U, s.size());
}

TEST(Stack, TestTop)
{
    MAKE_STACK(int32_t, 10, s);
    ::estd::stack<int32_t> const& cs = s;

    s.push(1);
    ASSERT_EQ(1, s.top());
    ASSERT_EQ(1, cs.top());
    s.push() = 2;
    ASSERT_EQ(2, s.top());
}

TEST(Stack, TestUncopyable)
{
    MAKE_STACK(::internal::UncopyableObject, 10, q);

    q.emplace().construct(1);
    q.emplace().construct(2);

    ASSERT_EQ(2U, q.size());
    ASSERT_EQ(2, q.top().i);
    q.pop();
    ASSERT_EQ(1, q.top().i);
}

TEST(Stack, TestNoDefaultConstructor)
{
    MAKE_STACK(::internal::NoDefaultConstructorObject, 10, q);

    q.emplace().construct(1);
    q.emplace().construct(2);

    ASSERT_EQ(2U, q.size());
    ASSERT_EQ(2, q.top().i);
    q.pop();
    ASSERT_EQ(1, q.top().i);
}

TEST(Stack, RelationalOperators)
{
    MAKE_STACK(int32_t, 10, d1);
    MAKE_STACK(int32_t, 20, d2);
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

TEST(Stack, TestDestructorCalls)
{
    ::internal::TestClassCalls::reset();
    {
        MAKE_STACK(::internal::TestClassCalls, 10, a);

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

TEST(Stack, TestCopyConstructorCalls)
{
    MAKE_STACK(::internal::TestClassCalls, 10, a);

    // initialize array
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        ::internal::TestClassCalls::reset();
        a.emplace().construct(i);
        ASSERT_TRUE(::internal::TestClassCalls::verify(1, 0, 0, 0, 0, 0));
    }

    // now create new deque from other deque
    ::internal::TestClassCalls::reset();
    ::estd::declare::stack<::internal::TestClassCalls, 10> b(a);

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

TEST(Stack, TestCopyConstructorDeclareCalls)
{
    ::estd::declare::stack<::internal::TestClassCalls, 10> a;

    // initialize array
    for (int32_t i = 0; i < (int32_t)a.max_size(); ++i)
    {
        a.emplace().construct(i);
    }

    // now create new deque from other deque
    ::internal::TestClassCalls::reset();
    ::estd::declare::stack<::internal::TestClassCalls, 10> b(a);

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

void copy(::estd::declare::stack<::internal::TestClassCalls, 10> copy)
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

TEST(Stack, TestCopyConstructorCallsToFunction)
{
    MAKE_STACK(::internal::TestClassCalls, 10, a);

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

TEST(Stack, TestSimpleAssignmentCalls)
{
    MAKE_STACK(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        a.emplace().construct(i);
    }

    ::estd::declare::stack<::internal::TestClassCalls, 10> b;

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

TEST(Stack, TestAssignmentIdentical)
{
    ::estd::declare::stack<int32_t, 10> a;

    for (size_t i = 0; i < a.max_size(); ++i)
    {
        a.push(i);
    }

    ::estd::declare::stack<int32_t, 10> b;

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

TEST(Stack, TestSelfAssignmentCalls)
{
    MAKE_STACK(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        a.emplace().construct(i);
    }

    ::estd::declare::stack<::internal::TestClassCalls, 10> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ::estd::stack<::internal::TestClassCalls>& tmp = b;
    // test self assignment
    ::internal::TestClassCalls::reset();
    tmp = b;
    ASSERT_TRUE(::internal::TestClassCalls::verify(0, 0, 0, 0, 0, 0));
}

TEST(Stack, TestBaseAssignmentCalls)
{
    MAKE_STACK(::internal::TestClassCalls, 10, a);

    // initialize deque
    for (int32_t i = 0; i < (int32_t)a.max_size() - 2; ++i)
    {
        a.emplace().construct(i);
    }

    ::estd::declare::stack<::internal::TestClassCalls, 10> b;

    // verify that the assignment operator works
    ::internal::TestClassCalls::reset();

    b = a;

    ::estd::stack<::internal::TestClassCalls>& tmp = b;

    MAKE_STACK(::internal::TestClassCalls, 10, c);

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
