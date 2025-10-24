// Copyright 2025 BMW AG

#pragma once

#include <etl/span.h>

#include <cstddef>
#include <cstdint>

namespace bsp
{

class Uart
{
public:
    size_t write(::etl::span<uint8_t const> const& data);

    size_t read(::etl::span<uint8_t> data);

    void init();
};

} // namespace bsp
