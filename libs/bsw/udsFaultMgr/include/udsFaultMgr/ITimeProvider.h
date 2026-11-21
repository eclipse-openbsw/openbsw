/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/
#pragma once
#include <cstdint>

namespace uds
{

/**
 * Interface for global synchronized time source (platform time driver or equivalent).
 * Returns nanosecond-resolution global timestamp
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
