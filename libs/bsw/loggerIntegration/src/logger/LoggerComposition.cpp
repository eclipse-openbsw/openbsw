/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
