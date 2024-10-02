// Copyright 2024 Accenture.

#ifndef GUARD_367DD5E3_1B67_4833_B198_6D3E1C2B51D4
#define GUARD_367DD5E3_1B67_4833_B198_6D3E1C2B51D4

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

#endif /* GUARD_367DD5E3_1B67_4833_B198_6D3E1C2B51D4 */
