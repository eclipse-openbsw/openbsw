// Copyright 2025 BMW AG

#pragma once

#include "middleware/concurrency/LockStrategies.h"

#include <etl/delegate.h>
#include <etl/iterator.h>
#include <etl/memory.h>

#include <cstdint>

namespace middleware::core
{

struct AllocatorStatistics
{
    uint32_t allocations;
    uint32_t deallocations;
    uint32_t unknownPtrsError;
};

/**
 * CRTP base class for memory allocators.
 * Provides thread-safe allocation and deallocation of unique and shared memory buffers.
 * Derived classes must implement allocateImpl, deallocateImpl, regionStartImpl and
 * isPtrValidImpl.
 *
 * \tparam TAllocatorImpl CRTP derived allocator type
 */
template<typename TAllocatorImpl>
class AllocatorBase
{
public:
    /**
     * Allocates a unique ownership space of \p payloadSize bytes and returns a pointer
     * to that space's address if successful, otherwise returns nullptr.
     *
     * \param payloadSize size of the payload to be allocated
     * \return pointer to the allocated payload, or nullptr on failure
     */
    uint8_t* allocate(uint32_t payloadSize);

    /**
     * Allocates a shared ownership space of \p payloadSize + 1 bytes (the extra byte
     * stores the reference counter) and returns a pointer to that space's address if
     * successful, otherwise returns nullptr.
     *
     * \param payloadSize size of the payload to be allocated
     * \param referenceCounter number of references that share this payload
     * \return pointer to the allocated payload, or nullptr on failure
     */
    uint8_t* allocateShared(uint32_t payloadSize, uint8_t referenceCounter);

    /**
     * Deallocates the unique ownership space that \p ptr points to.
     *
     * \param ptr pointer to the payload to be deallocated
     * \return true on success, false otherwise
     */
    bool deallocate(uint8_t* ptr);

    /**
     * Deallocates the shared ownership space that \p ptr points to, only if the
     * reference counter is 1, otherwise it decrements the reference counter.
     *
     * \param ptr pointer to the payload to be deallocated
     * \param payloadSize size of the payload to be deallocated
     * \return true on success, false otherwise
     */
    bool deallocateShared(uint8_t* ptr, uint32_t payloadSize);

    /**
     * Returns the starting address of the memory block used for runtime allocation.
     *
     * \return pointer to the start of the memory region
     */
    uint8_t* regionStart();

    /**
     * Checks if \p ptr points to a valid and allocated element inside the allocator.
     *
     * \return true if \p ptr is valid and allocated
     */
    bool isPtrValid(uint8_t const* ptr);

    /**
     * Returns the statistics of this allocator.
     *
     * \return const reference to the allocator statistics
     */
    AllocatorStatistics const& getStats();

protected:
    AllocatorBase(uint8_t volatile& mutex) : _mutexPtr(&mutex), _stats() {}

private:
    uint8_t volatile* _mutexPtr;
    AllocatorStatistics _stats;
};

template<typename TAllocatorImpl>
uint8_t* AllocatorBase<TAllocatorImpl>::allocate(uint32_t const payloadSize)
{
    concurrency::ScopedECULock const lockElement(_mutexPtr);
    uint8_t* externalPtr = static_cast<TAllocatorImpl*>(this)->allocateImpl(payloadSize);
    if (externalPtr != nullptr)
    {
        _stats.allocations++;
    }

    return externalPtr;
}

template<typename TAllocatorImpl>
uint8_t* AllocatorBase<TAllocatorImpl>::allocateShared(
    uint32_t const payloadSize, uint8_t const referenceCounter)
{
    concurrency::ScopedECULock const lockElement(_mutexPtr);
    uint8_t* externalPtr
        = static_cast<TAllocatorImpl*>(this)->allocateImpl(payloadSize + sizeof(referenceCounter));
    if (externalPtr != nullptr)
    {
        _stats.allocations++;
        etl::construct_object_at<uint8_t>(etl::next(externalPtr, payloadSize), referenceCounter);
    }

    return externalPtr;
}

template<typename TAllocatorImpl>
bool AllocatorBase<TAllocatorImpl>::deallocate(uint8_t* const ptr)
{
    concurrency::ScopedECULock const lockElement(_mutexPtr);
    bool res = true;
    if (isPtrValid(ptr))
    {
        static_cast<TAllocatorImpl*>(this)->deallocateImpl(ptr);
        _stats.deallocations++;
    }
    else
    {
        _stats.unknownPtrsError++;
        res = false;
    }
    return res;
}

template<typename TAllocatorImpl>
bool AllocatorBase<TAllocatorImpl>::deallocateShared(uint8_t* const ptr, uint32_t const payloadSize)
{
    concurrency::ScopedECULock const lockElement(_mutexPtr);
    bool res = true;
    if (isPtrValid(ptr))
    {
        auto& referenceCounter = etl::get_object_at<uint8_t>(etl::next(ptr, payloadSize));
        if (referenceCounter > 1U)
        {
            referenceCounter--;
        }
        else
        {
            static_cast<TAllocatorImpl*>(this)->deallocateImpl(ptr);
            _stats.deallocations++;
        }
    }
    else
    {
        _stats.unknownPtrsError++;
        res = false;
    }
    return res;
}

template<typename TAllocatorImpl>
uint8_t* AllocatorBase<TAllocatorImpl>::regionStart()
{
    return static_cast<TAllocatorImpl*>(this)->regionStartImpl();
}

template<typename TAllocatorImpl>
bool AllocatorBase<TAllocatorImpl>::isPtrValid(uint8_t const* const ptr)
{
    return static_cast<TAllocatorImpl*>(this)->isPtrValidImpl(ptr);
}

template<typename TAllocatorImpl>
AllocatorStatistics const& AllocatorBase<TAllocatorImpl>::getStats()
{
    return _stats;
}

} // namespace middleware::core
