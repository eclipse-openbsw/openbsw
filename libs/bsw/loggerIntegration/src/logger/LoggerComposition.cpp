// Copyright 2024 Accenture.

#include "logger/LoggerComposition.h"

#include <logger/IComponentConfig.h>

namespace logger
{
LoggerComposition::LoggerComposition(
    ::util::logger::IComponentMapping& componentMapping, char const* const name)
: _loggerTime("%u")
, _loggerFormatter(_loggerTime, name)
, _consoleLoggerOutput(_loggerFormatter)
, _bufferedLoggerOutput(componentMapping, _loggerTime)
, _entryRef(0)
{}

void LoggerComposition::start(ConfigStart const& configStart)
{
    configStart(_bufferedLoggerOutput);
}

void LoggerComposition::run()
{
    (void)_bufferedLoggerOutput.outputEntry(_consoleLoggerOutput, _entryRef);
}

void LoggerComposition::stop(ConfigStop const& configStop)
{
    while (_bufferedLoggerOutput.outputEntry(_consoleLoggerOutput, _entryRef)) {}
    configStop();
}

} // namespace logger
