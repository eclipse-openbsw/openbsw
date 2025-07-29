// Copyright 2025 BMW AG

#pragma once

#include <cstdint>

namespace bsp
{

class UartId
{
public:
    enum class Id
    {
        TERMINAL,
        INVALID,
    };
};

static constexpr uint8_t NUMBER_OF_UARTS = static_cast<uint8_t>(UartId::Id::INVALID);

} // namespace bsp
