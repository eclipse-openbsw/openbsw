// Copyright 2025 BMW AG

#include "Uart.h"

namespace bsp
{

size_t Uart::write(::etl::span<uint8_t const> const& data)
{
    (void)data;
    return 0;
}

size_t Uart::read(::etl::span<uint8_t> data)
{
    (void)data;
    return 0;
}

void Uart::init() {}

} // namespace bsp
