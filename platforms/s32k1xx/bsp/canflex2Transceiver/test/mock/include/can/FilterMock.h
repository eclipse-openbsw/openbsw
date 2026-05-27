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

#include <can/filter/IFilter.h>

#include <gmock/gmock.h>

namespace can
{
class FilterMock : public IFilter
{
public:
    MOCK_METHOD(void, add, (uint32_t), (override));
    MOCK_METHOD(void, add, (uint32_t, uint32_t), (override));
    MOCK_METHOD(bool, match, (uint32_t), (const, override));
    MOCK_METHOD(void, clear, (), (override));
    MOCK_METHOD(void, open, (), (override));
    MOCK_METHOD(void, acceptMerger, (IMerger&), (override));
};

} // namespace can
