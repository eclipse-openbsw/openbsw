/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <middleware/core/MiddlewarePduBase.h>

#include <etl/algorithm.h>
#include <etl/platform.h>
#include <etl/span.h>
#include <middleware/concurrency/LockStrategies.h>
#include <middleware/concurrency/MemoryFences.h>
#include <middleware/concurrency/SpinLock.h>
#include <cstdint>

namespace middleware::core
{

__attribute__((no_sanitize("thread"))) void
MiddlewarePduBase::store(etl::span<uint8_t const> const& src, etl::span<uint8_t> const& dst)
{
    {
        ETL_MAYBE_UNUSED concurrency::ScopedCoreLock const lock{};

        uintptr_t const seq = seqCounter_.load(etl::memory_order_relaxed);

        seqCounter_.store(seq + 1U, etl::memory_order_release);

        // Orders the odd counter before the payload write.
        middleware::concurrency::atomicThreadFence();
        etl::copy(src.begin(), src.end(), dst.begin());

        // Orders the data write before the counter is incremented to even.
        middleware::concurrency::atomicThreadFence();
        seqCounter_.store(seq + 2U, etl::memory_order_release);
    }
}

uintptr_t
MiddlewarePduBase::load(etl::span<uint8_t> const& dst, etl::span<uint8_t const> const& src) const
{
    uintptr_t lastSeqNumber{};
    while (!tryLoad(dst, src, lastSeqNumber))
    {
        middleware::concurrency::spinLockOptimization();
    }
    return lastSeqNumber;
}

__attribute__((no_sanitize("thread"))) bool MiddlewarePduBase::tryLoad(
    etl::span<uint8_t> const& dst,
    etl::span<uint8_t const> const& src,
    uintptr_t& lastSeqNumber) const
{
    uintptr_t const seq0 = seqCounter_.load(etl::memory_order_acquire);
    if (static_cast<bool>(seq0 & 1))
    {
        return false;
    }

    // Orders the even counter check before the payload read.
    middleware::concurrency::atomicThreadFence();
    etl::copy(src.begin(), src.end(), dst.begin());

    // Orders the payload read before the subsequent counter check.
    middleware::concurrency::atomicThreadFence();
    lastSeqNumber = seqCounter_.load(etl::memory_order_acquire);
    return (seq0 == lastSeqNumber);
}

} // namespace middleware::core
