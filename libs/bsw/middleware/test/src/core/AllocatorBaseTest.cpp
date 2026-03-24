#include <etl/array.h>
#include <etl/memory.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "middleware/core/AllocatorBase.h"

namespace middleware
{
namespace core
{
namespace test
{

class TestAllocatorBase : public ::testing::Test
{
public:
    class AllocatorImpl : public core::AllocatorBase<AllocatorImpl>
    {
    public:
        AllocatorImpl() : core::AllocatorBase<AllocatorImpl>(_mutex) {}

        MOCK_METHOD(uint8_t*, allocateImpl, (uint32_t const));
        MOCK_METHOD(void, deallocateImpl, (void*));
        MOCK_METHOD(bool, isPtrValidImpl, (void const* const));
        MOCK_METHOD(uint8_t*, regionStartImpl, ());

    private:
        uint8_t volatile _mutex{};
    };

    TestAllocatorBase() = default;

    void SetUp() final
    {
        ON_CALL(_impl, isPtrValidImpl)
            .WillByDefault(testing::Invoke(
                [this](void const* const ptr) -> bool { return ptr == _storage.data(); }));
        ON_CALL(_impl, regionStartImpl).WillByDefault(testing::Return(_storage.begin()));
        ON_CALL(_impl, allocateImpl)
            .WillByDefault(testing::Invoke(
                [this](size_t const size) -> uint8_t*
                { return (size <= _storage.max_size()) ? _storage.data() : nullptr; }));
        ON_CALL(_impl, deallocateImpl)
            .WillByDefault(testing::Invoke([this](void*) -> void { _storage.fill(0U); }));
    }

protected:
    testing::NiceMock<AllocatorImpl> _impl;
    etl::array<uint8_t, 32U> _storage;
};

TEST_F(TestAllocatorBase, TestSuccessAllocate)
{
    uint32_t const size  = 20U;
    uint8_t* externalPtr = _impl.allocate(size);
    EXPECT_EQ(externalPtr, _storage.begin());
    EXPECT_EQ(_impl.getStats().allocations, 1U);
}

TEST_F(TestAllocatorBase, TestFailedAllocate)
{
    uint32_t const size  = 64U;
    uint8_t* externalPtr = _impl.allocate(size);
    EXPECT_EQ(externalPtr, nullptr);
    EXPECT_EQ(_impl.getStats().allocations, 0U);
}

TEST_F(TestAllocatorBase, TestAllocateShared)
{
    uint32_t const size            = 20U;
    uint8_t const referenceCounter = 5U;
    uint8_t* externalPtr           = _impl.allocateShared(size, referenceCounter);
    EXPECT_EQ(externalPtr, _storage.begin());
    EXPECT_EQ(_impl.getStats().allocations, 1U);
    EXPECT_EQ(etl::get_object_at<uint8_t>(etl::next(_storage.data(), size)), 5U);
}

TEST_F(TestAllocatorBase, TestFailedAllocateShared)
{
    uint32_t const size            = 64U;
    uint8_t const referenceCounter = 5U;
    uint8_t* externalPtr           = _impl.allocateShared(size, referenceCounter);
    EXPECT_EQ(externalPtr, nullptr);
    EXPECT_EQ(_impl.getStats().allocations, 0U);
}

TEST_F(TestAllocatorBase, TestDeallocate)
{
    uint32_t const size        = 20U;
    uint8_t* const externalPtr = _impl.allocate(size);
    EXPECT_CALL(_impl, deallocateImpl).Times(1U);
    _impl.deallocate(externalPtr);
    EXPECT_EQ(_impl.getStats().deallocations, 1U);
    EXPECT_EQ(_impl.getStats().unknownPtrsError, 0U);
}

TEST_F(TestAllocatorBase, TestFailedDeallocate)
{
    uint32_t const size        = 20U;
    uint8_t* const externalPtr = _impl.allocate(size);
    EXPECT_CALL(_impl, deallocateImpl).Times(0U);
    _impl.deallocate(externalPtr + 1U);
    EXPECT_EQ(_impl.getStats().deallocations, 0U);
    EXPECT_EQ(_impl.getStats().unknownPtrsError, 1U);
}

TEST_F(TestAllocatorBase, TestDeallocateShared)
{
    uint32_t const size            = 20U;
    uint8_t const referenceCounter = 5U;
    uint8_t* const externalPtr     = _impl.allocateShared(size, referenceCounter);
    _impl.allocateShared(size, referenceCounter);

    for (size_t counter = 1U; counter < referenceCounter; counter++)
    {
        EXPECT_CALL(_impl, deallocateImpl).Times(0U);
        _impl.deallocateShared(externalPtr, size);
        EXPECT_EQ(_impl.getStats().deallocations, 0U);
        EXPECT_EQ(_impl.getStats().unknownPtrsError, 0U);
        EXPECT_EQ(
            etl::get_object_at<uint8_t>(etl::next(_storage.data(), size)),
            referenceCounter - counter);
    }

    EXPECT_CALL(_impl, deallocateImpl).Times(1U);
    _impl.deallocateShared(externalPtr, size);
    EXPECT_EQ(_impl.getStats().deallocations, 1U);
    EXPECT_EQ(_impl.getStats().unknownPtrsError, 0U);
}

TEST_F(TestAllocatorBase, TestFailedDeallocateShared)
{
    uint32_t const size            = 64U;
    uint8_t const referenceCounter = 5U;
    uint8_t* const externalPtr     = _impl.allocateShared(size, referenceCounter);
    EXPECT_CALL(_impl, deallocateImpl).Times(0U);
    _impl.deallocateShared(externalPtr, size);
    EXPECT_EQ(_impl.getStats().deallocations, 0U);
    EXPECT_EQ(_impl.getStats().unknownPtrsError, 1U);
}

} // namespace test
} // namespace core
} // namespace middleware
