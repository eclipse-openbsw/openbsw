#include "AllocatorMock.h"

#include "middleware/core/AllocatorBase.h"
#include "middleware/core/types.h"
#include "middleware/memory/AllocatorSelector.h"

namespace middleware
{
namespace memory
{

namespace test
{

AllocatorMock* AllocatorMock::_instance{nullptr};
etl::array<uint8_t, AllocatorMock::MAX_STORAGE> AllocatorMock::_storage{};

AllocatorMock& AllocatorMock::getInstance()
{
    ETL_ASSERT(_instance != nullptr, ETL_ERROR_GENERIC("AllocatorMock instance not set."));
    return *_instance;
}

void AllocatorMock::setAllocatorMock(AllocatorMock& mock)
{
    _instance = &mock;
    ON_CALL(*_instance, allocateImpl(::testing::_))
        .WillByDefault(
            [](uint32_t const size) -> uint8_t*
            {
                ETL_ASSERT(
                    size <= _storage.max_size(),
                    ETL_ERROR_GENERIC("Allocation size exceeds maximum storage size."));
                return _storage.data();
            });
    ON_CALL(*_instance, deallocateImpl(::testing::_))
        .WillByDefault(
            [](void* ptr) -> void
            {
                ETL_ASSERT(ptr == _storage.data(), ETL_ERROR_GENERIC("Pointer is not valid."));
                _storage.fill(0U);
            });
    ON_CALL(*_instance, regionStartImpl())
        .WillByDefault([](void) -> uint8_t* { return _storage.data(); });
    ON_CALL(*_instance, isPtrValidImpl(::testing::_))
        .WillByDefault([](void const* const ptr) -> bool { return ptr == _storage.data(); });
}

void AllocatorMock::unsetAllocatorMock() { _instance = nullptr; }

AllocatorMock::AllocatorMock() : Base(_dummyMutex) {}

AllocatorMock::~AllocatorMock() { unsetAllocatorMock(); }

} // namespace test

AllocateFunction getAllocFunction(uint16_t /*unused*/)
{
    using AllocatorBase          = test::AllocatorMock::Base;
    AllocatorBase& allocatorMock = test::AllocatorMock::getInstance();
    return AllocateFunction::create<AllocatorBase, &AllocatorBase::allocate>(allocatorMock);
}

AllocateSharedFunction getAllocSharedFunction(uint16_t /*unused*/)
{
    using AllocatorBase          = test::AllocatorMock::Base;
    AllocatorBase& allocatorMock = test::AllocatorMock::getInstance();
    return AllocateSharedFunction::create<AllocatorBase, &AllocatorBase::allocateShared>(
        allocatorMock);
}

DeallocateFunction getDeallocFunction(uint16_t /*unused*/)
{
    using AllocatorBase          = test::AllocatorMock::Base;
    AllocatorBase& allocatorMock = test::AllocatorMock::getInstance();
    return DeallocateFunction::create<AllocatorBase, &AllocatorBase::deallocate>(allocatorMock);
}

DeallocateSharedFunction getDeallocSharedFunction(uint16_t /*unused*/)
{
    using AllocatorBase          = test::AllocatorMock::Base;
    AllocatorBase& allocatorMock = test::AllocatorMock::getInstance();
    return DeallocateSharedFunction::create<AllocatorBase, &AllocatorBase::deallocateShared>(
        allocatorMock);
}

RegionStartFunction getRegionStartFunction(uint16_t /*unused*/)
{
    using AllocatorBase          = test::AllocatorMock::Base;
    AllocatorBase& allocatorMock = test::AllocatorMock::getInstance();
    return RegionStartFunction::create<AllocatorBase, &AllocatorBase::regionStart>(allocatorMock);
}

PointerValidationFunction getPtrValidationFunction(uint16_t /*unused*/)
{
    using AllocatorBase          = test::AllocatorMock::Base;
    AllocatorBase& allocatorMock = test::AllocatorMock::getInstance();
    return PointerValidationFunction::create<AllocatorBase, &AllocatorBase::isPtrValid>(
        allocatorMock);
}

} // namespace memory
} // namespace middleware
