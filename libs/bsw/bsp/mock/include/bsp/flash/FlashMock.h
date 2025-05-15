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
    MOCK_METHOD(::etl::span<uint8_t const>, memory, (), (const));
    MOCK_METHOD(uint32_t, write, (uint32_t, ::etl::span<uint8_t const>));
    MOCK_METHOD(bool, flush, ());
    MOCK_METHOD(bool, erase, (FlashBlock const&));
    MOCK_METHOD(FlashBlock, block, (uint32_t, uint32_t), (const));
};

} // namespace flash
} // namespace bsp
