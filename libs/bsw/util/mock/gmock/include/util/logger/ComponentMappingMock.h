// Copyright 2024 Accenture.

#ifndef GUARD_2C555BC5_D1D6_4075_8E16_FC200684F015
#define GUARD_2C555BC5_D1D6_4075_8E16_FC200684F015

#include "util/logger/IComponentMapping.h"

#include <gmock/gmock.h>

namespace util
{
namespace logger
{
class ComponentMappingMock : public IComponentMapping
{
public:
    MOCK_CONST_METHOD2(isEnabled, bool(uint8_t componentIndex, Level level));
    MOCK_CONST_METHOD1(getLevel, Level(uint8_t componentIndex));
    MOCK_CONST_METHOD1(getLevelInfo, LevelInfo(Level level));
    MOCK_CONST_METHOD1(getComponentInfo, ComponentInfo(uint8_t componentIndex));
};

} // namespace logger
} // namespace util

#endif /* GUARD_2C555BC5_D1D6_4075_8E16_FC200684F015 */
