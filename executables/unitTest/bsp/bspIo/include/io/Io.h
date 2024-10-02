// Copyright 2024 Accenture.

#ifndef GUARD_12EDB516_8790_42D9_83DA_0AB3885A6FFA
#define GUARD_12EDB516_8790_42D9_83DA_0AB3885A6FFA

#include "bsp/Bsp.h"
#include "platform/estdint.h"

namespace bios
{
class Io
{
    Io();

public:
    enum Level
    {
        LOW,
        HIGH
    };

    enum
    {
        unlock = false,
        lock   = true
    };

    static bool const LOW_ACTIVE  = true;
    static bool const HIGH_ACTIVE = false;

#undef BSP_IO_PIN_CONFIGURATION
#include "bsp/io/io/ioConfiguration.h"

    using PinConfiguration = struct
    {
        uint32_t id;
    };

    static bsp::BspReturnCode setDefaultConfiguration(uint16_t io);
    static bool getPin(uint16_t io);
    static bsp::BspReturnCode setPin(uint16_t io, bool vol);

    static uint32_t getPinNumber(PinId pinId);
    static bsp::BspReturnCode resetConfig(uint16_t io);

    static bsp::BspReturnCode safe(uint16_t io, bool lock);

    static uint16_t const NUMBER_OF_IOS = NUMBER_OF_INPUTS_AND_OUTPUTS;

private:
    static uint16_t const MAX_GPIO_NUMBER = 512;

    static PinConfiguration const fPinConfiguration[NUMBER_OF_IOS];
};

} /* namespace bios */

#endif /* GUARD_12EDB516_8790_42D9_83DA_0AB3885A6FFA */
