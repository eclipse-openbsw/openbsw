// Copyright 2024 Accenture.

#include "estd/object_pool.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
#define MAKE_POOL(T, N, Name)                 \
    estd::declare::object_pool<T, N> Name##_; \
    estd::object_pool<T>& Name(Name##_)

class MyClass
{
public:
    MyClass() = default;
};

void example_construction()
{
    //[EXAMPLE_START construction]
    // declare the object pool of size 5
    ::estd::declare::object_pool<int, 5> pool{};
    // construct using allocate and insert value 17 and 23
    auto& i1 = pool.allocate().construct(17);
    pool.allocate().construct(23);

    // Checking the values using position
    EXPECT_EQ(17, *pool.begin());
    EXPECT_EQ(17, *pool.cbegin());

    // release the first one
    pool.release(i1);
    EXPECT_EQ(23, *pool.begin());
    EXPECT_EQ(23, *pool.cbegin());
    //[EXAMPLE_END construction]

    //[EXAMPLE_START Usage]
    // declare object pool p with size 10
    MAKE_POOL(MyClass, 10, p);
    ASSERT_EQ(10U, p.size());

    // create a reference and use acquire
    MyClass& a = p.acquire();
    // object acquired, so size of p is reduced
    ASSERT_EQ(9U, p.size());

    p.acquire();

    ASSERT_TRUE(p.contains(a));
    ASSERT_EQ(8U, p.size());

    // release the reference a
    p.release(a);
    // clears the objects in object pool
    p.clear();
    //[EXAMPLE_END Usage]
}
} // namespace

TEST(ObjectPool, run_examples) { example_construction(); }
