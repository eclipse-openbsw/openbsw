// Copyright 2025 Accenture.

#include "mdio/MdioTja1101.h"

#include "bsp/timer/SystemTimer.h"

namespace
{
static uint32_t const MIIM_READ_FRAME   = 0x60000000;
static uint32_t const MIIM_WRITE_FRAME  = 0x50020000;
static uint32_t const CLOCK_DELAY_TICKS = 2;

uint32_t
prepareMIIMFrame(uint8_t const phyAddr, uint8_t const regAddr, uint16_t const data, bool const read)
{
    uint32_t frame = read ? MIIM_READ_FRAME : MIIM_WRITE_FRAME;
    frame |= ((phyAddr & 0x1f) << 23);
    frame |= ((regAddr & 0x1f) << 18);
    frame |= data;
    return frame;
}

} // namespace

namespace enetphy
{

::bsp::BspReturnCode
MdioTja1101::miimRead(uint8_t const phyAddr, uint8_t const regAddr, uint16_t& pData)
{
    uint32_t frame                    = prepareMIIMFrame(phyAddr, regAddr, 0, true);
    ::bsp::BspReturnCode const status = transfer(&frame, true);
    pData                             = frame & 0xffff;
    return status;
}

::bsp::BspReturnCode
MdioTja1101::miimWrite(uint8_t const phyAddr, uint8_t const regAddr, uint16_t const data)
{
    uint32_t frame = prepareMIIMFrame(phyAddr, regAddr, data, false);
    return transfer(&frame, false);
}

uint16_t MdioTja1101::getDataPin() const
{
    bool rv = false;
    ::bios::Output::get(_config.dataPin, rv);
    return (true == rv) ? 1U : 0U;
}

::bsp::BspReturnCode MdioTja1101::transfer(uint32_t* const frame, bool const read) const
{
    // Get Mdio Pin Configuration
    Io::PinConfiguration mdioPinConf;
    Io::getConfiguration(tja1101config.mdioPin, mdioPinConf);

    // MDIO - Make Output Pin
    mdioPinConf.dir = Io::Direction::_OUT;
    Io::setConfiguration(tja1101config.mdioPin, mdioPinConf);

    Output::set(_config.dataPin, 1U);
    // write preamble
    for (uint8_t i = 0; i < 32; ++i)
    {
        Output::set(_config.clockPin, 0U);
        Output::set(_config.clockPin, 1U);
    }
    // transfer first part of frame (header)
    uint16_t data = *frame >> 16;

    for (uint8_t i = 0; i < 14; ++i)
    {
        Output::set(_config.clockPin, 0U);
        Output::set(_config.dataPin, ((data & 0x8000) > 0));
        sysDelayUs(CLOCK_DELAY_TICKS);
        Output::set(_config.clockPin, 1U);
        data <<= 1;
    }
    Output::set(_config.clockPin, 0U);
    Output::set(_config.dataPin, 1U);
    sysDelayUs(CLOCK_DELAY_TICKS);
    Output::set(_config.clockPin, 1U);
    Output::set(_config.clockPin, 0U);

    if (read)
    {
        // MDIO - Make Input Pin
        mdioPinConf.dir = Io::Direction::_IN;
        Io::setConfiguration(tja1101config.mdioPin, mdioPinConf);

        // transfer second part of frame (read data)
        Output::set(_config.clockPin, 1U);
        data = 0;
        for (int8_t i = 15; i >= 0; --i)
        {
            Output::set(_config.clockPin, 0U);
            sysDelayUs(CLOCK_DELAY_TICKS);
            data |= (getDataPin() << i);
            Output::set(_config.clockPin, 1U);
        }

        // MDIO - Make Output Pin
        mdioPinConf.dir = Io::Direction::_OUT;
        Io::setConfiguration(tja1101config.mdioPin, mdioPinConf);

        *frame = data & 0xffff;
    }
    else
    {
        Output::set(_config.dataPin, 0U);
        sysDelayUs(CLOCK_DELAY_TICKS);
        Output::set(_config.clockPin, 1U);
        // transfer second part of frame (write data)
        data = *frame & 0xffff;
        for (uint8_t i = 0; i < 16; i++)
        {
            Output::set(_config.clockPin, 0U);
            Output::set(_config.dataPin, ((data & 0x8000) > 0));
            data <<= 1;
            sysDelayUs(CLOCK_DELAY_TICKS);
            Output::set(_config.clockPin, 1U);
        }
    }
    sysDelayUs(CLOCK_DELAY_TICKS);
    Output::set(_config.clockPin, 0U);
    Output::set(_config.dataPin, 1U);

    // MDIO - Make Input Pin
    mdioPinConf.dir = Io::Direction::_IN;
    Io::setConfiguration(tja1101config.mdioPin, mdioPinConf);

    return ::bsp::BSP_OK;
}

} // namespace enetphy
