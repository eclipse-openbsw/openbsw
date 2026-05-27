/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
