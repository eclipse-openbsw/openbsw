// Copyright 2024 Accenture.

#pragma once

#include <etl/intrusive_forward_list.h>

#include <cstdint>

namespace timer
{
template<class T>
class Timer;

/**
 * A class providing interface for Timeout objects.
 *
 */
struct Timeout : public ::etl::forward_link<0>
{
    Timeout() = default;

    /**
     * The pure virtual function to be implemented in the inherited classes.
     */
    virtual void expired() = 0;

    /// The time when the timeout expires.
    uint32_t _time      = 0U;
    /// The period of the cyclic timeout (0 for single shot).
    uint32_t _cycleTime = 0U;
};

} // namespace timer
