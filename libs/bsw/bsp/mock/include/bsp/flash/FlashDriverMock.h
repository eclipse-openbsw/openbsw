// Copyright 2024 Accenture.

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
