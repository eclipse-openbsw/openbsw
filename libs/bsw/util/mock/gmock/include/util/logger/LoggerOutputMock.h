// Copyright 2024 Accenture.

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
    MOCK_METHOD4(
        logOutput,
        void(
            ComponentInfo const& componentInfo,
            LevelInfo const& levelInfo,
            char const* str,
            va_list));
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
