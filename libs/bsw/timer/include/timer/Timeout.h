// Copyright 2024 Accenture.

#ifndef GUARD_C2A4CB49_29FE_4F33_B998_55F6D425BB00
#define GUARD_C2A4CB49_29FE_4F33_B998_55F6D425BB00

#include <estd/forward_list.h>

#include <cstdint>

namespace timer
{
template<class T>
class Timer;

/**
 * A class providing interface for Timeout objects.
 *
 */
struct Timeout : public ::estd::forward_list_node<Timeout>
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

#endif /* GUARD_C2A4CB49_29FE_4F33_B998_55F6D425BB00 */
