// Copyright 2024 Accenture.

#ifndef GUARD_E048A4BE_29AC_489D_B187_3E6F34207286
#define GUARD_E048A4BE_29AC_489D_B187_3E6F34207286

#include <bsp/timer/SystemTimer.h>

#include <platform/estdint.h>

namespace bsp
{

/**
 * Check for equality with timeout.
 * \param ptr Address of the value to check.
 * \param mask Bitmask to apply to value.
 * \param value Value to compare to.
 * \param timeout Timeout in us.
 * \return *             - true, if values are equal after timeout
 *             - false, as soon as values differ
 */
template<typename T>
bool isEqualAfterTimeout(T const* const ptr, T const mask, T const value, uint32_t const timeout)
{
    uint64_t const endTime = getSystemTimeUs() + timeout;

    while (((*ptr & mask) == (value & mask)) && (getSystemTimeUs() <= endTime)) {}
    return (*ptr & mask) == (value & mask);
}

} // namespace bsp

#endif // GUARD_E048A4BE_29AC_489D_B187_3E6F34207286
