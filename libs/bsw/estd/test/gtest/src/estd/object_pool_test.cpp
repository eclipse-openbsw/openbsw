// Copyright 2024 Accenture.

#include "estd/object_pool.h"

#include "estd/assert.h"
#include "internal/TestClassCalls.h"

#include <gmock/gmock.h>

// explicit instantiation to get correct test coverage
template class ::estd::declare::object_pool<::internal::TestClassCalls, 10>;
template class ::estd::object_pool<::internal::TestClassCalls>;

namespace
{
using namespace ::testing;

using TestClassPool = ::estd::object_pool<::internal::TestClassCalls>;

#define MAKE_POOL(T, N, Name)                   \
    ::estd::declare::object_pool<T, N> Name##_; \
    ::estd::object_pool<T>& Name(Name##_)

size_t calculatePadding(size_t dataSize, size_t usedSize)
{
    if (sizeof(size_t) == 4)
    {
        return (dataSize) % 4 + 4 - (usedSize % 4);
    }

    size_t padding1 = dataSize % 8;
    if (padding1 >= 4)
    {
        return (dataSize % 4 + 4 - (usedSize % 4)) % 8;
    }

    return (padding1 + 8 - (usedSize % 8)) % 8;
}

template<size_t total>
void validateObjectPoolSize()
{
    size_t bucketSize = sizeof(int32_t);
    size_t usedSize   = (total + 7) >> 3;

    size_t padding = calculatePadding(total * bucketSize, usedSize);

    size_t poolSize = sizeof(::estd::declare::object_pool<int, total>);

    if (sizeof(size_t) == 4)
    {
        ASSERT_EQ(16U, sizeof(::estd::object_pool<int32_t>));
        ASSERT_EQ(total * bucketSize + usedSize + 16 + padding, poolSize)
            << "32bit:" << total << ":" << padding;
    }
    else
    {
        ASSERT_EQ(32U, sizeof(::estd::object_pool<int32_t>));
        ASSERT_EQ(total * bucketSize + usedSize + 32 + padding, poolSize)
            << "64bit:" << total << ":" << padding;
    }
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies the memory consumption of a object_pool.
 */
TEST(ObjectPool, TestSizeOf)
{
    validateObjectPoolSize<10>();
    validateObjectPoolSize<5>();
    validateObjectPoolSize<1>();
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that a default constructed object_pool is full and no
 * constructor of any element has been called.
 */
TEST(ObjectPool, Constructor)
{
    ::internal::TestClassCalls::reset();
    MAKE_POOL(::internal::TestClassCalls, 10, p);
    ::internal::TestClassCalls::verify(0, 0, 0, 0, 0, 0);
    ASSERT_TRUE(p.full());
    ASSERT_EQ(10U, p.size());
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that the maximum size of an object_pool is the capacity
 * passed as template parameter to the declare::object_pool.
 */
TEST(ObjectPool, MaxSize)
{
    MAKE_POOL(::internal::TestClassCalls, 10, p);
    ASSERT_EQ(10U, p.max_size());
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that acquiring capacity number of elements from an object_pool
 * will invoke the same number of default constructors.
 */
TEST(ObjectPool, Acquire)
{
    using ::internal::TestClassCalls;
    TestClassCalls::reset();
    MAKE_POOL(TestClassCalls, 10, p);
    TestClassCalls& o = p.acquire();
    TestClassCalls::verify(1, 0, 0, 0, 0, 0);
    ASSERT_EQ(9U, p.size());
    while (p.size() > 0)
    {
        p.acquire();
    }
    o.verify(10, 0, 0, 0, 0, 0);
    EXPECT_TRUE(p.empty());
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that acquiring from an empty object_pool will assert.
 */
TEST(ObjectPool, AcquireFromEmptyPoolWillAssert)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    using ::internal::TestClassCalls;
    TestClassCalls::reset();
    MAKE_POOL(TestClassCalls, 10, p);
    TestClassCalls& o = p.acquire();
    while (p.size() > 0)
    {
        p.acquire();
    }
    o.verify(10, 0, 0, 0, 0, 0);
    EXPECT_TRUE(p.empty());
    ASSERT_THROW({ p.acquire(); }, ::estd::assert_exception);
    TestClassCalls::verify(10, 0, 0, 0, 0, 0);
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that capacity elements from an object_pool can be allocated.
 */
TEST(ObjectPool, Allocate)
{
    using ::internal::TestClassCalls;
    TestClassCalls::reset();
    MAKE_POOL(TestClassCalls, 10, p);
    ::estd::constructor<TestClassCalls> c = p.allocate();
    TestClassCalls::verify(0, 0, 0, 0, 0, 0);
    ASSERT_EQ(9U, p.size());
    TestClassCalls& t(c.construct());
    t.verify(1, 0, 0, 0, 0, 0);
    while (p.size() > 0)
    {
        p.allocate().construct();
    }
    t.verify(10, 0, 0, 0, 0, 0);
    EXPECT_TRUE(p.empty());
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that allocating from an empty object_pool will assert.
 */
TEST(ObjectPool, AllocateTooMany)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    using ::internal::TestClassCalls;
    TestClassCalls::reset();
    MAKE_POOL(TestClassCalls, 1, p);
    p.allocate().release();
    TestClassCalls::verify(0, 0, 0, 0, 0, 0);
    ASSERT_EQ(0U, p.size());
    ASSERT_TRUE(p.empty());
    ASSERT_THROW({ p.allocate(); }, ::estd::assert_exception);
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that releasing objects to the pool will call their destructor
 * and increase the number of available objects of the pool.
 */
TEST(ObjectPool, Release)
{
    using ::internal::TestClassCalls;
    TestClassCalls::reset();
    MAKE_POOL(TestClassCalls, 10, p);
    TestClassCalls& tmp = p.acquire();
    ASSERT_EQ(9U, p.size());
    p.release(tmp);
    ASSERT_EQ(10U, p.size());
    TestClassCalls::verify(1, 0, 0, 1, 0, 0);
    // now acquire all elements and release them again
    TestClassCalls::reset();
    TestClassCalls* acquired[10];
    int32_t i = 0;
    while (p.size() > 0)
    {
        acquired[i++] = &p.acquire();
    }
    TestClassCalls::verify(10, 0, 0, 0, 0, 0);

    while (p.size() < p.max_size())
    {
        p.release(*acquired[p.size()]);
    }

    TestClassCalls::verify(10, 0, 0, 10, 0, 0);
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that releasing an object which is not from the pool will have no effect.
 */
TEST(ObjectPool, ReleaseDoesNotCallDestructorIfItDoesNotContainReleasedElement)
{
    using ::internal::TestClassCalls;
    TestClassCalls::reset();
    MAKE_POOL(TestClassCalls, 10, p);
    TestClassCalls tmp;
    p.release(tmp);
    TestClassCalls::verify(1, 0, 0, 0, 0, 0);
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that releasing an already released object has no effect.
 */
TEST(ObjectPool, ReleaseDoesNotCallDestructorIfReleaseAlreadyCalled)
{
    using ::internal::TestClassCalls;
    TestClassCalls::reset();
    MAKE_POOL(TestClassCalls, 10, p);
    TestClassCalls& tmp = p.acquire();
    p.release(tmp);
    EXPECT_TRUE(TestClassCalls::verify(1, 0, 0, 1, 0, 0));

    // shouldn't call destructor
    p.release(tmp);
    EXPECT_TRUE(TestClassCalls::verify(1, 0, 0, 1, 0, 0));
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that contains() will report true for all objects that
 * have been acquired from a pool.
 */
TEST(ObjectPool, ContainsAllAcquiredElements)
{
    using ::internal::TestClassCalls;
    MAKE_POOL(TestClassCalls, 10, p);
    while (!p.empty())
    {
        TestClassCalls& o = p.acquire();
        ASSERT_TRUE(p.contains(o));
    }
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that contains() will report false for an object that
 * has not been allocated from the pool.
 */
TEST(ObjectPool, DoesNotContainExternalElement)
{
    using ::internal::TestClassCalls;
    MAKE_POOL(TestClassCalls, 10, p);
    TestClassCalls tmp;
    ASSERT_FALSE(p.contains(tmp));
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that contains() will only report true for the pool
 * from which it has been acquired.
 */
TEST(ObjectPool, ContainsItemInOtherPool)
{
    ::estd::declare::object_pool<int32_t, 10> a;
    ::estd::declare::object_pool<int32_t, 10> b;
    ::estd::declare::object_pool<int32_t, 10> c;

    int32_t& bItem = b.acquire();
    ASSERT_FALSE(a.contains(bItem));
    ASSERT_TRUE(b.contains(bItem));
    ASSERT_FALSE(c.contains(bItem));
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that an object_pool reports empty after capacity
 * objects have been acquired.
 */
TEST(ObjectPool, Empty)
{
    using ::internal::TestClassCalls;
    TestClassCalls::reset();
    MAKE_POOL(TestClassCalls, 10, p);
    for (int32_t i = 0; i < 10; ++i)
    {
        TestClassCalls& o = p.acquire();
        o.verify(i + 1, 0, 0, 0, 0, 0);
    }
    ASSERT_TRUE(p.empty());
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Veriefies that calling clear() on an empty object_pool doesn't
 * have any effect.
 */
TEST(ObjectPool, ClearEmpty)
{
    using ::internal::TestClassCalls;
    TestClassCalls::reset();
    MAKE_POOL(TestClassCalls, 10, p);

    // make sure that no destructors are called if nothing has been allocated
    p.clear();
    TestClassCalls::verify(0, 0, 0, 0, 0, 0);
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that calling clear() will destroy the number of
 * currently allocated objects.
 */
TEST(ObjectPool, ClearSeveral)
{
    using ::internal::TestClassCalls;
    MAKE_POOL(TestClassCalls, 10, p);

    TestClassCalls& a = p.acquire();
    p.acquire();
    p.acquire();

    p.release(a);
    TestClassCalls::reset();
    // 2 elements remain -> expect 2 destructor calls
    p.clear();
    ASSERT_TRUE(TestClassCalls::verify(0, 0, 0, 2, 0, 0));
    ASSERT_EQ(10U, p.size());
}

/**
 * \refs: SMD_estd_objectPool
 * \desc
 * Verifies that calling clear after all elements have been allocated
 * allows to allocate all elements again.
 */
TEST(ObjectPool, AllocateClearAllocate)
{
    ::estd::declare::object_pool<int, 5> pool;
    EXPECT_TRUE(pool.full());
    for (int i = 0; i < 5; ++i)
    {
        ::estd::constructor<int> c = pool.allocate();
        ASSERT_TRUE(c.has_memory());
    }
    EXPECT_TRUE(pool.empty());
    pool.clear();
    EXPECT_TRUE(pool.full());
    for (int i = 0; i < 5; ++i)
    {
        ::estd::constructor<int> c = pool.allocate();
        ASSERT_TRUE(c.has_memory());
    }
}

/**
 * \refs: SMD_estd_objectPoolIterator
 * \desc
 * Verifies that a default constructed iterator is equal to an end iterator.
 */
TEST(ObjectPool, DefaultConstructedIteratorEqualsEnd)
{
    ::estd::declare::object_pool<int, 5>::iterator itr{};
    ::estd::declare::object_pool<int, 5> pool{};
    EXPECT_EQ(itr, pool.end());
}

/**
 * \refs: SMD_estd_object_const_pool_iterator
 * \desc
 * Verifies that a default constructed iterator is equal to an end iterator.
 */
TEST(ObjectPool, DefaultConstructedConstIteratorEqualsEnd)
{
    ::estd::declare::object_pool<int, 5>::const_iterator itr{};
    ::estd::declare::object_pool<int, 5> pool{};
    EXPECT_EQ(itr, pool.end());
    EXPECT_EQ(itr, pool.cend());
}

/**
 * \refs: SMD_estd_objectPoolIterator, SMD_estd_object_const_pool_iterator
 * \desc
 * Verifies that begin() and end() are equal if no element has been acquired from a pool.
 */
TEST(ObjectPool, BeginEqualsEndOnFullObjectPool)
{
    ::estd::declare::object_pool<int, 5> pool{};
    // Test for iterator
    auto const itr   = pool.begin();
    auto const e_itr = pool.end();
    EXPECT_EQ(itr, e_itr);
    // Test for const_iterator
    auto const c_itr   = pool.cbegin();
    auto const e_c_itr = pool.cend();
    EXPECT_EQ(c_itr, e_c_itr);
}

/**
 * \refs: SMD_estd_objectPoolIterator, SMD_estd_object_const_pool_iterator
 * \desc
 * Verifies that begin() returns the first element of the internal data, which is not
 * necessarily the first acquired one.
 *
 * This test is based on the knowledge of the internal behavior and demonstrates that
 * no assumptions about the order of allocations and the order of the iterators can
 * be made.
 */
TEST(ObjectPool, BeginPointsToFirstUsedElement)
{
    ::estd::declare::object_pool<int, 5> pool{};
    // acquire 2 elements
    auto& i1 = pool.allocate().construct(17);
    pool.allocate().construct(23);
    EXPECT_EQ(17, *pool.begin());
    EXPECT_EQ(17, *pool.cbegin());
    // release the first one
    pool.release(i1);
    EXPECT_EQ(23, *pool.begin());
    EXPECT_EQ(23, *pool.cbegin());
    // allocate a third element
    pool.allocate().construct(29);
    // now begin() will NOT point to the earlier allocated element 23
    EXPECT_EQ(29, *pool.begin());
    EXPECT_EQ(29, *pool.cbegin());
}

/**
 * \refs: SMD_estd_objectPoolIterator, SMD_estd_object_const_pool_iterator
 * \desc
 * Verifies that iterating over the range from begin() to end() iterates over all
 * acquired elements of the object_pool.
 */
TEST(ObjectPool, BeginToEndAreRangeOfAcquiredElements)
{
    ::estd::declare::object_pool<int, 5> pool{};
    pool.allocate().construct(17);
    pool.allocate().construct(23);
    pool.allocate().construct(29);
    // Test for iterator
    auto itr = pool.begin();
    EXPECT_EQ(17, *itr);
    EXPECT_EQ(23, *++itr);
    EXPECT_EQ(23, *itr++);
    EXPECT_EQ(29, *itr);
    // Test for const_iterator
    auto c_itr = pool.cbegin();
    EXPECT_EQ(17, *c_itr);
    EXPECT_EQ(23, *++c_itr);
    EXPECT_EQ(23, *c_itr++);
    EXPECT_EQ(29, *c_itr);
    // This also tests const_iterator
    int values[3] = {17, 23, 29};
    EXPECT_THAT(pool, ElementsAreArray(values));
}

/**
 * \refs: SMD_estd_objectPoolIterator, SMD_estd_object_const_pool_iterator
 * \desc
 * Verifies that a const_iterator can be constructed from an iterator.
 */
TEST(ObjectPool, ConstIteratorCanBeCreatedFromIterator)
{
    ::estd::declare::object_pool<int, 5> pool{};
    pool.allocate().construct(17);
    auto itr = pool.begin();
    decltype(pool)::const_iterator c_itr{itr};
    EXPECT_EQ(pool.cbegin(), c_itr);
}

/**
 * \refs: SMD_estd_objectPoolIterator, SMD_estd_object_const_pool_iterator
 * \desc
 * Verifies that operator*() and operator->() of iterators point to the same value.
 */
TEST(ObjectPool, IndirectionOperatorAndStructureDereferenceOperatorReturnSameValue)
{
    ::estd::declare::object_pool<int, 3> pool{};
    pool.allocate().construct(17);
    pool.allocate().construct(23);
    pool.allocate().construct(29);
    // Test for iterator
    auto itr = pool.begin();
    EXPECT_EQ(17, *itr);
    EXPECT_EQ(17, *itr.operator->());
    ++itr;
    EXPECT_EQ(23, *itr);
    EXPECT_EQ(23, *itr.operator->());
    ++itr;
    EXPECT_EQ(29, *itr);
    EXPECT_EQ(29, *itr.operator->());
    // Test for const_iterator
    auto c_itr = pool.cbegin();
    EXPECT_EQ(17, *c_itr);
    EXPECT_EQ(17, *c_itr.operator->());
    ++c_itr;
    EXPECT_EQ(23, *c_itr);
    EXPECT_EQ(23, *c_itr.operator->());
    ++c_itr;
    EXPECT_EQ(29, *c_itr);
    EXPECT_EQ(29, *c_itr.operator->());
}

} // namespace
