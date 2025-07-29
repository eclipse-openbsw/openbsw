// Copyright 2024 Accenture.
// Copyright 2025 BMW AG

#include <bsp/UartPrivate.h>
#include <bsp/uart/Uart.h>
#include <util/estd/assert.h>

namespace bsp
{

UartBaudRate const baudRateConfig[] = {
    {(LPUART_BAUD_OSR(15)) + LPUART_BAUD_SBR(26)}, // = 115200 48MHz FIRC
    {(LPUART_BAUD_OSR(9)) + LPUART_BAUD_SBR(8)}    // = 2MBit 80MHz PLL
};

UartImpl::UartDevice const UartImpl::config_uart[] = {
    {
        *LPUART1,
        bios::Io::UART1_TX,
        bios::Io::UART1_RX,
        static_cast<uint8_t>(sizeof(baudRateConfig) / sizeof(UartBaudRate)),
        baudRateConfig,
    },
    {
        *LPUART1,
        bios::Io::PORT_UNAVAILABLE,
        bios::Io::PORT_UNAVAILABLE,
        0,
        nullptr, // Dummy UART does not have a valid configuration
    },
};

bsp::Uart& Uart::getInstance(Id id)
{
    switch (id)
    {
        case Id::TERMINAL:
        {
            static Uart instance{
                Uart(id),
            };
            return instance;
        }
        break;
        case Id::DUMMY_UART:
        {
            estd_assert(false);
        }
        break;
        default:
        {
            estd_assert(false);
        }
        break;
    }
}

} // namespace bsp
