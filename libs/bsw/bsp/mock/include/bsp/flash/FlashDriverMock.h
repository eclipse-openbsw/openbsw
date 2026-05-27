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

#include "bsp/flash/IFlashDriver.h"

#include <gmock/gmock.h>

namespace flash
{
struct FlashDriverMock : IFlashDriver
{
    MOCK_METHOD(FlashOperationStatus, write, (uint32_t, uint8_t const*, uint32_t));
    MOCK_METHOD(FlashOperationStatus, erase, (uint32_t, uint32_t));
    MOCK_METHOD(FlashOperationStatus, flush, ());
    MOCK_METHOD(FlashOperationStatus, getBlockSize, (uint32_t, uint32_t&));
};

} /* namespace flash */
