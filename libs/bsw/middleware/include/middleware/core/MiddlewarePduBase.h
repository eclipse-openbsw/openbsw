/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/atomic.h>
#include <etl/limits.h>
#include <etl/span.h>

#include <cstdint>

namespace middleware::core
{

/**
 * \brief Sequence-locked PDU for thread-safe read and write operations.
 *
 * The payload is copied non-atomically on purpose: a reader that overlaps a write detects it
 * through the counter and retries, so a torn copy is never returned
 *
 * Thread safety: any number of readers, exactly one writer. load() spins while a write
 * is in progress.
 */
class MiddlewarePduBase
{
protected:
    /**
     * \brief Stores a new value into the sequence-locked PDU.
     *
     * This function updates the data in a thread-safe manner using a sequence lock.
     *
     * \param src The source data to be stored.
     * \param dst The destination buffer where the data will be stored.
     */
    void store(etl::span<uint8_t const> const& src, etl::span<uint8_t> const& dst);

    /**
     * \brief Loads the current value from the sequence-locked PDU in a thread-safe manner.
     *
     * \param dst The buffer to store the loaded PDU data.
     * \param src The current data stored in the PDU.
     * \return Returns the last sequence number after successfully loading the PDU data.
     */
    uintptr_t load(etl::span<uint8_t> const& dst, etl::span<uint8_t const> const& src) const;

private:
    bool tryLoad(
        etl::span<uint8_t> const& dst,
        etl::span<uint8_t const> const& src,
        uintptr_t& lastSeqNumber) const;

    etl::atomic<uintptr_t> seqCounter_{};
    static_assert(decltype(seqCounter_)::is_always_lock_free);
};

} // namespace middleware::core
