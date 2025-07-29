// Copyright 2024 Accenture.
// Copyright 2025 BMW AG

#include <bsp/UartPrivate.h>
#include <util/estd/assert.h>

namespace bsp
{

UartBaudRate const baudRateConfig[] = {
    {(LPUART_BAUD_OSR(15)) + LPUART_BAUD_SBR(26)}, // = 115200 48MHz FIRC
    {(LPUART_BAUD_OSR(9)) + LPUART_BAUD_SBR(8)}    // = 2MBit 80MHz PLL
};

Uart::UartDevice const config_uart[] = {
    {
        *LPUART1,
        bios::Io::UART1_TX,
        bios::Io::UART1_RX,
        static_cast<uint8_t>(sizeof(baudRateConfig) / sizeof(UartBaudRate)),
        baudRateConfig,
    },
};

static Uart instances[] = {
    Uart(config_uart[static_cast<uint8_t>(Uart::Id::TERMINAL)]),
};

bsp::Uart& Uart::getInstance(Id id)
{
    // estd_assert(static_cast<uint8_t>(Uart::Id::INVALID) <
    // static_cast<uint8_t>(etl::size(instances)));
    return instances[static_cast<uint8_t>(id)];
}

} // namespace bsp
