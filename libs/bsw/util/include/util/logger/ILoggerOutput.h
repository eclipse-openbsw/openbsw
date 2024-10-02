// Copyright 2024 Accenture.

#ifndef GUARD_CA5724D3_1F37_4BEA_AD36_F9E16C96D4FF
#define GUARD_CA5724D3_1F37_4BEA_AD36_F9E16C96D4FF

#include "util/logger/ComponentInfo.h"
#include "util/logger/LevelInfo.h"

#include <cstdarg>

namespace util
{
namespace logger
{
class LoggerComponent;

/**
 * Interface for reception of filtered and enriched log message information ready for
 * printing/storing.
 */
class ILoggerOutput
{
public:
    ILoggerOutput(ILoggerOutput const&)            = delete;
    ILoggerOutput& operator=(ILoggerOutput const&) = delete;

    /**
     * Called for each filtered log message.
     * \param componentInfo reference to component info holding a human readable name of the
     * component
     * \param levelInfo reference to the level info holding a human readable text for the severity
     * of the message
     * \param str Printf-like format string
     * \param va_list holding additional parameters
     */
    virtual void logOutput(
        ComponentInfo const& componentInfo, LevelInfo const& levelInfo, char const* str, va_list ap)
        = 0;

protected:
    ILoggerOutput() = default;
};

} // namespace logger
} // namespace util

#endif // GUARD_CA5724D3_1F37_4BEA_AD36_F9E16C96D4FF
