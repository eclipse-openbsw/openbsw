// Copyright 2024 Accenture.
// Copyright 2025 BMW AG

#include "bsp/UartPrivate.h"
#include "bsp/uart/Uart.h"

#include <bsp/clock/boardClock.h>
#include <util/estd/assert.h>

using bsp::Uart;
using bsp::UartSpecific;

static uint32_t const WRITE_TIMEOUT = 1000U;

size_t Uart::read(::etl::span<uint8_t> data)
{
    size_t bytes_read = 0;

    if (data.size() == 0U)
    {
        return 0;
    }

    while (isRxReady() && (bytes_read < data.size()))
    {
        data[bytes_read] = _uartDevice.uart.DATA & 0xFFU;
        bytes_read++;
    }

    return bytes_read;
}

size_t Uart::write(::etl::span<uint8_t const> const& data)
{
    size_t counter = 0;

    if (data.size() == 0U)
    {
        return 0;
    }

    while (counter < data.size())
    {
        if (!writeByte(data[counter]))
        {
            break;
        }
        counter++;
    }

    return counter;
}

UartSpecific::UartSpecific(Id id) : _uartDevice(config_uart[static_cast<uint8_t>(id)]) {}

void UartSpecific::init()
{
    (void)bios::Io::setDefaultConfiguration(_uartDevice.txPin);
    (void)bios::Io::setDefaultConfiguration(_uartDevice.rxPin);
    _uartDevice.uart.GLOBAL = 0;
    _uartDevice.uart.CTRL   = 0;

    _uartDevice.uart.PINCFG = 0;
    _uartDevice.uart.BAUD   = 0;
    _uartDevice.uart.BAUD   = _uartDevice.baudRate[0].baud;
    _uartDevice.uart.STAT   = 0xFFFFFFFFU;
    _uartDevice.uart.STAT   = 0;
    _uartDevice.uart.MODIR  = 0;

    _uartDevice.uart.FIFO  = 0;
    _uartDevice.uart.WATER = 0;
    // Last
    _uartDevice.uart.CTRL  = LPUART_CTRL_RE(1U) + LPUART_CTRL_TE(1U);
}

bool UartSpecific::isRxReady() const
{
    if ((_uartDevice.uart.STAT & LPUART_STAT_OR_MASK) != 0)
    {
        _uartDevice.uart.STAT = _uartDevice.uart.STAT | LPUART_STAT_OR_MASK;
    }
    return ((_uartDevice.uart.STAT & LPUART_STAT_RDRF_MASK) != 0);
}

bool UartSpecific::isTxActive() const
{
    return ((_uartDevice.uart.STAT & LPUART_STAT_TDRE_MASK) == 0);
}

bool UartSpecific::writeByte(uint8_t data)
{
    if (!isTxActive())
    {
        _uartDevice.uart.DATA = (static_cast<uint32_t>(data) & 0xFFU);
        uint32_t count        = 0U;
        while (isTxActive())
        {
            if (++count > WRITE_TIMEOUT)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}
