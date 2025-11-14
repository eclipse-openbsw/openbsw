// Copyright 2024 Accenture.
// Copyright 2025 BMW AG

#include "bsp/Uart.h"

#include <io/Io.h>
#include <mcu/mcu.h>

namespace bsp
{

struct Uart::UartConfig
{
    LPUART_Type& uart;
    bios::Io::PinId txPin;
    bios::Io::PinId rxPin;
    uint32_t baudRate;
};

} // namespace bsp
