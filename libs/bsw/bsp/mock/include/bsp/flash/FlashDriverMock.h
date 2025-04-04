// Copyright 2024 Accenture.

#pragma once

#include "bsp/flash/IFlashDriver.h"

#include <gmock/gmock.h>

namespace flash
{
struct FlashDriverMock : IFlashDriver
{
    MOCK_METHOD3(write, FlashOperationStatus(uint32_t, uint8_t const*, uint32_t));
    MOCK_METHOD2(erase, FlashOperationStatus(uint32_t, uint32_t));
    MOCK_METHOD0(flush, FlashOperationStatus());
    MOCK_METHOD2(getBlockSize, FlashOperationStatus(uint32_t, uint32_t&));
};

} /* namespace flash */
