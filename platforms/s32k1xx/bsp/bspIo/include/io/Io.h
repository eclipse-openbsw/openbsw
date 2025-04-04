// Copyright 2024 Accenture.

#pragma once

#include "bsp/Bsp.h"
#include "io/ioPorts.h"
#include "mcu/mcu.h"
#include "platform/estdint.h"

namespace bios
{

class Io
{
public:
    Io();

    enum Level
    {
        LOW,
        HIGH
    };

    enum Direction
    {
        _DISABLED = 0,
        _IN       = (1 << 0),
        _OUT      = (1 << 1),
    };

    using tDirection = Direction;

    enum
    {
        clearISR = (1 << 24)
    };

    enum
    {
        unlock = 0,
        lock   = (1 << 15)
    };

    enum
    {
        IRQC_0  = 0,
        IRQC_1  = (1 << 16),
        IRQC_2  = (2 << 16),
        IRQC_3  = (3 << 16),
        IRQC_8  = (8 << 16),
        IRQC_9  = (9 << 16),
        IRQC_10 = (10 << 16),
        IRQC_11 = (11 << 16)
    };

    enum
    {
        PINDISABLE = 0,
        GPIO       = (1 << 8),
        ALT2       = (2 << 8),
        ALT3       = (3 << 8),
        ALT4       = (4 << 8),
        ALT5       = (5 << 8),
        ALT6       = (6 << 8),
        ALT7       = (7 << 8)
    };

    enum
    {
        STRENGTH_ON = (1 << 6)
    };

    enum
    {
        FILTER_ON = (1 << 4)
    };

    enum
    {
        PULLUP   = 3,
        PULLDOWN = 2
    };

    enum
    {
        FILTER_ACTIVE    = 0x80,
        FILTER_CLOCK_LPO = (1 << 5),
        FILTER_TICK0     = 0,
        FILTER_TICK1     = 1,
        FILTER_TICK2     = 2,
        FILTER_TICK3     = 3,
        FILTER_TICK4     = 4,
        FILTER_TICK5     = 5,
        FILTER_TICK6     = 6,
        FILTER_TICK7     = 7,
        FILTER_TICK8     = 8,
        FILTER_TICK9     = 9,
        FILTER_TICK10    = 10,
        FILTER_TICK11    = 11,
        FILTER_TICK12    = 12,
        FILTER_TICK13    = 13,
        FILTER_TICK14    = 14,
        FILTER_TICK15    = 15,
        FILTER_TICK16    = 16,
        FILTER_TICK17    = 17,
        FILTER_TICK18    = 18,
        FILTER_TICK19    = 19,
        FILTER_TICK20    = 20,
        FILTER_TICK21    = 21,
        FILTER_TICK22    = 22,
        FILTER_TICK23    = 23,
        FILTER_TICK24    = 24,
        FILTER_TICK25    = 25,
        FILTER_TICK26    = 26,
        FILTER_TICK27    = 27,
        FILTER_TICK28    = 28,
        FILTER_TICK29    = 29,
        FILTER_TICK30    = 30,
        FILTER_TICK31    = 31,
    };

    static bool const LOW_ACTIVE  = true;
    static bool const HIGH_ACTIVE = false;

#undef BSP_IO_PIN_CONFIGURATION
#include "bsp/io/io/ioConfiguration.h"

    /**
     * \brief port configuration
     */
    struct PinConfiguration
    {
        uint8_t port;
        uint8_t pinNumber;
        uint8_t dir;
        uint8_t pinCfgExt; // reserve
        uint32_t pinCfg;
    };

    static bsp::BspReturnCode setDefaultConfiguration(uint16_t io);
    static bsp::BspReturnCode setConfiguration(uint16_t io, PinConfiguration const& cfg);
    static bsp::BspReturnCode getConfiguration(uint16_t io, PinConfiguration& cfg);
    static bool getPin(uint16_t io);
    static bsp::BspReturnCode setPin(uint16_t io, bool vol);

    static uint32_t getPinNumber(uint16_t io);
    static uint16_t const NUMBER_OF_IOS = NUMBER_OF_INPUTS_AND_OUTPUTS;
    static bsp::BspReturnCode resetConfig(uint16_t io);

    static GPIO_Type* const gpioPtrs[_PORTS_MAX_];
    static PORT_Type* const gpioPrfCfgPtrs[_PORTS_MAX_];

    static bool getPinIrq(uint16_t io);

private:
    static uint16_t const MAX_GPIO_NUMBER = NUM_PORTS;

    static PinConfiguration const fPinConfiguration[NUMBER_OF_IOS];
};

} /* namespace bios */
