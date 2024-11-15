// Copyright 2024 Accenture.

#pragma once

#include "bsp/flash/IFlash.h"

#include <etl/span.h>

#include <gmock/gmock.h>

namespace bsp
{
namespace flash
{
struct FlashMock : IFlash
{
    MOCK_CONST_METHOD0(memory, ::etl::span<uint8_t const>());
    MOCK_METHOD2(write, uint32_t(uint32_t, ::etl::span<uint8_t const>));
    MOCK_METHOD0(flush, bool());
    MOCK_METHOD1(erase, bool(FlashBlock const&));
    MOCK_CONST_METHOD2(block, FlashBlock(uint32_t, uint32_t));
};

} // namespace flash
} // namespace bsp
