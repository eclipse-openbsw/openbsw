// Copyright 2025 Accenture.

#include "ethernet/EthernetLogger.h"
#include "tcp/TcpLogger.h"
#include "util/logger/TestConsoleLogger.h"

using namespace ::util::logger;

LoggerComponentInfo components[] = {
    LoggerComponentInfo(ETHERNET, "ETHERNET", LEVEL_DEBUG),
    LoggerComponentInfo(TCP, "TCP", LEVEL_DEBUG),
};

TestConsoleLogger logger(components);
