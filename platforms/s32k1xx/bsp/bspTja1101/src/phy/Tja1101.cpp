// Copyright 2025 Accenture.

#include "phy/Tja1101.h"

#include "ethernet/EthernetLogger.h"

namespace enetphy
{
using namespace ::util::logger;

uint8_t const Tja1101::cRegisterAccess[] = {
    REGISTER_0,
    REGISTER_1,
    REGISTER_2,
    REGISTER_3,
    REGISTER_15,
    REGISTER_17,
    REGISTER_18,
    REGISTER_19,
    REGISTER_20,
    REGISTER_21,
    REGISTER_22,
    REGISTER_23,
    REGISTER_24,
    REGISTER_25,
    REGISTER_26,
    REGISTER_27,
    REGISTER_28,
};

uint8_t const Tja1101::NUMBER_OF_SMI_REGISTERS
    = sizeof(Tja1101::cRegisterAccess) / sizeof(Tja1101::cRegisterAccess[0U]);

Tja1101::Tja1101(MdioTja1101& mdio, uint8_t const phyAddressOfAttachedPort = 0U)
: _mdio(mdio)
, _errorStatus(0)
, _phyAddress(phyAddressOfAttachedPort)
, _state(IDLE)
, _linkStatus(DOWN)
{}

void Tja1101::cableTest()
{
    uint16_t value = 0U;
    writeMiimRegister(REGISTER_17, 0x1824U); // forces TDR-based cable test

    _mdio.miimRead(_phyAddress, REGISTER_23, value);
    if (CABLE_TEST == (0x07U & value))
    {
        _state = INITIALIZING;
        _mdio.miimRead(_phyAddress, REGISTER_25, value);
        if (((value & SHORT_DETECT) != 0U) && ((value & OPEN_DETECT) != 0U))
        {
            _errorStatus &= ~ERROR_SHORT;
            _errorStatus &= ~ERROR_OPEN;
        }
        else
        {
            if ((value & SHORT_DETECT) != 0U)
            {
                _errorStatus |= ERROR_SHORT;
            }
            else
            {
                _errorStatus &= ~ERROR_SHORT;
            }
            if ((value & OPEN_DETECT) != 0U)
            {
                _errorStatus |= ERROR_OPEN;
            }
            else
            {
                _errorStatus &= ~ERROR_OPEN;
            }
        }
    }
}

bool Tja1101::up()
{
    // We currently rely completely on the strapping pin configuration.
    // TODO: this is board specific and should go into a platform file.
    return true;
}

bool Tja1101::isLinkUp()
{
    uint16_t tmpValue = 0U;
    readMiimRegister(REGISTER_1, tmpValue);
    if ((tmpValue & 0x4U) != 0U)
    {
        _errorStatus = 0U;
        if (DOWN == _linkStatus)
        {
            _linkStatus = UP;
            Logger::info(ETHERNET, "link is UP by change");
        }
        return true;
    }
    tmpValue = 0U;
    readMiimRegister(REGISTER_21, tmpValue);
    if ((tmpValue & 0x200U) != 0U)
    {
        _errorStatus = 0U;
        if (DOWN == _linkStatus)
        {
            _linkStatus = UP;
            Logger::info(ETHERNET, "link is UP by set");
        }
        return true;
    }
    if (UP == _linkStatus)
    {
        _linkStatus = DOWN;
        Logger::info(ETHERNET, "link is DOWN");
    }
    return false;
}

void Tja1101::down()
{
    // Set PowerMode to SleepRequestMode
    writeMiimRegister(REGISTER_17, 0x5800U);
}

void Tja1101::stop()
{
    down();
    _state = OFFLINE;
    Logger::debug(ETHERNET, "Tja1101 stopped");
}

void Tja1101::start()
{
    _state = IDLE;
    if (!up())
    {
        Logger::error(ETHERNET, "Tja1101::up() failed");
        return;
    }
    (void)isLinkUp();
    Logger::debug(ETHERNET, "Tja1101 started");
}

bool Tja1101::writeMiimRegister(uint8_t const regAddress, uint16_t const value)
{
    if (OFFLINE == _state)
    {
        return false;
    }
    return (::bsp::BSP_OK == _mdio.miimWrite(_phyAddress, regAddress, value));
}

bool Tja1101::readMiimRegister(uint8_t const regAddress, uint16_t& value)
{
    return (::bsp::BSP_OK == _mdio.miimRead(_phyAddress, regAddress, value));
}

} // namespace enetphy
