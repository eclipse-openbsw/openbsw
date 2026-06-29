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

#include <cstdint>

namespace bsw
{
namespace time
{

/**
 * A class for providing timestamps.
 *
 * Uses underlying BSP / BSW functionality.
 */
class TimestampProvider
{
public:
    /**
     * Provides a 32 bit CPU local and relative timestamp in micro seconds resolution.
     *
     * Will overflow after about 71 minutes. Therefore only suitable for short term measurements.
     *
     * A 32 bit timestamp is less expensive compared to a 64 bit timestamp, at least on 32 bit
     * platforms. Therefore prefer 32 bit timestamp if feasible.
     *
     * \return  32 bit CPU local timestamp micro seconds.
     */
    static uint32_t getTimestampUs32Bit();

    /**
     * Provides a 64 bit CPU local and relative timestamp in micro seconds resolution.
     *
     * The 64 bit timestamp value is guaranteed to not wrap around before 2^64, i.e. using
     * the whole 64 bits for counting.
     *
     * \return  64 bit CPU local timestamp micro seconds.
     */
    static uint64_t getTimestampUs64Bit();

    /**
     * Provides a 32 bit CPU local and relative timestamp in milli seconds resolution.
     *
     * Will overflow after about 49 days. Therefore only suitable for medium term measurements.
     *
     * A 32 bit timestamp is less expensive compared to a 64 bit timestamp, at least on 32 bit
     * platforms. Therefore prefer 32 bit timestamp if feasible.
     *
     * \return  32 bit CPU local timestamp milli seconds.
     */
    static uint32_t getTimestampMs32Bit();

    /**
     * Provides a 64 bit CPU local and relative timestamp in milli seconds resolution.
     *
     * The 64 bit timestamp value is guaranteed to not wrap around before 2^64, i.e. using
     * the whole 64 bits for counting.
     *
     * \return  64 bit CPU local timestamp milli seconds.
     */
    static uint64_t getTimestampMs64Bit();
};

} // namespace time
} // namespace bsw
