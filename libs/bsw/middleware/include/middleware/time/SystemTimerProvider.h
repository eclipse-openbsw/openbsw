// Copyright 2025 BMW AG

#pragma once

#include <cstdint>

namespace middleware
{
namespace time
{
/**
 * \brief Get the current system time in milliseconds.
 * \details Platform-specific function that returns the current system time in milliseconds.
 * This is used for timeout management and timestamp operations in the middleware. The
 * implementation must be provided for each platform integration and should return a monotonic
 * timestamp.
 *
 * \return the current system time in milliseconds
 */
extern uint32_t getCurrentTimeInMs();

/**
 * \brief Get the current system time in microseconds.
 * \details Platform-specific function that returns the current system time in microseconds.
 * This provides higher resolution timing for precise timeout management and timestamp operations
 * in the middleware. The implementation must be provided for each platform integration and should
 * return a monotonic timestamp.
 *
 * \return the current system time in microseconds
 */
extern uint32_t getCurrentTimeInUs();

} // namespace time
} // namespace middleware
