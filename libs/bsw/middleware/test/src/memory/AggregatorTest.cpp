// Copyright 2025 BMW AG

#include <cstdint>

#include <etl/delegate.h>
#include <gtest/gtest.h>

#include "middleware/memory/Aggregator.h"

namespace middleware
{

uint8_t volatile message_allocator_mutex{0U};

namespace memory
{

using SimpleAllocator = Aggregator<Pool<2U, sizeof(int32_t)>>;

using SingleByteAllocator = Aggregator<Pool<1U, 1U>>;

using MessageAllocator = Aggregator<
    Pool<200U, 20U>,
    Pool<200U, 40U>,
    Pool<200U, 80U>,
    Pool<100U, 100U>,
    Pool<50U, 200U>,
    Pool<10U, 4096U + 2U + 2U>>;

template<>
SimpleAllocator* SimpleAllocator::_instance = nullptr;
template<>
SingleByteAllocator* SingleByteAllocator::_instance = nullptr;
template<>
MessageAllocator* MessageAllocator::_instance = nullptr;

namespace test
{

class AggregatorTest : public testing::Test
{
public:
    AggregatorTest()
    : _simpleAllocator(&middleware::message_allocator_mutex)
    , _singleByteAllocator(&middleware::message_allocator_mutex)
    , _messageAllocator(&middleware::message_allocator_mutex)
    {}

    void collector(uint64_t, memory::PoolStats stats)
    {
        _failedAllocations     = stats.failedAllocations;
        _successfulAllocations = stats.successfulAllocations;
    }

protected:
    void SetUp() override
    {
        SimpleAllocator::initialize(_simpleAllocator);
        SingleByteAllocator::initialize(_singleByteAllocator);
        MessageAllocator::initialize(_messageAllocator);

        _failedAllocations     = 0U;
        _successfulAllocations = 0U;
    }

    void TearDown() override {}

    SimpleAllocator _simpleAllocator;
    SingleByteAllocator _singleByteAllocator;
    MessageAllocator _messageAllocator;

    uint32_t _failedAllocations{0};
    uint32_t _successfulAllocations{0};
};

TEST_F(AggregatorTest, SingleByteAllocator)
{
    // ARRANGE
    uint32_t const chunkSize = sizeof(int32_t);
    auto const collectorFcn  = etl::delegate<void(
        uint64_t, memory::PoolStats)>::create<AggregatorTest, &AggregatorTest::collector>(*this);
    auto* pool = reinterpret_cast<typename SingleByteAllocator::PoolByType<int32_t*>::type*>(
        SingleByteAllocator::getPool<chunkSize>());
    EXPECT_TRUE(static_cast<bool>(pool));
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 1U);

    // ACT AND ASSERT
    EXPECT_EQ(_singleByteAllocator.allocateImpl(sizeof(int32_t)), nullptr);
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 1U);

    auto* secondIntPtr = static_cast<uint8_t*>(_singleByteAllocator.allocateImpl(sizeof(uint8_t)));
    *secondIntPtr      = 9U;
    EXPECT_EQ(*secondIntPtr, 9);
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 0U);

    SingleByteAllocator::collectStats(collectorFcn);
    EXPECT_EQ(_successfulAllocations, 1U);
    EXPECT_EQ(_failedAllocations, 0U);

    EXPECT_EQ(_singleByteAllocator.allocateImpl(sizeof(uint8_t)), nullptr);
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 0U);

    SingleByteAllocator::collectStats(collectorFcn);
    EXPECT_EQ(_successfulAllocations, 0U);
    EXPECT_EQ(_failedAllocations, 1U);

    EXPECT_EQ(_singleByteAllocator.allocateImpl(0U), nullptr);

    EXPECT_TRUE(static_cast<bool>(secondIntPtr));

    _singleByteAllocator.deallocateImpl(secondIntPtr);
    _singleByteAllocator.deallocateImpl(secondIntPtr);
}

TEST_F(AggregatorTest, SingleByteAllocatorMultipleAllocations)
{
    // ARRANGE
    uint32_t const chunkSize = sizeof(int32_t);
    uintptr_t tmp            = 0U;

    for (unsigned i = 0U; i < 50U; ++i)
    {
        auto* pool = reinterpret_cast<typename SingleByteAllocator::PoolByType<int32_t*>::type*>(
            SingleByteAllocator::getPool<chunkSize>());
        EXPECT_TRUE(static_cast<bool>(pool));
        EXPECT_EQ(etl::get<0>(pool->getProfile()), 1U);

        EXPECT_EQ(_singleByteAllocator.allocateImpl(sizeof(int32_t)), nullptr);
        EXPECT_EQ(etl::get<0>(pool->getProfile()), 1U);

        auto* secondIntPtr
            = static_cast<uint8_t*>(_singleByteAllocator.allocateImpl(sizeof(uint8_t)));
        if (i == 0U)
        {
            tmp = reinterpret_cast<uintptr_t>(secondIntPtr);
        }
        else
        {
            EXPECT_TRUE(tmp == reinterpret_cast<uintptr_t>(secondIntPtr));
        }

        *secondIntPtr = 9U;
        EXPECT_EQ(*secondIntPtr, 9);
        EXPECT_EQ(etl::get<0>(pool->getProfile()), 0U);

        EXPECT_EQ(_singleByteAllocator.allocateImpl(sizeof(uint8_t)), nullptr);
        EXPECT_EQ(etl::get<0>(pool->getProfile()), 0U);

        EXPECT_TRUE(static_cast<bool>(secondIntPtr));

        _singleByteAllocator.deallocateImpl(secondIntPtr);
    }
}

TEST_F(AggregatorTest, SimpleAllocator)
{
    // ARRANGE
    uint32_t const chunkSize = sizeof(int32_t);
    auto* pool = reinterpret_cast<typename SimpleAllocator::PoolByType<int32_t*>::type*>(
        SimpleAllocator::getPool<chunkSize>());
    EXPECT_TRUE(static_cast<bool>(pool));
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 2U);

    // ACT AND ASSERT
    auto* intPtr
        = static_cast<int32_t*>(static_cast<void*>(_simpleAllocator.allocateImpl(sizeof(int32_t))));
    *intPtr = 5;
    EXPECT_EQ(*intPtr, 5);
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 1U);

    auto* secondIntPtr
        = static_cast<int32_t*>(static_cast<void*>(_simpleAllocator.allocateImpl(sizeof(int32_t))));
    *secondIntPtr = 9;
    EXPECT_EQ(*secondIntPtr, 9);
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 0U);

    EXPECT_EQ(_simpleAllocator.allocateImpl(sizeof(int32_t)), nullptr);
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 0U);

    EXPECT_EQ(_simpleAllocator.allocateImpl(0U), nullptr);

    EXPECT_TRUE(static_cast<bool>(intPtr));
    EXPECT_TRUE(static_cast<bool>(secondIntPtr));
}

TEST_F(AggregatorTest, InvalidDeallocations)
{
    // ARRANGE
    uint32_t const chunkSize = sizeof(int32_t);
    auto* pool = reinterpret_cast<typename SimpleAllocator::PoolByType<int32_t*>::type*>(
        SimpleAllocator::getPool<chunkSize>());
    EXPECT_TRUE(static_cast<bool>(pool));
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 2U);

    // ACT AND ASSERT
    auto* intPtr
        = static_cast<int32_t*>(static_cast<void*>(_simpleAllocator.allocateImpl(sizeof(int32_t))));
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 1U);
    EXPECT_NE(intPtr, nullptr);

    auto* intPtr_2
        = static_cast<int32_t*>(static_cast<void*>(_simpleAllocator.allocateImpl(sizeof(int32_t))));
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 0U);
    EXPECT_NE(intPtr_2, nullptr);

    auto* intPtrNull
        = static_cast<int32_t*>(static_cast<void*>(_simpleAllocator.allocateImpl(sizeof(int32_t))));
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 0U);
    EXPECT_EQ(intPtrNull, nullptr);

    _simpleAllocator.deallocateImpl(reinterpret_cast<void*>(intPtr));
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 1U);
    EXPECT_FALSE(_simpleAllocator.isPtrValidImpl(reinterpret_cast<void*>(intPtr)));

    EXPECT_FALSE(_simpleAllocator.isPtrValidImpl(nullptr));
    EXPECT_FALSE(_singleByteAllocator.isPtrValidImpl(nullptr));
    EXPECT_FALSE(_singleByteAllocator.isPtrValidImpl(reinterpret_cast<void*>(intPtr_2)));
    auto* simplePool = reinterpret_cast<typename SimpleAllocator::PoolByType<void*>::type*>(
        SimpleAllocator::getPool<chunkSize>());
    auto* poolBegin = simplePool->allocate(chunkSize);
    EXPECT_FALSE(_singleByteAllocator.isPtrValidImpl(
        reinterpret_cast<void*>(reinterpret_cast<size_t>(poolBegin) - 1U)));
}

TEST_F(AggregatorTest, AllocateInLargerChunkIfNecessary)
{
    // ARRANGE
    size_t const len = sizeof(etl::array<uint8_t, 40>);

    auto* p = reinterpret_cast<memory::Pool<200, 40>*>(MessageAllocator::instance().getPool<len>());
    auto* p2
        = reinterpret_cast<memory::Pool<200, 80>*>(MessageAllocator::instance().getPool<len + 1>());
    EXPECT_NE(reinterpret_cast<void*>(p), reinterpret_cast<void*>(p2));

    // ACT AND ASSERT
    for (int i = 0; i < 200; i++)
    {
        EXPECT_NE(_messageAllocator.allocateImpl(len), nullptr);
    }
    EXPECT_EQ(etl::get<0>(p->getProfile()), 0U);
    EXPECT_EQ(etl::get<0>(p2->getProfile()), 200U);

    EXPECT_NE(_messageAllocator.allocateImpl(len), nullptr);
    EXPECT_EQ(etl::get<0>(p->getProfile()), 0U);
    EXPECT_EQ(etl::get<0>(p2->getProfile()), 199U);

    EXPECT_EQ(p->getPoolStats().delegatedAllocations, 1U);
    EXPECT_EQ(p2->getPoolStats().delegatedAllocations, 0U);

    auto* pSmaller = reinterpret_cast<memory::Pool<200, 20>*>(
        MessageAllocator::instance().getPool<sizeof(etl::array<uint8_t, 20>)>());
    EXPECT_EQ(pSmaller->getPoolStats().delegatedAllocations, 0U);
}

TEST_F(AggregatorTest, FailedAllocationStats)
{
    // ARRANGE
    using Pool100T  = memory::Pool<100, 100>;
    using Pool200T  = memory::Pool<50, 200>;
    using Pool4010T = memory::Pool<10, 4096 + 2 + 2>;

    size_t const len = sizeof(etl::array<uint8_t, 200>);

    Pool100T* pool100 = reinterpret_cast<Pool100T*>(
        MessageAllocator::instance().getPool<sizeof(etl::array<uint8_t, 100>)>());
    Pool200T* pool200   = reinterpret_cast<Pool200T*>(MessageAllocator::instance().getPool<len>());
    Pool4010T* pool4010 = reinterpret_cast<Pool4010T*>(
        MessageAllocator::instance().getPool<sizeof(etl::array<uint8_t, 4096 + 2 + 2>)>());

    // ACT AND ASSERT
    for (int i = 0; i < 50; i++)
    {
        EXPECT_NE(_messageAllocator.allocateImpl(len), nullptr);
    }
    EXPECT_EQ(etl::get<0>(pool200->getProfile()), 0U);
    EXPECT_EQ(etl::get<0>(pool4010->getProfile()), 10U);

    EXPECT_EQ(pool100->getPoolStats().delegatedAllocations, 0U);
    EXPECT_EQ(pool200->getPoolStats().delegatedAllocations, 0U);
    EXPECT_EQ(pool4010->getPoolStats().delegatedAllocations, 0U);

    for (int i = 0; i < 10; i++)
    {
        EXPECT_NE(_messageAllocator.allocateImpl(len), nullptr);
    }
    EXPECT_EQ(etl::get<0>(pool200->getProfile()), 0U);
    EXPECT_EQ(etl::get<0>(pool4010->getProfile()), 0U);

    EXPECT_EQ(pool100->getPoolStats().delegatedAllocations, 0U);
    EXPECT_EQ(pool200->getPoolStats().delegatedAllocations, 10U);
    EXPECT_EQ(pool4010->getPoolStats().delegatedAllocations, 0U);

    EXPECT_EQ(_messageAllocator.allocateImpl(len), nullptr);

    EXPECT_EQ(pool100->getPoolStats().delegatedAllocations, 0U);
    EXPECT_EQ(pool200->getPoolStats().delegatedAllocations, 10U);
    EXPECT_EQ(pool4010->getPoolStats().delegatedAllocations, 0U);

    EXPECT_EQ(pool100->getPoolStats().failedAllocations, 0U);
    EXPECT_EQ(pool200->getPoolStats().failedAllocations, 1U);
    EXPECT_EQ(pool4010->getPoolStats().failedAllocations, 0U);
}

TEST_F(AggregatorTest, InvalidDeallocationsMessageAllocator)
{
    // ARRANGE
    using BadArgs  = etl::array<uint8_t, 4101>;
    using GoodArgs = etl::array<uint8_t, 4096>;

    // ACT AND ASSERT
    EXPECT_EQ(_messageAllocator.allocateImpl(sizeof(BadArgs)), nullptr);
    auto* ptr = _messageAllocator.allocateImpl(sizeof(GoodArgs));
    EXPECT_NE(ptr, nullptr);
    EXPECT_FALSE(_messageAllocator.isPtrValidImpl(
        reinterpret_cast<void*>(reinterpret_cast<size_t>(ptr) + 1U)));
    _messageAllocator.deallocateImpl(ptr);
}

TEST_F(AggregatorTest, TwistedDeallocation)
{
    // ARRANGE
    uint32_t const chunkSize = sizeof(int32_t);
    auto* pool               = reinterpret_cast<typename SimpleAllocator::PoolByType<void*>::type*>(
        SimpleAllocator::getPool<chunkSize>());
    EXPECT_TRUE(static_cast<bool>(pool));
    EXPECT_EQ(etl::get<0>(pool->getProfile()), 2U);

    // ACT AND ASSERT
    auto* intPtr = _simpleAllocator.allocateImpl(sizeof(int32_t));
    EXPECT_FALSE(_simpleAllocator.isPtrValidImpl(
        reinterpret_cast<void*>(reinterpret_cast<size_t>(intPtr) + 1U)));
}

TEST_F(AggregatorTest, WhenASmallerPoolIsFullStatisticsAreUpdatedForRelevantPoolAndNotThatOne)
{
    // ARRANGE
    size_t const numOfSmallerSizePools = 100;
    size_t const numOfGoodFitSizePools = 50;
    size_t const numBiggerSizePools    = 10;

    size_t const smallerSize = 100U;
    size_t const goodFitSize = 200U;

    auto* const pool200 = MessageAllocator::instance().getPool<goodFitSize>();

    for (size_t i = 0; i < numOfSmallerSizePools; i++)
    {
        _messageAllocator.allocateImpl(smallerSize);
    }

    // ACT
    _messageAllocator.allocateImpl(goodFitSize);
    size_t const firstSuccessfulAllocation = pool200->getPoolStats().successfulAllocations;

    for (size_t i = 0; i < numOfGoodFitSizePools; i++)
    {
        _messageAllocator.allocateImpl(goodFitSize);
    }
    size_t const firstDelegatedAllocation = pool200->getPoolStats().delegatedAllocations;

    for (size_t i = 0; i < numBiggerSizePools; i++)
    {
        _messageAllocator.allocateImpl(goodFitSize);
    }
    size_t const firstFailedAllocation = pool200->getPoolStats().failedAllocations;

    // ASSERT
    EXPECT_EQ(firstSuccessfulAllocation, 1U);
    EXPECT_EQ(firstDelegatedAllocation, 1U);
    EXPECT_EQ(firstFailedAllocation, 1U);
}

} // namespace test
} // namespace memory
} // namespace middleware
