// Copyright 2024 Accenture.

#ifndef GUARD_C2A4CB49_29FE_4F33_B998_55F6D425BB00
#define GUARD_C2A4CB49_29FE_4F33_B998_55F6D425BB00

#include <estd/forward_list.h>

#include <cstdint>

namespace timer
{
template<class T>
class Timer;

struct Timeout : public ::estd::forward_list_node<Timeout>
{
    Timeout() = default;

    virtual void expired() = 0;

    uint32_t _time      = 0U;
    uint32_t _cycleTime = 0U;
};

} // namespace timer

#endif /* GUARD_C2A4CB49_29FE_4F33_B998_55F6D425BB00 */
