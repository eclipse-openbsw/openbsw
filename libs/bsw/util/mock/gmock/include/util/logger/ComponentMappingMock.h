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
