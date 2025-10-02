// Copyright 2024 Accenture.
// Copyright 2025 BMW AG

#include <bsp/UartPrivate.h>
#include <bsp/uart/Uart.h>
#include <etl/tuple.h>
#include <util/estd/assert.h>

namespace bsp
{

UartBaudRate const baudRateConfig[] = {
    {(LPUART_BAUD_OSR(15)) + LPUART_BAUD_SBR(26)}, // = 115200 48MHz FIRC
    {(LPUART_BAUD_OSR(9)) + LPUART_BAUD_SBR(8)}    // = 2MBit 80MHz PLL
};

UartSpecific::UartDevice const UartSpecific::config_uart[] = {
    {
        *LPUART1,
        bios::Io::UART1_TX,
        bios::Io::UART1_RX,
        static_cast<uint8_t>(sizeof(baudRateConfig) / sizeof(UartBaudRate)),
        baudRateConfig,
    },
};

etl::tuple<bsp::Uart> uart_instances{Uart(Uart::Id::TERMINAL)};

bsp::Uart& UartSpecific::getInstance(Id id)
{
    switch (id)
    {
        case Id::TERMINAL: return etl::get<0>(uart_instances); break;
        case Id::INVALID:  estd_assert(false); break;
        default:           estd_assert(false); break;
    }
}

} // namespace bsp
