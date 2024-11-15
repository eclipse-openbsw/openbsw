// Copyright 2024 Accenture.

#include "util/memory/BuddyMemoryManager.h"

#include <etl/vector.h>

#include <gtest/gtest.h>

static size_t const NUM_BUCKETS = 8U;

using ::util::memory::declare::BuddyMemoryManager;
using MyBuddyMemoryManager = BuddyMemoryManager<NUM_BUCKETS>;
using Bucket               = MyBuddyMemoryManager::AcquireResult;

namespace util
{
namespace memory
{
bool operator==(Bucket const& left, Bucket const& right)
{
    return (left.firstBucketIndex() == right.firstBucketIndex())
           && (left.numBuckets() == right.numBuckets());
}
} // namespace memory
} // namespace util

class BuddyMemoryManagerTest : public ::testing::Test
{
protected:
    void SetUp() override { _memoryManager.clear(); }

    MyBuddyMemoryManager _memoryManager;
};

TEST_F(BuddyMemoryManagerTest, Constants)
{
    EXPECT_EQ(1U, BuddyMemoryManager<1U>().numBuckets());
    EXPECT_EQ(2U, BuddyMemoryManager<2U>().numBuckets());
    EXPECT_EQ(4U, BuddyMemoryManager<3U>().numBuckets());
    EXPECT_EQ(4U, BuddyMemoryManager<4U>().numBuckets());
    EXPECT_EQ(8U, BuddyMemoryManager<5U>().numBuckets());
    EXPECT_EQ(8U, BuddyMemoryManager<6U>().numBuckets());
    EXPECT_EQ(8U, BuddyMemoryManager<7U>().numBuckets());
    EXPECT_EQ(8U, BuddyMemoryManager<8U>().numBuckets());
    EXPECT_EQ(16U, BuddyMemoryManager<9U>().numBuckets());
    EXPECT_EQ(16U, BuddyMemoryManager<15U>().numBuckets());
    EXPECT_EQ(4096U, BuddyMemoryManager<3000U>().numBuckets());
}

TEST_F(BuddyMemoryManagerTest, Init)
{
    EXPECT_EQ(8U, _memoryManager.numBuckets());
    EXPECT_TRUE(_memoryManager.isEmpty());
}

TEST_F(BuddyMemoryManagerTest, RequestZeroMemory)
{
    EXPECT_EQ(Bucket(0U, 1U), _memoryManager.acquireMemory(0U));
    EXPECT_FALSE(_memoryManager.isEmpty());
}

TEST_F(BuddyMemoryManagerTest, RequestFullMemory)
{
    EXPECT_EQ(Bucket(0U, 8U), _memoryManager.acquireMemory(8U));
    EXPECT_FALSE(_memoryManager.isEmpty());
}

TEST_F(BuddyMemoryManagerTest, RequestFullMemorySeveralTimes)
{
    EXPECT_EQ(Bucket(0U, 8U), _memoryManager.acquireMemory(8U));
    EXPECT_FALSE(_memoryManager.isEmpty());

    for (size_t i = 0U; i < 10U; i++)
    {
        EXPECT_FALSE(_memoryManager.acquireMemory(0U).isValid());
    }
}

TEST_F(BuddyMemoryManagerTest, RequestInvalidLevel)
{
    EXPECT_FALSE(_memoryManager.acquireMemory(9U).isValid());
    EXPECT_TRUE(_memoryManager.isEmpty());
}

TEST_F(BuddyMemoryManagerTest, RequestMinimumChunk)
{
    EXPECT_EQ(Bucket(0U, 1U), _memoryManager.acquireMemory(1U));
}

TEST_F(BuddyMemoryManagerTest, RequestAndReleaseFullMemory)
{
    EXPECT_EQ(Bucket(0U, 8U), _memoryManager.acquireMemory(8U));
    EXPECT_EQ(8U, _memoryManager.releaseMemoryExtended(0U));

    EXPECT_TRUE(_memoryManager.isEmpty());
}

TEST_F(BuddyMemoryManagerTest, RequestAndReleaseMinChunk)
{
    EXPECT_EQ(Bucket(0U, 1U), _memoryManager.acquireMemory(1U));
    EXPECT_EQ(1U, _memoryManager.releaseMemoryExtended(0U));

    EXPECT_TRUE(_memoryManager.isEmpty());
}

TEST_F(BuddyMemoryManagerTest, RequestAndReleaseHalfMemory)
{
    EXPECT_EQ(Bucket(0U, 4U), _memoryManager.acquireMemory(4U));
    EXPECT_EQ(4U, _memoryManager.releaseMemoryExtended(0U));

    EXPECT_TRUE(_memoryManager.isEmpty());
}

TEST_F(BuddyMemoryManagerTest, RequestAndReleaseAllMemoryWithRounding)
{
    EXPECT_EQ(Bucket(0U, 8U), _memoryManager.acquireMemory(5U));
    EXPECT_EQ(8U, _memoryManager.releaseMemoryExtended(0U));

    EXPECT_TRUE(_memoryManager.isEmpty());
}

TEST_F(BuddyMemoryManagerTest, RequestAndRelease2TimesMinChunk)
{
    EXPECT_EQ(Bucket(0U, 1U), _memoryManager.acquireMemory(1U));
    EXPECT_EQ(Bucket(1U, 1U), _memoryManager.acquireMemory(1U));
    EXPECT_EQ(1U, _memoryManager.releaseMemoryExtended(0U));
    EXPECT_EQ(1U, _memoryManager.releaseMemoryExtended(1U));

    EXPECT_TRUE(_memoryManager.isEmpty());
}

TEST_F(BuddyMemoryManagerTest, Request2TimesMinChunkAndReleaseInReverseOrder)
{
    EXPECT_EQ(Bucket(0U, 1U), _memoryManager.acquireMemory(1U));
    EXPECT_EQ(Bucket(1U, 1U), _memoryManager.acquireMemory(1U));
    EXPECT_EQ(1U, _memoryManager.releaseMemoryExtended(1U));
    EXPECT_EQ(1U, _memoryManager.releaseMemoryExtended(0U));

    EXPECT_TRUE(_memoryManager.isEmpty());
}

TEST_F(BuddyMemoryManagerTest, RequestAndReleaseAllMemoryInDifferentChunks)
{
    EXPECT_EQ(Bucket(0U, 4U), _memoryManager.acquireMemory(4U));
    EXPECT_EQ(Bucket(4U, 2U), _memoryManager.acquireMemory(2U));
    EXPECT_EQ(Bucket(6U, 1U), _memoryManager.acquireMemory(1U));
    EXPECT_EQ(Bucket(7U, 1U), _memoryManager.acquireMemory(1U));

    EXPECT_EQ(4U, _memoryManager.releaseMemoryExtended(0U));
    EXPECT_EQ(2U, _memoryManager.releaseMemoryExtended(4U));
    EXPECT_EQ(1U, _memoryManager.releaseMemoryExtended(6U));
    EXPECT_EQ(1U, _memoryManager.releaseMemoryExtended(7U));

    EXPECT_TRUE(_memoryManager.isEmpty());
}

TEST_F(BuddyMemoryManagerTest, AllocateOutOfOrder)
{
    EXPECT_EQ(Bucket(0U, 2U), _memoryManager.acquireMemory(2U));
    EXPECT_EQ(Bucket(4U, 4U), _memoryManager.acquireMemory(4U));
    EXPECT_EQ(Bucket(2U, 1U), _memoryManager.acquireMemory(1U));
    EXPECT_EQ(Bucket(3U, 1U), _memoryManager.acquireMemory(1U));

    EXPECT_EQ(4U, _memoryManager.releaseMemoryExtended(4U));
    EXPECT_EQ(2U, _memoryManager.releaseMemoryExtended(0U));
    EXPECT_EQ(1U, _memoryManager.releaseMemoryExtended(3U));
    EXPECT_EQ(1U, _memoryManager.releaseMemoryExtended(2U));

    EXPECT_TRUE(_memoryManager.isEmpty());
}

TEST_F(BuddyMemoryManagerTest, ReleaseMemoryOutsideManager)
{
    EXPECT_EQ(Bucket(0U, 8U), _memoryManager.acquireMemory(8U));

    EXPECT_EQ(0U, _memoryManager.releaseMemoryExtended(_memoryManager.numBuckets()));

    EXPECT_EQ(8U, _memoryManager.releaseMemoryExtended(0U));
    EXPECT_TRUE(_memoryManager.isEmpty());
}

TEST_F(BuddyMemoryManagerTest, ReleaseMemoryOutsideManagerWhenFull)
{
    for (size_t i = 0U; i < 8U; i++)
    {
        Bucket bucket = _memoryManager.acquireMemory(1U);
        EXPECT_EQ(Bucket(i, 1U), bucket);
    }

    EXPECT_EQ(0U, _memoryManager.releaseMemoryExtended(_memoryManager.numBuckets()));

    for (size_t i = 0U; i < _memoryManager.numBuckets(); i++)
    {
        EXPECT_EQ(1U, _memoryManager.releaseMemoryExtended(i));
    }

    EXPECT_TRUE(_memoryManager.isEmpty());
}

static uint32_t nextPowerOfTwo(uint32_t v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

TEST_F(BuddyMemoryManagerTest, AllBucketSizes)
{
    static size_t const SIZE = 256U;
    BuddyMemoryManager<SIZE> manager;

    for (size_t i = 1U; i <= SIZE; i++)
    {
        BuddyMemoryManager<SIZE>::AcquireResult rv = manager.acquireMemory(i);
        ASSERT_TRUE(rv.isValid());
        ASSERT_EQ(nextPowerOfTwo(i), rv.numBuckets());
        ASSERT_EQ(nextPowerOfTwo(i), manager.releaseMemoryExtended(rv.firstBucketIndex()));
        ASSERT_TRUE(manager.isEmpty());
    }
}

TEST_F(BuddyMemoryManagerTest, AcquireTillFullAndRelease)
{
    static size_t const SIZE = 256U;
    BuddyMemoryManager<SIZE> manager;

    size_t numAllocated  = 1U;
    size_t maxNumBuckets = SIZE;
    do
    {
        ASSERT_TRUE(manager.isEmpty());
        for (size_t i = 0U; i < maxNumBuckets; ++i)
        {
            auto rv = manager.acquireMemory(numAllocated);
            ASSERT_TRUE(rv.isValid());
            ASSERT_EQ(numAllocated, rv.numBuckets());
        }
        ASSERT_FALSE(manager.acquireMemory(numAllocated).isValid());
        for (size_t i = 0U; i < maxNumBuckets; ++i)
        {
            ASSERT_EQ(numAllocated, manager.releaseMemoryExtended(i * numAllocated));
        }
        numAllocated *= 2U;
        maxNumBuckets /= 2U;
    } while (maxNumBuckets != 1U);
}

TEST_F(BuddyMemoryManagerTest, AcquireTillFullAndReleaseReverse)
{
    static size_t const SIZE = 256U;
    BuddyMemoryManager<SIZE> manager;

    size_t numAllocated  = 1U;
    size_t maxNumBuckets = SIZE;
    do
    {
        ASSERT_TRUE(manager.isEmpty());
        for (size_t i = 0U; i < maxNumBuckets; ++i)
        {
            auto rv = manager.acquireMemory(numAllocated);
            ASSERT_TRUE(rv.isValid());
            ASSERT_EQ(numAllocated, rv.numBuckets());
        }
        ASSERT_FALSE(manager.acquireMemory(numAllocated).isValid());
        for (size_t i = (maxNumBuckets - 1U); i != 0U; --i)
        {
            ASSERT_EQ(numAllocated, manager.releaseMemoryExtended(i * numAllocated));
        }
        ASSERT_EQ(numAllocated, manager.releaseMemoryExtended(0U));
        numAllocated *= 2U;
        maxNumBuckets /= 2U;
    } while (maxNumBuckets != 1U);
}

TEST_F(BuddyMemoryManagerTest, AcquireHalfTreeAndRelease)
{
    static size_t const SIZE = 256U;
    BuddyMemoryManager<SIZE> manager;
    ::etl::vector<BuddyMemoryManager<SIZE>::AcquireResult, SIZE> results;

    BuddyMemoryManager<SIZE>::AcquireResult rv;
    for (size_t i = 1U; i <= (SIZE / 2U); i *= 2U)
    {
        rv = manager.acquireMemory(i);
        ASSERT_TRUE(rv.isValid());
        ASSERT_EQ(i, rv.numBuckets());
        results.push_back(rv);
    }
    rv = manager.acquireMemory(1U);
    results.push_back(rv);
    ASSERT_TRUE(rv.isValid());
    ASSERT_FALSE(manager.acquireMemory(1U).isValid());

    for (auto const& rv : results)
    {
        ASSERT_EQ(rv.numBuckets(), manager.releaseMemoryExtended(rv.firstBucketIndex()));
    }
    ASSERT_TRUE(manager.isEmpty());
    for (size_t i = 0U; i <= SIZE; i++)
    {
        ASSERT_EQ(0U, manager.releaseMemoryExtended(i));
    }
}

TEST_F(BuddyMemoryManagerTest, AcquireHalfTreeAndReleaseReverse)
{
    static size_t const SIZE = 256U;
    BuddyMemoryManager<SIZE> manager;
    ::etl::vector<BuddyMemoryManager<SIZE>::AcquireResult, SIZE> results;

    BuddyMemoryManager<SIZE>::AcquireResult rv;
    rv = manager.acquireMemory(1U);
    results.push_back(rv);
    ASSERT_TRUE(rv.isValid());
    for (size_t i = (SIZE / 2U); i != 0U; i /= 2U)
    {
        rv = manager.acquireMemory(i);
        ASSERT_TRUE(rv.isValid());
        ASSERT_EQ(i, rv.numBuckets());
        results.push_back(rv);
    }
    ASSERT_FALSE(manager.acquireMemory(1U).isValid());

    for (auto const& rv : results)
    {
        ASSERT_EQ(rv.numBuckets(), manager.releaseMemoryExtended(rv.firstBucketIndex()));
    }
    ASSERT_TRUE(manager.isEmpty());
    for (size_t i = 0U; i <= SIZE; i++)
    {
        ASSERT_EQ(0U, manager.releaseMemoryExtended(i));
    }
}
