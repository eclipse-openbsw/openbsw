// Copyright 2024 Accenture.

#pragma once

#include "bsp/flash/IFlash.h"

#include <gmock/gmock.h>

namespace bsp
{
namespace flash
{
struct FlashMock : IFlash
{
    MOCK_METHOD(::estd::slice<uint8_t const>, memory, (), (const, override));
    MOCK_METHOD(uint32_t, write, (uint32_t, ::estd::slice<uint8_t const>), (override));
    MOCK_METHOD(bool, flush, (), (override));
    MOCK_METHOD(bool, erase, (FlashBlock const&), (override));
    MOCK_METHOD(FlashBlock, block, (uint32_t, uint32_t), (const, override));
};

} // namespace flash
} // namespace bsp
