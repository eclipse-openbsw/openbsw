// Copyright 2025 BMW AG

#pragma once

#include "bsp/Uart.h"
#include <cstddef>
#include <cstdint>

namespace bsp
{

enum class Uart::Id : size_t
{
    TERMINAL,
    INVALID,
};

static constexpr uint8_t NUMBER_OF_UARTS = static_cast<uint8_t>(Uart::Id::INVALID);

} // namespace bsp
