// Copyright 2025 Accenture.

#include "ethernet/EthernetLogger.h"
#include "tcp/TcpLogger.h"
#include "util/logger/TestConsoleLogger.h"

using namespace ::util::logger;

etl::span<LoggerComponentInfo> getComponents()
{
    static auto components = etl::make_array<LoggerComponentInfo>(
        LoggerComponentInfo(ETHERNET, "ETHERNET", LEVEL_DEBUG),
        LoggerComponentInfo(TCP, "TCP", LEVEL_DEBUG));
    return components;
}

TestConsoleLogger& getLogger()
{
    static TestConsoleLogger logger(getComponents());
    return logger;
}
