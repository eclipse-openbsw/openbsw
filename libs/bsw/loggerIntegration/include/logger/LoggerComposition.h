// Copyright 2024 Accenture.

#pragma once

#include "logger/Config.h"
#include "logger/ConsoleEntryFormatter.h"
#include "logger/DefaultLoggerTime.h"

#include <etl/delegate.h>
#include <logger/ConsoleEntryOutput.h>
#include <util/logger/IComponentMapping.h>
#include <util/logger/ILoggerOutput.h>

namespace logger
{
class LoggerComposition
{
public:
    using ConfigStart = ::etl::delegate<void(::util::logger::ILoggerOutput&)>;
    using ConfigStop  = ::etl::delegate<void()>;

    explicit LoggerComposition(
        ::util::logger::IComponentMapping& componentMapping, char const* name);

    void start(ConfigStart const& configStart);
    void run();
    void stop(ConfigStop const& configStop);

private:
    DefaultLoggerTime<> _loggerTime;

    ConsoleEntryFormatter<
        BufferedLoggerOutputType::EntryIndexType,
        BufferedLoggerOutputType::TimestampType>
        _loggerFormatter;

    ConsoleEntryOutput<
        BufferedLoggerOutputType::EntryIndexType,
        BufferedLoggerOutputType::TimestampType>
        _consoleLoggerOutput;

    BufferedLoggerOutputType _bufferedLoggerOutput;
    BufferedLoggerOutputType::EntryRefType _entryRef;
};

} // namespace logger
