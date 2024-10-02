// Copyright 2024 Accenture.

#ifndef GUARD_C33BDF65_EAA1_4EE7_8F00_0A590F568CFF
#define GUARD_C33BDF65_EAA1_4EE7_8F00_0A590F568CFF

// This is the place where a logger framework implementation will be connected.
// The framework include path should be injected before util's include path.
// clang-format off
#include "util/logger/LoggerBinding.h"   // IWYU pragma: export
#include "util/logger/LoggerApiMacros.h" // IWYU pragma: export
// clang-format on

#include "util/logger/LevelInfo.h"

#include <cstdarg>
#include <cstdint>

#ifndef DECLARE_LOGGER_COMPONENT
#define DECLARE_LOGGER_COMPONENT(_uniqueName) \
    namespace util                            \
    {                                         \
    namespace logger                          \
    {                                         \
    extern uint8_t _uniqueName;               \
    }                                         \
    }
#endif // DECLARE_LOGGER_COMPONENT

#ifndef DEFINE_LOGGER_COMPONENT
#define DEFINE_LOGGER_COMPONENT(_uniqueName)              \
    namespace util                                        \
    {                                                     \
    namespace logger                                      \
    {                                                     \
    uint8_t _uniqueName = ::util::logger::COMPONENT_NONE; \
    }                                                     \
    }
#endif // DEFINE_LOGGER_COMPONENT

#ifndef LOGGER_NO_LEGACY_API

#include "util/logger/IComponentMapping.h"

namespace util
{
namespace logger
{
class ILoggerOutput;

/**
 * This class is a simple facade that allows the separation of emitting log messages from
 * handling of the messages. Whenever a module wants to provide log messages it can emit
 * the messages of a specified severity (level) by using the static methods of this class.
 * The messages are then handled in an application-defined manner by delegation to interface
 * implementations. If the Logger isn't initalized by an application all calls to any of
 * the log-methods will simply be ignored or will indicate disabled logging.
 */
class Logger
{
public:
    /**
     * Initialize the Logger with the necessary interfaces to the component mapping (deciding
     * whether a log will be accepted) and to the logger output (handling all accepted logs).
     * All calls to any of the log methods will be forwarded to the specified interface until
     * the shutdown method is being called.
     * \param componentMapping reference to the component mapping
     * \param output reference to the logger output
     */
    static void init(IComponentMapping& componentMapping, ILoggerOutput& output);

    /**
     * Cuts the connection to the previously specified mapping and output.
     * \note if lower priority tasks are preempted during logging calls to the interfaces may
     * also occur after shutdown.
     */
    static void shutdown();

    /**
     * Check whether a component is enabled for logging.
     * \param componentIndex index of the logger component
     * \param level level to check for enabled logs
     * \return true if logs of given severity or higher are enabled for the component
     */
    static bool isEnabled(uint8_t componentIndex, Level level);

    /**
     * Returns the minimum severity/level of messages which are enabled for a component.
     * \param componentIndex index of the logger component
     * \return minimum level for which logging is enabled
     */
    static Level getLevel(uint8_t componentIndex);

    /**
     * Emit a log message for a given component and severity.
     * \note this method should only be called if the log level is determined on runtime,
     * otherwise calls to the specialized functions should be preferred.
     * \param componentIndex index of the logger component
     * \param level severity of message
     * \param str printf-format string for message
     * \param ... printf arguments depending on format string
     */
    static void log(uint8_t componentIndex, Level level, char const* str, ...);

    /**
     * Emit a log message for a given component and severity.
     * \note this method should only be called if the log level is determined on runtime,
     * otherwise calls to the specialized functions should be preferred.
     * \param componentIndex index of the logger component
     * \param level severity of message
     * \param str printf-format string for message
     * \param va_list printf arguments depending on format string
     */
    static void log(uint8_t componentIndex, Level level, char const* str, va_list ap);

    /**
     * Emit a log message of severity LEVEL_INFO for a given component and severity.
     * \param componentIndex index of the logger component
     * \param str printf-format string for message
     * \param ... printf arguments depending on format string
     */
    static void info(uint8_t componentIndex, char const* str, ...);

    /**
     * Emit a log message of severity LEVEL_DEBUG for a given component and severity.
     * \param componentIndex index of the logger component
     * \param str printf-format string for message
     * \param ... printf arguments depending on format string
     */
    static void debug(uint8_t componentIndex, char const* str, ...);

    /**
     * Emit a log message of severity LEVEL_WARN for a given component and severity.
     * \param componentIndex index of the logger component
     * \param str printf-format string for message
     * \param ... printf arguments depending on format string
     */
    static void warn(uint8_t componentIndex, char const* str, ...);

    /**
     * Emit a log message of severity LEVEL_ERROR for a given component and severity.
     * \param componentIndex index of the logger component
     * \param str printf-format string for message
     * \param ... printf arguments depending on format string
     */
    static void error(uint8_t componentIndex, char const* str, ...);

    /**
     * Emit a log message of severity LEVEL_CRITICAL for a given component and severity.
     * \param componentIndex index of the logger component
     * \param str printf-format string for message
     * \param ... printf arguments depending on format string
     */
    static void critical(uint8_t componentIndex, char const* str, ...);

private:
    static void doLog(uint8_t componentIndex, Level level, char const* str, va_list ap);

    static IComponentMapping* _componentMapping;
    static ILoggerOutput* _output;
};

} // namespace logger
} // namespace util

#ifndef DISABLE_LOGGING
#define LOGGER_DOLOG(_level)                    \
    if (isEnabled(componentIndex, _level))      \
    {                                           \
        va_list ap;                             \
        va_start(ap, str);                      \
        doLog(componentIndex, _level, str, ap); \
        va_end(ap);                             \
    }
#else
#define LOGGER_DOLOG(_level)
#endif

namespace util
{
namespace logger
{
inline bool Logger::isEnabled(uint8_t const componentIndex, Level const level)
{
    return (_componentMapping != nullptr) ? _componentMapping->isEnabled(componentIndex, level)
                                          : false;
}

inline void Logger::info(uint8_t const componentIndex, char const* const str, ...)
{
    LOGGER_DOLOG(LEVEL_INFO)
}

inline void Logger::debug(uint8_t const componentIndex, char const* const str, ...)
{
    LOGGER_DOLOG(LEVEL_DEBUG)
}

inline void Logger::warn(uint8_t const componentIndex, char const* const str, ...)
{
    LOGGER_DOLOG(LEVEL_WARN)
}

inline void Logger::error(uint8_t const componentIndex, char const* const str, ...)
{
    LOGGER_DOLOG(LEVEL_ERROR)
}

inline void Logger::critical(uint8_t const componentIndex, char const* const str, ...)
{
    LOGGER_DOLOG(LEVEL_CRITICAL)
}

inline void Logger::log(uint8_t const componentIndex, Level const level, char const* const str, ...)
{
    LOGGER_DOLOG(level)
}

inline void
Logger::log(uint8_t const componentIndex, Level const level, char const* const str, va_list ap)
{
    if (isEnabled(componentIndex, level))
    {
        doLog(componentIndex, level, str, ap);
    }
}

} // namespace logger
} // namespace util

#endif // LOGGER_NO_LEGACY_API

#endif // GUARD_C33BDF65_EAA1_4EE7_8F00_0A590F568CFF
