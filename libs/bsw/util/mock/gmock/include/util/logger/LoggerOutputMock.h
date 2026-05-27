/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "util/logger/ComponentInfo.h"
#include "util/logger/ILoggerOutput.h"
#include "util/logger/LevelInfo.h"

#include <gmock/gmock.h>

namespace util
{
namespace logger
{
class LoggerOutputMock : public ILoggerOutput
{
public:
    MOCK_METHOD(
        void,
        logOutput,
        (ComponentInfo const& componentInfo, LevelInfo const& levelInfo, char const* str, va_list),
        (override));
};

inline bool operator==(LevelInfo const& a, LevelInfo const& b)
{
    if (a.isValid() && b.isValid())
    {
        return (a.getPlainInfoString() == b.getPlainInfoString());
    }
    else
    {
        return false;
    }
}

inline bool operator==(ComponentInfo const& a, ComponentInfo const& b)
{
    if (a.isValid() && b.isValid())
    {
        return (a.getPlainInfoString() == b.getPlainInfoString());
    }
    else
    {
        return false;
    }
}

} // namespace logger
} // namespace util
