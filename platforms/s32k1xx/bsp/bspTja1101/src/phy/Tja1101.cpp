/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "phy/Tja1101.h"

#include "ethernet/EthernetLogger.h"

namespace enetphy
{
using namespace ::util::logger;

uint8_t const Tja1101::cRegisterAccess[] = {
    REG_BCR,
    REG_BSR,
    REG_PHYIDR1,
    REG_PHYIDR2,
    REG_ESR,
    REG_ECR,
    REG_CFG1,
    REG_CFG2,
    REG_SYM_ERR_CNT,
    REG_INTSRC,
    REG_22,
    REG_GENSTAT,
    REG_24,
    REG_EXTS,
    REG_LINK_FAIL_CNT,
    REG_CCFG,
    REG_28,
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
    writeMiimRegister(REG_ECR, 0x1824U); // forces TDR-based cable test

    _mdio.miimRead(_phyAddress, REG_GENSTAT, value);
    if (CABLE_TEST == (0x07U & value))
    {
        _state = INITIALIZING;
        _mdio.miimRead(_phyAddress, REG_EXTS, value);
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
    readMiimRegister(REG_BSR, tmpValue);
    if ((tmpValue & BSR_LINK_STATUS_MASK) == BSR_LINK_STATUS(LinkStatus::UP))
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
    readMiimRegister(REG_INTSRC, tmpValue);
    if ((tmpValue & INTSRC_LINK_STATUS_UP_MASK) == INTSRC_LINK_STATUS_UP(LinkStatus::UP))
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
    writeMiimRegister(REG_ECR, 0x5800U);
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
