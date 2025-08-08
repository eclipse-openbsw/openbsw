// Copyright 2025 Accenture.

#pragma once

namespace logger
{
class ILoggerOutput;

enum Component
{
    _ETHERNET,
    _TCP,
    NUMBER_OF_LOGGER_COMPONENTS
};

enum Level
{
    _WARN,
    _ERROR
};

struct Logger
{
    static void log(Component component, Level level, char const* str, ...) {}

    static void init(ILoggerOutput& loggerOutput) {}

    static void setLevelForAll(Level level) {}
};

} // namespace logger
