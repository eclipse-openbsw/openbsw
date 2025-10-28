#include "charInputOutput/charIoSerial.h"

#include "bsp/Uart.h"
#include "charInputOutput/CharIOSerialCfg.h"
#include "platform/estdint.h"
#include <etl/span.h>

#include <stdio.h>

using bsp::Uart;

extern "C"
{
namespace // file-local variables moved from global to anonymous namespace
{
char SerialLogger_buffer[CHARIOSERIAL_BUFFERSIZE];
etl::span<char> LoggerBuffer(SerialLogger_buffer, CHARIOSERIAL_BUFFERSIZE);
int SerialLogger_bufferInd           = 0;
// use synchronous by default so that less memory is needed
int SerialLogger_consoleAsynchronous = 0;
Uart& uart                           = Uart::getInstance(Uart::Id::TERMINAL);
} // namespace

/**
 * Make logging asynchronous
 */
void SerialLogger_setAsynchronous() { SerialLogger_consoleAsynchronous = 1; }

/**
 * Make logging synchronous
 */
void SerialLogger_setSynchronous() { SerialLogger_consoleAsynchronous = 0; }

/**
 * For checking if logger is initialized or not
 * \return
 * - 1 if already initialized
 * - 0 if not yet initialized
 */
uint8_t SerialLogger_getInitState() { return (static_cast<uint8_t>(uart.isInitialized())); }

// below functions documented in the header file
void SerialLogger_init()
{
    // setup UART on ESCIA
    uart.init();
}

int SerialLogger_putc(int const c)
{
    uint8_t const byte = static_cast<uint8_t>(c);
    return uart.write(etl::span<uint8_t const>(&byte, 1U));
}

int SerialLogger_getc()
{
    etl::span<uint8_t> data{};
    uart.read(data);
    if (data.size() == 0)
    {
        return -1;
    }
    return data[0];
}

void SerialLogger_Idle()
{
    etl::span<uint8_t const> const data(
        reinterpret_cast<uint8_t const*>(LoggerBuffer.data()), SerialLogger_bufferInd);
    uart.write(data);
    SerialLogger_bufferInd = 0;
}

int SerialLogger__outchar(int const c, int const last)
{
    (void)last;
    if (SerialLogger_consoleAsynchronous == 0)
    {
        // synchronous output
        (void)SerialLogger_putc(c);
        return 0;
    }

    // asynchronous output
    LoggerBuffer[SerialLogger_bufferInd] = static_cast<char>(c);
    SerialLogger_bufferInd++;
    if (SerialLogger_bufferInd >= CHARIOSERIAL_BUFFERSIZE)
    {
        SerialLogger_bufferInd = 0;
    }
    return 0;
}

int SerialLogger__inchar()
{
    if (SerialLogger_getInitState() == 0U)
    {
        SerialLogger_init();
    }

    return SerialLogger_getc();
}

} // extern "C"
