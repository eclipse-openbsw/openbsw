// Copyright 2025 BMW AG

#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp
{

class UartId
{
public:
    enum class Id : size_t
    {
        TERMINAL,
        INVALID,
    };
};

static constexpr size_t NUMBER_OF_UARTS = static_cast<size_t>(UartId::Id::INVALID);

} // namespace bsp
