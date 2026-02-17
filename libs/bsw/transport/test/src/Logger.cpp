// Copyright 2024 Accenture.

#include "transport/TransportLogger.h"
#include <util/logger/TestConsoleLogger.h>

#include <etl/array.h>
#include <etl/span.h>

using namespace ::util::logger;

etl::span<LoggerComponentInfo> getComponents()
{
    static auto components = etl::make_array<LoggerComponentInfo>(
        LoggerComponentInfo(TRANSPORT, "TRANSPORT", LEVEL_DEBUG));
    return components;
}

TestConsoleLogger& getLogger()
{
    static TestConsoleLogger logger(getComponents());
    return logger;
}
