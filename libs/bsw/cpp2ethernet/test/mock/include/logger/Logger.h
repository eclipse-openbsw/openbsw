/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
