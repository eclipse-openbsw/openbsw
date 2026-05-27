/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/logger/Logger.h"

#ifndef LOGGER_NO_LEGACY_API

#include "util/logger/ILoggerOutput.h"

namespace util
{
namespace logger
{
IComponentMapping* Logger::_componentMapping;
ILoggerOutput* Logger::_output;

void Logger::init(IComponentMapping& componentMapping, ILoggerOutput& output)
{
    _componentMapping = &componentMapping;
    _output           = &output;
}

void Logger::shutdown()
{
    if (_componentMapping != nullptr)
    {
        _componentMapping = nullptr;
    }
    _output = nullptr;
}

void Logger::doLog(
    uint8_t const componentIndex, Level const level, char const* const str, va_list ap)
{
    ComponentInfo const componentInfo = _componentMapping->getComponentInfo(componentIndex);
    LevelInfo const levelInfo         = _componentMapping->getLevelInfo(level);
    _output->logOutput(componentInfo, levelInfo, str, ap);
}

Level Logger::getLevel(uint8_t const componentIndex)
{
    return (_componentMapping != nullptr) ? _componentMapping->getLevel(componentIndex)
                                          : LEVEL_NONE;
}

} /* namespace logger */
} /* namespace util */

#endif /* LOGGER_NO_LEGACY_API */
