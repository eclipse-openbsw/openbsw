// Copyright 2025 BMW AG

#pragma once

#include "middleware/core/AllocatorBase.h"
#include "middleware/memory/Pool.h"
#include "middleware/memory/impl/PoolIndexBySize.h"
#include "middleware/memory/impl/TupleSelectionSort.h"

#include <etl/delegate.h>
#include <etl/tuple.h>

#include <cstdint>

namespace middleware
{
namespace memory
{

/**
 * Memory aggregator that aggregates multiple memory pools of different chunk sizes.
 *
 * \tparam T Types of memory pools to aggregate.
 */
template<typename... T>
class Aggregator : public core::AllocatorBase<Aggregator<T...>>
{
    using TupleType = typename ::middleware::impl::
        TupleSelectionSort<::middleware::impl::Ascending, etl::tuple<T...>>::type;

    static size_t const TUPLE_SIZE = etl::tuple_size<TupleType>::value;

    static Aggregator* _instance;

    TupleType _pools;

    /**
     * Tries to allocate / deallocate / validate from the pool at index I.
     */
    template<typename Tuple, uint32_t I>
    class TryDo
    {
        Tuple& _tuplePools;

    public:
        explicit TryDo(Tuple& pools) : _tuplePools(pools) {}

        uint8_t* allocate(uint32_t const size, PoolStats* poolStats = nullptr)
        {
            auto& currentPool  = etl::get<I>(_tuplePools);
            uint8_t* const ptr = currentPool.allocate(size);
            if (ptr != nullptr)
            {
                if (poolStats != nullptr)
                {
                    poolStats->delegatedAllocations++;
                }
                return ptr;
            }
            if ((size <= currentPool.chunkSize()) && (poolStats == nullptr) && currentPool.isFull())
            {
                poolStats = &(currentPool.getPoolStats());
            }
            return TryDo<Tuple, I + 1U>(_tuplePools).allocate(size, poolStats);
        }

        void deallocate(void* const ptr)
        {
            auto& pool = etl::get<I>(_tuplePools);
            if (pool.deallocate(ptr))
            {
                return;
            }
            TryDo<Tuple, I + 1U>(_tuplePools).deallocate(ptr);
        }

        bool isPtrValid(void const* const ptr)
        {
            auto& pool                     = etl::get<I>(_tuplePools);
            uint8_t const* const bufferPtr = reinterpret_cast<uint8_t const*>(ptr); // NOLINT
            if (pool.isValidPointer(bufferPtr))
            {
                return true;
            }
            return TryDo<Tuple, I + 1U>(_tuplePools).isPtrValid(ptr);
        }

        void collectStats(etl::delegate<void(size_t const, PoolStats const)> const& collector)
        {
            auto& pool            = etl::get<I>(_tuplePools);
            PoolStats const stats = pool.getPoolStats();
            collector(I, stats);
            pool.resetStats();
            TryDo<Tuple, I + 1U>(_tuplePools).collectStats(collector);
        }

        void initialize()
        {
            auto& pool = etl::get<I>(_tuplePools);
            pool.initialize();
            TryDo<Tuple, I + 1U>(_tuplePools).initialize();
        }
    };

    /**
     * End-of-tuple specialization.
     */
    template<typename Tuple>
    class TryDo<Tuple, TUPLE_SIZE>
    {
    public:
        explicit TryDo(Tuple const&) {}

        uint8_t* allocate(uint32_t const, PoolStats* const poolStats = nullptr)
        {
            if (poolStats != nullptr)
            {
                poolStats->failedAllocations++;
            }
            return nullptr;
        }

        void deallocate(void const* const) {}

        bool isPtrValid(void const* const) { return false; }

        void collectStats(etl::delegate<void(size_t const, PoolStats const)> const&) {}

        void initialize() {}
    };

    Aggregator(Aggregator const&)            = delete;
    Aggregator(Aggregator&&)                 = delete;
    Aggregator& operator=(Aggregator const&) = delete;
    Aggregator& operator=(Aggregator&&)      = delete;

public:
    using Base = core::AllocatorBase<Aggregator<T...>>;

    /** Returns the number of pools aggregated. */
    static constexpr size_t size() { return TUPLE_SIZE; }

    // Constructor intentionally leaves pool members uninitialised.
    // Shared-RAM pools are constructed asynchronously by all cores.
    explicit Aggregator(uint8_t volatile* const pLockElement) : Base(*pLockElement) {}

    /** Allocates memory from the appropriate memory pool. */
    uint8_t* allocateImpl(uint32_t const size)
    {
        uint8_t* ptr = nullptr;
        if (size != 0U)
        {
            ptr = TryDo<TupleType, 0>(_pools).allocate(size);
        }
        return ptr;
    }

    /** Deallocates memory previously allocated by the aggregator. */
    void deallocateImpl(void* const ptr) { TryDo<TupleType, 0>(_pools).deallocate(ptr); }

    /** Checks if \p ptr points to a valid and allocated element. */
    bool isPtrValidImpl(void const* const ptr)
    {
        return TryDo<TupleType, 0>(_pools).isPtrValid(ptr);
    }

    /** Returns the start address of the first memory pool. */
    uint8_t* regionStartImpl()
    {
        return reinterpret_cast<uint8_t*>( // NOLINT
            etl::addressof(etl::get<0U>(_pools)));
    }

    /** Registers the singleton instance used by static helpers. */
    static void setInstance(Aggregator& inst) { _instance = &inst; }

    /** Initialises all aggregated pools and registers the instance. */
    static void initialize(Aggregator& inst)
    {
        _instance = &inst;
        TryDo<TupleType, 0>(_instance->_pools).initialize();
    }

    /** Finds the best-fitting pool by object type. */
    template<typename U>
    struct PoolByType
    {
        using type = typename etl::tuple_element<
            impl::PoolIndexBySize<static_cast<uint32_t>(sizeof(U)), TupleType>::value,
            TupleType>::type;
    };

    /** Returns the pool selected for allocations of size S. */
    template<uint32_t S>
    static PoolBase* getPool()
    {
        return &(
            etl::get<impl::PoolIndexBySize<S, TupleType>::value>((Aggregator::instance())._pools));
    }

    /** Collects and resets statistics from all pools. */
    static void collectStats(etl::delegate<void(size_t const, PoolStats const)> const& collector)
    {
        TryDo<TupleType, 0>((instance())._pools).collectStats(collector);
    }

    /** Returns the registered singleton instance. */
    static Aggregator& instance()
    {
        ETL_ASSERT(_instance != nullptr, ETL_ERROR_GENERIC("Aggregator instance not set."));
        return *_instance;
    }
};

} // namespace memory
} // namespace middleware
