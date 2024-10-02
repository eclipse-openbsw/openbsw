#include <stdio.h>
#include "platform/estdint.h"
#include "charInputOutput/CharIOSerialCfg.h"
#include "sci/SciDevice.h"
#include "charInputOutput/charIoSerial.h"

extern "C"
{

namespace // file-local variables moved from global to anonymous namespace
{
    char SerialLogger_buffer[CHARIOSERIAL_BUFFERSIZE];
    int SerialLogger_bufferInd = 0;
    // use synchronous by default so that less memory is needed
    int SerialLogger_consoleAsynchron = 0;
}

/**
 * Make logging asynchronous
 */
void SerialLogger_setAsynchron()
{
    SerialLogger_consoleAsynchron = 1;
}

/**
 * Make logging synchronous
 */
void SerialLogger_setSynchron()
{
    SerialLogger_consoleAsynchron = 0;
}

/**
 * For checking if logger is initialized or not
 * \return
 * - 1 if already initialized
 * - 0 if not yet initialized
 */
uint8_t SerialLogger_getInitState()
{
    return (sciGetInitState());
}

// below functions documented in the header file
void SerialLogger_init()
{
    // setup UART on ESCIA
    sciInit();
}

int SerialLogger_putc(const int c)
{
    int i = 0;
    if (SerialLogger_getInitState() == 0U)
    {
        SerialLogger_init();
    }
    while (sciGetTxNotReady() != 0U)
    {
        i++;
        if (i > SCI_LOGGERTIMEOUT)
        {
            break;
        }
    }

    sciPuth(c);

    return 1;
}

int SerialLogger_getc()
{
    if (sciGetRxReady() != 0U)
    {
        return(static_cast<int>(sciGeth()));
    }
    else
    {
        return 0;
    }
}

void SerialLogger_Idle()
{
    for (int i = 0; i < SerialLogger_bufferInd; i++)
    {
        (void)SerialLogger_putc(static_cast<int>(SerialLogger_buffer[i]));
    }
    SerialLogger_bufferInd = 0;
}

int SerialLogger__outchar(const int c, const int last)
{
    (void)last;
    if (SerialLogger_consoleAsynchron == 0)
    {
        // synchronous output
        (void)SerialLogger_putc(c);
        return 0;
    }

    // asynchronous output
    SerialLogger_buffer[SerialLogger_bufferInd] = static_cast<char>(c);
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

    while (sciGetRxReady() != 1U)
    {
        ; // wait for receive data to be available
    }

    return static_cast<int>(sciGeth());
}

} // extern "C"
