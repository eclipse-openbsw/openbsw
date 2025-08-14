// Copyright 2025 BMW AG

#pragma once

#include <cstdint>

namespace bsp
{

class UartConfig
{
public:
    enum class Id
    {
        TERMINAL,
        DUMMY_UART,
        NUMBER_OF_UARTS,
        INVALID = NUMBER_OF_UARTS,
    };
};

static constexpr uint8_t NUMBER_OF_UARTS = static_cast<uint8_t>(UartConfig::Id::NUMBER_OF_UARTS);

} // namespace bsp
