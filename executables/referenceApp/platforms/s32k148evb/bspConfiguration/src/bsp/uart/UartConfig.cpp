// Copyright 2024 Accenture.
// Copyright 2025 BMW AG

#include <bsp/UartConfig.h>

namespace bsp
{

uint32_t const baudRateConfig[] = {
    (LPUART_BAUD_OSR(15)) + LPUART_BAUD_SBR(26), // = 115200 48MHz FIRC
    (LPUART_BAUD_OSR(9)) + LPUART_BAUD_SBR(8)    // = 2MBit 80MHz PLL
};

Uart::UartConfig const config_uart[] = {
    {
        *LPUART1,
        bios::Io::UART1_TX,
        bios::Io::UART1_RX,
        baudRateConfig[0],
    },
};

static Uart instances[] = {
    Uart(config_uart[static_cast<uint8_t>(Uart::Id::TERMINAL)]),
};

bsp::Uart& Uart::getInstance(Id id)
{
    static_assert(
        static_cast<uint8_t>(Uart::Id::INVALID) >= static_cast<uint8_t>(etl::size(instances)),
        "Not enough Uart instances defined");
    return instances[static_cast<uint8_t>(id)];
}

} // namespace bsp
