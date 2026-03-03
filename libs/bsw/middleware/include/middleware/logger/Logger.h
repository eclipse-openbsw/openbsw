// Copyright 2025 BMW AG

#pragma once

#include <cstdint>

#include <etl/span.h>

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
    Allocation                   = 0U, ///< Memory allocation failure
    Deallocation                 = 1U, ///< Memory deallocation failure
    ProxyInitialization          = 2U, ///< Proxy initialization failure
    SkeletonInitialization       = 3U, ///< Skeleton initialization failure
    DispatchMessage              = 4U, ///< Message dispatch failure
    SendMessage                  = 5U, ///< Message sending failure
    ProxyCrossThreadViolation    = 6U, ///< Proxy cross-thread access violation
    SkeletonCrossThreadViolation = 7U, ///< Skeleton cross-thread access violation
};

/**
 * \brief Generic logging function for formatted messages.
 * \details Platform-specific implementation of the logging function that accepts printf-style
 * format strings. This function must be implemented when integrating the middleware into a new
 * platform to route log messages to the appropriate logging backend.
 *
 * \param level the log level for this message
 * \param f the format string (printf-style)
 * \param ... variadic arguments for the format string
 */
extern void log(LogLevel const level, char const* const f, ...);

/**
 * \brief Generic logging function for binary data.
 * \details Platform-specific implementation of the logging function that logs binary data.
 * This function must be implemented when integrating the middleware into a new platform to
 * handle binary log data, which may be used for structured logging or diagnostic purposes.
 *
 * \param level the log level for this data
 * \param data span containing the binary data to log
 */
extern void log_binary(LogLevel const level, etl::span<uint8_t const> const data);

/**
 * \brief Get the message ID associated with an error type.
 * \details Returns a unique message identifier for the given error type. This is useful for
 * extending logs to contain DLT (Diagnostic Log and Trace) information or other structured
 * logging formats.
 *
 * \param id the error type
 * \return the message ID associated with the error
 */
extern uint32_t getMessageId(Error const id);

} // namespace logger
} // namespace middleware
