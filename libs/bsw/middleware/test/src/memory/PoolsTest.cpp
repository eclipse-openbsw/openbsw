// Copyright 2025 BMW AG

#include <cstring>

#include <gmock/gmock.h>

#include "middleware/memory/Pool.h"

namespace middleware::memory::test
{

class TestPool : public ::testing::Test
{
public:
    static constexpr size_t CHUNK_SIZE         = 20U;
    static constexpr size_t CHUNK_COUNT        = 10U;
    static constexpr size_t ALIGNED_CHUNK_SIZE = Pool<CHUNK_COUNT, CHUNK_SIZE>::valueSize();

    virtual void SetUp() override { _testPool.initialize(); }

    Pool<CHUNK_COUNT, CHUNK_SIZE> _testPool{};
};

TEST_F(TestPool, WhenPayloadEqualSizeThenAllocateSuccessExpectValidPtr)
{
    uint8_t* ptr = _testPool.allocate(CHUNK_SIZE);
    EXPECT_NE(nullptr, ptr);
    EXPECT_TRUE(_testPool.isValidPointer(ptr));
    EXPECT_FALSE(_testPool.isEmpty());
    EXPECT_GE(_testPool.size(), 0U);
    EXPECT_GE(_testPool.maxSize(), 0U);
    EXPECT_GE(_testPool.available(), 0U);
}

TEST_F(TestPool, WhenPayloadBiggerSizeThenAllocateFailsExpectInvalidPtr)
{
    uint8_t* ptr = _testPool.allocate(CHUNK_SIZE * 2U);
    EXPECT_EQ(nullptr, ptr);
    EXPECT_FALSE(_testPool.isValidPointer(ptr));
}

TEST_F(TestPool, WhenPoolFullThenAllocateFailsExpectInvalidPtr)
{
    for (size_t i = 0U; i < CHUNK_COUNT; ++i)
    {
        _testPool.allocate(CHUNK_SIZE);
    }
    EXPECT_TRUE(_testPool.isFull());
    uint8_t* ptr = _testPool.allocate(CHUNK_SIZE);
    EXPECT_EQ(nullptr, ptr);
    EXPECT_FALSE(_testPool.isValidPointer(ptr));
}

TEST_F(TestPool, WhenDeallocateAllocatedPtrThenDeallocateSuccessExpectTrue)
{
    uint8_t* ptr = _testPool.allocate(CHUNK_SIZE);
    EXPECT_TRUE(_testPool.deallocate(ptr));
}

TEST_F(TestPool, WhenDeallocateTwiceThenSecondDeallocateFailsExpectFalse)
{
    uint8_t* ptr = _testPool.allocate(CHUNK_SIZE);
    EXPECT_TRUE(_testPool.deallocate(ptr));
    EXPECT_FALSE(_testPool.deallocate(ptr));
}

TEST_F(TestPool, WhenDeallocateSuccessThenPtrBecomesFirstPtrExpectFirstPtrIsLastDeallocatedPtr)
{
    uint8_t* ptr1            = _testPool.allocate(CHUNK_SIZE);
    uint8_t* ptr2            = _testPool.allocate(CHUNK_SIZE);
    uint8_t* ptr3            = _testPool.allocate(CHUNK_SIZE);
    uint8_t* ptr4            = _testPool.allocate(CHUNK_SIZE);
    uintptr_t const ptrValue = reinterpret_cast<uintptr_t>(ptr4);
    EXPECT_TRUE(_testPool.deallocate(ptr1));
    EXPECT_TRUE(_testPool.deallocate(ptr3));
    EXPECT_TRUE(_testPool.deallocate(ptr2));
    EXPECT_TRUE(_testPool.deallocate(ptr4));
    EXPECT_EQ(ptrValue, reinterpret_cast<uintptr_t>(_testPool.allocate(CHUNK_SIZE)));
}

TEST_F(TestPool, WhenDeallocatingRandomPointerThenDeallocateFailsExpectFalse)
{
    uint8_t buff[CHUNK_SIZE * CHUNK_COUNT]{};
    uint8_t* ptr = &buff[2 * CHUNK_SIZE];
    EXPECT_FALSE(_testPool.isValidPointer(ptr));
    EXPECT_EQ(false, _testPool.deallocate(ptr));
}

TEST_F(TestPool, WhenDeallocateSucessThenNextPtrWrittenExpectNextPtrIsAfterDeallocatedPtr)
{
    uint8_t* ptr = _testPool.allocate(CHUNK_SIZE);
    memset(ptr, 0xFFU, CHUNK_SIZE);
    EXPECT_TRUE(_testPool.deallocate(ptr));
    uintptr_t expectNextPtrValue = reinterpret_cast<uintptr_t>(ptr + ALIGNED_CHUNK_SIZE);
    uintptr_t actualNextPtrValue = *reinterpret_cast<uintptr_t*>(ptr);
    EXPECT_EQ(expectNextPtrValue, actualNextPtrValue);
}

TEST_F(TestPool, WhenDeallocateWithPtrNotInUseThenFailDeallocationExpectValidPtrFalse)
{
    uint8_t* begin        = _testPool.allocate(CHUNK_SIZE);
    uint8_t* nextValidPtr = begin + ALIGNED_CHUNK_SIZE;
    EXPECT_FALSE(_testPool.isValidPointer(nextValidPtr));
}

} // namespace middleware::memory::test
