/********************************************************************************
 * Copyright (c) 2025 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/memory/AllocatorBase.h"
#include "middleware/memory/Pool.h"
#include "middleware/memory/impl/PoolIndexBySize.h"
#include "middleware/memory/impl/TupleSelectionSort.h"

#include <etl/delegate.h>
#include <etl/tuple.h>

#include <cstdint>

namespace middleware::memory
{

/**
 * Memory aggregator that aggregates multiple memory pools of different chunk sizes.
 *
 * \tparam T Types of memory pools to aggregate.
 */
template<typename... T>
class Aggregator : public memory::AllocatorBase<Aggregator<T...>>
{
    using TupleType = typename ::middleware::impl::
        TupleSelectionSort<::middleware::impl::Ascending, ::etl::tuple<T...>>::type;

    static size_t const TUPLE_SIZE = ::etl::tuple_size<TupleType>::value;

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
            auto& currentPool  = ::etl::get<I>(_tuplePools);
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
            auto& pool = ::etl::get<I>(_tuplePools);
            if (pool.deallocate(ptr))
            {
                return;
            }
            TryDo<Tuple, I + 1U>(_tuplePools).deallocate(ptr);
        }

        bool isPtrValid(void const* const ptr)
        {
            auto& pool                     = ::etl::get<I>(_tuplePools);
            uint8_t const* const bufferPtr = reinterpret_cast<uint8_t const*>(ptr); // NOLINT
            if (pool.isValidPointer(bufferPtr))
            {
                return true;
            }
            return TryDo<Tuple, I + 1U>(_tuplePools).isPtrValid(ptr);
        }

        void collectStats(::etl::delegate<void(size_t const, PoolStats const)> const& collector)
        {
            auto& pool            = ::etl::get<I>(_tuplePools);
            PoolStats const stats = pool.getPoolStats();
            collector(I, stats);
            pool.resetStats();
            TryDo<Tuple, I + 1U>(_tuplePools).collectStats(collector);
        }

        void initialize()
        {
            auto& pool = ::etl::get<I>(_tuplePools);
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

        void collectStats(::etl::delegate<void(size_t const, PoolStats const)> const&) {}

        void initialize() {}
    };

    Aggregator(Aggregator const&)            = delete;
    Aggregator(Aggregator&&)                 = delete;
    Aggregator& operator=(Aggregator const&) = delete;
    Aggregator& operator=(Aggregator&&)      = delete;

public:
    using Base = memory::AllocatorBase<Aggregator<T...>>;

    /** Returns the number of pools aggregated. */
    static constexpr size_t size() { return TUPLE_SIZE; }

    /**
     * Constructor which initializes the aggregator.
     *
     * This ensures pools are always initialized when an `Aggregator` is constructed,
     * eliminating the risk of using an uninitialized instance in multicore systems,
     * construction must complete on one core before others access the aggregator.
     * The external lock element and all pools must be fully initialized before any allocation.
     *
     * \param pLockElement pointer to the external lock element used by the base allocator
     */
    explicit Aggregator(uint8_t volatile* const pLockElement) : Base(*pLockElement)
    {
        TryDo<TupleType, 0>(_pools).initialize();
    }

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
            ::etl::addressof(::etl::get<0U>(_pools)));
    }

    /** Finds the best-fitting pool by object type. */
    template<typename U>
    struct PoolByType
    {
        using type = typename ::etl::tuple_element<
            impl::PoolIndexBySize<static_cast<uint32_t>(sizeof(U)), TupleType>::value,
            TupleType>::type;
    };

    /** Returns the pool selected for allocations of size S. */
    template<uint32_t S>
    PoolBase* getPool()
    {
        return &(::etl::get<impl::PoolIndexBySize<S, TupleType>::value>(_pools));
    }

    /** Collects and resets statistics from all pools. */
    void collectStats(::etl::delegate<void(size_t const, PoolStats const)> const& collector)
    {
        TryDo<TupleType, 0>(_pools).collectStats(collector);
    }
};

} // namespace middleware::memory
