/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

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
