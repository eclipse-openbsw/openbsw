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

#include <cstdint>

#include <gmock/gmock.h>

namespace middleware::time::test
{

class SystemTimerProviderMock
{
public:
    MOCK_METHOD(uint32_t, getCurrentTimeInMs, ());
    MOCK_METHOD(uint32_t, getCurrentTimeInUs, ());
};

void setSystemTimerProviderMock(SystemTimerProviderMock* const ptr);
void unsetSystemTimerProviderMock();

} // namespace middleware::time::test
