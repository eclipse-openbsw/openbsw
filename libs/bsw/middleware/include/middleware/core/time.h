// Copyright 2025 BMW AG

#pragma once

#include <cstdint>

#include <etl/chrono.h>

namespace middleware::core
{

/**
 * \brief Utility class providing steady-clock timestamps.
 *
 * Wraps \c etl::chrono::steady_clock to expose millisecond and microsecond
 * timestamps.
 */
class Time
{
public:
    /**
     * \brief Returns the current steady-clock time in milliseconds.
     *
     * \return Milliseconds elapsed since the steady-clock epoch.
     */
    static uint32_t getCurrentTimeInMs()
    {
        return static_cast<uint32_t>(etl::chrono::duration_cast<etl::chrono::milliseconds>(
                                         etl::chrono::steady_clock::now().time_since_epoch())
                                         .count());
    }

    /**
     * \brief Returns the current steady-clock time in microseconds.
     *
     * \return Microseconds elapsed since the steady-clock epoch.
     */
    static uint32_t getCurrentTimeInUs()
    {
        return static_cast<uint32_t>(etl::chrono::duration_cast<etl::chrono::microseconds>(
                                         etl::chrono::steady_clock::now().time_since_epoch())
                                         .count());
    }
};

} // namespace middleware::core
