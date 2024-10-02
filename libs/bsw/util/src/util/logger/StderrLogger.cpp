// Copyright 2024 Accenture.

#include "util/logger/StderrLogger.h"

#include "util/logger/Logger.h"

#include <cstdarg>
#include <cstdio>

namespace util
{
namespace logger
{
StderrLogger* StderrLogger::_instance = 0;

StderrLogger::~StderrLogger()
{
    if (_instance == this)
    {
        _instance = _prevInstance;
    }
}

void StderrLogger::init()
{
    if (_instance)
    {
        _instance->applyMapping();
        Logger::init(*_instance, *_instance);
    }
}

void StderrLogger::shutdown()
{
    if (_instance)
    {
        Logger::shutdown();
        _instance->clearMapping();
    }
}

bool StderrLogger::isEnabled(uint8_t componentIndex, Level level) const
{
    return componentIndex < _count && level >= _firstComponent[componentIndex].getLevel();
}

Level StderrLogger::getLevel(uint8_t componentIndex) const
{
    return componentIndex < _count ? _firstComponent[componentIndex].getLevel() : LEVEL_NONE;
}

LevelInfo StderrLogger::getLevelInfo(Level level) const
{
    return LevelInfo(LevelInfo::getDefaultTable() + level);
}

ComponentInfo StderrLogger::getComponentInfo(uint8_t componentIndex) const
{
    return componentIndex < _count ? _firstComponent[componentIndex].getComponentInfo()
                                   : ComponentInfo();
}

void StderrLogger::logOutput(
    ComponentInfo const& componentInfo, LevelInfo const& levelInfo, char const* str, va_list ap)
{
    fprintf(
        stderr, "%s: %s: ", componentInfo.getName().getString(), levelInfo.getName().getString());
    vfprintf(stderr, str, ap);
    fprintf(stderr, "\n");
}

void StderrLogger::applyMapping()
{
    for (uint8_t idx = 0; idx < _count; ++idx)
    {
        _firstComponent[idx].getComponent() = idx;
    }
}

void StderrLogger::clearMapping()
{
    for (uint8_t idx = 0; idx < _count; ++idx)
    {
        _firstComponent[idx].getComponent() = COMPONENT_NONE;
    }
}

TestLoggingGuard::TestLoggingGuard() { StderrLogger::init(); }

TestLoggingGuard::~TestLoggingGuard() { StderrLogger::shutdown(); }

LoggerComponentInfo::LoggerComponentInfo(uint8_t& component, char const* name, Level level)
: _component(component), _level(level)
{
    _componentInfo._nameInfo._string     = name;
    _componentInfo._nameInfo._attributes = ::util::format::StringAttributes().getAttributes();
}

uint8_t& LoggerComponentInfo::getComponent() { return _component; }

ComponentInfo LoggerComponentInfo::getComponentInfo() const
{
    return ComponentInfo(_component, &_componentInfo);
}

Level LoggerComponentInfo::getLevel() const { return _level; }

} /* namespace logger */
} /* namespace util */
