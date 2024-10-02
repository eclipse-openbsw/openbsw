// Copyright 2024 Accenture.

#include "charInputOutput/charIoSerial.h"
#include "platform/estdint.h"

extern "C" void putByteToStdout(uint8_t const byte)
{
    (void)SerialLogger_putc(static_cast<int32_t>(byte));
}

extern "C" int32_t getByteFromStdin()
{
    uint8_t const byte = static_cast<uint8_t>(SerialLogger_getc());
    return (byte != 0U) ? byte : -1;
}
