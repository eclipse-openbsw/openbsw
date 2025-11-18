// Copyright 2024 Accenture.

#include <bsp/uart/UartConfig.h>

#include <platform/estdint.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

using bsp::Uart;

void terminal_cleanup(void) { Uart::getInstance(Uart::Id::TERMINAL).deinit(); }

extern "C" void putByteToStdout(uint8_t byte)
{
    static Uart& uart = Uart::getInstance(Uart::Id::TERMINAL);
    uart.write(etl::span<uint8_t const>(&byte, 1U));
}

extern "C" int32_t getByteFromStdin()
{
    static Uart& uart = Uart::getInstance(Uart::Id::TERMINAL);
    uint8_t data_byte = 0;
    etl::span<uint8_t> data(&data_byte, 1U);
    uart.read(data);
    if (data.size() == 0)
    {
        return -1;
    }
    return static_cast<int32_t>(data[0]);
}
