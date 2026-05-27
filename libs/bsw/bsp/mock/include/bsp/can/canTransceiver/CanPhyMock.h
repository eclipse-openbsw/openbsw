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

#include "bsp/can/canTransceiver/CanPhy.h"

#include <gmock/gmock.h>

namespace bios
{
class CanPhyMock : public CanPhy
{
public:
    MOCK_METHOD(void, init, (uint32_t), (override));
    MOCK_METHOD(bool, setMode, (Mode, uint32_t), (override));
    MOCK_METHOD(Mode, getMode, (uint32_t), (override));
    MOCK_METHOD(ErrorCode, getPhyErrorStatus, (uint32_t), (override));
};

} // namespace bios
