/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "uds/UdsLogger.h"
#include "util/logger/TestConsoleLogger.h"

#include <etl/array.h>
#include <etl/span.h>

DEFINE_LOGGER_COMPONENT(GLOBAL)

using namespace ::util::logger;

etl::span<LoggerComponentInfo> getComponents()
{
    static auto components = etl::make_array<LoggerComponentInfo>(
        LoggerComponentInfo(GLOBAL, "GLOBAL", LEVEL_DEBUG),
        LoggerComponentInfo(UDS, "UDS", LEVEL_DEBUG));
    return components;
}

TestConsoleLogger& getLogger()
{
    static TestConsoleLogger logger(getComponents());
    return logger;
}
