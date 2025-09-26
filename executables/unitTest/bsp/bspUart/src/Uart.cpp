// Copyright 2025 BMW AG

#include "Uart.h"

namespace bsp
{

size_t Uart::write(uint8_t const* data, size_t length)
{
    (void)data;
    (void)length;
    return 0;
}

size_t Uart::read(uint8_t* data, size_t length)
{
    (void)data;
    (void)length;
    return 0;
}

void Uart::init() {}

} // namespace bsp
