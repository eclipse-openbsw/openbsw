// Copyright 2025 BMW AG

#pragma once

#include <etl/tuple.h>

#include <cstdint>

namespace middleware::memory
{

struct PoolStats
{
    uint32_t chunkSize;
    uint32_t capacity;
    uint32_t failedAllocations;
    uint32_t delegatedAllocations;
    uint32_t successfulAllocations;
    uint32_t internalFragmentation;
    uint32_t maxLoad;

    // Default constructor intentionally leaves members uninitialised.
    // Pools placed in shared RAM are asynchronously initialised by all cores.
    PoolStats() {} // NOLINT

    void init()
    {
        chunkSize             = 0U;
        capacity              = 0U;
        failedAllocations     = 0U;
        delegatedAllocations  = 0U;
        successfulAllocations = 0U;
        internalFragmentation = 0U;
        maxLoad               = 0U;
    }
};

/**
 * Base class for memory pool implementations.
 * A memory pool manages memory, letting its callers allocate fixed-size memory elements.
 */
class PoolBase
{
public:
    /**
     * Constructor that initialises only constant member attributes.
     * Non-constant members are initialised by initialize().
     */
    PoolBase(
        uint8_t* buff,
        size_t elementSize,
        size_t elementAlignedSize,
        size_t elementCount,
        uint8_t* flagsBuff);

    PoolBase(PoolBase const&)            = delete;
    PoolBase(PoolBase&&)                 = delete;
    PoolBase& operator=(PoolBase const&) = delete;
    PoolBase& operator=(PoolBase&&)      = delete;

    /** Initializes pool values, buffers and the pointer to the next free element. */
    void initialize();

    /** Reserves an available element for allocation of a payload with \p size.
     * \return pointer to an element, or nullptr on failure.
     */
    uint8_t* allocate(size_t size);

    /** Tries to deallocate the element pointed to by \p ptr. */
    bool deallocate(void* ptr);

    /** true if the pool is completely available for allocations. */
    bool isEmpty() const;

    /** true if the pool is completely used. */
    bool isFull() const;

    /** Number of available elements for allocations. */
    size_t available() const;

    /** Number of elements currently in use. */
    size_t size() const;

    /** Total number of elements the pool contains. */
    size_t maxSize() const;

    /**
     * Checks if \p ptr points to a valid and allocated element inside the storage buffer.
     *
     * \return true if \p ptr is valid and allocated.
     */
    bool isValidPointer(uint8_t const* ptr) const;

    /** \return reference to the pool statistics. */
    PoolStats& getPoolStats();

    /** Resets the statistics of the pool. */
    void resetStats();

    /**
     * Returns a tuple showing the profile of the pool.
     * Elements: <available, maxLoad, fragmentation / successfulAllocations>.
     */
    etl::tuple<size_t, size_t, double> getProfile() const;

private:
    void updatePtrFlag(size_t position, bool ptrBusy);
    bool checkPtrFlag(size_t position) const;
    static uint8_t* offsetAddress(uint8_t* ptr, size_t offset);

    uint8_t* const _buffer;
    size_t const _elementSize;
    size_t const _elementAlignedSize;
    size_t const _elementCount;
    uint8_t* const _flags;
    uint8_t* _nextChunk;
    size_t _available;
    PoolStats _stats;
};

} // namespace middleware::memory
