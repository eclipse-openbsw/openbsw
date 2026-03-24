// Copyright 2025 BMW AG

#pragma once

#include <etl/span.h>

#include <cstdint>

namespace middleware
{
namespace logger
{

enum class LogLevel : uint8_t
{
    None     = 0U,
    Critical = 1U,
    Error    = 2U,
    Warning  = 3U,
    Info     = 4U,
    Debug    = 5U,
    Trace    = 6U
};

enum class Error : uint8_t
{
    /** Memory allocation failure. */
    Allocation                   = 0U,
    /** Memory deallocation failure. */
    Deallocation                 = 1U,
    /** Proxy initialization failure. */
    ProxyInitialization          = 2U,
    /** Skeleton initialization failure. */
    SkeletonInitialization       = 3U,
    /** Message dispatch failure. */
    DispatchMessage              = 4U,
    /** Message sending failure. */
    SendMessage                  = 5U,
    /** Proxy cross-thread access violation. */
    ProxyCrossThreadViolation    = 6U,
    /** Skeleton cross-thread access violation. */
    SkeletonCrossThreadViolation = 7U,
};

/**
 * Generic logging function for formatted messages.
 * Platform-specific implementation of the logging function that accepts printf-style
 * format strings. This function must be implemented when integrating the middleware into a new
 * platform to route log messages to the appropriate logging backend.
 *
 * \param level the log level for this message
 * \param f the format string (printf-style)
 * \param ... variadic arguments for the format string
 */
extern void log(LogLevel level, char const* f, ...);

/**
 * Generic logging function for binary data.
 * Platform-specific implementation of the logging function that logs binary data.
 * This function must be implemented when integrating the middleware into a new platform to
 * handle binary log data, which may be used for structured logging or diagnostic purposes.
 *
 * \param level the log level for this data
 * \param data span containing the binary data to log
 */
extern void logBinary(LogLevel level, etl::span<uint8_t const> data);

/**
 * Get the message ID associated with an error type.
 * Returns a unique message identifier for the given error type. This is useful for
 * extending logs to contain DLT (Diagnostic Log and Trace) information or other structured
 * logging formats.
 *
 * \param id the error type
 * \return the message ID associated with the error
 */
extern uint32_t getMessageId(Error id);

} // namespace logger
} // namespace middleware
