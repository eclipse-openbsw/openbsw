// Copyright 2024 Accenture.

#pragma once

#include "util/logger/IComponentMapping.h"

#include <gmock/gmock.h>

namespace util
{
namespace logger
{
class ComponentMappingMock : public IComponentMapping
{
public:
    MOCK_METHOD(bool, isEnabled, (uint8_t componentIndex, Level level), (const, override));
    MOCK_METHOD(Level, getLevel, (uint8_t componentIndex), (const, override));
    MOCK_METHOD(LevelInfo, getLevelInfo, (Level level), (const, override));
    MOCK_METHOD(ComponentInfo, getComponentInfo, (uint8_t componentIndex), (const, override));
};

} // namespace logger
} // namespace util
