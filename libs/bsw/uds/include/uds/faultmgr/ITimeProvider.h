// Copyright 2026 KPIT Technologies
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdint>

namespace uds
{

/**
 * Interface for global synchronized time source (StbM-backed or equivalent).
 * Returns nanosecond-resolution global timestamp (DGN-FEL012).
 * Returns INVALID_TIMESTAMP when time is not yet synchronized.
 */
class ITimeProvider
{
public:
    virtual ~ITimeProvider() = default;

    /** Returns nanoseconds since global epoch. */
    virtual uint64_t getNanoseconds() const = 0;

    /** Returns true when global time sync has been achieved. */
    virtual bool isSynchronized() const = 0;

    /** Sentinel value stored in timestamps when time source is not synchronized. */
    static constexpr uint64_t INVALID_TIMESTAMP = 0xFFFFFFFFFFFFFFFFULL;
};

} // namespace uds
